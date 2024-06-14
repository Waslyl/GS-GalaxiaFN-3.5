#pragma once
#include "SDK/FortniteGame_classes.hpp"

namespace Inventory
{
	void UpdateInventory(AFortInventory* WorldInventory)
	{
		if (!WorldInventory)
			return;

		WorldInventory->HandleInventoryLocalUpdate();
		WorldInventory->Inventory.MarkArrayDirty();
	}

	int32 GetItemLevel(const FDataTableCategoryHandle& LootLevelData)
	{
		int32(*GetItemLevel)(const FDataTableCategoryHandle & DataTableCategoryHandle, int WorldLevel) = decltype(GetItemLevel)(0x14589F0 + uintptr_t(GetModuleHandle(0))); // 7FF6B96C89F0

		return GetItemLevel(LootLevelData, Globals::GetGameState()->WorldLevel);
	}
}