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


DWORD WINAPI MainThread(LPVOID)
{
    Util::InitConsole(); // initialisation de la console grâce à la fonction 'InitConsole' de la classe 'Util'

    if (Init::InitializeAll()) // on regarde si tout s'initialise grace à la fonction 'InitializeALL' de la classe Init et si c'est bon ça lance le code
    {
        UFortEngine* FortEngine = Globals::GetFortEngine(); // On crée une variable "FortEngine" qui récuperer la fonction 'GetFortEngine' depuis la classe Global 
        UGameplayStatics* GameplayStatics = Globals::GetGameplayStatics();
        APlayerController* PlayerController = Globals::GetServerPlayerController();

        /*s// je créee une variable de type int qui a pour nom lagrossedaronnedeheliato qui est = a 180kilo

        int poids_grosse_daronne_heliato = 180;

        // crée une variable de type de type std::string pour nom heliatodehors

        /*std::string larabeheliatodehors = "c'est vrai heliato à faux cette arabe ^^";

        FString oui = L"cestfaux"; // L = char -> transforme le string en char car ce fdp ne comprend pas les strings (le FString)

        oui = L"tg"; // on peut mettre le Const et le & que si on change pas la variable pour rendre plus beau et utiliser moins de truc

        FName non = Globals::GetStringLibrary()->Conv_StringToName(L"FuckNigger"); // 
        HLog("non" << non.ToString()); // c'est une fonction

        UWorld* Prout = Globals::GetWorld(); 
        auto NetDriver = Prout->NetDriver;*/

        if (FortEngine && GameplayStatics && PlayerController)
        {
            UFortConsole* NewConsole = (UFortConsole*)GameplayStatics->SpawnObject(UFortConsole::StaticClass(), FortEngine->GameViewport);

            if (NewConsole)
            {
                FortEngine->GameViewport->ViewportConsole = NewConsole;
                PlayerController->SwitchLevel(L"Athena_Terrain");
            }
        }
        Hooks::InitDetour();

        HLog("GObjects->Num() :" << UObject::GObjects->Num())
        HLog("Init Success !")
    }
    else
    {
        HLog("Init Failed !")
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID res)
{
    if (reason == 1)
        CreateThread(0, 0, MainThread, mod, 0, 0);

    return TRUE;
}