#pragma once
#include "SDK/FortniteGame_classes.hpp"

UWorld* World;


namespace Logging
{
    inline std::string getCurrentDateTime(std::string s) {
        time_t now = time(0);
        struct tm  tstruct;
        char  buf[80];
        tstruct = *localtime(&now);
        if (s == "now")
            strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
        else if (s == "date")
            strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
        return std::string(buf);
    };
    inline void FileLogger(std::string logMsg) {

        std::string filePath = "C:\\ServerLogs" + getCurrentDateTime("date") + ".txt";
        std::string now = getCurrentDateTime("now");
        std::ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
        ofs << now << '\t' << logMsg << '\n';
        ofs.close();
    }
}

template<class TArrayType>
class TArray_
{

public:
    FORCEINLINE int Num() const
    {
        return Count;
    }
    FORCEINLINE int Max() const
    {
        return MaxElements;
    }
    FORCEINLINE int Slack() const
    {
        return MaxElements - Count;
    }
    FORCEINLINE void Reserve(const int NumElements)
    {
        Data = Slack() >= NumElements ? Data : (TArrayType*)ReallocMemory(Data, (MaxElements = Count + NumElements) * sizeof(TArrayType), 0);
    }
    FORCEINLINE void Reset(int MinSizeAfterReset = 0)
    {
        MaxElements = MinSizeAfterReset;
        Count = 0;

        FreeMemory(Data);
        Data = MallocMemory(MinSizeAfterReset * sizeof(TArrayType), alignof(TArrayType));
    }
    FORCEINLINE bool RemoveSingle(const int Index)
    {
        if (Index < Count)
        {
            if (Index != Count - 1)
                Data[Index] = Data[Count - 1];

            --Count;

            return true;
        }
        return false;
    }
    FORCEINLINE void RemoveAt(int Index, int Lenght = 1)
    {
        for (; Lenght != 0; --Lenght)
        {
            if (!RemoveSingle(Index++))
                break;
        }
    }
    FORCEINLINE void AddSingle(TArrayType InData)
    {
        Reserve(1);
        Data[Count] = InData;
        ++Count;
    }
    FORCEINLINE void Add(int n, TArrayType InData...)
    {
        va_list MyList;
        va_start(MyList, n);

        for (int i = 0; i < n; i++)
        {
            AddSingle(va_arg(MyList, TArrayType));
        }
        va_end(MyList);
    };
    FORCEINLINE void FreeArray()
    {
        FreeMemory(Data);
        Data = nullptr;
        Count = 0;
        MaxElements = 0;
    }

    FORCEINLINE explicit operator bool()
    {
        return MaxElements > 0 && Count > 0 && Data;
    }
    FORCEINLINE TArrayType& operator[](int i)
    {
        return Data[i];
    };

    FORCEINLINE const TArrayType& operator[](int i) const
    {
        return Data[i];
    };

    FORCEINLINE int MaxIndex()
    {
        return Count - 1;
    }

    class FBaseArrayIterator
    {
        int32_t Index;
        TArray<TArrayType>& ItArray;

    public:
        FBaseArrayIterator(TArray<TArrayType>& Array)
            : ItArray(Array), Index(Array.Count)
        {
        }
        FBaseArrayIterator(TArray<TArrayType>& Array, int32_t CurrentIndex)
            : ItArray(Array), Index(CurrentIndex)
        {
        }

        FORCEINLINE TArrayType& operator*()
        {
            return ItArray[Index];
        }
        FORCEINLINE TArrayType& operator->()
        {
            return ItArray[Index];
        }
        FORCEINLINE FBaseArrayIterator& operator++()
        {
            ++Index;
            return *this;
        }
        FORCEINLINE bool operator==(const FBaseArrayIterator& other) const
        {
            return Index == other.Index;
        }
        FORCEINLINE bool operator!=(const FBaseArrayIterator& other) const
        {
            return Index != other.Index;
        }
    };

