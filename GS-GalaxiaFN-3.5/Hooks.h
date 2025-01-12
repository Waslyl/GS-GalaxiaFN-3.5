#pragma once

enum ENetMode
{
	NM_Standalone,
	NM_DedicatedServer,
	NM_ListenServer,
	NM_Client,
	NM_MAX,
};

namespace ToHooks
{ 
	void (*ProcessEvent)(UObject* Object, UFunction* Function, void* Parms);
	ENetMode(*GetNetMode)();
	bool(*KickPlayer)();
	UAbilitySystemComponent(*InternalTryActivateAbility)(FGameplayAbilitySpecHandle, FPredictionKey, UGameplayAbility**, FGameplayEventData*);
}

namespace Hooks
{	
	bool bPreReadyToStartMatch = false;
	bool bReadyToStartMatch = false;
	bool bWorldReady = false;
	bool bLogs = false;

	static inline int64_t GIsServer()
	{
		return __int64(GetModuleHandleW(0)) + 0x4A61415;
	}

	static inline int64_t GIsClient()
	{
		return __int64(GetModuleHandleW(0)) + 0x4A61414;
	}

	static ENetMode GetNetModeHook()
	{
		return ENetMode::NM_DedicatedServer;
	}

	static bool KickPlayerHook()
	{
		return false;
	}

	static bool InternalTryActivateAbility()
	{
		return false;
	}

