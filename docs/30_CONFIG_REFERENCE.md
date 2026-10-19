# CONFIG REFERENCE — DT_Rules 노브 표 (생성 문서)

> `python -I Tools/make_config_reference.py`가 `Tools/data/DT_Rules.json`·`schema.json`·`Source/`에서 만든다. **손으로 고치지 않는다** — 값은 JSON에서, 규칙은 schema에서, 읽는 곳은 코드에서 바뀐다. 노브 46개 가운데 런타임이 읽는 것 28개, 예약 18개(읽는 코드가 올 Phase에서 산다 — `docs/40_DEV_GUIDE.md` §4). 밸런스 상수 가운데 아직 코드에 박힌 것(경제·연구 계수 — `Sim/GalaxySimulationSubsystem.cpp`의 `namespace Economy`)은 이 표에 없다; MVP에서 옮긴다.

| 노브 | 값 | 종류 | 스키마 규칙 | 읽는 코드(테스트 제외) | 상태 |
|---|---|---|---|---|---|
| `LeapTicksMin` | `800` | number | `{"integer": true}` | — | 예약 |
| `LeapTicksMax` | `1600` | number | `{"integer": true}` | — | 예약 |
| `LeapTotalCap` | `4500` | number | `{"integer": true}` | — | 예약 |
| `BirthProtectionTicks` | `300` | number | `{"integer": true}` | Events/CosmicEventSubsystem.cpp | 읽힘 |
| `BaseLifespanTicks` | `0` | number | `{"integer": true}` | Events/CosmicEventSubsystem.cpp | 읽힘 |
| `SuccessionHazardWeight` | `2.0` | number |  | — | 예약 |
| `SuccessionCivTierWeight` | `1.0` | number |  | — | 예약 |
| `SuccessionReverenceWeight` | `2.0` | number |  | — | 예약 |
| `SuccessionReverenceThreshold` | `0.3` | number |  | — | 예약 |
| `SuccessionSameCivPenalty` | `1.0` | number |  | — | 예약 |
| `SuccessionMinHazardTicks` | `300` | number | `{"integer": true}` | — | 예약 |
| `ReverenceDecay` | `0.5` | number |  | Player/PlayerRoleSubsystem.cpp | 읽힘 |
| `AIAscensionSpeedMultiplier` | `1.0` | number |  | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `PostRebellionStability` | `0.15` | number |  | Player/PlayerRoleSubsystem.cpp | 읽힘 |
| `ThroneLockTicks` | `600` | number | `{"integer": true}` | Player/PlayerRoleSubsystem.cpp | 읽힘 |
| `PressureBySourceTier` | `[0.0, 0.0, 0.25, 1.0, 2.0]` | numbers | `{"len": 5, "min": 0}` | Sim/CosmicHazards.cpp, Sim/GalaxySimulationSubsystem.cpp, World/CosmicMapLook.cpp | 읽힘 |
| `PressureAdjacencyFalloff` | `0.5` | number | `{"min": 0, "max": 1}` | Sim/CosmicHazards.cpp, Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `SpiritualPressureStabilityDrain` | `0.002` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `PressureFleetAttrition` | `0.01` | number |  | — | 예약 |
| `FleetEffectivenessByTierGap` | `[1.0, 0.5, 0.0]` | numbers | `{"len": 3}` | — | 예약 |
| `FleetDelayPerPower` | `50.0` | number |  | — | 예약 |
| `FleetDelayCap` | `100.0` | number |  | — | 예약 |
| `ImmutableLaws` | `"(GameplayTags=((TagName=\"Law.Judgment\"),(TagName=\"Law.Sabbath\"…` | tags | `{"root": "Law."}` | — | 예약 |
| `FallenTranscendentCount` | `1` | number | `{"min": 0, "max": 3, "integer": true}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `PlayerArchetypeRow` | `"arch_terran"` | row | `{"table": "DT_Civs"}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `ClaimAdjacencyLanes` | `1` | number | `{"integer": true}` | Sim/CosmicHazards.cpp | 읽힘 |
| `VigilClaimExtension` | `100` | number | `{"integer": true}` | — | 예약 |
| `SphereEventWeightStep` | `0.1` | number | `{"min": 0}` | Events/CosmicEventSubsystem.cpp | 읽힘 |
| `ReturnLureMaxLanes` | `2` | number | `{"integer": true}` | — | 예약 |
| `ReturnedLivesToSeize` | `3` | number | `{"integer": true}` | — | 예약 |
| `LureAcceptByPolicy` | `{"Cautious": 0.1, "Reckless": 0.6, "Faithful": 0.0, "Tribute": 0.85}` | word_map | `{"words": "policy"}` | — | 예약 |
| `MaxNotablesPerCiv` | `4` | number | `{"min": 3, "max": 5, "integer": true}` | Events/CosmicEventSubsystem.cpp | 읽힘 |
| `PopulationBandThresholds` | `[0.15, 0.5, 2.0]` | numbers | `{"len": 3, "ascending": true}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `PopulationRegrowthPerTick` | `0.0002` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `EventSlotTicks` | `10` | number | `{"min": 1, "max": 20, "integer": true}` | Events/CosmicEventSubsystem.cpp | 읽힘 |
| `EventChancePerSlot` | `0.02` | number | `{"min": 0}` | Events/CosmicEventSubsystem.cpp | 읽힘 |
| `HomeGalaxyRow` | `"gal_milkyway"` | row | `{"table": "DT_Galaxies", "target": [{"col": "BridgeTicks", "is": 0}…` | Sim/GalaxySimulationSubsystem.cpp, UI/CosmicGalaxyPanelWidget.cpp, World/CosmicGalaxyMapActor.cpp, World/CosmicGameMode.cpp | 읽힘 |
| `RealmCapByTier` | `[1, 3, 4, 7, 10]` | numbers | `{"len": 5, "ascending": true, "integer": true}` | Sim/CosmicMegaprojects.cpp | 읽힘 |
| `SightLanesByTier` | `[1, 2, 4, 6, 10]` | numbers | `{"len": 5, "min": 1, "integer": true}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `SightIntervalTicks` | `10` | number | `{"min": 1, "integer": true}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `FirstSightTick` | `330` | number | `{"min": 0, "integer": true}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `FarLineCooldownTicks` | `300` | number | `{"min": 0, "integer": true}` | Log/CosmicLogSubsystem.cpp | 읽힘 |
| `LivingWorldStarShares` | `[0.01, 0.05, 0.14, 0.31, 0.57, 0.82, 1.0]` | numbers | `{"len": 7, "ascending": true, "min": 0, "max": 1, "ends_at": 1}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `FallenCoreMinLanesFromHome` | `3` | number | `{"integer": true}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `BackgroundStarCount` | `20000` | number | `{"min": 0, "max": 100000, "integer": true}` | World/CosmicMapLook.cpp | 읽힘 |
| `SceneryGalaxyCount` | `10000` | number | `{"min": 0, "max": 50000, "integer": true}` | World/CosmicUniverseLook.cpp | 읽힘 |