    FORCEINLINE FBaseArrayIterator begin()
    {
        return FBaseArrayIterator(*this, 0);
    }
    FORCEINLINE FBaseArrayIterator begin() const
    {
        return FBaseArrayIterator(*this, 0);
    }
    FORCEINLINE FBaseArrayIterator end()
    {
        return FBaseArrayIterator(*this);
    }
    FORCEINLINE FBaseArrayIterator end() const
    {
        return FBaseArrayIterator(*this);
    }

private:
    TArrayType* Data;
    int32_t Count;
    int32_t MaxElements;
};

namespace Inventory
{
	void (*MarkArrayDirty)(FFastArraySerializer*);
	static void* (*MarkAbilitySpecDirty)(UAbilitySystemComponent*, FGameplayAbilitySpec, bool);
	static UObject* (*StaticConstructObjectInternal)(void*, void*, void*, int, unsigned int, void*, bool, void*, bool);

	static void UpdateInventory(AFortPlayerController* TargetController)
	{
		MarkArrayDirty(&TargetController->WorldInventory->Inventory);

		TargetController->HandleWorldInventoryLocalUpdate();
		TargetController->WorldInventory->HandleInventoryLocalUpdate();
		TargetController->OnRep_QuickBar();
		TargetController->QuickBars->OnRep_PrimaryQuickBar();
		TargetController->QuickBars->OnRep_SecondaryQuickBar();

		TargetController->ForceUpdateQuickbar(EFortQuickBars::Primary);
		TargetController->ForceUpdateQuickbar(EFortQuickBars::Secondary);
	}

	int32 GetItemLevel(const FDataTableCategoryHandle& LootLevelData)
	{
		int32(*GetItemLevel)(const FDataTableCategoryHandle & DataTableCategoryHandle, int WorldLevel) = decltype(GetItemLevel)(0x14589F0 + uintptr_t(GetModuleHandle(0))); // 7FF6B96C89F0

		return GetItemLevel(LootLevelData, Globals::GetGameState()->WorldLevel);
	}

	static UFortItem* CreateItem(AFortPlayerController* TargetController, UFortItemDefinition* ItemDefinition, int count)
	{
		auto TempItemInstance = ItemDefinition->CreateTemporaryItemInstanceBP(count, 1);

		if (TempItemInstance)
		{
			TempItemInstance->SetOwningControllerForTemporaryItem(TargetController);
		}

		return TempItemInstance;
	}

    static void AddItemToInventory(UFortItemDefinition* ItemDefinition, EFortQuickBars Quickbar, AFortPlayerController* TargetController, int count, int Slot)
    {
        if (!ItemDefinition)
        {
            printf("Invalid Item Definition\n");
            return;
        }


        auto Inventory = TargetController->WorldInventory;

        auto QuickBars = TargetController->QuickBars;

        if (Quickbar == EFortQuickBars::Secondary)
        {
            UFortWorldItem* ItemInstance = nullptr;

            for (int i = 0; i < Inventory->Inventory.ItemInstances.Num(); i++)
            {
                if (Inventory->Inventory.ItemInstances[i]->GetItemDefinitionBP() == ItemDefinition)
                    ItemInstance = Inventory->Inventory.ItemInstances[i];
            }

            if (ItemInstance)
            {
                auto NewCount = ItemInstance->ItemEntry.Count + count;
                for (int i = 0; i < Inventory->Inventory.ReplicatedEntries.Num(); i++)
                {
                    if (Inventory->Inventory.ReplicatedEntries[i].ItemDefinition == ItemDefinition)
                    {
                        Inventory->Inventory.ReplicatedEntries[i].Count = NewCount;
                        Inventory->Inventory.ReplicatedEntries[i].ReplicationKey++;
                    }
                }
                ItemInstance->ItemEntry.Count = NewCount;
                ItemInstance->ItemEntry.ReplicationKey++;
                return;
            }
        }


        auto ItemInstance = (UFortWorldItem*)CreateItem(TargetController, ItemDefinition, count);
        ItemInstance->OwnerInventory = Inventory;
        ItemInstance->bTemporaryItemOwningController = true;

        if (Inventory && QuickBars)
        {
            auto ReplicatedEntries = (TArray_<FFortItemEntry>*) & Inventory->Inventory.ReplicatedEntries;
            auto ItemInstances = (TArray_<UFortWorldItem*>*) & Inventory->Inventory.ItemInstances;

            ReplicatedEntries->AddSingle(ItemInstance->ItemEntry);
            ItemInstances->AddSingle(ItemInstance);


            Logging::FileLogger("Added Item To Inventory");

            TargetController->AddItemToQuickBars(ItemDefinition, Quickbar, Slot);
            TargetController->QuickBars->ServerAddItemInternal(ItemInstance->GetItemGuid(), Quickbar, Slot);

            Logging::FileLogger("Added Item To QuickBars");
        }

        UpdateInventory(TargetController);
    }

