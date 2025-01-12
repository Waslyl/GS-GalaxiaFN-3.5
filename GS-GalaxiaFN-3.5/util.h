#pragma once

#include <consoleapi.h>
#include <cstdint>
#include <cstdio>
#include <libloaderapi.h>
#include <vector>
#include <winnt.h>

#define HLog(string) std::cout << "LOG : " << string << std::endl;


#define START_DETOUR         \
    DetourTransactionBegin(); \
    DetourUpdateThread(GetCurrentThread());

#define DetourAttach(Target, Detour) DetourAttach(&(void*&)Target, Detour);

#define DetourDetach(Target, Detour) DetourDetach(reinterpret_cast<void**>(&Target), Detour);

#define END_DETOUR DetourTransactionCommit();



int nb = 0;

void CheckPattern(uintptr_t Address, LPCSTR Message)
{
    if (!Address)
    {
        nb = nb + 1;
        MessageBoxA(0, Message, "ERROR", MB_ICONERROR);
    }
}

bool CheckAllPattern()
{
    if (nb == 0)
    {
        HLog("All patterns found!");
        return true;
    }
    if (nb == 1)
    {
        HLog("1 pattern not found!");
        return false;
    }
    if (nb > 1)
    {
        HLog(nb << "Patterns not found!")
            return false;
    }
    return false;
}



// find this class on google

class Util
{
public:
    static __forceinline VOID InitConsole()
    {
        FILE* fDummy;
        AllocConsole();
        freopen_s(&fDummy, "CONIN$", "r", stdin);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
    }

