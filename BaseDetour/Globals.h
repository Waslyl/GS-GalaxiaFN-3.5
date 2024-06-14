#pragma once

namespace Globals
{

	UWorld* World = nullptr;
	UFortEngine* FortEngine = nullptr;

	UFortEngine* GetFortEngine(bool SkipCheck = false)
	{
		if (FortEngine == nullptr && !SkipCheck)
			FortEngine = UObject::FindObjectSlow<UFortEngine>("FortEngine_");

		return FortEngine;
	}

	UWorld* GetWorld(bool SkipCheck = false)
	{
		if (World == nullptr && !SkipCheck)
		{
			UFortEngine* FortEngine = GetFortEngine();

			if (FortEngine)
			{
				if (FortEngine->GameViewport)
				{
					if (FortEngine->GameViewport->World)
						return FortEngine->GameViewport->World;
				}
			}

			return nullptr;
		}

		return World;
	}

	TArray<ULocalPlayer*> GetLocalPlayers()
	{
		UFortEngine* FortEngine = GetFortEngine();

		if (FortEngine)
		{
			if (FortEngine->GameInstance)
				return FortEngine->GameInstance->LocalPlayers;
		}
	}

	APlayerController* GetServerPlayerController()
	{
		TArray<ULocalPlayer*> LocalPlayers = GetLocalPlayers();
		ULocalPlayer* LocalPlayer = LocalPlayers[0];

		if (LocalPlayer)
		{
			APlayerController* PlayerController = LocalPlayer->PlayerController;

			if (PlayerController)
				return PlayerController;
		}

		return nullptr;
	}

	AFortGameModeAthena* GetGameMode()
	{
		UWorld* World = GetWorld();

		if (World)
		{
			AFortGameModeAthena* GameMode = (AFortGameModeAthena*)World->AuthorityGameMode;

			if (GameMode)
				return GameMode;
		}

		return nullptr;
	}

	AFortGameStateAthena* GetGameState()
	{
		UWorld* World = GetWorld();

		if (World)
		{
			AFortGameStateAthena* GameState = (AFortGameStateAthena*)World->GameState;

			if (GameState)
				return GameState;
		}

		return nullptr;
	}


	UKismetMathLibrary* GetMathLibrary()
	{
		return (UKismetMathLibrary*)UKismetMathLibrary::StaticClass();
	}

	UKismetSystemLibrary* GetSystemLibrary()
	{
		return (UKismetSystemLibrary*)UKismetSystemLibrary::StaticClass();
	}

	UKismetStringLibrary* GetStringLibrary()
	{
		return (UKismetStringLibrary*)UKismetStringLibrary::StaticClass();
	}

	UGameplayStatics* GetGameplayStatics()
	{
		return (UGameplayStatics*)UGameplayStatics::StaticClass();
	}

	UDataTableFunctionLibrary* GetFunctionLibrary()
	{
		return (UDataTableFunctionLibrary*)UDataTableFunctionLibrary::StaticClass();
	}

	UFortKismetLibrary* GetKismetLibrary()
	{
		return (UFortKismetLibrary*)UFortKismetLibrary::StaticClass();
	}
}