#pragma once
#include <random>
#include "SDK/FortniteGame_structs.hpp"

namespace Loots
{
	std::vector<FFortLootPackageData*> GetAvailableLootPackageData(UDataTable* AthenaLootPackages, FName LootPackageID)
	{
		std::vector<FFortLootPackageData*> AvailableLootPackageData;

		TArray<FName> RowNames;
		Globals::GetFunctionLibrary()->GetDataTableRowNames(AthenaLootPackages, &RowNames);

		for (int i = 0; i < RowNames.Num(); i++)
		{
			FName RowName = RowNames[i];

			FFortLootPackageData* LootPackageData = Globals::GetDataTableRowFromName<FFortLootPackageData>(AthenaLootPackages, RowName);
			if (!LootPackageData) continue;

			if (LootPackageData->LootPackageID != LootPackageID)
				continue;

			AvailableLootPackageData.push_back(LootPackageData);
		}

		return AvailableLootPackageData;
	}
}