	void ProcessEventHook(UObject* Object, UFunction* Function, void* Parms)
	{

		if (!Object || !Function) // || = or
		{
			ToHooks::ProcessEvent(Object, Function, Parms);
			return;
		}

		const std::string& FunctionName = Function->GetName();

		if (FunctionName.contains("Tick"))
		{
			if (GetAsyncKeyState(VK_F7) & 0x1)
			{
				HLog("F7 Truc bidule");
				bLogs = bLogs ? false : true;
			}
		}

		if (FunctionName.contains("ReadyToStartMatch"))
		{
			if (!bPreReadyToStartMatch)
			{
				Globals::GetFortEngine(true);
				Globals::GetWorld(true);

				/*(bool*)GIsClient() = false;
				*(bool*)GIsServer() = true;*/

				HLog("letrucmarche c'est l'essentiel");
				/*AFortPlayerController* PlayerController = (AFortPlayerController*)Globals::GetServerPlayerController();
				UFortCheatManager* CheatManager = (UFortCheatManager*)PlayerController->CheatManager;

				if (!CheatManager)
				{
					UFortCheatManager* NewCheatManager = (UFortCheatManager*)(Globals::GetGameplayStatics()->SpawnObject(UFortCheatManager::StaticClass(), PlayerController));
					CheatManager = NewCheatManager;
				}

				if (CheatManager)
					CheatManager->DestroyAll(APlayerController::StaticClass());*/

				bPreReadyToStartMatch = true;
				HLog("kiwi magique marche ici");
			}
			else if (bPreReadyToStartMatch && bWorldReady)
			{
				HLog("Préparation du Match en cours !");
				AFortGameModeAthena* GameMode = Globals::GetGameMode();
				AFortGameStateAthena* GameState = Globals::GetGameState();

				FName MatchState = GameMode->GetMatchState();
				if (GameMode->NumPlayers + GameMode->NumBots > 0 &&
				MatchState.ToString() == "WaitingToStart")
				{
					HLog("MatchState" << MatchState.ToString())
					FName inprogress = Globals::GetStringLibrary()->Conv_StringToName(L"InProgress"); // Transformation d'un string en FName pour ensuite modif la MatchState
					UFortPlaylistAthena* Playlist = UObject::FindObjectSlow<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo"); // Choix de la playlist
					GameState->CurrentPlaylistData = Playlist; // Set la playlist
					GameState->OnRep_CurrentPlaylistData(); // Bien Set la playlist (si il y a)

					GameState->GamePhase = EAthenaGamePhase::Warmup;
					GameState->OnRep_GamePhase(EAthenaGamePhase::None);

					GameMode->MatchState = inprogress;
					GameMode->K2_OnSetMatchState(inprogress);

					GameMode->StartPlay(); // jsp faut le mettre
					GameMode->StartMatch(); // Start la Game
					HLog("Heliato est de sorti !"); // "nique ta mere va te faire foutre" sa réponse à la log UwU
				}
			}

		}

		if (FunctionName.contains("OnWorldReady"))
		{
			if (!bWorldReady && bPreReadyToStartMatch)
			{
				HLog("popo comme disent les ptits enfants dans ma cave");
				Beacon::Listen(); // Listen = function
				HLog("je rigole salope laisse moi 2 secondes que j'ecris sa jvais te les mettres t'es ; salope");

				bWorldReady = true;
			}
		}

		if (FunctionName.contains("ServerAttemptAircraftJump"))
		{
			AFortPlayerControllerAthena* PlayerController = (AFortPlayerControllerAthena*)Object; // bon j'ecoute
			auto Params = (Params::FortPlayerControllerAthena_ServerAttemptAircraftJump*)Parms;

			if (PlayerController->IsInAircraft() && !PlayerController->Pawn)
			{
				AFortGameStateAthena* GameState = Globals::GetGameState();
				AFortAthenaAircraft* Aircraft = GameState->GetAircraft(0);

				APlayerPawn_Athena_C* PlayerPawn = Util::SpawnActor<APlayerPawn_Athena_C>(APlayerPawn_Athena_C::StaticClass(), Aircraft->K2_GetActorLocation()); // Spawn le joueur

				PlayerPawn->Owner = PlayerController;
				PlayerPawn->OnRep_Owner();

				PlayerController->Pawn = PlayerPawn;
				PlayerController->OnRep_Pawn();
				PlayerController->Possess(PlayerPawn);

				PlayerController->SetControlRotation(Params->ClientRotation);
			}
		}

		if (FunctionName.contains("ServerReturnToMainMenu"))
		{
			auto PC = (AFortPlayerController*)Object;
			PC->ClientTravel(L"/Game/Maps/Frontend", ETravelType::TRAVEL_Absolute, false, FGuid());
		}

		if (FunctionName.contains("HandleStartingNewPlayer"))
		{
			AGameModeBase* GameMode = (AGameModeBase*)Object;
			auto Params = (Params::GameModeBase_HandleStartingNewPlayer*)Parms; // <-- si parms marche pas #include truc si tu comprends pas demande a Heliato :nerd:

			AActor* PlayerStart = GameMode->ChoosePlayerStart(Params->NewPlayer);
			if (PlayerStart) 
			{
				// ServerReplicateActors((UReplicationGraph*)NetDriver->ReplicationDriver, DeltaSeconds);


				Util::SpawnPlayer((AFortPlayerControllerAthena*)Params->NewPlayer, PlayerStart->K2_GetActorLocation(), PlayerStart->K2_GetActorRotation(), true, false); // aie
			}
		}

		if (FunctionName.contains("ServerTryActivateAbility"))
		{
			// UAbilitySystemComponent_ServerTryActivateAbility_Params
			auto Params = (Params::AbilitySystemComponent_ServerTryActivateAbility*)Parms;
			auto AbilitySystemComp = (UAbilitySystemComponent*)Object;

			auto AbilityToActivate = Params->AbilityToActivate;
			auto PredictionKey = Params->PredictionKey;

			UGameplayAbility* InstancedAbility = nullptr;
			// InternalTryActivateAbilityLong
			// AbilitySystemComponent, &Spec.Handle
			if (InternalTryActivateAbility())
			{
				AbilitySystemComp->ClientActivateAbilityFailed(AbilityToActivate, PredictionKey.Base);
			}

			return;
		}

		if (FunctionName.contains("ServerAbilityRPCBatch"))
		{
			auto AbilityComp = (UAbilitySystemComponent*)Object;
			auto CurrentParams = (Params::AbilitySystemComponent_ServerAbilityRPCBatch*)Parms;

			UGameplayAbility* InstancedAbility = nullptr;
			if (!InternalTryActivateAbility())
			{
				AbilityComp->ClientActivateAbilityFailed(CurrentParams->BatchInfo.AbilitySpecHandle, CurrentParams->BatchInfo.PredictionKey.Base);
			}
		}

		if (bLogs)
		{
			if (!FunctionName.contains("Tick") &&
				!FunctionName.contains("Visual") &&
				!FunctionName.contains("Clown Spinner") &&
				!FunctionName.contains("CustomStateChanged") &&
				!FunctionName.contains("ReceiveBeginPlay") &&
				!FunctionName.contains("OnAttachToBuilding") &&
				!FunctionName.contains("OnWorldReady") &&
				!FunctionName.contains("K2_GetActorLocation") &&
				!FunctionName.contains("ReceiveDrawHUD") &&
				!FunctionName.contains("ServerUpdateCamera") &&
				!FunctionName.contains("ServerMove") &&
				!FunctionName.contains("ContrailCheck") &&
				!FunctionName.contains("GetViewTarget") &&
				!FunctionName.contains("GetAllActorsOfClass") &&
				!FunctionName.contains("ClientAckGoodMove") &&
				!FunctionName.contains("ReadyToEndMatch") &&
				!FunctionName.contains("Check Closest Point") &&
				!FunctionName.contains("ServerTriggerCombatEvent") &&
				!FunctionName.contains("UpdateTime") &&
				!FunctionName.contains("OnUpdateMusic") &&
				!FunctionName.contains("UpdateStateEvent") &&
				!FunctionName.contains("ServerTouchActiveTime") &&
				!FunctionName.contains("OnCheckIfSurrounded") &&
				!FunctionName.contains("ServerFireAIDirectorEventBatch") &&
				!FunctionName.contains("ServerTriggerCombatEventBatch") &&
				!FunctionName.contains("UserConstructionScript") &&
				!FunctionName.contains("K2_OnReset") &&
				!FunctionName.contains("K2_OnEndViewTarget") &&
				!FunctionName.contains("K2_OnBecomeViewTarget") &&
				!FunctionName.contains("ReceiveUnpossessed") &&
				!FunctionName.contains("ClientGotoState") &&
				!FunctionName.contains("K2_OnEndViewTarget") &&
				!FunctionName.contains("K2_OnBecomeViewTarget") &&
				!FunctionName.contains("ClientSetViewTarget") &&
				!FunctionName.contains("ServerClientPawnLoaded") &&
				!FunctionName.contains("ReceiveEndPlay") &&
				!FunctionName.contains("OnPerceptionStimuliSourceEndPlay") &&
				!FunctionName.contains("HandleOnHUDElementVisibilityChanged") &&
				!FunctionName.contains("OnHUDElementVisibilityChanged") &&
				!FunctionName.contains("HandleInteractionChanged") &&
				!FunctionName.contains("BlueprintModifyCamera") &&
				!FunctionName.contains("BlueprintModifyPostProcess") &&
				!FunctionName.contains("ServerSetSpectatorLocation") &&
				!FunctionName.contains("ServerFireAIDirectorEvent") &&
				!FunctionName.contains("ServerTryActivateAbility") &&
				!FunctionName.contains("ClientActivateAbilitySucceed") &&
				!FunctionName.contains("K2_CommitExecute") &&
				!FunctionName.contains("ServerSetSpectatorLocation") &&
				!FunctionName.contains("CanJumpInternal") &&
				!FunctionName.contains("K2_OnMovementModeChanged") &&
				!FunctionName.contains("ServerModifyStat") &&
				!FunctionName.contains("ReceiveHit") &&
				!FunctionName.contains("K2_OnEndAbility") &&
				!FunctionName.contains("OnWalkingOffLedge") &&
				!FunctionName.contains("ServerEndAbility") &&
				!FunctionName.contains("Execute") &&
				!FunctionName.contains("NetMulticast_InvokeGameplayCueExecuted_FromSpec") &&
				!FunctionName.contains("OnDamagePlayEffects") &&
				!FunctionName.contains("OnMontageStarted") &&
				!FunctionName.contains("OnNewDamageNumber") &&
				!FunctionName.contains("BP_GetTokenizedDescriptionText") &&
				!FunctionName.contains("NetMulticast_InvokeGameplayCueExecuted_WithParams") &&
				!FunctionName.contains("GameplayCue_InstantDeath") &&
				!FunctionName.contains("K2_GetActorRotation") &&
				!FunctionName.contains("K2_DestroyActor") &&
				!FunctionName.contains("OnDetachFromBuilding") &&
				!FunctionName.contains("OnRep_bAlreadySearched") &&
				!FunctionName.contains("OnSetSearched") &&
				!FunctionName.contains("GetAircraft") &&
				!FunctionName.contains("BeginSpawningActorFromClass") &&
				!FunctionName.contains("BlueprintInitializeAnimation") &&
				!FunctionName.contains("BlueprintUpdateAnimation") &&
				!FunctionName.contains("BlueprintPostEvaluateAnimation") &&
				!FunctionName.contains("FinishSpawningActor") &&
				!FunctionName.contains("PawnUniqueIDSet") &&
				!FunctionName.contains("OnRep_Owner") &&
				!FunctionName.contains("OnRep_Pawn") &&
				!FunctionName.contains("Possess") &&
				!FunctionName.contains("ReceivePossessed") &&
				!FunctionName.contains("ClientRestart") &&
				!FunctionName.contains("SetControlRotation") &&
				!FunctionName.contains("ClientRetryClientRestart") &&
				!FunctionName.contains("ExecuteUbergraph_PlayerPawn_Athena_Generic") &&
				!FunctionName.contains("ExecuteUbergraph_PlayerPawn_Athena") &&
				!FunctionName.contains("IsInAircraft") &&
				!FunctionName.contains("FindPlayerStart") &&
				!FunctionName.contains("SpawnDefaultPawnFor") &&
				!FunctionName.contains("MustSpectate") &&
				!FunctionName.contains("GetDefaultPawnClassForController") &&
				!FunctionName.contains("On Game Phase Change") &&
				!FunctionName.contains("ClientAdjustPosition") &&
				!FunctionName.contains("Movement Audio Crossfader__UpdateFunc") &&
				!FunctionName.contains("Holding Audio Crossfader__UpdateFunc") &&
				!FunctionName.contains("OnUpdateDirectionalLightForTimeOfDay") &&
				!FunctionName.contains("OnMontageEnded") &&
				!FunctionName.contains("ServerCancelAbility") &&
				!FunctionName.contains("K2_ActivateAbility") &&
				!FunctionName.contains("ServerHandleMissionEvent_ToggledCursorMode") &&
				!FunctionName.contains("OnBlendOut_") &&
				!FunctionName.contains("ClientEndAbility") &&
				!FunctionName.contains("OnSafeZoneStateChange") &&
				!FunctionName.contains("ClientVeryShortAdjustPosition") &&
				!FunctionName.contains("OnDayPhaseChange") &&
				!FunctionName.contains("On Day Phase Change") &&
				!FunctionName.contains("K2_OnStartCrouch") &&
				!FunctionName.contains("K2_OnEndCrouch") &&
				!FunctionName.contains("On Player Won") &&
				!FunctionName.contains("ClientFinishedInteractionInZone") &&
				!FunctionName.contains("ClientReceiveKillNotification") &&
				!FunctionName.contains("ReceiveCopyProperties") &&
				!FunctionName.contains("K2_OnLogout") &&
				!FunctionName.contains("ClientReceiveLocalizedMessage") &&
				!FunctionName.contains("ClientCancelAbility") &&
				!FunctionName.contains("ServerFinishedInteractionInZoneReport") &&
				!FunctionName.contains("FallingTimeline__UpdateFunc") &&
				!FunctionName.contains("BndEvt__InterceptCollision_K2Node_ComponentBoundEvent_5_ComponentBeginOverlapSignature__DelegateSignature") &&
				!FunctionName.contains("ReceiveActorBeginOverlap") &&
				!FunctionName.contains("Conv_StringToName") &&
				!FunctionName.contains("OnRep_GamePhase") &&
				!FunctionName.contains("K2_OnSetMatchState") &&
				!FunctionName.contains("OnAircraftEnteredDropZone") &&
				!FunctionName.contains("ServerShortTimeout") &&
				!FunctionName.contains("UpdateStateWidgetContent") &&
				!FunctionName.contains("PreConstruct") &&
				!FunctionName.contains("Construct") &&
				!FunctionName.contains("OnCurrentTextStyleChanged") &&
				!FunctionName.contains("UpdateButtonState") &&
				!FunctionName.contains("OnBangStateChanged") &&
				!FunctionName.contains("OnPlayerInfoChanged") &&
				!FunctionName.contains("Update") &&
				!FunctionName.contains("OnBeginIntro") &&
				!FunctionName.contains("HandleQuickBarChangedBP") &&
				!FunctionName.contains("HandleInventoryUpdatedEvent") &&
				!FunctionName.contains("OnActivated") &&
				!FunctionName.contains("OnBeginOutro") &&
				!FunctionName.contains("HandleActiveWidgetDeactivated") &&
				!FunctionName.contains("OnDeactivated") &&
				!FunctionName.contains("OnStateStarted") &&
				!FunctionName.contains("SetRenderTransform") &&
				!FunctionName.contains("OnAnimationFinished") &&
				!FunctionName.contains("ReadyToStartMatch") &&
				!FunctionName.contains("SetWidthOverride") &&
				!FunctionName.contains("SetHeightOverride") &&
				!FunctionName.contains("HandleMinimizeFinished") &&
				!FunctionName.contains("ServerUpdateLevelVisibility") &&
				!FunctionName.contains("OnDayPhaseChanged") &&
				!FunctionName.contains("ServerLoadingScreenDropped") &&
				!FunctionName.contains("On Game Phase Step Changed") &&
				!FunctionName.contains("HandleGamePhaseStepChanged") &&
				!FunctionName.contains("SetColorAndOpacity") &&
				!FunctionName.contains("OnAnimationStarted") &&
				!FunctionName.contains("UpdateMessaging") &&
				!FunctionName.contains("ServerSendLoadoutConfig") &&
				!FunctionName.contains("CalculateBaseMagnitude") &&
				!FunctionName.contains("ClientRegisterWithParty") &&
				!FunctionName.contains("InitializeHUDForPlayer") &&
				!FunctionName.contains("ClientSetHUD") &&
				!FunctionName.contains("ClientEnableNetworkVoice") &&
				!FunctionName.contains("ClientUpdateMultipleLevelsStreamingStatus") &&
				!FunctionName.contains("ClientFlushLevelStreaming") &&
				!FunctionName.contains("ClientOnGenericPlayerInitialization") &&
				!FunctionName.contains("ClientCapBandwidth") &&
				!FunctionName.contains("K2_PostLogin") &&
				!FunctionName.contains("OnRep_bHasStartedPlaying") &&
				!FunctionName.contains("SetOwner") &&
				!FunctionName.contains("OnRep_QuickBar") &&
				!FunctionName.contains("HandleStartingNewPlayer") &&
				!FunctionName.contains("ServerUpdateMultipleLevelsVisibility") &&
				!FunctionName.contains("ServerSetPartyOwner") &&
				!FunctionName.contains("PlayerCanRestart") &&
				!FunctionName.contains("ServerCreateCombatManager") &&
				!FunctionName.contains("ServerCreateAIDirectorDataManager") &&
				!FunctionName.contains("EnableSlot") &&
				!FunctionName.contains("BlueprintGetFailedInteractionString") &&
				!FunctionName.contains("OnBuildingActorInitialized") &&
				!FunctionName.contains("BlueprintCanInteract") &&
				!FunctionName.contains("DisableSlot") &&
				!FunctionName.contains("ServerSetShowHeroBackpack") &&
				!FunctionName.contains("ServerSetShowHeroHeadAccessories") &&
				!FunctionName.contains("ServerSetClientHasFinishedLoading") &&
				!FunctionName.contains("ServerReadyToStartMatch") &&
				!FunctionName.contains("Received_Notify") &&
				!FunctionName.contains("Received_NotifyBegin") &&
				!FunctionName.contains("AnimNotify_LeftFootStep") &&
				!FunctionName.contains("AnimNotify_RightFootStep") &&
				!FunctionName.contains("Completed_") &&
				!FunctionName.contains("InputActionHoldStopped") &&
				!FunctionName.contains("ServerCurrentMontageSetPlayRate") &&
				!FunctionName.contains("ServerSetReplicatedTargetData") &&
				!FunctionName.contains("Triggered_") &&
				!FunctionName.contains("ActorHasTag") &&
				!FunctionName.contains("RandomIntegerInRange") &&
				!FunctionName.contains("GetItemDefinitionBP") &&
				!FunctionName.contains("CreateTemporaryItemInstanceBP") &&
				!FunctionName.contains("SetOwningControllerForTemporaryItem") &&
				!FunctionName.contains("OnRep_PrimaryQuickBar") &&
				!FunctionName.contains("OnRep_SecondaryQuickBar") &&
				!FunctionName.contains("ServerSetupWeakSpotsOnBuildingActor") &&
				!FunctionName.contains("OnStartDirectionEffect") &&
				!FunctionName.contains("SetReplicates") &&
				!FunctionName.contains("ServerCurrentMontageSetNextSectionName") &&
				!FunctionName.contains("NetFadeOut") &&
				!FunctionName.contains("OnFadeOut") &&
				!FunctionName.contains("NetOnHitCrack") &&
				!FunctionName.contains("OnHitCrack") &&
				!FunctionName.contains("NetMulticast_InvokeGameplayCueAdded_WithParams") &&
				!FunctionName.contains("GameplayCue") &&
				!FunctionName.contains("ReceiveActorEndOverlap") &&
				!FunctionName.contains("PhysicsVolumeChanged") &&
				!FunctionName.contains("ServerAddItemInternal") &&
				!FunctionName.contains("FortClientPlaySound") &&
				!FunctionName.contains("GetPlayerController") &&
				!FunctionName.contains("TossPickup") &&
				!FunctionName.contains("OnRep_PrimaryPickupItemEntry") &&
				!FunctionName.contains("ServerActivateSlotInternal") &&
				!FunctionName.contains("EquipWeaponDefinition") &&
				!FunctionName.contains("OnInitAlteration") &&
				!FunctionName.contains("OnInitCosmeticAlterations") &&
				!FunctionName.contains("ClientGivenTo") &&
				!FunctionName.contains("K2_OnUnEquip") &&
				!FunctionName.contains("ClientInternalEquipWeapon") &&
				!FunctionName.contains("GetItemGuid") &&
				!FunctionName.contains("InternalServerSetTargeting") &&
				!FunctionName.contains("ServerReleaseInventoryItemKey") &&
				!FunctionName.contains("OnPawnMontageBlendingOut") &&
				!FunctionName.contains("OnSetTargeting") &&
				!FunctionName.contains("OnRep_DefaultMetadata") &&
				!FunctionName.contains("GetDataTableRowNames") &&
				!FunctionName.contains("GetMaxDurability") &&
				!FunctionName.contains("BeginDeferredActorSpawnFromClass") &&
				!FunctionName.contains("OnRep_PickupLocationData") &&
				!FunctionName.contains("GetControlRotation") &&
				!FunctionName.contains("OnVisibilitySetEvent") &&
				!FunctionName.contains("ShouldShowSoundIndicator") &&
				!FunctionName.contains("WaitForPawn") &&
				!FunctionName.contains("OnProjectileStopDelegate") &&
				!FunctionName.contains("Handle Parachute Audio State") &&
				!FunctionName.contains("EvaluateGraphExposedInputs_ExecuteUbergraph_Fortnite_M_Avg_Player_AnimBlueprint_AnimGraphNode_"))
			{
				HLog("FuncName: " << Function->GetName());
			}
		}

		ToHooks::ProcessEvent(Object, Function, Parms);
	}



	void InitDetour()
	{
		START_DETOUR;
			
		DetourAttach(ToHooks::ProcessEvent, ProcessEventHook);
		DetourAttach(ToHooks::KickPlayer, KickPlayerHook);
		// DetourAttach(ToHooks::GetNetMode, GetNetModeHook); //<- me tape pas

		END_DETOUR;

		HLog("InitDetour Success !");
	}
}