# MVP ④ 구현 메모 — 헤더 승인 뒤 A-4b의 메모

헤더 `docs/60_DECISIONS/headers/A4_provisional_end.md`에서 옮긴 것이다. 승인할 것은 헤더의 선언·ST_CA 키와 글·결정이다. 이 파일은 구현자가 따를 꼴과 본문 스케치다. 헤더와 어긋나면 헤더가 이긴다. 결정 15(이름)를 대안으로 정하면 `Provisional` → `Temporary`로 바꿔 읽는다.

## 0. 코드 사실 — 자리와 행 번호 (HEAD `2269ff9`, 2026-10-23에 파일을 열어 확인)

| 사실 | 자리 |
|---|---|
| 끝을 판정하는 코드가 없다. `RunHeadless`의 엔딩 검사는 자리 주석뿐이다 | `Sim/GalaxySimulationSubsystem.cpp:1094`, `.h:170-173` |
| `SecondsPerTick` 1.0, `MaxTicksPerFrame` 10. 프레임 고리는 걸음마다 `TimeScale > 0`을 다시 본다. `SetTimeScale(0)`은 `Accumulator`를 0으로 둔다 | `.h:503, 506`, `.cpp:571, 1061-1071` |
| `SimulateTick`: 열 단계 → `ResolvePendingRoleEffect`(폐위) → `SyncRngState` → `OnSimTick` | `.cpp:1351-1395` |
| `StepOnce`·`RunHeadless`는 `BlueprintCallable`이고 `Exec`가 아니다 | `.h:166-175` |
| `StartPlay`: `GalaxyID`가 없을 때만 `NewGalaxy(Seed, 0, 0)`. 시드는 `-Seed=`, 없으면 `FDateTime::UtcNow().GetTicks() & 0x7fffffff`. `-Speed=`는 `Sim->SetTimeScale` 직접. 그 뒤에 `Super::StartPlay()`가 온다 | `World/CosmicGameMode.cpp:89-108` |
| HUD `BeginPlay`: 피드·상단 바·패널 둘·카드(z 5, 만든 뒤 `FollowPetition(false)`)·연출(z 10) | `CosmicGameMode.cpp:179-266` |
| 컨트롤러 `BeginPlay`: `OnSimTick`·`OnGalaxyGenerated`·`OnLogEntry`를 묶고 `FollowOwnHazards()` | `CosmicGameMode.cpp:501-524` |
| `HandleSimTick` = `FollowSpeedGate`·`FollowOwnHazards`·`FollowPetition(true)`. `HandleGalaxyGenerated` = `FollowSpeedGate` → 까닭·위기 기억 지우기 → `FollowOwnHazards`·`FollowPetition(false)` | `CosmicGameMode.cpp:544-559` |
| `ApplySpeed`: 지금 배속 > 0이면 `SpeedBeforePause`를 적고, `CosmicUI::OpenSpeed`로 `SetTimeScale`, `AutoPauseReason = Reason`, HUD를 다시 그린다 | `CosmicGameMode.cpp:1335-1357` |
| 컨트롤러 `AnswerPetition`·`ClosePetitionCard`·`StartProject` | `CosmicGameMode.cpp:1277-1328`(`ClosePetitionCard` 1297) |
| `HandleWheel`은 카메라를 줌한다 | `CosmicGameMode.cpp:1247-1254` |
| 컨트롤러 없는 상단 바는 `Sim->SetTimeScale`을 직접 부른다(테스트 길) | `UI/CosmicTopBarWidget.cpp:245-249` |
| 지도: `OnGalaxyGenerated` → `bRebuildPending` → 다음 `Tick`의 `Rebuild` → `OnRebuilt` → 컨트롤러 `HandleMapRebuilt`(선택·은하 선택 해제, 보기 영적, `FrameRegion` + `SnapToTargets`) | `World/CosmicGalaxyMapActor.cpp:288-316`, `CosmicGameMode.cpp:946-996` |
| `EvaluateFeats`: 그릇이 Embodied Ruler이고 그 백성이 `bIsPlayerRuled`이면 `JudgePeople(.., true)`. `Ascend`의 `bPlayer`가 `Lives.Last().PeakTier`를 올린다. 승천 줄(S2) 뒤에 `OnTierAscended`가 방송된다 | `Ascension/AscensionSubsystem.cpp:44-53, 146-156, 196-204` |
| `ResolvePendingRoleEffect`가 열째 단계에서 `DeposeRuler`를 부른다 | `Sim/GalaxySimulationSubsystem.cpp:2293-2307` |
| `Install`: `Life.PeakTier = Calling.PersonalTier`(계승의 새 생애). 다스리면 `RuledCivIDs`·`ReignEndReasons`(빈 칸)·`ReignTicks`에 하나씩. `log_first_life`는 `Log.Calling` S2, 초점 백성 없음 | `Player/PlayerRoleSubsystem.cpp:107, 134-143, 172` |
| `DeposeRuler`: `log_fallen_monarch`(`Log.Calling`, S2, 초점 = 다스리던 백성) → `BecomeWanderer(.., Reign.Deposed)`. 마지막 `ReignEndReasons`가 채워진다 | `PlayerRoleSubsystem.cpp:176-202, 273-275` |
| 청원 결과 줄은 `Log.Civ` S1, 초점 백성 = 다스리는 백성, `SourceRow` = 카드 행 | `PlayerRoleSubsystem.cpp:420-426` |
| `PushRow`는 `SourceRow`에 행 이름을 적는다. 첫 접촉은 `log_first_contact` S1, 초점 = 상대 백성 | `Log/CosmicLogSubsystem.cpp:100`, `Sim/...cpp:1137` |
| `Log.Calling`·`Log.Claim`·`Log.Record`의 static 태그는 이미 있다. 이관은 `Log`가 2,000을 넘을 때 머리에서 S1·S2·Claim·Record를 `Chronicle`로 옮긴다 | `CosmicLogSubsystem.cpp:107-109, 181-182, 202-210` |
| `Log.Claim`·`Log.Record`를 내는 데이터 행이 없다(`DT_*.json`에 그 범주가 없다). `ClaimTemplates`는 빈 배열이다 | `Tools/data/DT_Transcendents.json:25` |
| `PersonNameBank`를 읽는 곳 | `PlayerRoleSubsystem.cpp:115`, `Events/CosmicEventSubsystem.cpp:284`, `Events/CosmicPetitions.cpp:78` |
| 승천 연출: `Show`가 움직이던 시뮬을 `Controller->SetSpeed(0)`로 멈추고 `PausedSpeed`를 쥔다. `Close`는 `PausedSpeed > 0`이고 시뮬이 멈췄으면 `SetSpeed(PausedSpeed)`. 셋째 칸·영적 보기면 `StartReveal`(`RevealSeconds` 3) | `UI/CosmicAscensionOverlayWidget.cpp:214-288`, `.h:23` |
| 자동 정지 켬이면 승천 줄(S2)의 `OnLogEntry`가 `OnTierAscended`보다 먼저 멈춘다. `RequestAutoPause`는 배속 > 0일 때만 멈춘다 | `docs/20` §6.3 '자동 일시정지', `World/CosmicGameMode.cpp:610-618` |
| `ECosmicAutoPause` = None·MythicLine·OwnHazard·Card(평범한 enum class). `WhyKeyOf`를 상단 바가 읽는다 | `World/CosmicAutoPause.h:16-25`, `World/CosmicAutoPause.cpp:50`, `UI/CosmicTopBarWidget.cpp:446` |
| 상단 바: 배속 단추의 `SetIsEnabled`, 시계 툴팁은 `RebuildWidget`에서 한 번 `Time.Tip` + `Time.TipPress` | `UI/CosmicTopBarWidget.cpp:138, 428` |
| 성계 패널의 사유 길(사유가 있으면 회색, 툴팁은 바뀔 때만) | `UI/CosmicSystemPanelWidget.cpp:688-700` |
| 커맨드렛: 정책 입력은 `RunHeadless(1)` 뒤. `DeposedTick`은 `RoleBefore == Ruler`이고 지금 아니면 적힌다. `RuledT3Tick`은 `bIsPlayerRuled` 백성의 Tier로 잰다. `expansion` 합이 틀리면 `Result = 1` | `Tests/CosmicBalanceCommandlet.cpp:459-488, 547-560, 618-627` |
| 커맨드렛 헤더 주석의 'six log lines'와 'as long as there are no endings' | `Tests/CosmicBalanceCommandlet.h:19, 36-38` |
| 테스트 도우미 `AcquireSim`(배속 0·TellAll). 테스트는 컨트롤러를 `GetWorld()->GetFirstPlayerController()`로 얻는다 | `Tests/CosmicTestHelpers.h:69-83`, 예 `Tests/CosmicAutoPauseTests.cpp:74` |
| `Sim.PhaseTwoExit`(방송 `StepOnce` 20,000틱, `ON_SCOPE_EXIT`의 `NewGalaxy`) | `Tests/CosmicSimTests.cpp:2134-2258` |
| `Sim.PlayerAscendsToT3`(방송 `StepOnce`로 T3, `ON_SCOPE_EXIT`의 `NewGalaxy`) | `Tests/CosmicAscensionTests.cpp:1579-1647` |
| `RunHeadless(20000)`의 상태를 남기는 테스트의 예(`Log.NoConsecutiveRepeat`). `CosmicAscensionTests.cpp:1568`은 1566행의 `ON_SCOPE_EXIT` `NewGalaxy`로 되돌리므로 예가 아니다 | `CosmicLogTests.cpp:134` |
| `UI.MouseOnlyAscent`(꼬리 753-770 — `PressViewButton`으로 실제 보기로 돌린 뒤 `Overlay->Close()`, ×25 단언) | `Tests/CosmicPanelTests.cpp:632-771` |
| `UI.ThirdRungSpeed`, `AscendNow`(방송만), `StepBroadcast` | `Tests/CosmicThirdRungTests.cpp:73-91, 147` |
| `UI.AscensionOverlay`·`UI.SpiritualSight`·`UI.AutoPause` | `Tests/CosmicAscensionOverlayTests.cpp:39`, `CosmicSpiritualSightTests.cpp:227`, `CosmicAutoPauseTests.cpp:58`(까닭 고리 156) |
| 검사기: `check_string_table`은 꼴만 본다. 열의 `forbidden` 확인, 열의 `no_digits` | `Tools/cosmic_data_schema.py:319-320, 646-647, 811` |
| 린트 기준 `tag_literals` 38·`unused_knobs` 18. `CosmicTags.h`는 이미 셈에서 빠진다 | `Tools/check_layers.py:24, 67` |
| ST_CA: `Sight.Claimed` = `{Transcendent}\|hpp(이,가) 주장하는 곳`, `List.Pair` = `{0} · {1}`, `Panel.Tier` = `칸`, `Time.YearsLeft`·`Panel.CyclesLeft`, `AutoPause.Why.*`, `Start.Block.*`·`Start.Suffix.*` | `Tools/data/ST_CA.csv:14, 21, 34, 50, 54-61, 78, 110-112` |
| `log_fallen_monarch`의 둘째 변형이 `{Civ}\|hpp(은,는)` | `Tools/data/DT_LogTemplates.json:62` |
| 연대기가 처음 비지 않는 표본: 16,800틱(1항목) | `Saved/Balance/a3_20k.csv` |
| 엔진: `UScrollBox::ScrollToStart`, `UPanelWidget::AddChild`·`ClearChildren`, `UWorld::GetRealTimeSeconds` | `UMG/Public/Components/ScrollBox.h:313`, `PanelWidget.h:59, 122`, `Engine/Classes/Engine/World.h:2861` |

