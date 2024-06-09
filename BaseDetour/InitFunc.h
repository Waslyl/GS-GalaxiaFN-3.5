#pragma once

namespace Init
{
	
	bool InitializeAll()
	{

		// Base

		auto PatternFMemoryRealloc = Util::FindPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 48 8B F1 41 8B D8 48 8B 0D");
		CheckPattern(PatternFMemoryRealloc, "Failed to find PatternFMemoryRealloc");

		// Beacon

		auto PatternInitHost = Util::FindPattern("48 8B C4 48 81 EC ? ? ? ? 48 89 58 18 4C 8D 05 ? ? ? ? 48 8B D9");
		CheckPattern(PatternInitHost, "Failed to find PatternInitHost");

		auto PatternInitListen = Util::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 50 48 8B BC 24 ? ? ? ? 49 8B F0 48 8B 01 48 8B D9");
		CheckPattern(PatternInitListen, "Failed to find PatternInitListen");

		auto PatternSetWorld = Util::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B B1 ? ? ? ? 48 8B FA 48 8B D9 48 85 F6 74 5C");
		CheckPattern(PatternSetWorld, "Failed to find PatternSetWorld");

		auto PatternTickFlush = Util::FindPattern("4C 8B DC 55 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 89 5B 18 48 8D 05 ? ? ? ? 49 89 7B E8 48 8B F9 4D 89 63 E0 45 33 E4 44 38 25 ? ? ? ?");
		CheckPattern(PatternTickFlush, "Failed to find PatternTickFlush");

		auto PatternServerReplicateActor = Util::FindPattern("4C 8B DC 55 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B 41 58");
		CheckPattern(PatternServerReplicateActor, "Failed to find PatternServerReplicateActor");

		// Ability

		auto PatternInternalTryActivateAbility = Util::FindPattern("4C 89 4C 24 ?? 4C 89 44 24 ?? 89 54 24 10 55 53 56 57 41 54 41 57");
		CheckPattern(PatternInternalTryActivateAbility, "Failed to find PatternInternalTryActivateAbility");

		auto PatternGiveAbility = Util::FindPattern("48 89 5C 24 ? 56 57 41 56 48 83 EC 20 83 B9 ? ? ? ? ? 49 8B F0 4C 8B F2 48 8B D9");
		CheckPattern(PatternGiveAbility, "Failed to find PatternGiveAbility");

		auto PatternCreateDefaultObject = Util::FindPattern("4C 8B DC 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 48 83 B9 ? ? ? ? ? 48 8B F9");
		CheckPattern(PatternCreateDefaultObject, "Failed to find PatternCreateDefaultObject");

		// Others

		auto PatternApplyCharacterCustomization = Util::FindPattern("48 8B C4 48 89 50 10 55 57 48 8D 68 A1 48 81 EC ? ? ? ? 80 B9 ? ? ? ? ? 48 8B F9 0F 85 ? ? ? ?");
		CheckPattern(PatternApplyCharacterCustomization, "Failed to find PatternApplyCharacterCustomization");

		auto PatternInternalGetNetMode = Util::FindPattern("48 83 79 ? ? 74 06 B8 ? ? ? ? C3 48 8B 81 ? ? ? ? 48 85 C0 74 08");
		CheckPattern(PatternInternalGetNetMode, "Failed to find PatternInternalGetNetMode");

		auto PatternKickPlayer = Util::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 49 8B F0 48 8B DA 48 85 D2");
		CheckPattern(PatternKickPlayer, "Failed to find PatternKickPlayer");


		if (CheckAllPattern())
		{
			
			("All Patterns found !");

			//Declypte func here//

			ToHooks::ProcessEvent = decltype(ToHooks::ProcessEvent)(InSDKUtils::GetImageBase() + Offsets::ProcessEvent);
			Beacon::SetWorld = decltype(Beacon::SetWorld)(PatternSetWorld);
			Beacon::InitHost = decltype(Beacon::InitHost)(PatternInitHost);
			Beacon::InitListen = decltype(Beacon::InitListen)(PatternInitListen);
			Beacon::TickFlush = decltype(Beacon::TickFlush)(PatternTickFlush);
			Beacon::ServerReplicateActors = decltype(Beacon::ServerReplicateActors)(PatternServerReplicateActor);
			ToHooks::KickPlayer = decltype(ToHooks::KickPlayer)(PatternKickPlayer);
			Abilities::GiveAbility = decltype(Abilities::GiveAbility)(PatternGiveAbility);
			Abilities::CreateDefaultObject = decltype(Abilities::CreateDefaultObject)(PatternCreateDefaultObject);
			ToHooks::InternalTryActivateAbility = decltype(ToHooks::InternalTryActivateAbility)(PatternInternalTryActivateAbility);




			return true;
		}
		return false;
	}
}