    static UFortTrapItemDefinition* GetRandomTrapDefinition()
    {
        while (true)
        {
            auto Idx = UKismetMathLibrary::RandomInteger(TrapsPool.size());
            auto Item = TrapsPool[Idx];
            auto Def = UObject::FindObjectFast<UFortTrapItemDefinition>(Item);
            if (!Def)
                continue;

            auto rng = std::default_random_engine{};
            std::shuffle(TrapsPool.begin(), TrapsPool.end(), rng);
            return Def;
        }
    }

    static UFortItemDefinition* GetRandomPickaxeDefinition()
    {
        while (true)
        {
            auto Idx = UKismetMathLibrary::RandomInteger(PickaxePool.size());
            auto Item = PickaxePool[Idx];
            auto Def = UObject::FindObjectFast<UFortItemDefinition>(Item);
            if (!Def)
                continue;

            auto rng = std::default_random_engine{};
            std::shuffle(PickaxePool.begin(), PickaxePool.end(), rng);
            return Def;
        }
    }

    static void SetupInventory(AFortPlayerController* TargetController)
    {
        auto Pawn = (APlayerPawn_Athena_C*)TargetController->Pawn;
        //Building Peaces	
        auto pickaxe = GetRandomPickaxeDefinition();
        static auto m_pWallBuildDef = UObject::FindObjectFast<UFortBuildingItemDefinition>("/Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall");
        static auto m_pFloorBuildDef = UObject::FindObjectFast<UFortBuildingItemDefinition>("/Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor");
        static auto m_pStairBuildDef = UObject::FindObjectFast<UFortBuildingItemDefinition>("/Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W");
        static auto m_pRoofBuildDef = UObject::FindObjectFast<UFortBuildingItemDefinition>("/Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS");
        //Building Mats	
        static auto WoodItemDef = UObject::FindObjectFast<UFortBuildingItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
        static auto BrickItemDef = UObject::FindObjectFast<UFortBuildingItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
        static auto MetalItemDef = UObject::FindObjectFast<UFortBuildingItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");
        //Weapons	
        static auto AssualtRifle = UObject::FindObjectFast<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_R_Ore_T03.WID_Assault_Auto_Athena_R_Ore_T03");
        static auto Smg = UObject::FindObjectFast<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_Scavenger_Athena_UC_Ore_T03.WID_Pistol_Scavenger_Athena_UC_Ore_T03");
        static auto PumpShotgun = UObject::FindObjectFast<UFortBuildingItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_UC_Ore_T03.WID_Shotgun_Standard_Athena_UC_Ore_T03");
        static auto Sniper = UObject::FindObjectFast<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_NoScope_Athena_UC_Ore_T03.WID_Sniper_NoScope_Athena_UC_Ore_T03");
        static auto slurp = UObject::FindObjectFast<UFortWeaponItemDefinition>("/Game/Athena/Items/Consumables/KnockGrenade/Athena_KnockGrenade.Athena_KnockGrenade");
        static auto JumpPad = UObject::FindObjectFast<UFortItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_Player_Launch_Pad_Athena.TID_Floor_Player_Launch_Pad_Athena");
        /*static auto SpikeTrap = UObject::FindObjectFast<UFortItemDefinition>("/Game/Items/Traps/Floor/TID_Floor_Spikes_UC_T03.TID_Floor_Spikes_UC_T03");*/
        static auto Rockets = UObject::FindObjectFast<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AmmoDataRockets.AmmoDataRockets");
        static auto Shells = UObject::FindObjectFast<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");
        static auto Medium = UObject::FindObjectFast<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
        static auto Light = UObject::FindObjectFast<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
        static auto Heavy = UObject::FindObjectFast<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");
        static auto Trap = UObject::FindObjectFast<UFortItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_Spikes_Athena_R_T03.TID_Floor_Spikes_Athena_R_T03");

        TargetController->QuickBars->ServerEnableSlot(SDK::EFortQuickBars::Secondary, 0);
        TargetController->QuickBars->ServerEnableSlot(SDK::EFortQuickBars::Secondary, 1);
        TargetController->QuickBars->ServerEnableSlot(SDK::EFortQuickBars::Secondary, 2);
        TargetController->QuickBars->ServerEnableSlot(SDK::EFortQuickBars::Secondary, 3);
        TargetController->QuickBars->ServerEnableSlot(SDK::EFortQuickBars::Secondary, 4);
        TargetController->QuickBars->ServerEnableSlot(SDK::EFortQuickBars::Secondary, 5);
        TargetController->QuickBars->ServerEnableSlot(SDK::EFortQuickBars::Primary, 0);
        TargetController->QuickBars->ServerEnableSlot(SDK::EFortQuickBars::Primary, 1);
        TargetController->QuickBars->ServerEnableSlot(SDK::EFortQuickBars::Primary, 2);
        TargetController->QuickBars->ServerEnableSlot(SDK::EFortQuickBars::Primary, 3);
        TargetController->QuickBars->ServerEnableSlot(SDK::EFortQuickBars::Primary, 4);
        TargetController->QuickBars->ServerEnableSlot(SDK::EFortQuickBars::Primary, 5);
        //Add Weapons	
        AddItemToInventory(pickaxe, EFortQuickBars::Primary, TargetController, 1, 0);
        //Add Building Peaces	
        AddItemToInventory(m_pWallBuildDef, EFortQuickBars::Secondary, TargetController, 1, 0);
        AddItemToInventory(m_pFloorBuildDef, EFortQuickBars::Secondary, TargetController, 1, 1);
        AddItemToInventory(m_pStairBuildDef, EFortQuickBars::Secondary, TargetController, 1, 2);
        AddItemToInventory(m_pRoofBuildDef, EFortQuickBars::Secondary, TargetController, 1, 3);
        //Add Mats	
        AddItemToInventory(WoodItemDef, EFortQuickBars::Secondary, TargetController, 500, 0);
        AddItemToInventory(BrickItemDef, EFortQuickBars::Secondary, TargetController, 300, 0);
        AddItemToInventory(MetalItemDef, EFortQuickBars::Secondary, TargetController, 100, 0);
        // Add Ammo

        AddItemToInventory(Rockets, EFortQuickBars::Secondary, TargetController, 4, 0);
        AddItemToInventory(Shells, EFortQuickBars::Secondary, TargetController, 16, 0);
        AddItemToInventory(Medium, EFortQuickBars::Secondary, TargetController, 90, 0);
        AddItemToInventory(Light, EFortQuickBars::Secondary, TargetController, 130, 0);
        AddItemToInventory(Heavy, EFortQuickBars::Secondary, TargetController, 24, 0);

        //traps 
        /*AddItemToInventory(GetRandomTrapDefinition(), EFortQuickBars::Secondary, TargetController, 3, 0);*/
        /*AddItemToInventory(JumpPad, EFortQuickBars::Secondary, TargetController, 2, 0);*/

    }

