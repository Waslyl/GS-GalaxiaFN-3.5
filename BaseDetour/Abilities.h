#pragma once

namespace Abilities
{
    void (*MarkArrayDirty)(FFastArraySerializer*);
    static void* (*MarkAbilitySpecDirty)(UAbilitySystemComponent*, FGameplayAbilitySpec, bool);
    static UObject* (*StaticConstructObjectInternal)(void*, void*, void*, int, unsigned int, void*, bool, void*, bool);


    static __forceinline uintptr_t BaseAddress()
    {
        return reinterpret_cast<uintptr_t>(GetModuleHandle(0));
    }

    FGameplayAbilitySpecHandle(*GiveAbility)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec);
    UObject* (*CreateDefaultObject)(UClass*);
    static __int64 (*specthing)(void*, void*, char, int, void*) = decltype(specthing)(BaseAddress() + 0xB9AF40);

    static void GameAbility(APlayerPawn_Athena_C* TargetPawn, UClass* GameplayAbilityClass) // static sert pour enregistrer la fonction
    {
        auto AbilitySystemComponent = TargetPawn->AbilitySystemComponent;

        if (!AbilitySystemComponent)
            return;

        auto GenerateNewSpec = [&]() -> FGameplayAbilitySpec
            {
                FGameplayAbilitySpecHandle Handle{ rand() };

                FGameplayAbilitySpec Spec{ -1, -1, -1, Handle, (UGameplayAbility*)CreateDefaultObject(GameplayAbilityClass), 1, -1, nullptr, 0, false, false, false };

                return Spec;
            };

        auto Spec = GenerateNewSpec();

        for (int i = 0; i < AbilitySystemComponent->ActivatableAbilities.Items.Num(); i++)
        {
            auto& CurrentSpec = AbilitySystemComponent->ActivatableAbilities.Items[i];

            if (CurrentSpec.Ability == Spec.Ability)
                return;
        }

        GiveAbility(AbilitySystemComponent, &Spec.Handle, Spec);
    }

    FGameplayAbilitySpec* FindAbilitySpecFromHandle(UAbilitySystemComponent* AbilitySystem, FGameplayAbilitySpecHandle Handle)
    {
        for (int i = 0; i < AbilitySystem->ActivatableAbilities.Items.Num(); i++)
        {
            auto Spec = AbilitySystem->ActivatableAbilities.Items[i];

            if (Spec.Handle.Handle == Handle.Handle)
            {
                return &Spec;
            }
        }

        return nullptr;
    }

    static FGameplayAbilitySpec* FindAbilitySpecFromClass(UAbilitySystemComponent* AbilityComp, UClass* Class)
    {
        for (int ConnIdx = 0; ConnIdx < AbilityComp->ActivatableAbilities.Items.Num(); ConnIdx++)
        {
            auto Spec = AbilityComp->ActivatableAbilities.Items[ConnIdx];

            if (Spec.Ability && Spec.Ability->IsA(Class))
            {
                return &Spec;
            }
        }

        return nullptr;
    }

    static void GiveGameplayAbilities(APlayerPawn_Athena_C* Pawn)
    {
        static auto AbilitySet = UObject::FindObjectFast<UFortAbilitySet>("/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_DefaultPlayer.GAS_DefaultPlayer");

        for (int i = 0; i < AbilitySet->GameplayAbilities.Num(); i++)
        {
            auto Ability = AbilitySet->GameplayAbilities[i];

            GrantGameplayAbility(Pawn, Ability);
        }

        GrantGameplayAbility((APlayerPawn_Athena_C*)Pawn, UGA_Ranged_GenericDamage_C::StaticClass());
    }

    static void GrantDelayedGameplayAbilities(int Delay, APlayerPawn_Athena_C* Pawn, UClass* GameplayAbilityClass)
    {
        std::this_thread::sleep_for(std::chrono::seconds(Delay));
        GrantGameplayAbility(Pawn, GameplayAbilityClass);
    }

    static UGameplayAbility* CreateNewInstanceOfAbility(UAbilitySystemComponent* Comp, FGameplayAbilitySpec& Spec, UGameplayAbility* Ability)
    {
        AActor* Owner = Comp->GetOwner();

        UGameplayAbility* AbilityInstance = (UGameplayAbility*)StaticConstructObjectInternal(Ability->StaticClass(), Owner, 0, 0, 0, 0, 0, 0, 0);

        auto ReplicatedInstances = (TArray_<UGameplayAbility*>*) & Spec.ReplicatedInstances;
        auto ReplicatedInstancedAbilities = (TArray_<UGameplayAbility*>*) & Comp->AllReplicatedInstancedAbilities;

        if (AbilityInstance)
        {
            /*ReplicatedInstances->AddSingle(AbilityInstance);*/
            ReplicatedInstancedAbilities->AddSingle(AbilityInstance);
            return AbilityInstance;
        }
        printf("Spawned Ability Object is Not Valid");
        return nullptr;
    }

    void ApplyGameplayAbility(UClass* AbilityClass, AFortPlayerPawn* Pawn)
    {
        if (!AbilityClass || !Pawn)
            return;

        auto AbilitySystemComponent = Pawn->AbilitySystemComponent;

        if (AbilitySystemComponent)
        {
            auto GenerateNewSpec = [&]() -> FGameplayAbilitySpec
                {
                    FGameplayAbilitySpecHandle Handle{ rand() };

                    FGameplayAbilitySpec Spec{ -1, -1, -1, Handle, (UGameplayAbility*)CreateDefaultObject(AbilityClass), 1, -1, nullptr, 0, false, false, false };

                    return Spec;
                };

            auto Spec = GenerateNewSpec();

            for (int i = 0; i < AbilitySystemComponent->ActivatableAbilities.Items.Num(); i++)
            {
                auto& CurrentSpec = AbilitySystemComponent->ActivatableAbilities.Items[i];

                if (CurrentSpec.Ability == Spec.Ability)
                    return;
            }

            GiveAbility(AbilitySystemComponent, &Spec.Handle, Spec);
        }
    }

    void GrantGameplayAbilities(APlayerPawn_Athena_C* InPawn)
    {
        static auto AbilitySet = UObject::FindObject<UFortAbilitySet>("FortAbilitySet GAS_DefaultPlayer.GAS_DefaultPlayer"); // <- find object

        if (AbilitySet)
        {
            std::cout << "AbilitySet trouvé avec succes slp .I." << std::endl;
        }
        for (int i = 0; i < AbilitySet->GameplayAbilities.Num(); i++) // crash ici
        {

            auto Ability = AbilitySet->GameplayAbilities[i];
            if (!Ability)
            {
                continue; // ; "avant que jte bz ta mere" - Heliato 11/05/24 03:15
            }
            GameAbility(InPawn, Ability);
        }
    }

    static void GrantGameplayAbility(APlayerPawn_Athena_C* TargetPawn, UClass* GameplayAbilityClass)
    {
        auto AbilitySystemComponent = TargetPawn->AbilitySystemComponent;

        static UGameplayEffect* DefaultGameplayEffect = UObject::FindObjectFast<UGameplayEffect>("/Game/Abilities/Player/Constructor/Perks/ContainmentUnit/GE_Constructor_ContainmentUnit_Applied.Default__GE_Constructor_ContainmentUnit_Applied_C");

        if (!DefaultGameplayEffect)
            return;

        TArray<FGameplayAbilitySpecDef> GrantedAbilities = DefaultGameplayEffect->GrantedAbilities;

        // overwrite current gameplay ability with the one we want to activate
        GrantedAbilities[0].Ability = GameplayAbilityClass;

        // give this gameplay effect an infinite duration
        DefaultGameplayEffect->DurationPolicy = EGameplayEffectDurationType::Infinite;

        static auto GameplayEffectClass = UObject::FindObjectFast<UClass>("/Game/Abilities/Player/Constructor/Perks/ContainmentUnit/GE_Constructor_ContainmentUnit_Applied.GE_Constructor_ContainmentUnit_Applied_C");

        if (!GameplayEffectClass)
            return;

        auto handle = FGameplayEffectContextHandle();

        AbilitySystemComponent->BP_ApplyGameplayEffectToTarget(GameplayEffectClass, AbilitySystemComponent, 1, handle);
    }

    FGameplayAbilitySpec* GiveAbilityToPlayerState(AFortPlayerStateAthena* PlayerState, UClass* AbilityClass, UObject* Source = nullptr, bool bActivateOnce = false)
    {
        if (!PlayerState || !AbilityClass)
            return nullptr;

        FGameplayAbilitySpec Spec{};
        specthing(&Spec, AbilityClass->DefaultObject, 1, -1, Source);
        Spec.RemoveAfterActivation = bActivateOnce;

        GiveAbility(PlayerState->AbilitySystemComponent, &Spec.Handle, Spec);

        return &Spec;
    }
}