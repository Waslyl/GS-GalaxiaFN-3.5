#pragma once

namespace Abilities
{

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