    static void GiveBackMats(AFortPlayerControllerAthena* TargetController)
    {
        static auto WoodItemDef = UObject::FindObjectFast<UFortBuildingItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
        static auto BrickItemDef = UObject::FindObjectFast<UFortBuildingItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
        static auto MetalItemDef = UObject::FindObjectFast<UFortBuildingItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

        AddItemToInventory(WoodItemDef, EFortQuickBars::Secondary, TargetController, 500, 0);
        AddItemToInventory(BrickItemDef, EFortQuickBars::Secondary, TargetController, 300, 0);
        AddItemToInventory(MetalItemDef, EFortQuickBars::Secondary, TargetController, 100, 0);
    }

    static void GiveWeapons(AFortPlayerControllerAthena* TargetController)
    {
        static auto AssualtRifle = UObject::FindObjectFast<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_R_Ore_T03.WID_Assault_Auto_Athena_R_Ore_T03");
        static auto Smg = UObject::FindObjectFast<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_Scavenger_Athena_UC_Ore_T03.WID_Pistol_Scavenger_Athena_UC_Ore_T03");
        static auto PumpShotgun = UObject::FindObjectFast<UFortBuildingItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_UC_Ore_T03.WID_Shotgun_Standard_Athena_UC_Ore_T03");
        static auto Sniper = UObject::FindObjectFast<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_NoScope_Athena_UC_Ore_T03.WID_Sniper_NoScope_Athena_UC_Ore_T03");
        static auto slurp = UObject::FindObjectFast<UFortWeaponItemDefinition>("/Game/Athena/Items/Consumables/KnockGrenade/Athena_KnockGrenade.Athena_KnockGrenade");

        AddItemToInventory(AssualtRifle, EFortQuickBars::Primary, TargetController, 1, 1);
        AddItemToInventory(Smg, EFortQuickBars::Primary, TargetController, 1, 2);
        AddItemToInventory(PumpShotgun, EFortQuickBars::Primary, TargetController, 1, 3);
        AddItemToInventory(Sniper, EFortQuickBars::Primary, TargetController, 1, 4);
        AddItemToInventory(slurp, EFortQuickBars::Primary, TargetController, 1, 5);
    }

