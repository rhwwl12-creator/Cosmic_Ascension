// Cosmic Ascension - automation tests of the ruler's research focus (MVP item 7): the world functions CosmicResearch and the
// command UPlayerRoleSubsystem::SetResearchFocus. Design: docs/60_DECISIONS/headers/A7_research_focus.md section 2 and 6.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include <limits>

#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "Events/CosmicEventSubsystem.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicResearch.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestHelpers.h"
#include "Tests/CosmicTestProbe.h"

namespace CosmicResearchFocusTests
{
	const FName Physics(TEXT("Physics"));
	const FName Society(TEXT("Society"));
	const FName Engineering(TEXT("Engineering"));
	const FName Transcendence(TEXT("Transcendence"));

	TMap<FName, float> MapOf(float P, float S, float E, float T)
	{
		TMap<FName, float> Map;
		Map.Add(Physics, P);
		Map.Add(Society, S);
		Map.Add(Engineering, E);
		Map.Add(Transcendence, T);
		return Map;
	}

	// The map Focus must write: the field the share, every other key (1 - share) / (keys - 1), in that float order.
	TMap<FName, float> FocusedMap(FName Field, float Share)
	{
		const float Rest = (1.f - Share) / 3.f;
		return MapOf(Field == Physics ? Share : Rest, Field == Society ? Share : Rest, Field == Engineering ? Share : Rest, Field == Transcendence ? Share : Rest);
	}

	int32 RuledIndexOf(const UGalaxySimulationSubsystem& Sim)
	{
		return Sim.GetStateIndex().FindCiv(Sim.GetState(), Sim.GetState().Player.RuledCivID);
	}
}