## 1. 컨트롤러 (`World/CosmicGameMode.h/.cpp`)

**include**: `World/CosmicGameMode.h`에 `Sim/CosmicEndings.h`를 더한다(World → Sim). `UCosmicChronicleWidget`은 앞선 선언으로 둔다.

**private 멤버**
```cpp
	// The provisional end (MVP item 4): presentation state, never saved, outside both hashes.
	ECosmicProvisionalEnd RunEnd = ECosmicProvisionalEnd::None;
	bool bProvisionalEnd = true;
	// Real seconds left before the chronicle opens; unset while nothing waits.
	TOptional<float> ChronicleWait;
	// The CosmicEnd line's two numbers: UWorld::GetRealTimeSeconds at this controller's BeginPlay or at the last new galaxy or
	// load (HandleGalaxyGenerated), and the real seconds the simulation stood still since then (any reason - a card, a
	// presentation, a pause; PlayerTick counts).
	double RunBeganRealSeconds = 0.0;
	double StoppedRealSeconds = 0.0;
```

**본문 스케치**
```cpp
void ACosmicPlayerController::HandleSimTick(int64 Tick)
{
	FollowSpeedGate();
	// The end before the card and the hazards: once the run is over, its own tick asks for nothing.
	FollowEnd(true);
	if (IsRunEnded())
	{
		return;
	}
	FollowOwnHazards();
	FollowPetition(true);
}

void ACosmicPlayerController::HandleGalaxyGenerated(int32 Seed)
{
	FollowSpeedGate();
	// Another game: the end, what stopped the old one and the hazards it had say nothing of this one.
	ClearRunEnd();
	AutoPauseReason = ECosmicAutoPause::None;
	OwnHazardsSeen.Reset();
	RunBeganRealSeconds = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;
	StoppedRealSeconds = 0.0;
	FollowEnd(false);
	if (IsRunEnded())
	{
		return;
	}
	FollowOwnHazards();
	FollowPetition(false);
}

void ACosmicPlayerController::FollowEnd(bool bArrived)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!bProvisionalEnd || !Sim)
	{
		return;
	}
	if (!IsRunEnded())
	{
		const ECosmicProvisionalEnd Cause = CosmicEndings::ProvisionalEndOf(Sim->GetState(), Sim->GetRules());
		if (Cause == ECosmicProvisionalEnd::None)
		{
			return;
		}
		RunEnd = Cause;
		ApplySpeed(0.f, ECosmicAutoPause::RunEnd);
		if (UCosmicPetitionCardWidget* Card = GetHUD<ACosmicHUD>() ? GetHUD<ACosmicHUD>()->GetPetitionCard() : nullptr)
		{
			Card->ShowCard(NAME_None, NAME_None);   // the card folds; the waiting row stays in Knowing
		}
		const double Now = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;
		UE_LOG(LogCosmic, Display, TEXT("CosmicEnd: %s at tick %lld; %.0f s since the run began, %.0f s of them stopped"),
			Cause == ECosmicProvisionalEnd::Rung ? TEXT("Rung") : TEXT("Clock"), Sim->GetState().Tick, Now - RunBeganRealSeconds, StoppedRealSeconds);
		if (bArrived)
		{
			ChronicleWait = CosmicChronicleScreen::HoldSeconds;
			return;
		}
	}
	if (!bArrived)
	{
		ChronicleWait.Reset();
		UCosmicChronicleWidget* Page = GetHUD<ACosmicHUD>() ? GetHUD<ACosmicHUD>()->GetChronicle() : nullptr;
		if (Page && !Page->IsOpen())
		{
			Page->Open(RunEnd, true);
		}
	}
}

void ACosmicPlayerController::AdvanceRunEnd(float DeltaSeconds)
{
	if (!ChronicleWait.IsSet())
	{
		return;
	}
	const UCosmicAscensionOverlayWidget* Overlay = GetHUD<ACosmicHUD>() ? GetHUD<ACosmicHUD>()->GetAscensionOverlay() : nullptr;
	if (Overlay && (Overlay->IsShowing() || Overlay->IsRevealing()))
	{
		return;   // the third rung's presentation and its reveal first
	}
	ChronicleWait = ChronicleWait.GetValue() - FMath::Max(DeltaSeconds, 0.f);
	if (ChronicleWait.GetValue() <= 0.f)
	{
		ChronicleWait.Reset();
		if (UCosmicChronicleWidget* Page = GetHUD<ACosmicHUD>() ? GetHUD<ACosmicHUD>()->GetChronicle() : nullptr)
		{
			Page->Open(RunEnd, false);
		}
	}
}

void ACosmicPlayerController::ClearRunEnd()
{
	RunEnd = ECosmicProvisionalEnd::None;
	ChronicleWait.Reset();
	if (UCosmicChronicleWidget* Page = GetHUD<ACosmicHUD>() ? GetHUD<ACosmicHUD>()->GetChronicle() : nullptr)
	{
		Page->Close();
	}
	// The speed and the switch stay. The RunEnd reason goes: AcquireSim stops the simulation with SetTimeScale, not through
	// ApplySpeed, so the top bar would otherwise keep saying the run is over.
	if (AutoPauseReason == ECosmicAutoPause::RunEnd)
	{
		AutoPauseReason = ECosmicAutoPause::None;
	}
}

// In ApplySpeed, before the time scale is set:
	// The run is over (MVP item 4): every path - the bar, the keys, the presentation's and the card's give-back, the gate's
	// lowering - stops here.
	if (IsRunEnded())
	{
		Speed = 0.f;
		Reason = ECosmicAutoPause::RunEnd;
	}

void ACosmicPlayerController::StartNewRun()
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	if (!Sim || Sim->IsBroadcasting())
	{
		return;
	}
	// The seed is an input (as in StartPlay); -Seed= named the first galaxy only.
	Sim->NewGalaxy(CosmicControlImpl::ClockSeed(), 0, 0);   // HandleGalaxyGenerated clears the end
	SetSpeed(1.f);
}
```