    std::wstring s2ws(const std::string& str)
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }

    static void RemoveBuildingAmount(UClass* BuildingClass, AFortPlayerControllerAthena* TargetController)
    {
        auto Inventory = TargetController->WorldInventory;

        auto ReplicatedEntries = Inventory->Inventory.ReplicatedEntries;
        auto ItemInstances = Inventory->Inventory.ItemInstances;

        for (int i = 0; i < Inventory->Inventory.ReplicatedEntries.Num(); i++)
        {
            if (Inventory->Inventory.ReplicatedEntries[i].ItemDefinition->GetName().contains("ItemData"))
            {
                int NewCount = Inventory->Inventory.ReplicatedEntries[i].Count - 10;

                if (BuildingClass->GetName().contains("W1") && Inventory->Inventory.ReplicatedEntries[i].ItemDefinition->GetName().contains("Wood"))
                {
                    Inventory->Inventory.ReplicatedEntries[i].Count -= 10;
                    Inventory->Inventory.ReplicatedEntries[i].ReplicationKey++;

                    for (int j = 0; j < Inventory->Inventory.ItemInstances.Num(); j++)
                    {
                        if (Inventory->Inventory.ItemInstances[j]->GetName().contains("Wood"))
                        {
                            Inventory->Inventory.ItemInstances[j]->ItemEntry.Count -= 10;
                            Inventory->Inventory.ItemInstances[j]->ItemEntry.ReplicationKey++;
                        }
                    }

                    //ItemInstances.RemoveSingle(i);
                    //AddItemToInventory(Inventory->Inventory.ReplicatedEntries[i].ItemDefinition, EFortQuickBars::Secondary, TargetController, Inventory->Inventory.ReplicatedEntries[i].Count - 10, 0);
                    UpdateInventory(TargetController);
                    return;
                }
                if (BuildingClass->GetName().contains("S1") && Inventory->Inventory.ReplicatedEntries[i].ItemDefinition->GetName().contains("Stone"))
                {
                    Inventory->Inventory.ReplicatedEntries[i].Count -= 10;
                    Inventory->Inventory.ReplicatedEntries[i].ReplicationKey++;

                    for (int j = 0; j < Inventory->Inventory.ItemInstances.Num(); j++)
                    {
                        if (Inventory->Inventory.ItemInstances[j]->GetName().contains("Stone"))
                        {
                            Inventory->Inventory.ItemInstances[j]->ItemEntry.Count -= 10;
                            Inventory->Inventory.ItemInstances[j]->ItemEntry.ReplicationKey++;
                        }
                    }

                    //ItemInstances.RemoveSingle(i);
                    //AddItemToInventory(Inventory->Inventory.ReplicatedEntries[i].ItemDefinition, EFortQuickBars::Secondary, TargetController, Inventory->Inventory.ReplicatedEntries[i].Count - 10, 1);
                    UpdateInventory(TargetController);
                    return;
                }
                if (BuildingClass->GetName().contains("M1") && Inventory->Inventory.ReplicatedEntries[i].ItemDefinition->GetName().contains("Metal"))
                {
                    Inventory->Inventory.ReplicatedEntries[i].Count -= 10;
                    Inventory->Inventory.ReplicatedEntries[i].ReplicationKey++;

                    for (int j = 0; j < Inventory->Inventory.ItemInstances.Num(); j++)
                    {
                        if (Inventory->Inventory.ItemInstances[j]->GetName().contains("Metal"))
                        {
                            Inventory->Inventory.ItemInstances[j]->ItemEntry.Count -= 10;
                            Inventory->Inventory.ItemInstances[j]->ItemEntry.ReplicationKey++;
                        }
                    }

                    //ItemInstances.RemoveSingle(i);
                    //AddItemToInventory(Inventory->Inventory.ReplicatedEntries[i].ItemDefinition, EFortQuickBars::Secondary, TargetController, Inventory->Inventory.ReplicatedEntries[i].Count - 10, 2);
                    UpdateInventory(TargetController);
                    return;
                }
            }
        }
    }

    static void EmptyInventory()
    {
        for (int i = 0; i < World->NetDriver->ClientConnections.Num(); i++)
        {
            auto Connection = World->NetDriver->ClientConnections[i];
            auto PlayerController = (AFortPlayerController*)Connection->PlayerController;
            auto Inventory = PlayerController->WorldInventory;

            if (!PlayerController)
                continue;

            if (!Inventory)
                continue;

            for (int i = 0; i < Inventory->Inventory.ItemInstances.Num(); i++)
            {

                if (i == 0)
                    continue;

                if (Inventory->Inventory.ReplicatedEntries[i].ItemDefinition->GetName().contains("Building"))
                    continue;

                if (Inventory->Inventory.ReplicatedEntries[i].ItemDefinition->GetName().contains("Harvest"))
                    continue;

                if (Inventory->Inventory.ReplicatedEntries[i].ItemDefinition->GetName().contains("Wood"))
                    continue;

                PlayerController->QuickBars->EmptySlot(EFortQuickBars::Primary, i);
                Inventory->Inventory.ItemInstances.Remove(i);
                Inventory->Inventory.ReplicatedEntries.Remove(i);
            }

            UpdateInventory(PlayerController);
            SetupInventory(PlayerController);
        }
    }

    static int GetNextSlot(AFortPlayerController* PC)
    {
        return PC->QuickBars->PrimaryQuickBar.Slots.Num() + 1;
    }
}

