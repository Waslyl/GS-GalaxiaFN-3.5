#include <Windows.h>
#include <iostream>
#include <thread>
#include <detours.h>

#include "SDK.hpp"
#include "Globals.h"

#include "Abilities.h"
#include "Util.h"
#include <fstream>
#include "beacon.h"
#include "Hooks.h"
#include "InitFunc.h"
#include "Inventory.h"
#include "Functions.h"
#include "Loots.h"

DWORD WINAPI MainThread(LPVOID)
{
    FILE* fDummy;
    AllocConsole();
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);

    //MH_Initialize();
    SetConsoleTitleA("3.5 Galaxia");
    Util::InitConsole(); // initialisation de la console grâce à la fonction 'InitConsole' de la classe 'Util'

    if (Init::InitializeAll()) // on regarde si tout s'initialise grace à la fonction 'InitializeALL' de la classe Init et si c'est bon ça lance le code
    {
        UFortEngine* FortEngine = Globals::GetFortEngine(); // On crée une variable "FortEngine" qui récuperer la fonction 'GetFortEngine' depuis la classe Global 
        UGameplayStatics* GameplayStatics = Globals::GetGameplayStatics();
        APlayerController* PlayerController = Globals::GetServerPlayerController();

        //FString oui = L"cestfaux"; // L = char -> transforme le string en char car yuteeyy ne comprend pas les strings (le FString) (waslyl: en faite Yuteeyy t'es débile)

        //oui = L"débile de yuteeyy"; // on peut mettre le Const et le & que si on change pas la variable pour rendre plus beau et utiliser moins de truc

        //FName non = Globals::GetStringLibrary()->Conv_StringToName(L"FuckNigger"); // ca non
        //FNLog("non" << non.ToString()); // c'est une fonction

        UWorld* Prout = Globals::GetWorld(); 
        auto NetDriver = Prout->NetDriver;

        if (FortEngine && GameplayStatics && PlayerController)
        {
            UFortConsole* NewConsole = (UFortConsole*)GameplayStatics->SpawnObject(UFortConsole::StaticClass(), FortEngine->GameViewport);

            if (NewConsole)
            {
                FortEngine->GameViewport->ViewportConsole = NewConsole;
                PlayerController->SwitchLevel(L"Athena_Terrain");
                //PlayerController->SwitchLevel(L"Athena_Faceoff"); // rappelle c'est map de test
            }
        }
        Hooks::InitDetour();

        FNLog("GObjects->Num() :" << UObject::GObjects->Num())
            FNLog("Init Success !")
    }
    else
    {
        FNLog("Init Failed !")
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID res)
{
    if (reason == 1)
        CreateThread(0, 0, MainThread, mod, 0, 0);

    return TRUE;
}