- `AnswerPetition`·`StartProject`의 맨 앞: `if (IsRunEnded()) { return FCosmicPetitionAnswer(); }` / `return false;`. `FollowPetition`의 맨 앞: `if (IsRunEnded()) { return; }`.
- `LookAtMap`은 `Page->Close()`만 부른다(래치·배속 그대로). `ReopenChronicle`은 `IsRunEnded()`일 때만 `Page->Open(RunEnd, true)`.
- `IsRunEndedFor`는 `TimeUnitFor`(`CosmicGameMode.h:268`)와 같은 꼴이다: 위젯의 `GetOwningPlayer()`를 컨트롤러로 바꿔 `IsRunEnded()`, 없으면 false.
- `BeginPlay`: 핸들러를 묶은 뒤 `RunBeganRealSeconds`를 잡고 `FollowEnd(false)`를 부른다.
- `PlayerTick`: `AdvanceRunEnd(DeltaTime)`. 끝 전에는 시뮬 배속이 0 이하일 때 `StoppedRealSeconds += DeltaTime`(엔진은 멈추지 않고 시간 늘림도 없어 `DeltaTime`이 실제 초다).
- `PostInitializeComponents`: `int32 GivenEnd = 1; if (FParse::Value(FCommandLine::Get(), TEXT("ProvisionalEnd="), GivenEnd)) { bProvisionalEnd = GivenEnd != 0; }`(`-AutoPause=`와 같은 꼴).
- `SetProvisionalEndEnabled(false)`는 `ClearRunEnd()` 뒤에 플래그를 내린다. `true`는 플래그만 올린다(다음 경계나 새 판에서 다시 본다).
- `HandleWheel`: `if (Page && Page->IsOpen()) { return; }`를 맨 앞에 둔다.
- `CosmicControlImpl::ClockSeed()`: `static_cast<int32>(FDateTime::UtcNow().GetTicks() & 0x7fffffff)`. `StartPlay`도 이것을 부르게 고친다(헤더 변경 없음).
- `CosmicPreviewChronicle`: `CosmicPreviewRung`과 같은 타이머 꼴. 시간이 지나면 `Page->Open(ECosmicProvisionalEnd::None, true)`.