std::vector<std::string> TrapsPool = {
    "/Game/Athena/Items/Traps/TID_Floor_Player_Launch_Pad_Athena.TID_Floor_Player_Launch_Pad_Athena",
    "/Game/Athena/Items/Traps/TID_Floor_Spikes_Athena_R_T03.TID_Floor_Spikes_Athena_R_T03"
};

std::vector<std::string> PickaxePool = {
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Anchor_Athena.WID_Harvest_Pickaxe_Anchor_Athena",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_ArtDeco.WID_Harvest_Pickaxe_ArtDeco",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Assassin.WID_Harvest_Pickaxe_Assassin",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_BoltOn_Athena_C_T01.WID_Harvest_Pickaxe_BoltOn_Athena_C_T01",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Brite.WID_Harvest_Pickaxe_Brite",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Carrot.WID_Harvest_Pickaxe_Carrot",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_CircuitBreaker.WID_Harvest_Pickaxe_CircuitBreaker",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_CuChulainn.WID_Harvest_Pickaxe_CuChulainn",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Deathvalley_Athena_C_T01.WID_Harvest_Pickaxe_Deathvalley_Athena_C_T01",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Disco_Athena.WID_Harvest_Pickaxe_Disco_Athena",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_District_Athena.WID_Harvest_Pickaxe_District_Athena",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Dragon.WID_Harvest_Pickaxe_Dragon",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Flamingo_Athena_C_T01.WID_Harvest_Pickaxe_Flamingo_Athena_C_T01",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Heart_Athena.WID_Harvest_Pickaxe_Heart_Athena",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_HolidayCandyCane_Athena.WID_Harvest_Pickaxe_HolidayCandyCane_Athena",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_HolidayGiftWrap_Athena.WID_Harvest_Pickaxe_HolidayGiftWrap_Athena",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_IcePick_Athena_C_T01.WID_Harvest_Pickaxe_IcePick_Athena_C_T01",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Keg_Athena.WID_Harvest_Pickaxe_Keg_Athena",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Lockjaw_Athena_C_T01.WID_Harvest_Pickaxe_Lockjaw_Athena_C_T01",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Medieval_Athena.WID_Harvest_Pickaxe_Medieval_Athena",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Megalodon_Athena.WID_Harvest_Pickaxe_Megalodon_Athena",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_PajamaParty.WID_Harvest_Pickaxe_PajamaParty",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Pizza.WID_Harvest_Pickaxe_Pizza",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Plunger.WID_Harvest_Pickaxe_Plunger",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_PotOfGold.WID_Harvest_Pickaxe_PotOfGold",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Prismatic.WID_Harvest_Pickaxe_Prismatic",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_RockerPunk.WID_Harvest_Pickaxe_RockerPunk",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Scavenger.WID_Harvest_Pickaxe_Scavenger",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Shark_Athena.WID_Harvest_Pickaxe_Shark_Athena",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_SickleBat_Athena_C_T01.WID_Harvest_Pickaxe_SickleBat_Athena_C_T01",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_SkiBoot.WID_Harvest_Pickaxe_SkiBoot",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Smiley_Athena_C_T01.WID_Harvest_Pickaxe_Smiley_Athena_C_T01",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Space.WID_Harvest_Pickaxe_Space",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Spikey_Athena_C_T01.WID_Harvest_Pickaxe_Spikey_Athena_C_T01",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Squeak.WID_Harvest_Pickaxe_Squeak",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Stealth.WID_Harvest_Pickaxe_Stealth",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Tactical.WID_Harvest_Pickaxe_Tactical",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_TacticalBlack.WID_Harvest_Pickaxe_TacticalBlack",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_TacticalUrban.WID_Harvest_Pickaxe_TacticalUrban",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Teslacoil_Athena.WID_Harvest_Pickaxe_Teslacoil_Athena",
"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_WinterCamo_Athena.WID_Harvest_Pickaxe_WinterCamo_Athena"
};