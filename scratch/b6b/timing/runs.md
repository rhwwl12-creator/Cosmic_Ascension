# G1 probe runs (SAND at 84f0d36 + temporary probe; seed 42)

## Headline: medians (us/tick) of on/off pairs run alternately in one build

Build 'probe' = HEAD + timers (level 0 = timers off, 1 = laps + counters + bench, 2 = also the primitives' own laps). Build 'proto' = probe build + the sorted-names memo (prototype.diff). Budget G1: 16.05.

| set | build | level | AI on | AI off | on - off | on/off | economy on/off | AI/economy on / off | crisis on/off | runs |
|---|---|---|---|---|---|---|---|---|---|---|
| idle20000 | probe | 0 | 24.28 | 12.72 | +11.56 | 1.91 | 2.99 / 3.30 | 8.09 / 3.86 | 29.17 / 14.64 | 3+3 |
| idle20000 | probe | 1 | 24.76 | 14.16 | +10.60 | 1.75 | 2.83 / 3.13 | 8.82 / 4.49 | 26.84 / 13.48 | 3+3 |
| idle20000 | probe | 2 | 25.93 | 15.61 | +10.32 | 1.66 | 2.83 / 3.22 | 9.16 / 4.85 | 26.77 / 13.98 | 1+1 |
| idle20000 | proto | 0 | 12.79 | 6.18 | +6.61 | 2.07 | 3.00 / 3.29 | 4.31 / 1.89 | 27.00 / 12.09 | 3+3 |
| idle3000 | probe | 0 | 18.02 | 11.34 | +6.68 | 1.59 | 1.63 / 1.65 | 11.04 / 6.88 | 10.30 / 9.53 | 3+3 |
| idle3000 | probe | 1 | 19.33 | 12.87 | +6.46 | 1.50 | 1.60 / 1.58 | 12.08 / 8.09 | 9.81 / 9.03 | 3+3 |
| idle3000 | proto | 0 | 7.31 | 5.61 | +1.70 | 1.30 | 1.60 / 1.57 | 4.57 / 3.60 | 8.63 / 7.76 | 3+3 |
| ruler20000 | probe | 0 | 20.37 | 11.49 | +8.88 | 1.77 | 2.78 / 3.28 | 7.35 / 3.50 | 27.11 / 13.42 | 3+3 |
| ruler20000 | probe | 1 | 21.29 | 13.30 | +7.99 | 1.60 | 2.63 / 3.25 | 8.01 / 4.10 | 26.54 / 13.28 | 3+3 |
| ruler20000 | proto | 0 | 10.03 | 5.90 | +4.13 | 1.70 | 2.70 / 3.41 | 3.76 / 1.73 | 25.10 / 12.12 | 3+3 |

## Prototype build: the runner's five seed-42 runs vs the committed Tools/baselines/seed42.json (84f0d36)

| run | same as baseline | AI us/tick | line (without the CSV path) |
|---|---|---|---|
| checks | yes | 7.16 | CosmicBalance: seed 42, 3000 ticks, hash 2557850423, world 2906170550 |
| checks_ruler | yes | 6.67 | CosmicBalance: seed 42, 3000 ticks, hash 1189870756, world 3359656751 / CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 17, answers 17,... |
| checks_wanderer | yes | 8.66 | CosmicBalance: seed 42, 6000 ticks, hash 1631315934, world 489794244 / CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 8, answers 8, st... |
| checks_20000 | yes | 12.90 | CosmicBalance: seed 42, 20000 ticks, hash 1957158585, world 3764704052 |
| checks_ruler_20000 | yes | 10.80 | CosmicBalance: seed 42, 20000 ticks, hash 1375149537, world 510604211 / CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 110, answers 11... |

## Per run

Per run: stage line (us/tick), probe blocks (us/tick), hash/world check against the lead's logs.

| run | hash ok | AI | econ | AI/econ | crisis | tick mean | choices | settle | revoke | laws | treaty (partner) | diplomacy (aim) | feats (brake, rows) | laps |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| idle20000_off_L0_1 | yes | 12.72 | 3.30 | 3.85 | 14.31 | 34.9 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle20000_off_L0_2 | yes | 12.58 | 3.26 | 3.86 | 14.64 | 35.0 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle20000_off_L0_3 | yes | 13.27 | 3.39 | 3.91 | 14.89 | 36.3 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle20000_off_L0proto_1 | yes | 6.27 | 3.29 | 1.91 | 12.33 | 26.5 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle20000_off_L0proto_2 | yes | 6.18 | 3.29 | 1.88 | 12.09 | 26.1 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle20000_off_L0proto_3 | yes | 6.16 | 3.26 | 1.89 | 11.98 | 25.8 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle20000_off_L1_1 | yes | 14.16 | 3.18 | 4.45 | 13.48 | 35.1 | 4.218 | 1.066 | 0.219 | 0.208 | 2.771 (0.000) | 1.027 (0.603) | 4.320 (0.346, 2.874) | 13.829 |
| idle20000_off_L1_2 | yes | 14.24 | 3.13 | 4.55 | 13.71 | 35.4 | 4.307 | 1.077 | 0.217 | 0.208 | 2.735 (0.000) | 1.042 (0.618) | 4.332 (0.337, 2.868) | 13.918 |
| idle20000_off_L1_3 | yes | 13.96 | 3.11 | 4.49 | 13.38 | 34.8 | 4.253 | 0.951 | 0.220 | 0.213 | 2.778 (0.000) | 0.995 (0.570) | 4.235 (0.353, 2.861) | 13.643 |
| idle20000_off_L2_1 | yes | 15.61 | 3.22 | 4.85 | 13.98 | 37.2 | 4.695 | 1.077 | 0.219 | 0.209 | 3.234 (0.000) | 1.058 (0.629) | 4.789 (0.355, 3.332) | 15.281 |
| idle20000_on_L0_1 | yes | 23.25 | 2.89 | 8.04 | 27.62 | 58.7 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle20000_on_L0_2 | yes | 24.28 | 3.00 | 8.09 | 29.17 | 61.6 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle20000_on_L0_3 | yes | 24.49 | 2.99 | 8.19 | 29.23 | 62.0 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle20000_on_L0proto_1 | yes | 12.69 | 2.94 | 4.32 | 26.34 | 47.0 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle20000_on_L0proto_2 | yes | 12.79 | 3.00 | 4.26 | 27.00 | 48.0 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle20000_on_L0proto_3 | yes | 13.20 | 3.06 | 4.31 | 28.02 | 49.6 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle20000_on_L1_1 | yes | 24.76 | 2.83 | 8.75 | 26.63 | 59.1 | 8.700 | 1.332 | 0.259 | 1.697 | 7.548 (2.192) | 1.147 (0.633) | 3.732 (0.279, 2.486) | 24.416 |
| idle20000_on_L1_2 | yes | 24.68 | 2.76 | 8.94 | 26.84 | 59.1 | 8.617 | 1.278 | 0.257 | 1.713 | 7.589 (2.194) | 1.111 (0.606) | 3.757 (0.279, 2.525) | 24.322 |
| idle20000_on_L1_3 | yes | 25.40 | 2.88 | 8.82 | 27.61 | 60.9 | 8.935 | 1.422 | 0.260 | 1.759 | 7.633 (2.274) | 1.184 (0.666) | 3.848 (0.280, 2.575) | 25.040 |
| idle20000_on_L2_1 | yes | 25.93 | 2.83 | 9.16 | 26.77 | 60.4 | 9.176 | 1.388 | 0.257 | 1.770 | 7.930 (2.210) | 1.162 (0.650) | 3.897 (0.264, 2.623) | 25.579 |
| idle3000_off_L0_1 | yes | 11.34 | 1.66 | 6.83 | 9.53 | 25.8 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle3000_off_L0_2 | yes | 11.41 | 1.65 | 6.92 | 9.55 | 25.9 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle3000_off_L0_3 | yes | 11.15 | 1.62 | 6.88 | 9.28 | 25.2 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle3000_off_L0proto_1 | yes | 5.61 | 1.56 | 3.60 | 7.60 | 17.7 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle3000_off_L0proto_2 | yes | 5.61 | 1.57 | 3.57 | 7.76 | 17.9 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle3000_off_L0proto_3 | yes | 6.00 | 1.64 | 3.66 | 8.23 | 19.0 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle3000_off_L1_1 | yes | 12.87 | 1.58 | 8.15 | 8.97 | 26.6 | 3.717 | 0.542 | 0.212 | 0.202 | 2.761 (0.000) | 1.151 (0.731) | 4.033 (0.058, 3.201) | 12.619 |
| idle3000_off_L1_2 | yes | 12.62 | 1.56 | 8.09 | 9.03 | 26.3 | 3.618 | 0.530 | 0.215 | 0.202 | 2.741 (0.000) | 1.113 (0.705) | 3.950 (0.060, 3.113) | 12.368 |
| idle3000_off_L1_3 | yes | 13.22 | 1.68 | 7.87 | 9.49 | 27.7 | 3.832 | 0.557 | 0.210 | 0.207 | 2.816 (0.000) | 1.189 (0.772) | 4.146 (0.068, 3.273) | 12.957 |
| idle3000_off_L1sanity_1 | yes | 12.48 | 1.54 | 8.10 | 8.74 | 25.8 | 3.560 | 0.511 | 0.214 | 0.202 | 2.736 (0.000) | 1.116 (0.697) | 3.892 (0.060, 3.078) | 12.232 |
| idle3000_on_L0_1 | yes | 18.02 | 1.63 | 11.06 | 10.30 | 33.1 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle3000_on_L0_2 | yes | 18.79 | 1.74 | 10.80 | 10.71 | 34.7 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle3000_on_L0_3 | yes | 17.56 | 1.59 | 11.04 | 9.95 | 32.3 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle3000_on_L0proto_1 | yes | 8.35 | 1.78 | 4.69 | 9.93 | 23.4 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle3000_on_L0proto_2 | yes | 7.11 | 1.56 | 4.56 | 8.58 | 20.2 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle3000_on_L0proto_3 | yes | 7.31 | 1.60 | 4.57 | 8.63 | 20.6 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| idle3000_on_L1_1 | yes | 19.65 | 1.63 | 12.06 | 10.06 | 34.6 | 5.591 | 0.530 | 0.215 | 0.202 | 5.350 (0.289) | 1.149 (0.720) | 6.352 (0.059, 5.533) | 19.389 |
| idle3000_on_L1_2 | yes | 19.33 | 1.60 | 12.08 | 9.81 | 33.8 | 5.463 | 0.553 | 0.216 | 0.200 | 5.266 (0.275) | 1.131 (0.707) | 6.236 (0.058, 5.410) | 19.065 |
| idle3000_on_L1_3 | yes | 19.07 | 1.54 | 12.38 | 9.81 | 33.5 | 5.371 | 0.520 | 0.215 | 0.209 | 5.226 (0.274) | 1.118 (0.693) | 6.153 (0.057, 5.363) | 18.812 |
| idle3000_on_L1sanity_1 | yes | 19.94 | 1.68 | 11.87 | 10.36 | 35.3 | 5.429 | 0.544 | 0.210 | 0.208 | 5.552 (0.279) | 1.140 (0.700) | 6.599 (0.055, 5.754) | 19.682 |
| ruler20000_off_L0_1 | - | 12.18 | 3.47 | 3.51 | 14.16 | 39.7 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| ruler20000_off_L0_2 | - | 11.49 | 3.28 | 3.50 | 13.42 | 37.6 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| ruler20000_off_L0_3 | - | 11.28 | 3.25 | 3.47 | 13.24 | 37.1 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| ruler20000_off_L0proto_1 | - | 5.90 | 3.41 | 1.73 | 12.12 | 28.0 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| ruler20000_off_L0proto_2 | - | 5.65 | 3.26 | 1.73 | 11.87 | 27.3 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| ruler20000_off_L0proto_3 | - | 6.16 | 3.56 | 1.73 | 12.83 | 29.4 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| ruler20000_off_L1_1 | - | 13.20 | 3.21 | 4.11 | 13.06 | 38.7 | 3.863 | 0.876 | 0.198 | 0.191 | 2.493 (0.000) | 0.989 (0.539) | 4.247 (0.309, 2.874) | 12.858 |
| ruler20000_off_L1_2 | - | 13.30 | 3.30 | 4.03 | 13.28 | 39.1 | 3.826 | 0.921 | 0.203 | 0.193 | 2.528 (0.000) | 1.009 (0.554) | 4.288 (0.311, 2.876) | 12.969 |
| ruler20000_off_L1_3 | - | 13.32 | 3.25 | 4.10 | 13.31 | 39.2 | 3.882 | 0.898 | 0.200 | 0.193 | 2.511 (0.000) | 1.004 (0.543) | 4.291 (0.312, 2.901) | 12.979 |
| ruler20000_on_L0_1 | yes | 20.58 | 2.79 | 7.38 | 27.25 | 61.5 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| ruler20000_on_L0_2 | yes | 20.37 | 2.77 | 7.35 | 27.11 | 61.1 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| ruler20000_on_L0_3 | yes | 20.08 | 2.78 | 7.22 | 26.86 | 60.5 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| ruler20000_on_L0proto_1 | yes | 10.53 | 2.80 | 3.76 | 26.12 | 47.0 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| ruler20000_on_L0proto_2 | yes | 10.03 | 2.62 | 3.83 | 24.27 | 44.0 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| ruler20000_on_L0proto_3 | yes | 9.91 | 2.70 | 3.67 | 25.10 | 44.9 | nan | nan | nan | nan | nan (nan) | nan (nan) | nan (nan, nan) | nan |
| ruler20000_on_L1_1 | yes | 21.02 | 2.63 | 7.99 | 26.09 | 60.0 | 8.048 | 1.276 | 0.222 | 1.267 | 5.464 (0.525) | 1.252 (0.760) | 3.123 (0.256, 1.958) | 20.653 |
| ruler20000_on_L1_2 | yes | 22.34 | 2.79 | 8.01 | 27.34 | 63.4 | 8.511 | 1.396 | 0.230 | 1.360 | 5.744 (0.563) | 1.337 (0.829) | 3.392 (0.284, 2.082) | 21.970 |
| ruler20000_on_L1_3 | yes | 21.29 | 2.60 | 8.19 | 26.54 | 60.8 | 8.191 | 1.277 | 0.224 | 1.273 | 5.490 (0.526) | 1.270 (0.774) | 3.214 (0.268, 2.020) | 20.939 |

## idle20000 level 0: medians of 3 on / 3 off

- AI stage: on 24.28 / off 12.72 us/tick; diff +11.56; ratio 1.909
- economy: on 2.99 / off 3.30; AI/economy on 8.09 / off 3.86 (ratio of ratios 2.097); off AI rescaled by economy on/off: 11.53
- crisis: on 29.17 / off 14.64; tick mean on 61.6 / off 35.0
## idle20000 level 0 proto: medians of 3 on / 3 off

- AI stage: on 12.79 / off 6.18 us/tick; diff +6.61; ratio 2.070
- economy: on 3.00 / off 3.29; AI/economy on 4.31 / off 1.89 (ratio of ratios 2.283); off AI rescaled by economy on/off: 5.64
- crisis: on 27.00 / off 12.09; tick mean on 48.0 / off 26.1
## idle20000 level 1: medians of 3 on / 3 off

- AI stage: on 24.76 / off 14.16 us/tick; diff +10.60; ratio 1.749
- economy: on 2.83 / off 3.13; AI/economy on 8.82 / off 4.49 (ratio of ratios 1.965); off AI rescaled by economy on/off: 12.80
- crisis: on 26.84 / off 13.48; tick mean on 59.1 / off 35.1

| block | on | off | on - off | share of the AI-stage diff |
|---|---|---|---|---|
| choices | 8.700 | 4.253 | +4.447 | 42% |
| settle | 1.332 | 1.066 | +0.266 | 3% |
| revoke | 0.259 | 0.219 | +0.040 | 0% |
| laws | 1.713 | 0.208 | +1.505 | 14% |
| treaty | 7.589 | 2.771 | +4.818 | 45% |
| diplomacy | 1.147 | 1.027 | +0.120 | 1% |
| feats | 3.757 | 4.320 | -0.563 | -5% |
| (partner, of treaty) | 2.194 | 0.000 | +2.194 | 21% |
| (aim, of diplomacy) | 0.633 | 0.603 | +0.030 | 0% |
| (brake, of feats) | 0.279 | 0.346 | -0.067 | -1% |
| (rows, of feats) | 2.525 | 2.868 | -0.343 | -3% |
| laps sum | 24.416 | 13.829 | +10.587 | 100% |
| stage - laps (loop, census, probe) | 0.344 | 0.331 | +0.013 | |

AI-stage calls per tick (median): aimCall 11.95/11.95, aimScan 0.64/0.63, aimSiegeChecks 0.04/0.04, asksLawsCall 11.95/11.95, asksLawsTrue 0.95/0.00, asksTreatyCall 11.95/11.95, asksTreatyTrue 3.17/0.00, choicesCall 11.95/11.95, choicesCapital 10.88/11.23, edictNames 0.95/0.00, enactBlock 2.86/0.00, featNames 29.24/35.17, findArch 0.00/0.00, findEdict 19.93/0.00, findFeat 137.01/105.51, findGalaxy 8.87/10.42, findHazard 65.67/6.60, findProject 3.02/2.76, findTrans 0.01/0.01, hazardAnswer 11.05/1.22, hazardByTag 1.14/1.67, hazardNames 1.14/1.67, judgeBraked 7.14/0.00, judgeCall 12.00/12.00, judgeCheckAll 15.27/2.97, markNeighbours 9.38/0.63, partnerCands 5.64/0.00, startBlock 0.70/0.22

Crisis-stage calls per tick (median, on/off): eventNames 0.50/0.50, findArch 43.30/37.02, findEdict 919.01/1.54, findEvent 31.20/28.82, findHazard 335.31/248.00, findProject 8.88/9.55, findTrans 1.08/1.87, hazardByTag 2.01/3.04, hazardNames 3.01/4.04

Census on: T1 0.783 T2 0.742 T3 8.865 T4 0.954 T5 0.603; T3 bare 3.165; blocks 0=6 7=1396 8=111402

Census off: T1 0.783 T2 0.742 T3 10.421 T4 0.000 T5 0.000; T3 bare 10.421; blocks

Same-state bench (final state of the on runs; min of 3 rounds; median over runs): ns per call / per tick-equivalent

| item | live (on tables) | copy (on, copied) | off (copied) | copy - off (B6b data's code cost on this state) |
|---|---|---|---|---|
| featNames | 351.3 | 350.7 | 174.5 | +176.2 |
| featRowNamesNoSort | 141.6 | 139.4 | 82.4 | +57.0 |
| hazardNames | 455.9 | 452.0 | 354.5 | +97.5 |
| edictNames | 682.0 | 680.7 | 570.3 | +110.4 |
| eventNames | 1088.3 | 1079.0 | 1031.0 | +48.0 |
| findFeat | 10.9 | 10.8 | 10.8 | +0.0 |
| hazardByTagAvg | 2953.5 | 2953.5 | 2370.4 | +583.1 |
| hazardAnswerMiss | 195.7 | 196.9 | 179.9 | +17.0 |
| hazardAnswerArk | 195.8 | 194.5 | 178.8 | +15.7 |
| tickChoices | 8774.0 | 8639.2 | 3583.5 | +5055.7 |
| tickAsksTreaty | 4966.2 | 4962.7 | 2450.5 | +2512.2 |
| tickPartnerOfLiveAskers | 2402.8 | 2402.2 | 2398.8 | +3.4 |
| tickAim | 372.7 | 373.4 | 374.4 | -1.0 |
| tickLaws | 1654.5 | 1640.6 | 447.9 | +1192.7 |
| tickJudge | 1762.6 | 1757.0 | 1078.5 | +678.5 |

Attribution model (us/tick; bench ns/call x the run's AI-stage calls/tick; the partner search is its lap):

| part | on | off | on - off | share of the AI-stage diff |
|---|---|---|---|---|
| GetFeatRowNames (alloc + TSet + sort) | 10.25 | 6.14 | +4.12 | 39% |
| FindFeat calls | 1.48 | 1.14 | +0.34 | 3% |
| IsHazardAnswer | 2.17 | 0.22 | +1.96 | 18% |
| FindHazardByTag (incl. GetHazardRowNames) | 0.56 | 0.66 | -0.10 | -1% |
| GetEdictRowNames | 0.65 | 0.00 | +0.65 | 6% |
| treaty partner search (measured lap) | 2.19 | 0.00 | +2.19 | 21% |
| modelled sum | 17.32 | 8.16 | +9.16 | 86% |
| rest (world divergence, other work, noise) | | | +1.44 | 14% |
| GetFeatRowNames share of the whole on stage | 41% | 43% | | |

lexical compare 12.7 ns; Cycles64 pair 30.0 ns; askers at the end 3

## idle20000 level 2: medians of 1 on / 1 off

- AI stage: on 25.93 / off 15.61 us/tick; diff +10.32; ratio 1.661
- economy: on 2.83 / off 3.22; AI/economy on 9.16 / off 4.85 (ratio of ratios 1.890); off AI rescaled by economy on/off: 13.72
- crisis: on 26.77 / off 13.98; tick mean on 60.4 / off 37.2

| block | on | off | on - off | share of the AI-stage diff |
|---|---|---|---|---|
| choices | 9.176 | 4.695 | +4.481 | 43% |
| settle | 1.388 | 1.077 | +0.311 | 3% |
| revoke | 0.257 | 0.219 | +0.038 | 0% |
| laws | 1.770 | 0.209 | +1.561 | 15% |
| treaty | 7.930 | 3.234 | +4.696 | 46% |
| diplomacy | 1.162 | 1.058 | +0.104 | 1% |
| feats | 3.897 | 4.789 | -0.892 | -9% |
| (partner, of treaty) | 2.210 | 0.000 | +2.210 | 21% |
| (aim, of diplomacy) | 0.650 | 0.629 | +0.021 | 0% |
| (brake, of feats) | 0.264 | 0.355 | -0.091 | -1% |
| (rows, of feats) | 2.623 | 3.332 | -0.709 | -7% |
| laps sum | 25.579 | 15.281 | +10.298 | 100% |
| stage - laps (loop, census, probe) | 0.351 | 0.329 | +0.022 | |
| prim featNames (inside the blocks) | 11.095 | 7.213 | +3.882 | 38% |
| prim hazardByTag (inside the blocks) | 0.595 | 0.705 | -0.110 | -1% |
| prim hazardAnswer (inside the blocks) | 2.529 | 0.245 | +2.284 | 22% |
| prim edictNames (inside the blocks) | 0.705 | 0.000 | +0.705 | 7% |

AI-stage calls per tick (median): aimCall 11.95/11.95, aimScan 0.64/0.63, aimSiegeChecks 0.04/0.04, asksLawsCall 11.95/11.95, asksLawsTrue 0.95/0.00, asksTreatyCall 11.95/11.95, asksTreatyTrue 3.17/0.00, choicesCall 11.95/11.95, choicesCapital 10.88/11.23, edictNames 0.95/0.00, enactBlock 2.86/0.00, featNames 29.24/35.17, findArch 0.00/0.00, findEdict 19.93/0.00, findFeat 137.01/105.51, findGalaxy 8.87/10.42, findHazard 65.67/6.60, findProject 3.02/2.76, findTrans 0.01/0.01, hazardAnswer 11.05/1.22, hazardByTag 1.14/1.67, hazardNames 1.14/1.67, judgeBraked 7.14/0.00, judgeCall 12.00/12.00, judgeCheckAll 15.27/2.97, markNeighbours 9.38/0.63, partnerCands 5.64/0.00, startBlock 0.70/0.22

Crisis-stage calls per tick (median, on/off): eventNames 0.50/0.50, findArch 43.30/37.02, findEdict 919.01/1.54, findEvent 31.20/28.82, findHazard 335.31/248.00, findProject 8.88/9.55, findTrans 1.08/1.87, hazardByTag 2.01/3.04, hazardNames 3.01/4.04

Census on: T1 0.783 T2 0.742 T3 8.865 T4 0.954 T5 0.603; T3 bare 3.165; blocks 0=6 7=1396 8=111402

Census off: T1 0.783 T2 0.742 T3 10.421 T4 0.000 T5 0.000; T3 bare 10.421; blocks

Same-state bench (final state of the on runs; min of 3 rounds; median over runs): ns per call / per tick-equivalent

| item | live (on tables) | copy (on, copied) | off (copied) | copy - off (B6b data's code cost on this state) |
|---|---|---|---|---|
| featNames | 351.2 | 348.3 | 174.7 | +173.6 |
| featRowNamesNoSort | 139.9 | 140.1 | 84.4 | +55.7 |
| hazardNames | 444.8 | 441.9 | 348.4 | +93.5 |
| edictNames | 708.0 | 700.6 | 571.8 | +128.8 |
| eventNames | 1058.1 | 1054.9 | 1009.8 | +45.1 |
| findFeat | 10.7 | 10.7 | 10.4 | +0.3 |
| hazardByTagAvg | 2954.6 | 2952.9 | 2372.3 | +580.6 |
| hazardAnswerMiss | 198.3 | 198.8 | 179.5 | +19.3 |
| hazardAnswerArk | 198.0 | 195.2 | 181.0 | +14.2 |
| tickChoices | 8784.8 | 8656.1 | 3653.6 | +5002.5 |
| tickAsksTreaty | 4983.9 | 4988.3 | 2449.0 | +2539.3 |
| tickPartnerOfLiveAskers | 2470.4 | 2462.8 | 2457.5 | +5.3 |
| tickAim | 363.8 | 363.5 | 362.6 | +0.9 |
| tickLaws | 1680.9 | 1658.9 | 457.3 | +1201.6 |
| tickJudge | 1824.2 | 1792.0 | 1107.8 | +684.2 |

Attribution model (us/tick; bench ns/call x the run's AI-stage calls/tick; the partner search is its lap):

| part | on | off | on - off | share of the AI-stage diff |
|---|---|---|---|---|
| GetFeatRowNames (alloc + TSet + sort) | 10.18 | 6.14 | +4.04 | 39% |
| FindFeat calls | 1.47 | 1.13 | +0.34 | 3% |
| IsHazardAnswer | 2.20 | 0.22 | +1.98 | 19% |
| FindHazardByTag (incl. GetHazardRowNames) | 0.56 | 0.66 | -0.10 | -1% |
| GetEdictRowNames | 0.67 | 0.00 | +0.67 | 6% |
| treaty partner search (measured lap) | 2.21 | 0.00 | +2.21 | 21% |
| modelled sum | 17.29 | 8.15 | +9.13 | 89% |
| rest (world divergence, other work, noise) | | | +1.19 | 11% |
| GetFeatRowNames share of the whole on stage | 39% | 39% | | |

lexical compare 12.7 ns; Cycles64 pair 29.9 ns; askers at the end 3

## idle3000 level 0: medians of 3 on / 3 off

- AI stage: on 18.02 / off 11.34 us/tick; diff +6.68; ratio 1.589
- economy: on 1.63 / off 1.65; AI/economy on 11.04 / off 6.88 (ratio of ratios 1.605); off AI rescaled by economy on/off: 11.20
- crisis: on 10.30 / off 9.53; tick mean on 33.1 / off 25.8
## idle3000 level 0 proto: medians of 3 on / 3 off

- AI stage: on 7.31 / off 5.61 us/tick; diff +1.70; ratio 1.303
- economy: on 1.60 / off 1.57; AI/economy on 4.57 / off 3.60 (ratio of ratios 1.270); off AI rescaled by economy on/off: 5.72
- crisis: on 8.63 / off 7.76; tick mean on 20.6 / off 17.9
## idle3000 level 1: medians of 3 on / 3 off

- AI stage: on 19.33 / off 12.87 us/tick; diff +6.46; ratio 1.502
- economy: on 1.60 / off 1.58; AI/economy on 12.08 / off 8.09 (ratio of ratios 1.493); off AI rescaled by economy on/off: 13.03
- crisis: on 9.81 / off 9.03; tick mean on 33.8 / off 26.6

| block | on | off | on - off | share of the AI-stage diff |
|---|---|---|---|---|
| choices | 5.463 | 3.717 | +1.746 | 27% |
| settle | 0.530 | 0.542 | -0.012 | -0% |
| revoke | 0.215 | 0.212 | +0.003 | 0% |
| laws | 0.202 | 0.202 | +0.000 | 0% |
| treaty | 5.266 | 2.761 | +2.505 | 39% |
| diplomacy | 1.131 | 1.151 | -0.020 | -0% |
| feats | 6.236 | 4.033 | +2.203 | 34% |
| (partner, of treaty) | 0.275 | 0.000 | +0.275 | 4% |
| (aim, of diplomacy) | 0.707 | 0.731 | -0.024 | -0% |
| (brake, of feats) | 0.058 | 0.060 | -0.002 | -0% |
| (rows, of feats) | 5.410 | 3.201 | +2.209 | 34% |
| laps sum | 19.065 | 12.619 | +6.446 | 100% |
| stage - laps (loop, census, probe) | 0.265 | 0.251 | +0.014 | |

AI-stage calls per tick (median): aimCall 11.64/11.64, aimScan 3.12/3.12, aimSiegeChecks 0.23/0.23, asksLawsCall 11.64/11.64, asksTreatyCall 11.64/11.64, asksTreatyTrue 0.95/0.00, choicesCall 11.64/11.64, choicesCapital 6.84/6.84, featNames 30.48/30.48, findArch 0.00/0.00, findEdict 0.00/0.00, findFeat 150.46/91.42, findGalaxy 1.70/1.70, findHazard 32.95/22.29, findProject 1.20/1.20, findTrans 0.03/0.03, hazardAnswer 6.87/5.17, hazardByTag 1.10/1.10, hazardNames 1.10/1.10, judgeCall 12.00/12.00, judgeCheckAll 23.02/18.89, markNeighbours 5.01/3.12, partnerCands 1.06/0.00, startBlock 0.02/0.02

Crisis-stage calls per tick (median, on/off): eventNames 0.50/0.50, findArch 14.54/14.54, findEdict 13.71/10.28, findEvent 31.21/28.81, findHazard 153.93/129.11, findProject 23.21/23.21, findTrans 1.94/1.94, hazardByTag 0.69/0.69, hazardNames 1.69/1.69

Census on: T1 5.222 T2 4.717 T3 1.703 T4 0.000 T5 0.000; T3 bare 0.955; blocks 0=4 7=399 8=2782

Census off: T1 5.222 T2 4.717 T3 1.703 T4 0.000 T5 0.000; T3 bare 1.703; blocks

Same-state bench (final state of the on runs; min of 3 rounds; median over runs): ns per call / per tick-equivalent

| item | live (on tables) | copy (on, copied) | off (copied) | copy - off (B6b data's code cost on this state) |
|---|---|---|---|---|
| featNames | 348.6 | 348.5 | 175.1 | +173.4 |
| featRowNamesNoSort | 142.2 | 141.9 | 82.1 | +59.8 |
| hazardNames | 456.1 | 457.6 | 354.4 | +103.2 |
| edictNames | 678.8 | 677.5 | 567.9 | +109.6 |
| eventNames | 1088.6 | 1080.6 | 1028.3 | +52.3 |
| findFeat | 10.6 | 10.5 | 10.9 | -0.4 |
| hazardByTagAvg | 2959.8 | 3003.8 | 2388.4 | +615.4 |
| hazardAnswerMiss | 204.1 | 200.6 | 181.7 | +18.9 |
| hazardAnswerArk | 206.7 | 206.3 | 181.4 | +24.9 |
| tickChoices | 9597.9 | 9515.7 | 4950.0 | +4565.7 |
| tickAsksTreaty | 5012.9 | 4961.1 | 2524.3 | +2436.8 |
| tickPartnerOfLiveAskers | 785.0 | 781.1 | 778.3 | +2.8 |
| tickAim | 254.8 | 253.5 | 254.9 | -1.4 |
| tickLaws | 23.9 | 23.8 | 23.8 | +0.0 |
| tickJudge | 6011.1 | 5990.2 | 3344.8 | +2645.4 |

Attribution model (us/tick; bench ns/call x the run's AI-stage calls/tick; the partner search is its lap):

| part | on | off | on - off | share of the AI-stage diff |
|---|---|---|---|---|
| GetFeatRowNames (alloc + TSet + sort) | 10.62 | 5.34 | +5.29 | 82% |
| FindFeat calls | 1.58 | 0.96 | +0.62 | 10% |
| IsHazardAnswer | 1.38 | 0.94 | +0.44 | 7% |
| FindHazardByTag (incl. GetHazardRowNames) | 0.55 | 0.44 | +0.11 | 2% |
| GetEdictRowNames | 0.00 | 0.00 | +0.00 | 0% |
| treaty partner search (measured lap) | 0.28 | 0.00 | +0.28 | 4% |
| modelled sum | 14.41 | 7.68 | +6.73 | 104% |
| rest (world divergence, other work, noise) | | | -0.27 | -4% |
| GetFeatRowNames share of the whole on stage | 55% | 41% | | |

lexical compare 12.7 ns; Cycles64 pair 30.0 ns; askers at the end 2

## idle3000 level 1 sanity: medians of 1 on / 1 off

- AI stage: on 19.94 / off 12.48 us/tick; diff +7.46; ratio 1.598
- economy: on 1.68 / off 1.54; AI/economy on 11.87 / off 8.10 (ratio of ratios 1.465); off AI rescaled by economy on/off: 13.61
- crisis: on 10.36 / off 8.74; tick mean on 35.3 / off 25.8

| block | on | off | on - off | share of the AI-stage diff |
|---|---|---|---|---|
| choices | 5.429 | 3.560 | +1.869 | 25% |
| settle | 0.544 | 0.511 | +0.033 | 0% |
| revoke | 0.210 | 0.214 | -0.004 | -0% |
| laws | 0.208 | 0.202 | +0.006 | 0% |
| treaty | 5.552 | 2.736 | +2.816 | 38% |
| diplomacy | 1.140 | 1.116 | +0.024 | 0% |
| feats | 6.599 | 3.892 | +2.707 | 36% |
| (partner, of treaty) | 0.279 | 0.000 | +0.279 | 4% |
| (aim, of diplomacy) | 0.700 | 0.697 | +0.003 | 0% |
| (brake, of feats) | 0.055 | 0.060 | -0.005 | -0% |
| (rows, of feats) | 5.754 | 3.078 | +2.676 | 36% |
| laps sum | 19.682 | 12.232 | +7.450 | 100% |
| stage - laps (loop, census, probe) | 0.258 | 0.248 | +0.010 | |

AI-stage calls per tick (median): aimCall 11.64/11.64, aimScan 3.12/3.12, aimSiegeChecks 0.23/0.23, asksLawsCall 11.64/11.64, asksTreatyCall 11.64/11.64, asksTreatyTrue 0.95/0.00, choicesCall 11.64/11.64, choicesCapital 6.84/6.84, featNames 30.48/30.48, findArch 0.00/0.00, findEdict 0.00/0.00, findFeat 150.46/91.42, findGalaxy 1.70/1.70, findHazard 32.95/22.29, findProject 1.20/1.20, findTrans 0.03/0.03, hazardAnswer 6.87/5.17, hazardByTag 1.10/1.10, hazardNames 1.10/1.10, judgeCall 12.00/12.00, judgeCheckAll 23.02/18.89, markNeighbours 5.01/3.12, partnerCands 1.06/0.00, startBlock 0.02/0.02

Crisis-stage calls per tick (median, on/off): eventNames 0.50/0.50, findArch 14.54/14.54, findEdict 13.71/10.28, findEvent 31.21/28.81, findHazard 153.93/129.11, findProject 23.21/23.21, findTrans 1.94/1.94, hazardByTag 0.69/0.69, hazardNames 1.69/1.69

Census on: T1 5.222 T2 4.717 T3 1.703 T4 0.000 T5 0.000; T3 bare 0.955; blocks 0=4 7=399 8=2782

Census off: T1 5.222 T2 4.717 T3 1.703 T4 0.000 T5 0.000; T3 bare 1.703; blocks

Same-state bench (final state of the on runs; min of 3 rounds; median over runs): ns per call / per tick-equivalent

| item | live (on tables) | copy (on, copied) | off (copied) | copy - off (B6b data's code cost on this state) |
|---|---|---|---|---|
| featNames | 348.1 | 345.0 | 176.0 | +169.0 |
| featRowNamesNoSort | 143.3 | 140.6 | 83.7 | +56.9 |
| hazardNames | 458.9 | 456.5 | 360.2 | +96.3 |
| edictNames | 682.0 | 679.8 | 564.4 | +115.4 |
| eventNames | 1088.1 | 1095.2 | 1041.9 | +53.3 |
| findFeat | 10.7 | 10.6 | 11.2 | -0.6 |
| hazardByTagAvg | 2953.2 | 2928.8 | 2368.3 | +560.5 |
| hazardAnswerMiss | 199.2 | 198.0 | 181.7 | +16.3 |
| hazardAnswerArk | 199.2 | 198.2 | 181.3 | +16.9 |
| tickChoices | 9427.1 | 9391.8 | 4909.6 | +4482.2 |
| tickAsksTreaty | 5706.2 | 5262.0 | 2728.4 | +2533.6 |
| tickPartnerOfLiveAskers | 793.9 | 783.5 | 779.0 | +4.5 |
| tickAim | 250.0 | 248.1 | 247.3 | +0.8 |
| tickLaws | 24.0 | 23.9 | 23.9 | +0.0 |
| tickJudge | 6030.5 | 5989.0 | 3364.6 | +2624.4 |

Attribution model (us/tick; bench ns/call x the run's AI-stage calls/tick; the partner search is its lap):

| part | on | off | on - off | share of the AI-stage diff |
|---|---|---|---|---|
| GetFeatRowNames (alloc + TSet + sort) | 10.52 | 5.36 | +5.15 | 69% |
| FindFeat calls | 1.59 | 0.97 | +0.63 | 8% |
| IsHazardAnswer | 1.36 | 0.94 | +0.42 | 6% |
| FindHazardByTag (incl. GetHazardRowNames) | 0.54 | 0.44 | +0.10 | 1% |
| GetEdictRowNames | 0.00 | 0.00 | +0.00 | 0% |
| treaty partner search (measured lap) | 0.28 | 0.00 | +0.28 | 4% |
| modelled sum | 14.29 | 7.71 | +6.58 | 88% |
| rest (world divergence, other work, noise) | | | +0.88 | 12% |
| GetFeatRowNames share of the whole on stage | 53% | 43% | | |

lexical compare 12.9 ns; Cycles64 pair 30.5 ns; askers at the end 2

## ruler20000 level 0: medians of 3 on / 3 off

- AI stage: on 20.37 / off 11.49 us/tick; diff +8.88; ratio 1.773
- economy: on 2.78 / off 3.28; AI/economy on 7.35 / off 3.50 (ratio of ratios 2.099); off AI rescaled by economy on/off: 9.74
- crisis: on 27.11 / off 13.42; tick mean on 61.1 / off 37.6
## ruler20000 level 0 proto: medians of 3 on / 3 off

- AI stage: on 10.03 / off 5.90 us/tick; diff +4.13; ratio 1.700
- economy: on 2.70 / off 3.41; AI/economy on 3.76 / off 1.73 (ratio of ratios 2.173); off AI rescaled by economy on/off: 4.67
- crisis: on 25.10 / off 12.12; tick mean on 44.9 / off 28.0
## ruler20000 level 1: medians of 3 on / 3 off

- AI stage: on 21.29 / off 13.30 us/tick; diff +7.99; ratio 1.601
- economy: on 2.63 / off 3.25; AI/economy on 8.01 / off 4.10 (ratio of ratios 1.954); off AI rescaled by economy on/off: 10.76
- crisis: on 26.54 / off 13.28; tick mean on 60.8 / off 39.1

| block | on | off | on - off | share of the AI-stage diff |
|---|---|---|---|---|
| choices | 8.191 | 3.863 | +4.328 | 54% |
| settle | 1.277 | 0.898 | +0.379 | 5% |
| revoke | 0.224 | 0.200 | +0.024 | 0% |
| laws | 1.273 | 0.193 | +1.080 | 14% |
| treaty | 5.490 | 2.511 | +2.979 | 37% |
| diplomacy | 1.270 | 1.004 | +0.266 | 3% |
| feats | 3.214 | 4.288 | -1.074 | -13% |
| (partner, of treaty) | 0.526 | 0.000 | +0.526 | 7% |
| (aim, of diplomacy) | 0.774 | 0.543 | +0.231 | 3% |
| (brake, of feats) | 0.268 | 0.311 | -0.043 | -1% |
| (rows, of feats) | 2.020 | 2.876 | -0.856 | -11% |
| laps sum | 20.939 | 12.969 | +7.970 | 100% |
| stage - laps (loop, census, probe) | 0.351 | 0.331 | +0.020 | |

AI-stage calls per tick (median): aimCall 11.00/11.00, aimScan 1.23/0.46, aimSiegeChecks 0.06/0.06, asksLawsCall 11.00/11.00, asksLawsTrue 0.82/0.00, asksTreatyCall 11.00/11.00, asksTreatyTrue 0.87/0.00, choicesCall 11.00/11.00, choicesCapital 10.21/10.31, edictNames 0.82/0.00, enactBlock 2.45/0.00, featNames 25.95/33.31, findArch 0.00/0.00, findEdict 15.53/0.00, findFeat 125.55/99.93, findGalaxy 8.66/9.49, findHazard 65.25/5.54, findProject 3.20/2.35, findTrans 0.00/0.00, hazardAnswer 11.00/1.00, hazardByTag 0.83/1.41, hazardNames 0.83/1.41, judgeBraked 8.08/0.00, judgeCall 12.00/12.00, judgeCheckAll 10.02/3.22, markNeighbours 3.78/0.46, partnerCands 1.69/0.00, startBlock 0.76/0.10

Crisis-stage calls per tick (median, on/off): eventNames 0.50/0.50, findArch 41.11/38.05, findEdict 901.47/3.81, findEvent 31.20/28.81, findHazard 322.99/252.53, findProject 7.80/7.86, findTrans 0.70/1.86, hazardByTag 1.78/2.13, hazardNames 2.78/3.13

Census on: T1 0.751 T2 0.770 T3 8.662 T4 0.817 T5 0.000; T3 bare 0.872; blocks 0=2 7=199 8=33533

Census off: T1 0.751 T2 0.763 T3 9.486 T4 0.000 T5 0.000; T3 bare 3.470; blocks

Same-state bench (final state of the on runs; min of 3 rounds; median over runs): ns per call / per tick-equivalent

| item | live (on tables) | copy (on, copied) | off (copied) | copy - off (B6b data's code cost on this state) |
|---|---|---|---|---|
| featNames | 361.2 | 358.2 | 179.9 | +178.3 |
| featRowNamesNoSort | 145.4 | 143.1 | 84.9 | +58.2 |
| hazardNames | 458.4 | 452.7 | 357.9 | +94.8 |
| edictNames | 688.4 | 701.7 | 577.3 | +124.4 |
| eventNames | 1074.1 | 1083.7 | 1029.2 | +54.5 |
| findFeat | 10.7 | 10.6 | 11.3 | -0.7 |
| hazardByTagAvg | 2974.3 | 2996.2 | 2390.8 | +605.4 |
| hazardAnswerMiss | 200.6 | 199.2 | 182.1 | +17.1 |
| hazardAnswerArk | 204.0 | 197.5 | 181.6 | +15.9 |
| tickChoices | 8318.3 | 8226.8 | 3386.0 | +4840.8 |
| tickAsksTreaty | 4670.0 | 4632.8 | 2288.8 | +2344.0 |
| tickPartnerOfLiveAskers | 526.8 | 522.7 | 522.9 | -0.2 |
| tickAim | 539.6 | 539.0 | 538.4 | +0.6 |
| tickLaws | 1246.9 | 1239.0 | 236.1 | +1002.9 |
| tickJudge | 1174.3 | 1170.7 | 782.6 | +388.1 |

Attribution model (us/tick; bench ns/call x the run's AI-stage calls/tick; the partner search is its lap):

| part | on | off | on - off | share of the AI-stage diff |
|---|---|---|---|---|
| GetFeatRowNames (alloc + TSet + sort) | 9.29 | 5.99 | +3.30 | 41% |
| FindFeat calls | 1.33 | 1.06 | +0.27 | 3% |
| IsHazardAnswer | 2.19 | 0.18 | +2.01 | 25% |
| FindHazardByTag (incl. GetHazardRowNames) | 0.41 | 0.56 | -0.15 | -2% |
| GetEdictRowNames | 0.57 | 0.00 | +0.57 | 7% |
| treaty partner search (measured lap) | 0.53 | 0.00 | +0.53 | 7% |
| modelled sum | 14.33 | 7.80 | +6.53 | 82% |
| rest (world divergence, other work, noise) | | | +1.46 | 18% |
| GetFeatRowNames share of the whole on stage | 44% | 45% | | |

lexical compare 12.7 ns; Cycles64 pair 30.0 ns; askers at the end 1