## 2. HUD (`ACosmicHUD::BeginPlay`)

```cpp
	// Over the card (5) and the presentation (10): the chronicle, collapsed until the run ends.
	Chronicle = CreateWidget<UCosmicChronicleWidget>(Controller);
	if (Chronicle)
	{
		Chronicle->AddToViewport(20);
		Chronicle->SetVisibility(ESlateVisibility::Collapsed);
	}
	// A galaxy that started ended (a load before StartPlay - item 5): the second of the two BeginPlay calls opens the page.
	if (ACosmicPlayerController* Cosmic = Cast<ACosmicPlayerController>(Controller))
	{
		Cosmic->FollowEnd(false);
	}
```

- 이 블록은 `BeginPlay`의 맨 끝, 연출을 만든 뒤에 둔다. 카드의 `FollowPetition(false)`가 앞에서 카드를 보였더라도, 같은 프레임에 `FollowEnd`가 접는다.
- `GetChronicle()`과 `UPROPERTY(Transient) TObjectPtr<UCosmicChronicleWidget> Chronicle`을 둔다(헤더 §5.2).

## 3. 연대기 위젯 (`UI/CosmicChronicleWidget.h/.cpp`)

**상수와 include**
```cpp
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Misc/Optional.h"
#include "Sim/CosmicEndings.h"
#include "CosmicChronicleWidget.generated.h"

class UCosmicActionButton;
class UScrollBox;
class UTextBlock;
class UWidget;

namespace CosmicChronicleScreen
{
	// Real seconds the stopped map stays in view before the veil (after the third rung's reveal, or at the end tick).
	inline constexpr float HoldSeconds = 1.5f;
	// The veil darkens over this many real seconds.
	inline constexpr float VeilSeconds = 1.f;
	// The lines show one after another LineSeconds apart, the whole page within RevealCapSeconds.
	inline constexpr float LineSeconds = 0.25f;
	inline constexpr float RevealCapSeconds = 6.f;
	// The opacity of another people's lines, against 1 for the life's own (inside the four colours of section 8.2).
	inline constexpr float WorldLineOpacity = 0.7f;
	// A second press of 'new run' within these real seconds starts it.
	inline constexpr float ConfirmSeconds = 3.f;
	// The top bar's quiet notice stands over the last EndSoonTicks ticks before the clock end (one minute at x10).
	inline constexpr int64 EndSoonTicks = 600;
}
```

**protected·private**
```cpp
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	// A press on the page stops here (PressVeil while the reveal runs): it is not a click on the map under it.
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void HandleAction(FName Action, float Value);

	UPROPERTY(Transient) TObjectPtr<UWidget> Veil;
	UPROPERTY(Transient) TObjectPtr<UScrollBox> Page;
	UPROPERTY(Transient) TArray<TObjectPtr<UWidget>> LineRows;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> NoteText;
	UPROPERTY(Transient) TObjectPtr<UWidget> ButtonRow;
	UPROPERTY(Transient) TObjectPtr<UCosmicActionButton> ToMapButton;
	UPROPERTY(Transient) TObjectPtr<UCosmicActionButton> NewRunButton;

	ECosmicProvisionalEnd ShownCause = ECosmicProvisionalEnd::None;
	bool bOpen = false;
	bool bRevealed = false;
	float Elapsed = 0.f;
	// Real seconds left for the second press of 'new run'; unset while none was half pressed.
	TOptional<float> ConfirmLeft;
```

**꼴**
- 베일은 `(0, 0, 0.02, 0.85)`이다. 승천 연출의 0.6보다 어둡다(읽는 화면이다). 가운데 기둥은 너비 760(연출의 서술 상자와 같다)이다.
- 글씨: 장 제목 26 금빛(`CosmicPalette::Gold`), 사실 줄 14 회색, 에필로그 첫 줄 22 흰색, 둘째 줄 18 회색, 지평 문장 18 흰색, 넘침 16 회색, 까닭 줄과 안내 13 회색.
- 줄은 두 칸이다. 왼쪽 90px에 스탬프(13, 회색, 오른쪽 맞춤), 오른쪽에 `Message`(16, 줄바꿈). 색은 피드의 `CosmicLogFeed::ColorOf(Severity, Category)`(`UI/CosmicLogFeedWidget.h:63`)라 회·백·금·보라 넷을 넘지 않는다. 다른 백성의 줄은 불투명도 0.7이고, 모르던 줄에 따로 표지는 없다.
- 단추 둘은 `UCosmicActionButton`(`Setup`·`Press`, 회색이면 누름을 받지 않는다)이다. 단추 줄은 다 드러난 뒤에만 `Visible`이다.
- 사실 줄의 칸은 승천 연출처럼 `FText::Format(INVTEXT("{0} {1}"), CosmicUI::Label(TEXT("Panel.Tier")), FText::AsNumber(PeakTier + 1))`로 쓴다(`CosmicAscensionOverlayWidget.cpp:227`).

**동작**
- `Open(Cause, bAtOnce)`: `Build`(`UCosmicLogSubsystem::BuildChronicle`)로 줄을 다시 짓는다(`Page->ClearChildren()` 뒤 `AddChild`, `ScrollToStart`). 까닭 줄은 Rung → `Note.Rung`(`{Civ}`의 규칙은 헤더 §6), Clock → `Note.Clock`(`{0}` = `State.Tick`), None → 없음. Rung·Clock일 때만 그 뒤에 `Chronicle.Provisional`을 둔다(미리보기는 끝이 아니다). 까닭 줄·안내·단추 줄은 스크롤 상자 **아래**에 둔다(고정 첫 줄과 붙지 않는다 — 헤더 §4.3). `bAtOnce`이면 베일·줄·단추를 곧바로 보인다.
- `Advance`: 베일의 알파(0 → 0.85, `VeilSeconds`) → 줄마다 `LineSeconds`. 줄이 많으면 간격을 `RevealCapSeconds / 줄 수`로 줄인다. 다 드러나면 `bRevealed`이고 단추 줄이 선다. `ConfirmLeft`를 센다(0이면 `NewRun` 글로 되돌린다).
- `NativeOnMouseButtonDown`: 드러냄 중이면 `PressVeil`. 늘 `FReply::Handled()`(지도의 클릭이 아니다).
- 엔진은 멈추지 않고 시뮬만 멈추므로 `NativeTick`이 돈다(승천 연출과 같다).
- `UScrollBox`는 이 프로젝트에서 처음 쓴다. `docs/45`에 적는다.

