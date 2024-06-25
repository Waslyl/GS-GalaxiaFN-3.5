#pragma once
#include <iostream>
#include <cctype>

namespace Functions
{
	void SummonFloorLoots()
	{
		UBlueprintGeneratedClass* FloorLootClass = UObject::FindObjectFast<UBlueprintGeneratedClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C");
		UBlueprintGeneratedClass* FloorLootWarmupClass = UObject::FindObjectFast<UBlueprintGeneratedClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C");

		if (FloorLootClass && FloorLootWarmupClass)
		{
			TArray<AActor*> FloorLootsResult;
			Globals::GetGameplayStatics()->GetAllActorsOfClass(Globals::GetWorld(), FloorLootClass, &FloorLootsResult);

			TArray<AActor*> FloorLootWarmupResult;
			Globals::GetGameplayStatics()->GetAllActorsOfClass(Globals::GetWorld(), FloorLootWarmupClass, &FloorLootWarmupResult);

			TArray<AActor*> FloorLoots;

			for (int i = 0; i < FloorLootsResult.Num(); i++)
			{
				FloorLoots.Add(FloorLootsResult[i]);
			}

			for (int i = 0; i < FloorLootWarmupResult.Num(); i++)
			{
				FloorLoots.Add(FloorLootWarmupResult[i]);
			}

			for (int i = 0; i < FloorLoots.Num(); i++)
			{
				ABuildingContainer* FloorLoot = (ABuildingContainer*)FloorLoots[i];
				if (!FloorLoot) continue;

				bool bSuccess;
				std::vector<FFortItemEntry> LootToDrops = Loots::ChooseLootToDrops(FloorLoot->SearchLootTierGroup, &bSuccess);

				if (!bSuccess)
					continue;

				FVector FloorLootLoc = FloorLoot->K2_GetActorLocation();

				FloorLootLoc.Z += 50;

				for (auto& LootToDrop : LootToDrops)
				{
					UFortWorldItemDefinition* ItemDefinition = (UFortWorldItemDefinition*)LootToDrop.ItemDefinition;

					if (!ItemDefinition)
						continue;

					Inventory::SpawnPickup(ItemDefinition, nullptr, &LootToDrop, FloorLootLoc, true);
				}

				FloorLoot->bAlreadySearched = true;
				FloorLoot->OnRep_bAlreadySearched();
			}
		}
	}
}