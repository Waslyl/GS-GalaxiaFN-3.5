#pragma once

namespace Beacon
{
	// bool AOnlineBeaconHost::InitHost()
	bool (*InitHost)(AOnlineBeaconHost* OnlineBeaconHost);
	void (*SetWorld)(UNetDriver* NetDriver, UWorld* InWorld); // mettre point virgule sinon Heliato pas content
	bool (*InitListen)(UNetDriver* IpNetDriver, void* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error);
	void (*TickFlush)(UNetDriver* NetDriver, float DeltaSeconds);
	int32 (*ServerReplicateActors)(UReplicationGraph* ReplicationGraph, float DeltaSeconds);

	void TickFlushHook(UNetDriver* NetDriver, float DeltaSeconds)
	{
		if (NetDriver->ClientConnections.Num() > 0 && NetDriver->ClientConnections[0]->InternalAck == false) // FORCER COMME LES NOIRS
		{
			if (NetDriver->ReplicationDriver)
			{
				ServerReplicateActors((UReplicationGraph*)NetDriver->ReplicationDriver, DeltaSeconds); // merci heliato le goat
			}
		}
		// ToHooks::ProcessEvent(Object, Function, Parms);
		TickFlush(NetDriver, DeltaSeconds); // la c bon ?
	}

	void Listen()
	{
		AOnlineBeaconHost* Beacon = Util::SpawnActor<AOnlineBeaconHost>(AOnlineBeaconHost::StaticClass(), {}, {});
		UWorld* World = Globals::GetWorld();

		if (!Beacon || !World)
		{
			return;
		}

		Beacon->ListenPort = 7777 - 1;

		if (World->NetDriver)
		{ 
			return;
		}

		if (InitHost(Beacon))
		{
			World->NetDriver = Beacon->NetDriver;

		}
		else
		{
			return;
		}

		if (World->NetDriver)
		{
			World->NetDriver->World = World;
			World->NetDriver->NetDriverName = Globals::GetStringLibrary()->Conv_StringToName(L"GameNetDriver");

			FString Error;
			auto URL = FURL();
			URL.Port = 7777; // 7777 = Port de base Unreal

			InitListen(World->NetDriver, World, URL, true, Error);
			SetWorld(World->NetDriver, World);

			World->LevelCollections[0].NetDriver = World->NetDriver;
			World->LevelCollections[1].NetDriver = World->NetDriver;

			START_DETOUR;

			DetourAttach(TickFlush, TickFlushHook); 

			END_DETOUR;
		}
	}
}