## 4. 상단 바와 성계 패널

- **배속 단추**: `Refresh`의 배속 단추 고리(`CosmicTopBarWidget.cpp:415-429`)에서, 끝났으면 모든 단추가 `SetIsEnabled(false)`다. ×25는 문이 열린 판이면 서되 회색이다. 툴팁은 바꾸지 않는다(`SetToolTipText`는 부를 때마다 새 툴팁을 만든다).
- **'연대기' 단추**(`Chronicle.Reopen`): 배속 단추 뒤에 둔다. `IsRunEnded()` ∧ `!Page->IsOpen()`일 때만 `SelfHitTestInvisible`이다. 누르면 `ReopenChronicle()`.
- **시계 툴팁**: `RebuildWidget`에서 한 번 짓는 지금의 두 줄(`:138`) 뒤에 셋째 줄을 붙인다. 노브 > 0이면 `Chronicle.Rule`(`{0}` = `CosmicUI::ClockLabel(노브, ECosmicTimeUnit::Cycle)` — 한 번만 지으므로 늘 주기, "주기 6,000"), 노브 ≤ 0이면 `Chronicle.RuleRung`, 스위치가 꺼져 있으면 없다. 한 번만 지으므로, 테스트는 스위치를 바꾼 뒤 위젯을 다시 짓고 본다.
- **회색 줄**(`EndSoonText`, 시계 오른쪽, 12 회색): 끝 전에만 선다.
  - `bProvisionalEnd` ∧ 노브 > 0 ∧ 0 < 노브 − Tick ≤ `EndSoonTicks`이면 `Chronicle.Soon`(`{0}` = `CosmicUI::TimeLeftLabel(노브 − Tick, Unit)`)이다.
  - 그 밖에 `bProvisionalEnd` ∧ 노브 > 0 ∧ 폐위됨(`Lives.Last().ReignEndReasons`의 마지막 칸이 유효)이면 `Chronicle.Until`(`{0}` = `CosmicUI::ClockLabel(노브, Unit)`)이다.
  - 글이 바뀔 때만 `SetText`한다.
- **성계 패널**: 명령 단추의 사유를 고를 때 `ACosmicPlayerController::IsRunEndedFor(*this)`를 맨 앞에 본다. 끝났으면 사유 `Chronicle.Block`, 짧은 사유 `Chronicle.Suffix`다. 길은 지금 그대로다(회색·툴팁은 바뀔 때만 — `CosmicSystemPanelWidget.cpp:695-700`).
- **정착 단추**(`:645-660`, 따로 가는 길): 끝났으면 `Wanted`의 사유와 툴팁을 `Chronicle.Block` 하나로 두고(값 줄 `StartHintOf` 없음, 이름 뒤 짧은 사유 없음), 단추 아래 사유 줄(`:703-704`의 `ReasonOf(ShownSettleBlock)`)도 `Chronicle.Block`으로 둔다. `ShownSettleBlock`은 지금처럼 셈하되 끝났을 때는 그 사유를 보이지 않는다.

## 5. 커맨드렛 (`Tests/CosmicBalanceCommandlet.cpp`)

```cpp
	// FRunCounts
	ECosmicProvisionalEnd FirstEnd = ECosmicProvisionalEnd::None;
	int64 FirstEndTick = -1, RungTick = -1, ClockTick = -1;
	int32 ChapterShown = -1, ChapterCandidates = -1, ChapterMore = -1, ChapterLife = -1;

	// In the tick loop, right after Sim->RunHeadless(1) and before the policy input:
	const ECosmicProvisionalEnd End = CosmicEndings::ProvisionalEndOf(State, Sim->GetRules());
	Counts.RungTick = (Counts.RungTick < 0 && End == ECosmicProvisionalEnd::Rung) ? State.Tick : Counts.RungTick;
	const FCosmicRulesDef& Rules = Sim->GetRules();
	Counts.ClockTick = (Counts.ClockTick < 0 && Rules.ProvisionalEndTick > 0 && State.Tick >= Rules.ProvisionalEndTick) ? State.Tick : Counts.ClockTick;
	if (Counts.FirstEnd == ECosmicProvisionalEnd::None && End != ECosmicProvisionalEnd::None)
	{
		Counts.FirstEnd = End;
		Counts.FirstEndTick = State.Tick;
		const TArray<FCosmicChronicleChapter> Chapters = CosmicChronicle::Build(State);
		if (!Chapters.IsEmpty())
		{
			Counts.ChapterLife = State.Calling.Lives[Chapters.Last().LifeAt].LifeIndex;
			Counts.ChapterShown = Chapters.Last().Lines.Num();
			Counts.ChapterCandidates = Chapters.Last().Candidates;
			Counts.ChapterMore = Chapters.Last().bMore ? 1 : 0;
		}
	}

	// After the ruler line:
	UE_LOG(LogCosmic, Display, TEXT("CosmicBalance end: first %s at %lld; rung at %lld, clock at %lld, deposed at %lld; chronicle life %d shown %d of %d, more %d"), ...);
	const bool bSameTickDeposition = Counts.RuledT3Tick < 0 && Counts.DeposedTick >= 0 && Counts.DeposedTick == Counts.RungTick;
	if (Counts.RungTick != Counts.RuledT3Tick && !bSameTickDeposition)
	{
		// A measure, not a gate: Result stays.
		// A prefix of its own: a tool that takes the last "CosmicBalance end:" line never takes this one.
		UE_LOG(LogCosmic, Display, TEXT("CosmicBalance end check: the rung end at %lld is not the ruled people's T3 at %lld"), Counts.RungTick, Counts.RuledT3Tick);
	}
```

- `ClockTick`은 칸이 먼저 서도 적는다(정책 판에서 시계까지 얼마 남는지 본다). 술어의 Clock은 칸이 이기면 보이지 않으므로 노브로 따로 잰다.
- include: `Sim/CosmicEndings.h`, `Log/CosmicChronicle.h`(Tests는 무엇이든 include할 수 있다).
- 헤더 주석은 'six log lines' → 'seven'이다. 일곱째를 열거에 더하고, 'as long as there are no endings …' 뒤에 헤더 §3.4의 구절을 둔다.

## 6. 개발 도구와 창 없이 찍는 화면

