# CONFIG REFERENCE — DT_Rules 노브 표 (생성 문서)

> `python -I Tools/make_config_reference.py`가 `Tools/data/DT_Rules.json`·`schema.json`·`Source/`에서 만든다. **손으로 고치지 않는다** — 값은 JSON에서, 규칙은 schema에서, 읽는 곳은 코드에서 바뀐다. 노브 117개 가운데 런타임이 읽는 것 108개, 예약 9개(읽는 코드가 올 Phase에서 산다 — `docs/40_DEV_GUIDE.md` §4). 밸런스 상수 가운데 아직 코드에 박힌 것(별 등급의 에너지 `StarClassEnergy` — `Sim/GalaxySimulationSubsystem.cpp`의 `namespace Economy`, 단계 5까지)은 이 표에 없다 — 경제·연구의 나머지 계수는 MVP ⑧부터 이 표의 노브다.

| 노브 | 값 | 종류 | 스키마 규칙 | 읽는 코드(테스트 제외) | 상태 |
|---|---|---|---|---|---|
| `LeapTicksMin` | `800` | number | `{"min": 0, "integer": true}` | Sim/CosmicSuccession.cpp | 읽힘 |
| `LeapTicksMax` | `1600` | number | `{"min": 0, "integer": true}` | Sim/CosmicSuccession.cpp | 읽힘 |
| `LeapTotalCap` | `4500` | number | `{"min": 0, "integer": true}` | Sim/CosmicSuccession.cpp | 읽힘 |
| `BirthProtectionTicks` | `301` | number | `{"min": 0, "integer": true}` | Sim/CosmicHazards.cpp | 읽힘 |
| `BaseLifespanTicks` | `0` | number | `{"integer": true}` | Events/CosmicEventSubsystem.cpp | 읽힘 |
| `SuccessionHazardWeight` | `2.0` | number | `{"min": 0}` | — | 예약 |
| `SuccessionCivTierWeight` | `1.0` | number | `{"min": 0}` | — | 예약 |
| `SuccessionReverenceWeight` | `2.0` | number | `{"min": 0}` | — | 예약 |
| `SuccessionReverenceThreshold` | `0.3` | number | `{"min": 0}` | — | 예약 |
| `SuccessionSameCivPenalty` | `1.0` | number | `{"min": 0}` | — | 예약 |
| `SuccessionMinHazardTicks` | `300` | number | `{"min": 0, "integer": true}` | Sim/CosmicSuccession.cpp | 읽힘 |
| `ReverenceDecay` | `0.5` | number | `{"min": 0, "max": 1}` | Player/PlayerRoleSubsystem.cpp | 읽힘 |
| `AIAscensionSpeedMultiplier` | `1.0` | number |  | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `AIUpperRungProgressMultiplier` | `0.375` | number |  | Sim/CosmicMegaprojects.cpp | 읽힘 |
| `PostRebellionStability` | `0.15` | number |  | Player/PlayerRoleSubsystem.cpp | 읽힘 |
| `ThroneLockTicks` | `600` | number | `{"integer": true}` | Player/PlayerRoleSubsystem.cpp, UI/CosmicSystemPanelWidget.cpp | 읽힘 |
| `PressureBySourceTier` | `[0.0, 0.0, 0.25, 1.0, 2.0]` | numbers | `{"len": 5, "min": 0}` | Sim/CosmicHazards.cpp, Sim/GalaxySimulationSubsystem.cpp, World/CosmicMapLook.cpp | 읽힘 |
| `PressureAdjacencyFalloff` | `0.5` | number | `{"min": 0, "max": 1}` | Sim/CosmicHazards.cpp, Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `SpiritualPressureStabilityDrain` | `0.002` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `PressureFleetAttrition` | `0.01` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `FleetEffectivenessByTierGap` | `[1.0, 0.5, 0.0]` | numbers | `{"len": 3}` | Sim/CosmicConflict.cpp | 읽힘 |
| `FleetDelayPerPower` | `50.0` | number | `{"min": 0}` | Sim/CosmicConflict.cpp | 읽힘 |
| `FleetDelayCap` | `100.0` | number | `{"min": 0}` | Sim/CosmicConflict.cpp | 읽힘 |
| `FleetDelayCountdownShare` | `0.5` | number | `{"min": 0, "max": 1}` | Sim/CosmicConflict.cpp | 읽힘 |
| `ImmutableLaws` | `"(GameplayTags=((TagName=\"Law.Judgment\"),(TagName=\"Law.Sabbath\"…` | tags | `{"root": "Law."}` | — | 예약 |
| `FallenTranscendentCount` | `1` | number | `{"min": 0, "max": 3, "integer": true}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `PlayerArchetypeRow` | `"arch_seeker"` | row | `{"table": "DT_Civs"}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `ClaimAdjacencyLanes` | `1` | number | `{"integer": true}` | Sim/CosmicHazards.cpp | 읽힘 |
| `SphereEventWeightStep` | `0.1` | number | `{"min": 0}` | Events/CosmicEventSubsystem.cpp | 읽힘 |
| `ReturnLureMaxLanes` | `2` | number | `{"integer": true}` | — | 예약 |
| `ReturnedLivesToSeize` | `3` | number | `{"integer": true}` | — | 예약 |
| `LureAcceptByPolicy` | `{"Cautious": 0.1, "Reckless": 0.6, "Faithful": 0.0, "Tribute": 0.85}` | word_map | `{"words": "policy"}` | — | 예약 |
| `MaxNotablesPerCiv` | `4` | number | `{"min": 3, "max": 5, "integer": true}` | Events/CosmicEventSubsystem.cpp | 읽힘 |
| `PopulationBandThresholds` | `[0.15, 0.5, 2.0]` | numbers | `{"len": 3, "ascending": true}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `PopulationRegrowthPerTick` | `0.0002` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `EventSlotTicks` | `10` | number | `{"min": 1, "max": 20, "integer": true}` | Events/CosmicEventSubsystem.cpp | 읽힘 |
| `EventChancePerSlot` | `0.02` | number | `{"min": 0}` | Events/CosmicEventSubsystem.cpp | 읽힘 |
| `HomeGalaxyRow` | `"gal_milkyway"` | row | `{"table": "DT_Galaxies", "target": [{"col": "BridgeTicks", "is": 0}…` | Sim/GalaxySimulationSubsystem.cpp, UI/CosmicGalaxyPanelWidget.cpp, World/CosmicGalaxyMapActor.cpp, World/CosmicGameMode.cpp, World/CosmicHashRun.cpp | 읽힘 |
| `RealmCapByTier` | `[1, 3, 4, 7, 10]` | numbers | `{"len": 5, "ascending": true, "integer": true}` | Sim/CosmicMegaprojects.cpp | 읽힘 |
| `SightLanesByTier` | `[2, 3, 4, 6, 10]` | numbers | `{"len": 5, "min": 1, "integer": true}` | Player/PlayerRoleSubsystem.cpp, Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `SightIntervalTicks` | `10` | number | `{"min": 1, "integer": true}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `FirstSightTick` | `330` | number | `{"min": 0, "integer": true}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `FarLineCooldownTicks` | `300` | number | `{"min": 0, "integer": true}` | Log/CosmicLogSubsystem.cpp | 읽힘 |
| `LivingWorldStarShares` | `[0.01, 0.05, 0.14, 0.31, 0.57, 0.82, 1.0]` | numbers | `{"len": 7, "ascending": true, "min": 0, "max": 1, "ends_at": 1}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `FallenCoreMinLanesFromHome` | `3` | number | `{"integer": true}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `BackgroundStarCount` | `20000` | number | `{"min": 0, "max": 100000, "integer": true}` | World/CosmicMapLook.cpp | 읽힘 |
| `SceneryGalaxyCount` | `10000` | number | `{"min": 0, "max": 50000, "integer": true}` | World/CosmicUniverseLook.cpp | 읽힘 |
| `PetitionPeriodTicks` | `300` | number | `{"min": 1, "integer": true}` | Events/CosmicPetitions.cpp | 읽힘 |
| `StabilityRecoveryPerTick` | `0.001` | number | `{"min": 0, "max": 1}` | Sim/CosmicEdicts.cpp, Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `StabilityRestingLevel` | `0.6` | number | `{"min": 0, "max": 1}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `ClaimLeaveProfiles` | `"(GameplayTags=((TagName=\"AI.Profile.Expansionist\"),(TagName=\"AI…` | tags | `{"root": "AI.Profile."}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `FirstCrisisTick` | `150` | number | `{"min": 0, "integer": true}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `FirstCrisisHazard` | `"Hazard.AIAwakening"` | tag | `{"root": "Hazard.", "optional": true}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `ProvisionalEndTick` | `9000` | number | `{"min": 0, "integer": true}` | Sim/CosmicEndings.cpp, UI/CosmicTopBarWidget.cpp | 읽힘 |
| `ChronicleCap` | `1500` | number | `{"min": 0, "integer": true}` | Log/CosmicLogSubsystem.cpp | 읽힘 |
| `ResearchFocusShare` | `0.55` | number | `{"min": 0.3, "max": 1}` | Player/PlayerRoleSubsystem.cpp, UI/CosmicSystemPanelWidget.cpp, UI/CosmicTopBarWidget.cpp | 읽힘 |
| `TierMultiplier` | `[1.0, 1.5, 2.2, 3.0, 3.0]` | numbers | `{"len": 5, "min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `MaterialistEnergyBonus` | `0.2` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `InsightPerPopulation` | `1.0` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `SpiritualistInsightBonus` | `0.2` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `InfluencePerPopulation` | `1.0` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `OpenInfluenceBonus` | `0.2` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `ResearchInsightShare` | `0.5` | number | `{"min": 0, "max": 1}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `ResearchPerInsight` | `0.003` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `LaneTravelTicks` | `20` | number | `{"min": 1, "integer": true}` | Sim/CosmicWanderer.cpp | 읽힘 |
| `ScanSightBonusLanes` | `1` | number | `{"min": 0, "integer": true}` | Player/PlayerRoleSubsystem.cpp | 읽힘 |
| `EncounterChancePerArrival` | `0.25` | number | `{"min": 0, "max": 1}` | Events/CosmicPetitions.cpp | 읽힘 |
| `CoronationReverence` | `0.6` | number | `{"min": 0, "max": 1}` | Sim/CosmicWanderer.cpp, UI/CosmicSystemPanelWidget.cpp, World/CosmicWandererPolicy.cpp | 읽힘 |
| `CoronationStabilityBelow` | `0.2` | number | `{"min": 0, "max": 1}` | Sim/CosmicWanderer.cpp, UI/CosmicSystemPanelWidget.cpp | 읽힘 |
| `AbdicationStabilityLoss` | `0.1` | number | `{"min": 0, "max": 1}` | Player/PlayerRoleSubsystem.cpp | 읽힘 |
| `StartingEdict` | `"edict_mobilization"` | row | `{"table": "DT_Edicts", "optional": true}` | Sim/CosmicEdicts.cpp, Sim/CosmicExistenceBricks.cpp, Sim/GalaxySimulationSubsystem.cpp, UI/CosmicEdictWidget.cpp, World/CosmicGameMode.cpp | 읽힘 |
| `EdictSlotSocietyMarks` | `[0.5, 1.0]` | numbers | `{"min": 0, "max": 1, "ascending": true}` | Sim/CosmicEdicts.cpp, UI/CosmicEdictWidget.cpp | 읽힘 |
| `SocietyRecoveryBonus` | `0.25` | number | `{"min": 0, "max": 1}` | Sim/CosmicEdicts.cpp, Sim/GalaxySimulationSubsystem.cpp, UI/CosmicSystemPanelWidget.cpp | 읽힘 |
| `AIEdictRevokeStability` | `1.0` | number | `{"min": 0, "max": 1}` | Sim/CosmicEdicts.cpp | 읽힘 |
| `RelationDriftRate` | `0.002` | number | `{"min": 0, "max": 1}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `RelationEthicsWeight` | `0.6` | number | `{"min": 0, "max": 1}` | Sim/CosmicDiplomacy.cpp, UI/CosmicEdictWidget.cpp | 읽힘 |
| `RelationStanceTerms` | `[0.4, -0.4]` | numbers | `{"len": 2, "min": -1, "max": 1}` | Sim/CosmicDiplomacy.cpp, UI/CosmicEdictWidget.cpp | 읽힘 |
| `RelationTreatyTerm` | `0.2` | number | `{"min": 0, "max": 1}` | Sim/CosmicDiplomacy.cpp, UI/CosmicEdictWidget.cpp | 읽힘 |
| `ConquestRelationPenalty` | `0.2` | number | `{"min": 0, "max": 2}` | Sim/GalaxySimulationSubsystem.cpp, UI/CosmicSystemPanelWidget.cpp | 읽힘 |
| `BetrayalRelationPenalty` | `0.3` | number | `{"min": 0, "max": 2}` | Player/PlayerRoleSubsystem.cpp, UI/CosmicEdictWidget.cpp | 읽힘 |
| `FriendlyUpkeepInfluence` | `0.2` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp, UI/CosmicEdictWidget.cpp, UI/CosmicPetitionCardWidget.cpp, World/CosmicRulerPolicy.cpp | 읽힘 |
| `FriendlySlotsByTier` | `[1, 1, 2, 2, 3]` | numbers | `{"len": 5, "min": 0, "integer": true}` | Sim/CosmicDiplomacy.cpp, UI/CosmicEdictWidget.cpp | 읽힘 |
| `TreatyCostInfluence` | `50.0` | number | `{"min": 0}` | Player/PlayerRoleSubsystem.cpp, Sim/CosmicDiplomacy.cpp, Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `TreatyAcceptRelation` | `-0.1` | number | `{"min": -1, "max": 1}` | Sim/CosmicDiplomacy.cpp | 읽힘 |
| `TradeInfluencePerTreaty` | `0.2` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp, UI/CosmicEdictWidget.cpp | 읽힘 |
| `FleetPerSettledByTier` | `[0.0, 1.0, 1.0, 1.0, 1.0]` | numbers | `{"len": 5, "min": 0}` | Sim/CosmicConflict.cpp | 읽힘 |
| `FleetGrowthPerTick` | `0.001` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `FleetUpkeepEnergyPerPower` | `0.5` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp, UI/CosmicEdictWidget.cpp | 읽힘 |
| `SiegeTicks` | `200` | number | `{"min": 1, "integer": true}` | Sim/CosmicConflict.cpp, UI/CosmicSystemPanelWidget.cpp | 읽힘 |
| `SiegeRestTicks` | `600` | number | `{"min": 0, "integer": true}` | Sim/CosmicConflict.cpp | 읽힘 |
| `SiegeDefenceShares` | `[1.0, 0.5, 0.25]` | numbers | `{"len": 3, "min": 0, "max": 1}` | Sim/CosmicConflict.cpp | 읽힘 |
| `SiegeRepulsedFleetFactor` | `0.5` | number | `{"min": 0, "max": 1}` | Sim/CosmicConflict.cpp | 읽힘 |
| `AIHostileRelation` | `-0.3` | number | `{"min": -1, "max": 1}` | Sim/CosmicConflict.cpp | 읽힘 |
| `AIHostilePowerRatio` | `0.79` | number | `{"min": 0}` | Sim/CosmicConflict.cpp | 읽힘 |
| `AIHostilePacifismMax` | `0.5` | number | `{"min": -1, "max": 1}` | Sim/CosmicConflict.cpp | 읽힘 |
| `PlagueSpreadMultiplier` | `8.0` | number | `{"min": 0}` | Sim/GalaxySimulationSubsystem.cpp | 읽힘 |
| `AIHostileRelationTowardRuled` | `0.0` | number | `{"min": -1, "max": 1}` | Sim/CosmicConflict.cpp | 읽힘 |
| `AIHostilePowerRatioTowardRuled` | `0.6` | number | `{"min": 0}` | Sim/CosmicConflict.cpp | 읽힘 |
| `ResistFleetFactor` | `0.5` | number | `{"min": 0, "max": 1}` | Sim/CosmicConflict.cpp | 읽힘 |
| `GalaxyLawsToHold` | `3` | number | `{"min": 1, "integer": true}` | Sim/CosmicEdicts.cpp | 읽힘 |
| `BrickDarkMatterEnergyPerTick` | `0.0` | number | `{"min": 0}` | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickStarEaterEnergyMultiplier` | `1.0` | number | `{"min": 0}` | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickStarEaterRelationTarget` | `0.0` | number | `{"min": -1, "max": 1}` | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickStarEaterDimPeriodTicks` | `0` | number | `{"min": 0, "integer": true}` | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickIdleInsightPerTick` | `0.0` | number | `{"min": 0}` | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickSpanEnergyPerLane` | `0.0` | number | `{"min": 0}` | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickParasiteCountdownBonusTicks` | `0` | number | `{"min": 0, "max": 100000, "integer": true}` | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickParasiteInsightPerHazard` | `0.0` | number | `{"min": 0}` | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickNomadSettleProgressMultiplier` | `1.0` | number |  | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickInwardResearchMultiplier` | `1.0` | number |  | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickInwardHazardSpawnMultiplier` | `1.0` | number |  | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickSwarmResearchMultiplier` | `1.0` | number |  | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickMergeRelationTarget` | `0.0` | number | `{"min": -1, "max": 1}` | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickEvolutionPeriodTicks` | `0` | number | `{"min": 0, "integer": true}` | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickMutationThresholdTicks` | `0` | number | `{"min": 0, "integer": true}` | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickSeedTags` | `"(GameplayTags=)"` | tags | `{"root": "Brick."}` | Sim/CosmicExistenceBricks.cpp | 읽힘 |
| `BrickStellarCrisisSpawnMultiplier` | `1.0` | number |  | Sim/CosmicExistenceBricks.cpp | 읽힘 |