// The world functions alone (section 2.1): the shares Focus writes and refuses, Even, and the three shapes.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimResearchFocusTest, "Cosmic_Ascension.Sim.ResearchFocus", CosmicTests::Flags)
bool FCosmicSimResearchFocusTest::RunTest(const FString& Parameters)
{
	using namespace CosmicResearchFocusTests;
	const float NaN = std::numeric_limits<float>::quiet_NaN();

	// (1) Focus and Even.
	{
		TMap<FName, float> Map = MapOf(0.25f, 0.25f, 0.25f, 0.25f);
		TestTrue(TEXT("Focus takes a field and a share"), CosmicResearch::Focus(Map, Physics, 0.55f));
		TestTrue(TEXT("and writes the share and (1 - share) / 3 to the others, to the bit"), Map.OrderIndependentCompareEqual(FocusedMap(Physics, 0.55f)));
		float Sum = 0.f;
		for (const TPair<FName, float>& Each : Map)
		{
			Sum += Each.Value;
		}
		TestTrue(TEXT("and the shares still sum to 1"), FMath::IsNearlyEqual(Sum, 1.f, 1.e-5f));
		const TMap<FName, float> Before = Map;
		TestFalse(TEXT("Focus refuses a key that is not in the map"), CosmicResearch::Focus(Map, FName(TEXT("Alchemy")), 0.5f));
		TestFalse(TEXT("and a share of 0"), CosmicResearch::Focus(Map, Society, 0.f));
		TestFalse(TEXT("and a share above 1"), CosmicResearch::Focus(Map, Society, 1.01f));
		TestFalse(TEXT("and a share that is not a number"), CosmicResearch::Focus(Map, Society, NaN));
		TestTrue(TEXT("(a refused focus leaves the map)"), Map.OrderIndependentCompareEqual(Before));
		TMap<FName, float> One;
		One.Add(Physics, 1.f);
		TestFalse(TEXT("Focus refuses a map of one key"), CosmicResearch::Focus(One, Physics, 0.5f));
		TestEqual(TEXT("(and leaves it)"), One.FindRef(Physics), 1.f);
		TestTrue(TEXT("A share of 1 is a share"), CosmicResearch::Focus(Map, Engineering, 1.f) && Map.OrderIndependentCompareEqual(MapOf(0.f, 0.f, 1.f, 0.f)));

		TestTrue(TEXT("Even writes 1/4 to every key"), CosmicResearch::Even(Map) && Map.OrderIndependentCompareEqual(MapOf(0.25f, 0.25f, 0.25f, 0.25f)));
		TMap<FName, float> Empty;
		TestFalse(TEXT("Even refuses an empty map"), CosmicResearch::Even(Empty));
	}

	// (1) The shapes.
	{
		FName Field = Physics;
		TestEqual(TEXT("The generation's map is even"), CosmicResearch::ShapeOf(MapOf(0.25f, 0.25f, 0.25f, 0.25f), &Field), ECosmicFocusShape::Even);
		TestEqual(TEXT("(with no field)"), Field, NAME_None);
		TestEqual(TEXT("A map leaning to physics is focused"), CosmicResearch::ShapeOf(FocusedMap(Physics, 0.55f), &Field), ECosmicFocusShape::Focused);
		TestEqual(TEXT("on physics"), Field, Physics);
		TestEqual(TEXT("The petition's society 0.55 is focused"), CosmicResearch::ShapeOf(FocusedMap(Society, 0.55f), &Field), ECosmicFocusShape::Focused);
		TestEqual(TEXT("on society"), Field, Society);
		TestEqual(TEXT("One key below three equal keys is mixed"), CosmicResearch::ShapeOf(MapOf(0.3f, 0.3f, 0.3f, 0.1f), &Field), ECosmicFocusShape::Mixed);
		TestEqual(TEXT("(with no field)"), Field, NAME_None);
		TestEqual(TEXT("Two keys at the top are mixed"), CosmicResearch::ShapeOf(MapOf(0.4f, 0.4f, 0.1f, 0.1f)), ECosmicFocusShape::Mixed);
		TestEqual(TEXT("A top key over unequal others is mixed"), CosmicResearch::ShapeOf(MapOf(0.5f, 0.3f, 0.1f, 0.1f)), ECosmicFocusShape::Mixed);
		TestEqual(TEXT("A share that is not a number is mixed"), CosmicResearch::ShapeOf(MapOf(0.55f, NaN, 0.15f, 0.15f)), ECosmicFocusShape::Mixed);
		TMap<FName, float> TwoWithNaN;
		TwoWithNaN.Add(Physics, 0.6f);
		TwoWithNaN.Add(Society, NaN);
		TestEqual(TEXT("even when it is the only other key"), CosmicResearch::ShapeOf(TwoWithNaN), ECosmicFocusShape::Mixed);
		TestEqual(TEXT("An empty map is mixed"), CosmicResearch::ShapeOf(TMap<FName, float>()), ECosmicFocusShape::Mixed);
		TMap<FName, float> One;
		One.Add(Physics, 1.f);
		TestEqual(TEXT("A map of one key is mixed"), CosmicResearch::ShapeOf(One), ECosmicFocusShape::Mixed);
		TestEqual(TEXT("An old test board's all-zero map is even"), CosmicResearch::ShapeOf(MapOf(0.f, 0.f, 0.f, 0.f)), ECosmicFocusShape::Even);
		TestEqual(TEXT("and its society-1 map is focused"), CosmicResearch::ShapeOf(MapOf(0.f, 1.f, 0.f, 0.f), &Field), ECosmicFocusShape::Focused);
		TestEqual(TEXT("on society"), Field, Society);
	}
	return true;
}