- `-ProvisionalEnd=0`: 세션 동안 끝을 끈다. 쓰는 곳은 ×25 프레임 시간 측정(`-Speed=25`), 6,000틱 너머의 화면, 끝을 넘기는 손 시험이다. 자동화 테스트는 `SetProvisionalEndEnabled`를 쓴다.
- `CosmicPreviewChronicle <초>`: 끝내지 않고 지금 상태의 연대기를 다 드러난 채 연다(까닭·'새 판' 없음). `Exec`의 기본 인자는 게임 바이너리에서 채워지지 않으므로 인자를 늘 받는다(`docs/20` §6.3의 `CosmicPreviewRung` 메모).
- **창 없이 찍는 두 장**(A-4b):
  - 미리보기: `-Seed=42 -ExecCmds="CosmicPreviewChronicle 20" -ShotAfterSeconds=22`.
  - 실제 끝: `-Seed=42 -Speed=25 -AutoPause=0 -ShotAfterSeconds=255`. ×25에서 6,000틱이 약 240초이고, 1.5초를 기다린 뒤 6초 안에 드러난다. 그 판이 먼저 T3에 닿으면 칸 끝의 쪽이 찍힌다(입력 없는 판이라 대개 시계 끝이다).
  - 둘 다 `-AutoPause=0`이 아니면 S2 줄에서 멈출 수 있다.
- 손 확인 뒤 `Saved/Logs/Cosmic_Ascension.log`에서 `CosmicEnd:` 줄을 찾아 두 초를 `CHANGELOG`에 옮긴다.

## 7. 테스트의 세부

- **`CosmicTests::AcquireSim`**(`Tests/CosmicTestHelpers.h`): `World/CosmicGameMode.h`를 include하고, 배속 0 뒤에 다음을 부른다.
```cpp
		UWorld* World = GameInstance->GetWorld();
		if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(World ? World->GetFirstPlayerController() : nullptr))
		{
			// A test that ended a run (a broadcast tick past the end, a load of an ended state) leaves nothing to the next one.
			Controller->SetProvisionalEndEnabled(true);
			Controller->ClearRunEnd();
		}
```
- **`Sim.ProvisionalEnd`**:
  - 표는 `FGalaxyState`를 손으로 짓는다(`Calling.Lives`에 생애 하나, `PeakTier`와 `Tick`만 바꾼다. '생애 둘' 칸만 둘째 생애를 덧붙인다). 노브는 `FCosmicRulesDef` 값으로 넘긴다(Sim 표 교체 불필요).
  - '멈추지 않는다'와 '노브 200·0' 블록만 산 Sim을 쓴다. `CosmicTests::WithRules`로 표를 바꾸고, `ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); }`로 되돌린다.
  - 방송 `StepOnce`가 컨트롤러에 래치를 걸어도 `ON_SCOPE_EXIT`의 `NewGalaxy`와 다음 테스트의 `AcquireSim`이 지운다.
- **`Log.ChronicleBuild`**: 상태를 손으로 짓는다(`Calling.Lives`, `Log`, `Chronicle`). `Message`는 `FText::AsCultureInvariant`로 넣는다. 테이블이 드는 것은 `VesselNameOf`뿐이다(`CosmicTests::NewTable`로 `FCivArchetypeDef` 두 행).
- **`UI.RunEnd`**:
  - ① 컨트롤러 `SetAutoPause(false)`. `NewGalaxy(42, 0, 0)` 뒤 `GetMutableState().Tick = 5997`로 두고, `Sim->SetBroadcastEnabled(true)`, `Controller->SetSpeed(10)`, `Sim->Tick(1.f)`.
  - ⑥ 켬은 `SetAutoPause(true)`로 실제 승천(`Sim.PlayerAscendsToT3`의 길 — 방송 `StepOnce`)을, 끔은 같은 길을 자동 정지 끔으로 돈다. 연출은 HUD의 것을 쓴다. **두 번 모두 승천하는 `StepOnce` 바로 앞에 `Controller->SetSpeed(10)`을 둔다** — `AcquireSim`이 배속을 0으로 두어, 그대로면 `RequestAutoPause`(배속 > 0일 때만, `CosmicGameMode.cpp:613`)도 연출의 `Show`(`CosmicAscensionOverlayWidget.cpp:235`)도 멈추지 않아 단언이 아무 일 없이 참이 된다. 켬: 틱 뒤 `Overlay->GetPausedSpeed()` 0(자동 정지 `MythicLine`이 연출보다 먼저 멈췄다), 까닭 `RunEnd`, `Close` 뒤 배속 0. 끔: `GetPausedSpeed()` 10(연출이 쥐었다), 까닭 `RunEnd`, `Close` 뒤 배속 0(되돌림이 잠금에 걸렸다).
  - ⑦ 끝난 상태의 사본은 `GetState()` 복사 → `Tick = 6000` → `LoadState`.
  - ⑨ `StartNewRun` 뒤에는 `Map->RefreshNow()`로 다시 짓기를 부른 다음 `GetSelectedSystem()`이 None인지 본다.
  - 끝에 `SetAutoPause(원래 값)`, `SetProvisionalEndEnabled(true)`, `NewGalaxy(42, 5, 3)`.
- **`UI.Chronicle`**: 위젯을 직접 `CreateWidget`하지 않고 HUD의 것을 쓴다. 쪽의 글은 `GetPageTexts()`로 본다. 회색 줄은 상단 바의 `GetEndSoonText()`(테스트 접근자)로 본다. '년' 단위는 `Controller->SetTimeUnit(Year)`.
- **`UI.MouseOnlyAscent`**: 꼬리(`CosmicPanelTests.cpp:753-770`)를 헤더 §7의 차례로 바꾼다. `Overlay->Advance(CosmicAscensionOverlay::RevealSeconds + 1.f)`는 `Close()` 바로 뒤에 둔다. 마지막의 `Sim->SetTimeScale(0.f)` 앞에 `SetProvisionalEndEnabled(true)`를 확인한다.
- **위험**: 방송을 켠 테스트가 6,000을 넘거나 T3에 닿은 채 `NewGalaxy` 없이 끝나도, 다음 테스트의 `AcquireSim`이 래치와 `RunEnd` 까닭을 지운다. 그래도 A-4b에서 전체를 두 번 연속 돌린다.

### 7.1 단언의 값과 변이 (헤더 §7 표의 세부)