    static __forceinline uintptr_t FindPattern(const char* signature, bool bRelative = false, uint32_t offset = 0)
    {
        uintptr_t base_address = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
        static auto patternToByte = [](const char* pattern)
        {
            auto bytes = std::vector<int> {};
            const auto start = const_cast<char*>(pattern);
            const auto end = const_cast<char*>(pattern) + strlen(pattern);

            for (auto current = start; current < end; ++current)
            {
                if (*current == '?')
                {
                    ++current;
                    if (*current == '?')
                        ++current;
                    bytes.push_back(-1);
                }
                else
                {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        const auto dosHeader = (PIMAGE_DOS_HEADER)base_address;
        const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)base_address + dosHeader->e_lfanew);

        const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = patternToByte(signature);
        const auto scanBytes = reinterpret_cast<std::uint8_t*>(base_address);

        const auto s = patternBytes.size();
        const auto d = patternBytes.data();

        for (auto i = 0ul; i < sizeOfImage - s; ++i)
        {
            bool found = true;
            for (auto j = 0ul; j < s; ++j)
            {
                if (scanBytes[i + j] != d[j] && d[j] != -1)
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                uintptr_t address = reinterpret_cast<uintptr_t>(&scanBytes[i]);
                if (bRelative)
                {
                    address = ((address + offset + 4) + *(int32_t*)(address + offset));
                    return address;
                }
                return address;
            }
        }
        return NULL;
    }

    template <typename ActorType>
    static ActorType* SpawnActor(UClass* Class, FVector Location, FRotator Rotation = FRotator(), FVector Scale3D = FVector(1, 1, 1))
    {
        FQuat Quat;
        FTransform Transform;

        auto DEG_TO_RAD = 3.14159 / 180;
        auto DIVIDE_BY_2 = DEG_TO_RAD / 2;

        auto SP = sin(Rotation.Pitch * DIVIDE_BY_2);
        auto CP = cos(Rotation.Pitch * DIVIDE_BY_2);

        auto SY = sin(Rotation.Yaw * DIVIDE_BY_2);
        auto CY = cos(Rotation.Yaw * DIVIDE_BY_2);

        auto SR = sin(Rotation.Roll * DIVIDE_BY_2);
        auto CR = cos(Rotation.Roll * DIVIDE_BY_2);

        Quat.X = CR * SP * SY - SR * CP * CY;
        Quat.Y = -CR * SP * CY - SR * CP * SY;
        Quat.Z = CR * CP * SY - SR * SP * CY;
        Quat.W = CR * CP * CY + SR * SP * SY;

        Transform.Rotation = Quat;
        Transform.Scale3D = Scale3D;
        Transform.Translation = Location;

        auto Actor = Globals::GetGameplayStatics()->BeginDeferredActorSpawnFromClass(Globals::GetWorld(), Class, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, nullptr);
        Globals::GetGameplayStatics()->FinishSpawningActor(Actor, Transform);
        return (ActorType*)Actor;
    }

    static AFortPlayerPawn* SpawnPlayer(AFortPlayerControllerAthena* PlayerController, const FVector& Location, const FRotator& Rotation, bool NewPlayer = true, bool bForceCanBeDamaged = false)
    {
        APlayerPawn_Athena_C* PlayerPawn = Util::SpawnActor<APlayerPawn_Athena_C>(APlayerPawn_Athena_C::StaticClass(), Location, Rotation);

        if (!PlayerPawn)
        {
            return nullptr;
        }

        UWorld* World = Globals::GetWorld();

        if (!World)
        {
            return nullptr;
        }

        PlayerPawn->bCanBeDamaged = (NewPlayer == true) ? false : true;
        PlayerPawn->Owner = PlayerController;
        PlayerPawn->OnRep_Owner();

        PlayerController->Pawn = PlayerPawn;
        PlayerController->OnRep_Pawn();
        PlayerController->Possess(PlayerPawn);

        if (NewPlayer)
        {
            AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;
            PlayerState->bHasFinishedLoading = true;
            PlayerState->bHasStartedPlaying = true;
            PlayerState->OnRep_bHasStartedPlaying();
            PlayerState->OnRep_CharacterParts();

            PlayerController->OverriddenBackpackSize = 100;

            PlayerPawn->SetMaxHealth(100);
            PlayerPawn->SetHealth(100);

            UFortHealthSet* HealthSet = PlayerPawn->HealthSet;

            HealthSet->CurrentShield.Minimum = 0;
            HealthSet->CurrentShield.Maximum = 100;
            HealthSet->CurrentShield.BaseValue = 0;
            HealthSet->OnRep_CurrentShield();

            HealthSet->Shield.Minimum = 0;
            HealthSet->Shield.Maximum = 100;
            HealthSet->Shield.BaseValue = 100;
            HealthSet->OnRep_Shield();

            PlayerController->bIsDisconnecting = false;
            PlayerController->bHasClientFinishedLoading = true;
            PlayerController->bHasServerFinishedLoading = true;
            PlayerController->bHasInitiallySpawned = true;

            Abilities::GrantGameplayAbilities(PlayerPawn);

            static auto FortRegisteredPlayerInfo = ((UFortGameInstance*)Globals::GetWorld()->OwningGameInstance)->RegisteredPlayers[0]; // UObject::FindObject<UFortRegisteredPlayerInfo>("FortRegisteredPlayerInfo Transient.FortEngine_0_1.FortGameInstance_0_1.FortRegisteredPlayerInfo_0_1");

            if (FortRegisteredPlayerInfo)
            {
                auto Hero = FortRegisteredPlayerInfo->AthenaMenuHeroDef;

                if (Hero)
                {
                    UFortHeroType* HeroType = Hero->GetHeroTypeBP(); // UObject::FindObject<UFortHeroType>("FortHeroType HID_Outlander_015_F_V1_SR_T04.HID_Outlander_015_F_V1_SR_T04");
                    PlayerState->HeroType = HeroType;
                    PlayerState->OnRep_HeroType();

                    static auto Head = UObject::FindObject<UCustomCharacterPart>("CustomCharacterPart F_Med_Head1.F_Med_Head1");
                    static auto Body = UObject::FindObject<UCustomCharacterPart>("CustomCharacterPart F_Med_Soldier_01.F_Med_Soldier_01");

                    PlayerState->CharacterParts[(uint8_t)EFortCustomPartType::Head] = Head;
                    PlayerState->CharacterParts[(uint8_t)EFortCustomPartType::Body] = Body;
                    PlayerState->OnRep_CharacterParts();
                }
            }
        }

        return PlayerPawn;
    }
};