// The command (section 2.3, 2.4): the shares it writes, its refusals, what it leaves alone, the next tick's research, the
// determinism of an input, a knob that cannot focus, and the save.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimResearchFocusCommandTest, "Cosmic_Ascension.Sim.ResearchFocusCommand", CosmicTests::Flags)
bool FCosmicSimResearchFocusCommandTest::RunTest(const FString& Parameters)
{
	using namespace CosmicResearchFocusTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UGameInstance* GameInstance = Sim->GetGameInstance();
	UPlayerRoleSubsystem* Role = GameInstance->GetSubsystem<UPlayerRoleSubsystem>();
	UCosmicSaveSubsystem* Save = GameInstance->GetSubsystem<UCosmicSaveSubsystem>();
	UCosmicLogSubsystem* Log = GameInstance->GetSubsystem<UCosmicLogSubsystem>();
	if (!TestNotNull(TEXT("(the role subsystem)"), Role) || !TestNotNull(TEXT("(the save subsystem)"), Save) || !TestNotNull(TEXT("(the log subsystem)"), Log))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	const FCosmicRulesDef Rules = Sim->GetRules();
	ON_SCOPE_EXIT
	{
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);
	};
	const float Share = Rules.ResearchFocusShare;
	TestEqual(TEXT("The game's knob is the opening petition's share"), Share, 0.55f);

	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	FGalaxyState& State = Sim->GetMutableState();
	const int32 Ruled = RuledIndexOf(*Sim);
	if (!TestTrue(TEXT("(a ruled people)"), State.Civs.IsValidIndex(Ruled)))
	{
		return false;
	}
	TestEqual(TEXT("A new run starts even"), CosmicResearch::ShapeOf(State.Civs[Ruled].ResearchFocus), ECosmicFocusShape::Even);

	// (3) What the command writes and refuses. (4) It changes the focus and nothing else: put back, the whole state hashes
	// as before (the sim stream, the tell stream, knowing and the log are in it).
	{
		const uint32 HashBefore = Sim->ComputeStateHash();
		const uint32 SeedBefore = Sim->GetRng().GetCurrentSeed();
		const int32 TellBefore = State.TellRngState;
		const int32 LogBefore = State.Log.Num();
		const TMap<FName, float> EvenMap = State.Civs[Ruled].ResearchFocus;
		TestFalse(TEXT("The even focus on an even people changes nothing"), Role->SetResearchFocus(NAME_None));
		TestTrue(TEXT("The command focuses a field"), Role->SetResearchFocus(Physics));
		TestTrue(TEXT("with the knob's share"), State.Civs[Ruled].ResearchFocus.OrderIndependentCompareEqual(FocusedMap(Physics, Share)));
		TestFalse(TEXT("The same focus again is no change"), Role->SetResearchFocus(Physics));
		TestFalse(TEXT("A key that is not a field is refused"), Role->SetResearchFocus(FName(TEXT("Alchemy"))));
		TestTrue(TEXT("(and leaves the focus)"), State.Civs[Ruled].ResearchFocus.OrderIndependentCompareEqual(FocusedMap(Physics, Share)));
		TestTrue(TEXT("The command takes transcendence too"), Role->SetResearchFocus(Transcendence) && State.Civs[Ruled].ResearchFocus.OrderIndependentCompareEqual(FocusedMap(Transcendence, Share)));
		TestTrue(TEXT("The even focus comes back"), Role->SetResearchFocus(NAME_None) && State.Civs[Ruled].ResearchFocus.OrderIndependentCompareEqual(EvenMap));
		TestEqual(TEXT("The command draws nothing from the sim stream"), Sim->GetRng().GetCurrentSeed(), SeedBefore);
		TestEqual(TEXT("nor from the tell stream"), State.TellRngState, TellBefore);
		TestEqual(TEXT("and tells no line"), State.Log.Num(), LogBefore);
		TestEqual(TEXT("Put back even, the whole state is as before"), Sim->ComputeStateHash(), HashBefore);

		const EPlayerRole RoleWas = State.Player.Role;
		State.Player.Role = EPlayerRole::Wanderer;
		TestFalse(TEXT("A wanderer gives no focus"), Role->SetResearchFocus(Physics));
		State.Player.Role = RoleWas;
		const ELifeState LifeWas = State.Calling.LifeState;
		State.Calling.LifeState = ELifeState::Interregnum;
		TestFalse(TEXT("nor a calling without a vessel"), Role->SetResearchFocus(Physics));
		State.Calling.LifeState = LifeWas;
		const FName RuledWas = State.Player.RuledCivID;
		State.Player.RuledCivID = FName(TEXT("civ_nobody"));
		TestFalse(TEXT("nor a ruler of a people that is not in the state"), Role->SetResearchFocus(Physics));
		State.Player.RuledCivID = RuledWas;
		TestTrue(TEXT("(the refusals left the focus even)"), State.Civs[Ruled].ResearchFocus.OrderIndependentCompareEqual(EvenMap));
	}

	// A handler of a Core Logic delegate gives no command.
	{
		const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
		Sim->SetBroadcastEnabled(true);
		ON_SCOPE_EXIT { Sim->SetBroadcastEnabled(bWasBroadcasting); };
		CosmicTests::FScopedProbe Probe(*Sim);
		bool bTried = false;
		bool bFocused = true;
		Probe.Probe->OnLog = [&](const FCosmicLogEntry&)
		{
			if (!bTried)
			{
				bTried = true;
				bFocused = Role->SetResearchFocus(Physics);
			}
		};
		AddExpectedError(TEXT("SetResearchFocus called from a handler"), EAutomationExpectedErrorFlags::Contains, 1);
		Log->Push(FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ"))), 0, FText::AsCultureInvariant(TEXT("a line")), FFormatNamedArguments(), NAME_None, FName(TEXT("log_test")));
		TestTrue(TEXT("The handler of the line ran"), bTried);
		TestFalse(TEXT("and focused nothing"), bFocused);
		TestEqual(TEXT("(the focus stays even)"), CosmicResearch::ShapeOf(State.Civs[Ruled].ResearchFocus), ECosmicFocusShape::Even);
	}

	// (2) The petition's effect and the command write the very same map.
	{
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = Ruled;
		FCosmicEffect Effect;
		Effect.Type = EEffectType::FocusResearch;
		Effect.Key = Engineering;
		Effect.Value = Share;
		TestTrue(TEXT("The effect focuses engineering"), UCosmicEventSubsystem::Apply(Effect, Ctx));
		const TMap<FName, float> ByEffect = State.Civs[Ruled].ResearchFocus;
		TestTrue(TEXT("(the command puts it back even)"), Role->SetResearchFocus(NAME_None));
		TestTrue(TEXT("and focuses engineering again"), Role->SetResearchFocus(Engineering));
		TestTrue(TEXT("The effect and the command write the same bits"), State.Civs[Ruled].ResearchFocus.OrderIndependentCompareEqual(ByEffect));
		Effect.Value = 0.6f;
		TestTrue(TEXT("The effect takes its own share"), UCosmicEventSubsystem::Apply(Effect, Ctx) && State.Civs[Ruled].ResearchFocus.OrderIndependentCompareEqual(FocusedMap(Engineering, 0.6f)));
		Role->SetResearchFocus(NAME_None);
	}

	// (5) The next tick's research spends by the new shares.
	{
		Sim->RunHeadless(10);
		TestTrue(TEXT("(the command focuses physics)"), Role->SetResearchFocus(Physics));
		const TMap<FName, float> Before = State.Civs[Ruled].Research;
		Sim->StepOnce();
		const float PhysicsGain = State.Civs[Ruled].Research.FindRef(Physics) - Before.FindRef(Physics);
		const float SocietyGain = State.Civs[Ruled].Research.FindRef(Society) - Before.FindRef(Society);
		TestTrue(TEXT("Physics gains in the next tick"), PhysicsGain > 0.f);
		TestTrue(TEXT("by the share against the others' (0.55 : 0.15)"), FMath::IsNearlyEqual(PhysicsGain * (1.f - Share) / 3.f, SocietyGain * Share, 1.e-7f));
	}

	// (6) An input is part of the world: the same input at the same tick gives the same hashes; no input, another world.
	{
		auto RunWith = [&](bool bFocus, uint32& OutState, uint32& OutWorld)
		{
			Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
			Sim->RunHeadless(50);
			if (bFocus)
			{
				Role->SetResearchFocus(Engineering);
			}
			Sim->RunHeadless(50);
			OutState = Sim->ComputeStateHash();
			OutWorld = Sim->ComputeWorldHash();
		};
		uint32 FirstState = 0, FirstWorld = 0, SecondState = 0, SecondWorld = 0, IdleState = 0, IdleWorld = 0;
		RunWith(true, FirstState, FirstWorld);
		RunWith(true, SecondState, SecondWorld);
		RunWith(false, IdleState, IdleWorld);
		TestEqual(TEXT("The same input gives the same state"), SecondState, FirstState);
		TestEqual(TEXT("and the same world"), SecondWorld, FirstWorld);
		TestNotEqual(TEXT("No input is another world"), IdleWorld, FirstWorld);
	}

	// (7) A knob that cannot focus: a field is refused with one error line each, the even focus still works; a good knob
	// other than the game's is the share written.
	{
		const float NaN = std::numeric_limits<float>::quiet_NaN();
		const float BadKnobs[] = { 0.25f, 0.2f, 0.f, 1.01f, NaN };
		AddExpectedError(TEXT("cannot focus a field"), EAutomationExpectedErrorFlags::Contains, UE_ARRAY_COUNT(BadKnobs));
		for (const float Bad : BadKnobs)
		{
			FCosmicRulesDef Knobbed = Rules;
			Knobbed.ResearchFocusShare = Bad;
			Sim->SetTables(CosmicTests::WithRules(Original, Knobbed));
			Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
			FGalaxyState& Board = Sim->GetMutableState();
			const int32 At = RuledIndexOf(*Sim);
			TestFalse(FString::Printf(TEXT("A knob of %f refuses a field"), Bad), Role->SetResearchFocus(Physics));
			TestFalse(TEXT("and a key that is no field, without a line of the knob"), Role->SetResearchFocus(FName(TEXT("Alchemy"))));
			TestEqual(TEXT("(and the focus stays even)"), CosmicResearch::ShapeOf(Board.Civs[At].ResearchFocus), ECosmicFocusShape::Even);
			CosmicResearch::Focus(Board.Civs[At].ResearchFocus, Society, 0.55f);
			TestTrue(TEXT("while the even focus is still taken"), Role->SetResearchFocus(NAME_None));
		}
		FCosmicRulesDef Steep = Rules;
		Steep.ResearchFocusShare = 0.7f;
		Sim->SetTables(CosmicTests::WithRules(Original, Steep));
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		TestTrue(TEXT("A knob of 0.7 focuses a field"), Role->SetResearchFocus(Physics));
		TestTrue(TEXT("with 0.7 and 0.1 to the others"), Sim->GetState().Civs[RuledIndexOf(*Sim)].ResearchFocus.OrderIndependentCompareEqual(FocusedMap(Physics, 0.7f)));
		Sim->SetTables(Original);
	}

	// (8) The focus goes through the save as it is: the exact shares, the shape, both hashes.
	{
		Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		const FGuid ID = Sim->GetState().GalaxyID;
		ON_SCOPE_EXIT { Save->DeleteSave(ID); };
		Save->DeleteSave(ID);
		Sim->RunHeadless(20);
		TestTrue(TEXT("(the command focuses physics)"), Role->SetResearchFocus(Physics));
		const TMap<FName, float> Written = Sim->GetState().Civs[RuledIndexOf(*Sim)].ResearchFocus;
		const uint32 StateHash = Sim->ComputeStateHash();
		const uint32 WorldHash = Sim->ComputeWorldHash();
		TestTrue(TEXT("(the run is written)"), Save->SaveCurrentGalaxy());
		Sim->NewGalaxy(7, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		TestTrue(TEXT("(and read back)"), Save->LoadGalaxy(ID));
		const TMap<FName, float>& Read = Sim->GetState().Civs[RuledIndexOf(*Sim)].ResearchFocus;
		TestTrue(TEXT("The focus is read back to the bit"), Read.OrderIndependentCompareEqual(Written));
		FName Field = NAME_None;
		TestEqual(TEXT("and is still a focus"), CosmicResearch::ShapeOf(Read, &Field), ECosmicFocusShape::Focused);
		TestEqual(TEXT("on physics"), Field, Physics);
		TestEqual(TEXT("The state hashes as before the save"), Sim->ComputeStateHash(), StateHash);
		TestEqual(TEXT("and the world too"), Sim->ComputeWorldHash(), WorldHash);
		TestFalse(TEXT("The same focus after the load is no change"), Role->SetResearchFocus(Physics));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