| 테스트 | 단언의 값과 단계 | 깨뜨릴 변이 |
|---|---|---|
| `Sim.ProvisionalEnd` | **표**(노브 6,000): Lives 빔 → 9,999에도 None. PeakTier T2 → 5,999 None, 6,000·6,001 Clock. T3 → 100·6,000 Rung. 노브 0 → 99,999 None. 노브 −5 → 시계 끝 없음. 노브 0·T3 → Rung. `Civs[].Tier`만 T3·PeakTier T2 → None. 생애 둘(앞 T3·뒤 T1), 틱 100 → Rung. 앎·로그·연대기·`TellRngState`를 바꿔도 같다. **순수**: 1,000번 불러도 두 해시·`RngState`가 같다. **멈추지 않는다**: T3 판에서 `RunHeadless(5)` +5, `StepOnce` +1. 시드 42 노브 200·0(`WithRules`)의 `RunHeadless(300)` → 두 해시가 같다. `ON_SCOPE_EXIT`에서 표와 판을 되돌린다 | `>=`→`>`, `Civs[].Tier` 읽기, Clock 먼저, 노브 가드 빼기, Lives 가드 빼기, `Lives.Last()`만 보기, `RunHeadless`에 끝 검사 |
| `Sim.PlayerAscendsToT3` | T3 틱에 처음 Rung, 그 전 틱 None | 칸 비교를 T4로 |
| `Log.ChronicleBuild` | **`Admits` 11칸**: S0 제 백성 ✗, S1 Birth·둘째 Ruled ✓, S1 남 ✗, `log_first_contact` ✓, `log_contact` ✗, 초점 없음 ✗, Calling S0 ✓, Claim S1 남 `bUntold` ✓, Record S0 ✓, S2 남 `bUntold` ✓. `bUntold`를 뒤집어도 같다. Claim·Record는 장에 없다. Chronicle 틱 5가 Log 틱 10보다 앞이다. **되풀이 ①**: 바로 뒤 같은 문장은 후보에서 빠진다. **상한**: 후보 40 → 거짓, 41 → 참. Calling 3 + r3 45 → r3는 `(i×45)/37`의 37줄. r0 2 + r1 5 + r2 50 → r2는 처음 17·마지막 16, r3 0. **되풀이 ②**: 솎기가 붙인 같은 문장은 하나만 남는다. **범위**: BornTick 0·100이면 99는 1장, 100은 2장, 1장 백성의 S1(150)은 빠진다. 1장 DiedTick 80이면 81~99는 어느 장에도 없다. 마지막 `ToTick` = `State.Tick`. 앎과 상관없다. `Build` 전후 두 해시가 같다. `VesselNameOf` 셋. `BuildChronicle()` = `Build` | `Log`만 읽기, `bUntold` 거르기, Birth 빼기, 상한 ±1, Claim 넣기, 남의 S1, 되풀이 ① 빼기, 되풀이 ② 빼기, 경계 `<`→`<=`, 솎기를 앞 자르기로 |
| `UI.RunEnd` | ① 틱 5,997·×10·방송 켬·자동 정지 끔에서 `Sim->Tick(1.f)` → 정확히 6,000, 배속 0, Clock, 까닭 `RunEnd`. ② 5,999의 `SetSpeed(2)` → 2. ③ 끝 뒤 `SetSpeed(10)`·키 Two·Space → 0, 단추 회색, `AnswerPetition(0)` → Refused(대기 행·`RngState` 그대로), `StartProject` → false, 남의 성계의 정착 단추는 툴팁 첫 줄과 아래 사유 줄이 `Chronicle.Block`. ④ 끝 틱의 카드가 접힌다. ⑤ `HoldSeconds − 0.1` 닫힘, +0.2 열림. ⑥ 위의 '⑥'(켬·끔 두 번, `SetSpeed(10)` 뒤 승천). 연출·드러냄 동안은 `AdvanceRunEnd(10)`에도 닫혀 있고, `Overlay->Advance(RevealSeconds + 1)` 뒤에 열린다. ⑦ 끝난 사본 `LoadState` → 곧 다 드러나고 카드 없음. ⑧ 끝난 상태(방송 없이 손으로)에서 `FollowEnd(false)` → 래치와 다 드러난 쪽. 두 번째 부름은 바꾸는 것이 없다. ⑨ `StartNewRun` → `GalaxyID` 바뀜, 틱 0, None, 쪽 닫힘, 배속 1, 지도를 다시 지은 뒤 선택이 빈다. ⑩ 스위치 끔 → 6,000에 아무 일 없음. ⑪ 래치를 둔 채 `AcquireSim` → None, 까닭 None(`RunEnd`가 남지 않는다). ⑫ 끝부터 다시 열기까지 두 해시가 같다 | `FollowEnd`를 `PlayerTick`에만(①이 6,007), 잠금 빼기(③⑥), 명령의 문 빼기(③), 정착 사유 줄을 그대로(③), `FollowPetition` 먼저(④), 연출 기다림 빼기(⑥), 지우기 전 `FollowEnd`(⑦⑨), `FollowEnd(false)`가 쪽을 열지 않기(⑧), 리셋 빼기(⑪), `ClearRunEnd`의 까닭 되돌림 빼기(⑪) |
| `UI.Chronicle` | **쪽(문서 순)**: "제 1대 · <이름>", "<백성> · 칸 3 · <n>주기", 줄들, (More), `Opened`, `Unopened`, 맨 끝 `Horizon`. '년'에서도 스탬프는 주기다. 불투명도 1/0.7, 색 `ColorOf`. **까닭**: Rung → `Note.Rung`(뒤에서 본 이름), 지워졌으면 `RungNoCiv`, Clock → `Note.Clock`, 모두 뒤에 `Provisional`. 미리보기(`Cause` None)는 까닭·`Provisional`·'새 판'이 없다. **드러냄**: 단추는 다 드러난 뒤에만, `PressVeil`이면 한꺼번에. **새 판**: 첫 누름은 글만, 3초 안 둘째가 새 판, `Advance(3.1)` 뒤는 다시 확인. **상단 바**: '연대기'는 끝났고 접혔을 때만. `Rule`은 "… 주기 6,000에 이르면 멈춘다."(0 → `RuleRung`, 꺼짐 → 없음). 예고 5,399 없음, 5,400 "이 판은 곧 멈춘다 — 남은 주기 600". 폐위 뒤 "이 판은 주기 6,000에 멈춘다.". 끝 뒤 없음 | 드러냄 전 단추, 확인 건너뛰기, 불투명도 하나로, 스탬프가 단위를 따르기, 예고 하나 차이, 고정 줄을 장 앞에, `{Civ}`를 Birth로, 미리보기에 `Provisional` 붙이기 |
| `UI.MouseOnlyAscent` | 연출이 서 있으면 `Close` → `Overlay->Advance(RevealSeconds + 1)` → `FollowEnd(true)`(헤드리스 판은 `OnSimTick`을 보내지 않는다) → Rung·배속 0 → `AdvanceRunEnd(HoldSeconds + 0.1)` → 쪽 → `PressVeil` → '지도 보기'. 이 테스트가 보기를 돌려 두어 드러냄은 대개 서지 않지만, 순서에 기대지 않으려고 `Advance`를 둔다 | 잠금 빼기 |
| `UI.AutoPause` · `test_data_schema.py` | 까닭 고리에 `RunEnd` → `Chronicle.Why` · 검사기 다섯 경우(헤더 §6)와 노브 범위 | `RunEnd` 줄 빼기 · 접두어 규칙 빼기, `{}` 안 숫자 보기 |

## 8. `CosmicChronicle`의 본문 스케치 (`Log/CosmicChronicle.cpp`)

```cpp
bool CosmicChronicle::Admits(const FCosmicLogEntry& Entry, TConstArrayView<FName> Peoples)
{
	static const FGameplayTag CallingTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Calling")));
	static const FGameplayTag ClaimTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Claim")));
	static const FGameplayTag RecordTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Record")));
	if (Entry.Category == CallingTag || Entry.Category == ClaimTag || Entry.Category == RecordTag || Entry.Severity >= 2)
	{
		return true;
	}
	return Entry.Severity == 1 && (Peoples.Contains(Entry.FocusCivID) || Entry.SourceRow == CosmicRows::LogFirstContact);
}
```

- `PeoplesOf`의 `NAME_None` 빼기 때문에, 초점 없는 S1(`FocusCivID` None)은 `Contains`에 걸리지 않는다.
- **`Build`의 걸음**(정수만):
  1. 생애마다 범위를 정한다. `From` = (i == 0 ? `INT64_MIN` : `BornTick`), `To` = (`DiedTick` > 0 ? `DiedTick` : (다음 생애 ? 다음 `BornTick` − 1 : `State.Tick`)).
  2. `Chronicle` → `Log` 차례로 훑어, 범위 안 ∧ `Admits` ∧ Claim·Record가 아닌 항목을 등급과 함께 모은다. r0 Calling, r1 S2 ∧ 초점이 그 생애의 백성, r2 S1, r3 그 밖의 S2.
  3. 되풀이 ①로 같은 `Message`(문자열 비교, `ToString()`)가 바로 앞과 같으면 뺀다.
  4. `Candidates` = 남은 수. ≤ 40이면 다 싣는다.
  5. 넘치면 r0을 다 싣고, 남은 자리 k로 r1 → r2 → r3을 채운다. 첫 '다 들지 못한' 등급은 솎는다. r2는 앞 ⌈k/2⌉ + 뒤 ⌊k/2⌋, 그 밖은 `(i * n) / k`(i = 0..k−1)이다. 뒤 등급은 0이고 `bMore`가 선다.
  6. 원래 인덱스로 정렬해 시간순으로 되돌린다.
  7. 되풀이 ②(바로 앞에 실은 줄과 같으면 뺀다)를 한 번 더 건다.
  - r0만으로 40을 넘으면 r0을 다 싣고 r1~r3은 0이다(`bMore`).
- **`VesselNameOf`**: `Install`의 규칙(`PlayerRoleSubsystem.cpp:113-131`)을 읽기로만 되풀이한다. 행이 없거나 뱅크가 비면 `CosmicRows::ArchDefault`, 그것도 비면 빈 글이다.
- 태그 리터럴 셋은 여기 static으로 둔다(결정 16의 권장). 대안이면 `Core/CosmicTags.h`의 접근자를 쓴다.

## 9. 뒤 엔딩의 저장 기록 — 스케치 (B5·B6, ④에서는 넣지 않는다)

`OPEN_QUESTIONS`의 새 행 '엔딩의 저장 기록(B6 헤더 전)'에 옮겨 둘 스케치다. 결론(해시 포함, 쓰는 자리, 거절)은 B6 헤더가 정한다.

```cpp
// (B5/B6 - planned, NOT part of MVP item 4) The end of the run (section 4.4), appended at the end of FGalaxyState with a
// frozen default: an empty Kind = the run goes on, so saves made before it load unchanged (SaveVersion stays 1).
USTRUCT()
struct FCosmicEndingRecord
{
	GENERATED_BODY()
	// Ending.CollectiveAscension / Ending.SoloAscension / Ending.OtherOrder / Ending.SilentUniverse (tag ini, two levels).
	UPROPERTY() FGameplayTag Kind;
	UPROPERTY() int64 Tick = 0;
	// The AI people of Ending.OtherOrder; NAME_None otherwise.
	UPROPERTY() FName CivID;
	// Reached during an interregnum leap ("the ending the calling watched", section 4.4).
	UPROPERTY() bool bWatched = false;
};
```

- 종류는 저장 UENUM이 아니라 태그 `Ending.*`다. 열거자 순서가 영구가 되지 않고, 변형마다 값이 늘지 않는다.
- 그 기록이 오면 `FollowEnd`는 술어 대신 `State.Ending.Kind`를 읽는다. 표현의 길(멈춤·잠금·연대기)은 그대로다.

## 10. 검사기 스케치 (`Tools/cosmic_data_schema.py`)

```python
DIGIT_OUTSIDE_ARGS = re.compile(r"\{[^{}]*\}")   # strip {...} arguments before looking for digits

def check_string_table(file_name, text, schema):
    """... and, for keys under string_table.checked_prefixes, the words and digits of the text (unlike a DT column's
    no_digits, which reads the whole text, digits inside {} arguments are allowed: a String Table text takes its numbers
    through arguments)."""
    ...
    prefixes = schema.get("string_table", {}).get("checked_prefixes", {})
    for prefix, rule in prefixes.items():
        if "forbidden" in rule and rule["forbidden"] not in schema["words"]:
            problems.append("schema: string_table prefix %r names the word list %r, which is not in the schema" % (prefix, rule["forbidden"]))
    # for each row whose key starts with a prefix: forbidden words -> problem; rule["no_digits"] and a digit in
    # DIGIT_OUTSIDE_ARGS.sub("", text) -> problem
```

- `check_sources`(`:366`)가 스키마를 넘긴다. `TOP_KEYS`에 `"string_table"`을 그 꼴(사전)로 더한다.
- `test_data_schema.py`의 `test_string_table`에 헤더 §6의 다섯 경우를 더한다. 같은 시험 안의 경우가 늘 뿐, 종류 수는 73 그대로다.
