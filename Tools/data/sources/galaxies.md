# Sources of the galaxy rows (DT_Galaxies) and their places on the universe level

Phase 3 work item 9 (guideline 5.11, 6.4 '우주 화면'). Not read by the checker, the stamp or any test (they read the top-level
`schema.json`, `DT_*.json` and `ST_*.csv`); the input of v0.8 step 5's coordinate tool. Every printed value of a row (its
`DistanceText` and `FeatureText`) and every value a place or a size comes from was read on **2026-10-18** from the page or
service named in the row's block below (copied from the fact check `Saved/Design/phase3_universe/galaxies_facts.md`, which
also lists what could not be retrieved). Places and sizes are never printed.

## The frame

- **Unit.** One home-galaxy radius: the home row's `RadiusLy` 50,000 ly. Before v0.8 step 5 the map's disc is drawn as the
  home galaxy itself (sol at `HomeRadiusShare` 0.53, as the Sun at about 26,500 of 50,000 ly), so one unit is the disc
  radius in world units (`CosmicUniverseLook::FrameOf`).
- **Origin and axes.** `MapPosition` is (X, Y) in units from the home galaxy's centre, in the map's own X and Y:
  world = centre + (X, Y, 0) x unit. The home row stands at the centre whatever it holds; any other row at (0,0), not a
  number, inside the home (under 1 unit) or farther than 20 units is not drawn (`CosmicUniverseLook::HasPlace`).
- **Distance.** r = 1 + ln(d / 50,000 ly), d the central value of the row's `DistanceText` (the midpoint of a range - one exception: the Sagittarius dwarf's text is told from the Sun, d 85,000 ly, while the rule measures from the home's centre, where McC12 gives D(MW) 18 kpc, about 59,000 ly, r about 1.17; the row keeps the text's 1.53 so its sprite stands apart from the home's core, and the frame mismatch is accepted - the place is never printed):
  continuous with the home's edge and with the disc's own slope there. Compressed: never a measure, no number on the
  screen comes from it.
- **Direction.** The galactic longitude l, growing clockwise as the camera sees it: X = -r cos l, Y = -r sin l. In map
  axes: l = 0 along -X, l = 90 along -Y. At yaw 0 +X is up the screen and +Y right, so l = 0 points from sol toward the
  centre (down the screen): the universe is seen from the south galactic pole's side, the mirror of the usual maps. Why:
  the named galaxies crowd at l about 90-160 (the Local Group, the M81 group, NGC 253, M101, M51) and l about 280-310 (the
  Magellanic Clouds, Centaurus A, M104, M87); clockwise puts them at the upper left and the lower right, the corners the
  HUD leaves free. Latitude is dropped (an azimuthal schematic).
- **Size.** The sprite's radius is 1.6 x sqrt(RadiusLy / 43,500) home units within [0.3, 2.6] (43,500 = the home's D25
  radius, guideline 3.4 '약 8만 7천 광년'; the home is drawn at 1.6 by definition); a row with `RadiusLy` 0 takes its kind's
  share (Dwarf 0.45, Spiral 1.3, GrandSpiral 1.9, Elliptical 1.6).
- **RadiusLy.** 1/2 x D25 (RC3, VizieR VII/155) in radians x the report's central distance of the row: RadiusLy =
  D25[arcmin] x 1.454441e-4 x d[ly], rounded to 100 ly. 0 where no D25 is sourced (the Sagittarius dwarf).
- **Nudges.** By hand where two galaxies would stand on one place or a label would cover another galaxy's core: keep a
  group's order and rough direction, keep every row's r unless written (the distance order kept), clear the half-bright
  cores (0.3 x the sprite) by 0.1 units - `UI.Universe` asserts centre distance >= 0.3 x (r1 + r2) + 0.1 on the game's
  rows - and keep the labels shown at the far and the near stop off the other galaxies' half-bright cores and off the HUD
  (an offline estimate at 1280x720, pitch -59, 90 degrees across, 13.5 px a Hangul syllable and 7 px a Latin one; the
  off-screen screenshots are the real check). Recorded per row: the rule's (r, l) and the used (r, l).
- **What stays under the panel at the near stop.** At the universe's near stop with the focus on the centre, the Fornax
  dwarf's label reaches under the galaxy panel (top right); every other label shown there is clear (the estimate).

## The rows

| Row | l (deg) | d of the text (ly) | rule r | used r | used l | `MapPosition` | sprite (units) | `RadiusLy` | distance order kept | nudge - why |
|---|---|---|---|---|---|---|---|---|---|---|
| `gal_milkyway` | - | - | - | - | - | (0, 0) | 1.60 | 50,000 (the unit) | - | the home: the centre whatever it holds |
| `gal_sgr_dsph` | 5.57 | 85,000 | 1.53 | = | = | (-1.52, -0.15) | 0.45 | 0 | yes | - |
| `gal_lmc` | 280.47 | 160,000 | 2.16 | = | = | (-0.39, 2.13) | 0.95 | 15,300 | yes | - |
| `gal_smc` | 302.80 | 200,000 | 2.39 | = | = | (-1.29, 2.01) | 0.74 | 9,200 | yes | - |
| `gal_sculptor_dsph` | 287.54 | 280,000 | 2.72 | = | = | (-0.82, 2.60) | 0.31 | 1,600 | yes | - |
| `gal_fornax_dsph` | 237.10 | 470,000 | 3.24 | = | = | (1.76, 2.72) | 0.30 | 1,200 | yes | - |
| `gal_ngc6822` | 25.34 | 1,600,000 | 4.47 | = | = | (-4.04, -1.91) | 0.45 | 3,500 | yes | - |
| `gal_ic1613` | 129.74 | 2,400,000 | 4.87 | = | 166 | (4.73, -1.18) | 0.57 | 5,600 | yes | off M31 and M33 by angle only (its r kept between NGC 6822's and M31's) |
| `gal_andromeda` | 121.17 | 2,500,000 | 4.91 | = | = | (2.54, -4.20) | 2.04 | 70,400 | yes | - |
| `gal_m32` | 121.15 | 2,500,000 | 4.91 | = | 111 | (1.76, -4.59) | 0.43 | 3,200 | yes | beside M31's core (its satellite) |
| `gal_m110` | 120.72 | 2,700,000 | 4.99 | = | 104 | (1.21, -4.84) | 0.71 | 8,600 | yes | beside M31 past M32 (its satellites), by angle only - its core and M31's label kept apart |
| `gal_triangulum` | 133.61 | 2,700,000 | 4.99 | = | 148 | (4.23, -2.64) | 1.30 | 28,800 | yes (the same r as M110 - both texts' centres are 270만) | off M31, its label clear of M81's core |
| `gal_ngc253` | 97.37 | 11,000,000 | 6.39 | = | 82 | (-0.89, -6.33) | 1.64 | 45,700 | yes | off M101, its label clear of M101's core (b -88: near the pole its longitude means little) |
| `gal_m81` | 142.09 | 12,000,000 | 6.48 | = | 136 | (4.66, -4.50) | 1.65 | 46,400 | yes | its label and core clear of M33's label (the pair kept together) |
| `gal_m82` | 141.41 | 12,000,000 | 6.48 | = | 158 | (6.01, -2.43) | 1.08 | 19,700 | yes | beside M81 on its far side (its pair), its label clear of M31's |
| `gal_cen_a` | 309.52 | 12,500,000 | 6.52 | = | = | (-4.15, 5.03) | 1.65 | 46,300 | yes | - |
| `gal_m101` | 102.04 | 22,000,000 | 7.09 | = | = | (1.48, -6.93) | 2.33 | 92,100 | yes | - |
| `gal_m51` | 104.85 | 27,000,000 | 7.29 | = | 115 | (3.08, -6.61) | 1.64 | 45,700 | yes | off M101 (b +69 and +60) |
| `gal_m104` | 298.46 | 30,000,000 | 7.40 | = | 295 | (-3.13, 6.70) | 1.52 | 39,500 | yes | its core clear of Centaurus A's label |
| `gal_m87` | 283.78 | 53,000,000 | 7.97 | = | = | (-1.90, 7.74) | 1.95 | 64,700 | yes | - |

Notes on the table:
- `gal_m110` and `gal_m32` stand on the same side of M31 (M32 nearer its core), by angle only: M31's label stays off their cores.
- `RadiusLy` of `gal_m81` is 46,400 by the rule (26.92 x 1.454441e-4 x 11,839,600 = 46,357); the fact check's summary
  table rounded it to 46,300.
- The kinds: M82 is drawn and named a spiral (SIMBAD "S", HyperLEDA "S?", near-infrared spiral arms - Mayya 2005) though
  RC3 and NED give I0; Centaurus A is an elliptical (ESO "massive elliptical") though RC3 gives S0 pec; M104 is a spiral
  (RC3 SA(s)a) though it is read as an elliptical with a disc (McQuinn 2016; Gadotti & Sanchez-Janssen 2012). The three
  `FeatureText`s end with "(분류는 논쟁 중)".

## Per row: the retrieved values (copied from the fact check)

### gal_andromeda — M31
- **NED preferred name:** `Messier 031` — NED-OL (name "M31"): `"Interpreted": {"Name": "MESSIER 031"}`, `"Preferred": {"Name": "Messier 031"}`. NED-D (`?name=M31`): "M31 is also known as MESSIER 031".
- **l, b = 121.174405, −21.572936** — NED-TAP row: `58141,"Messier 031","G",10.68479291667,41.269065,121.174405026715,-21.5729359805299,13,407,199`. Cross-check SIMBAD (`Ident=M31`): "Coordinates(Gal,ep=J2000,eq=2000): 121.174329  -21.573309".
- **Distance (glance):** NED-D (`?name=M31`): "407 Distances found in NED"; Metric Distance (Mpc) "Mean 0.784 | Std. Dev. 0.120 | Min. 0.440 | Max. 2.800 | Median 0.779" → median **2,540,800 ly**. McC12 row: "Andromeda … (m-M) 24.47 0.07 0.07 D 783 25 25" (kpc) → 2,553,800 ly. Consistent with "about 2.5 million ly".
- **D25 = 190.55′** — RC3 row: "NGC 224 | UGC 454 | PGC 2557 | 121.17 | -21.57 | .SAS3.. | 3.0 | 0.3 | 3.28 | | 0.01 | 0.49" (log D25 = 3.28 ± 0.01). LEDA (`o=M31`): "logd25 | 3.25 ± 0.01 | log(0.1 arcmin)" → 177.83′.
- **RadiusLy (RC3, NED median distance) = 70,400 ly** (HyperLEDA d25: 65,700).
- **Type:** RC3 `.SAS3..` = SA(s)b, T = 3.0; LEDA "type | Sb", "t | 3.0 ± 0.4"; SIMBAD "Morphological type: SA(s)b D 2013AJ....146...67B". Not disputed.
- **Printed:** `DistanceText` "우리은하에서 약 250만 광년"; `FeatureText` "질량은 우리은하와 비슷하다는 연구가 많다".

### gal_triangulum — M33
- **NED preferred name:** `Messier 033` — NED-OL: `"Interpreted": {"Name": "MESSIER 033"}`, `"Preferred": {"Name": "Messier 033"}`; NED-D: "M33 is also known as MESSIER 033".
- **l, b = 133.610161, −31.330587** — NED-TAP: `58193,"Messier 033","G",23.46204166667,30.66022222222,133.610161071486,-31.3305874625942,13,137,130`. SIMBAD: "Coordinates(Gal,ep=J2000,eq=2000): 133.6101946984090  -31.3306789592433".
- **Distance (glance):** NED-D (`?name=M33`, second attempt; the first returned HTTP 503): "137 Distances found in NED"; "Mean 0.869 | Std. Dev. 0.212 | Min. 0.600 | Max. 3.050 | Median 0.859" → median **2,801,700 ly**. McC12: "Triangulum … 24.54 0.06 0.06 809 22 22" → 2,638,600 ly. TRGB row in NED-D: "24.54 ; 0.06 ; 0.809 ; TRGB ; 2005MNRAS.356..979M". Range of central values 264만–280만 ly.
- **D25 = 70.79′** — RC3: "NGC 598 | UGC 1117 | PGC 5818 | 133.61 | -31.33 | .SAS6.. | 6.0 | 0.3 | 2.85 | | 0.01 | 0.23". LEDA: "logd25 | 2.79 ± 0.01" → 61.66′.
- **RadiusLy (RC3, NED median) = 28,800 ly** (with McC12 809 kpc: 27,200).
- **Type:** RC3 `.SAS6..` = SA(s)cd, T = 6.0; LEDA "Sc", t 5.9 ± 0.4; SIMBAD "SA(s)cd D 2013AJ....146...67B". Not disputed.
- **Feature (optional) "국부은하군에서 셋째로 큰 은하": supported.** NASA Hubble Messier 33 page, https://science.nasa.gov/mission/hubble/science/explore-the-night-sky/hubble-messier-catalog/messier-33/ : "M33 is the third-largest member of our Local Group of galaxies following the Andromeda galaxy (M31) and the Milky Way."
- **Printed:** `DistanceText` "우리은하에서 약 260만 ~ 280만 광년" (the two central values above, 264만 rounded to two figures under "약"; item 8's "약 270만 ~ 290만" had no source for 290만 - changed by item 9's review); `FeatureText` "국부은하군에서 셋째로 큰 은하".

### gal_lmc — Large Magellanic Cloud
- **NED preferred name:** `Large Magellanic Cloud` — NED-OL (name "LMC"): `"Interpreted": {"Name": "LMC"}`, `"Preferred": {"Name": "Large Magellanic Cloud"}`.
- **l, b = 280.465262, −32.888246** — NED-TAP: `58310,"Large Magellanic Cloud","G",80.89416666667,-69.75611111111,280.465262016725,-32.8882455699175,6,824,565`. SIMBAD: "Coordinates(Gal,ep=J2000,eq=2000): 280.4652  -32.8884".
- **Distance (glance):** NED-D (`?name=LMC`, second attempt): "824 Distances found in NED"; "Mean 0.049 | Std. Dev. 0.004 | Min. 0.033 | Max. 0.071 | Median 0.050" → **163,100 ly**. Example row: "18.49 ; 0.01 ; 0.050 ; Eclipsing Binary ; 2013Natur.495...76P". McC12: "LMC … 18.52 0.09 0.09 51 2 2" → 166,300 ly.
- **D25 = 645.65′** — RC3: "LMC | ESO 56-115 | PGC17223 | 280.47 | -32.89 | .SBS9.. | 9.0 | 0.3 | 3.81 | | 0.05 | 0.07". LEDA: "logd25 | 3.81 ± 0.01" (same 645.65′).
- **RadiusLy (RC3, NED median) = 15,300 ly.**
- **Type:** RC3 `.SBS9..` = SB(s)m, T = 9.0; LEDA "SBm", t 9.1 ± 0.5; SIMBAD "SB(s)m D 2013MNRAS.428.1927C"; McC12 "Irr". Not disputed.
- **Feature (optional) "우리은하를 도는 위성 은하 가운데 가장 크다": supported, with a caution on "도는".** NASA APOD 2008 April 9, https://science.nasa.gov/image-article/apod-2008-april-9-a-large-magellanic-cloud-deep-field/ : "it is the Large Magellanic Cloud (LMC), the largest satellite galaxy of our own Milky Way Galaxy". ESA, https://www.esa.int/ESA_Multimedia/Images/2021/08/Cluster_in_the_cloud : "It is considered a satellite galaxy because it is gravitationally bound to the Milky Way." But whether the Clouds have completed an orbit is open: Besla et al. 2007, ApJ 668, 949 (2007ApJ...668..949B, SREF): "We conclude that the LMC and SMC are either currently on their first passage about the MW or … their orbital period and apogalacticon distance must be a factor of 2 larger than previously estimated". Safer wording: **"우리은하의 위성 은하 가운데 가장 크다"**.
- **Printed:** `DistanceText` "우리은하에서 약 16만 광년"; `FeatureText` "우리은하의 위성 은하 가운데 가장 크다".

### gal_smc — Small Magellanic Cloud
- **NED preferred name:** `Small Magellanic Cloud` — NED-OL (name "SMC"): `"Interpreted": {"Name": "SMC"}`, `"Preferred": {"Name": "Small Magellanic Cloud"}`.
- **l, b = 302.796881, −44.299201** — NED-TAP: `58152,"Small Magellanic Cloud","G",13.1866666666667,-72.8286111111111,302.796881080246,-44.299201404284,3,294,285`. SIMBAD gives a different centre: "Coordinates(Gal,ep=J2000,eq=2000): 302.8084  -44.3277" (RC3: 302.81, −44.33). The two centres differ by about 0.03 deg.
- **Distance (glance):** NED-D (`?name=SMC`): "294 Distances found in NED"; "Mean 0.060 | Std. Dev. 0.007 | Min. 0.039 | Max. 0.098 | Median 0.061" → **199,000 ly**. McC12: "SMC … 19.03 0.12 0.12 64 4 4" → 208,700 ly.
- **D25 = 316.23′** — RC3: "SMC | ESO 29- 21 | PGC 3085 | 302.81 | -44.33 | .SBS9P. | 9.0 | 0.3 | 3.50 | | 0.02 | 0.23". LEDA: "logd25 | 3.48 ± 0.03" → 302.0′.
- **RadiusLy (RC3, NED median) = 9,200 ly.**
- **Type:** RC3 `.SBS9P.` = SB(s)m pec, T = 9.0; LEDA "SBm", t 8.9 ± 0.3; SIMBAD "dIrr D 2012AJ....144....4M"; McC12 "dIrr". Not disputed.
- **Feature (optional) "대마젤란은하와 짝을 이루어 우리은하를 돈다": needs rewording.** The pairing is observed, but an orbit around the Milky Way is not established. Besla et al. 2012, MNRAS 421, 2109 (2012MNRAS.421.2109B, SREF): "cosmological simulations of hierarchical structure formation favour a scenario where the Magellanic Clouds (MCs) are currently on their first infall towards our Galaxy. We illustrate here that the observed irregular morphology and internal kinematics of the Magellanic System … are naturally explained by interactions between the LMC and SMC". Proposed: **"대마젤란은하와 짝을 이루어 서로 끌어당기는 이웃 은하"**.
- **Printed:** `DistanceText` "우리은하에서 약 20만 광년"; `FeatureText` "대마젤란은하와 짝을 이루어 서로 끌어당기는 이웃 은하".

### gal_m87 — M87
- **NED preferred name:** `Messier 087` — NED-OL: `"Interpreted": {"Name": "MESSIER 087"}`, `"Preferred": {"Name": "Messier 087"}`; NED-D: "M87 is also known as MESSIER 087".
- **l, b = 283.777799, +74.491131** — NED-TAP: `57052,"Messier 087","G",187.705930779167,12.3911232583333,283.777798792116,74.491131082717,14,118,96`. SIMBAD: "Coordinates(Gal,ep=J2000,eq=2000): 283.77775499474  +74.49115452365".
- **Distance (glance):** NED-D (`?name=M87`): "118 Distances found in NED"; "Mean 16.757 | Std. Dev. 2.949 | Min. 9.200 | Max. 29.100 | Median 16.400" → **53,490,000 ly** (≈ 5,300만 ly). LEDA: "mod0 | 31.12 ± 0.06" (16.7 Mpc).
- **D25 = 8.32′** — RC3: "NGC 4486 | UGC 7654 | PGC41361 | 283.78 | 74.49 | .E+0+P. | -4.0 | 0.3 | 1.92 | | 0.01 | 0.10". LEDA: "logd25 | 1.85 ± 0.02" → 7.08′.
- **RadiusLy (RC3, NED median) = 64,700 ly** (HyperLEDA d25: 55,100).
- **Type:** RC3 coded `.E+0+P.` (elliptical, flagged peculiar), T = −4.0; LEDA "E", t −4.3 ± 0.6; SIMBAD "E-E/S0 C 2014A&A...569A.124V". Not disputed.
- **Printed:** `DistanceText` "우리은하에서 약 5,300만 광년 — 국부은하군 밖"; `FeatureText` "처녀자리 은하단의 중심에 선 거대 타원은하".

### gal_sgr_dsph — Sagittarius Dwarf Spheroidal (not SagDIG)
- **NED preferred name:** `Sagittarius Dwarf SPHEROIDAL` — NED-OL (names "Sagittarius Dwarf Spheroidal" and "Sgr dSph" both): `"Preferred": {"Name": "Sagittarius Dwarf SPHEROIDAL"}`, position RA 283.83125, Dec −30.545278. NED-D heading: "Distance Results for Sagittarius Dwarf Spheroidal".
- **l, b = 5.568865, −14.166338** — NED-TAP: `326262,"Sagittarius Dwarf SPHEROIDAL","G",283.83125,-30.545278,5.568865,-14.166338,0,66,49` (note `n_ddf = 0`: NED holds no diameter data for this object). SIMBAD (`Ident=Sgr dSph` → "Object NAME SDG"): "Coordinates(Gal,ep=J2000,eq=2000): 005.5682  -14.1648". McC12: GLON 5.6, GLAT −14.2.
- **Distance (from the Sun):**
  - Monaco et al. 2004, MNRAS 353, 874 (**2004MNRAS.353..874M**), SREF abstract: "The obtained distance modulus is ( m - M ) 0 = 17.10±0.15, corresponding to a heliocentric distance D = 26.30±1.8Kpc." → **85,780 ly** (79,900–91,700 ly). The same value is listed in NED-D: "17.10 ; 0.15 ; 0.026 ; TRGB ; 2004MNRAS.353..874M ; D(helio)" and adopted by McC12: "Sagittarius dSph … 17.10 0.15 0.15 26 2 2 … D(MW) 18" (Ref2 "3,4" = Monaco et al. / Ibata et al.).
  - NED-D (`?name=Sagittarius Dwarf Spheroidal`): "66 Distances found in NED"; "Mean 0.028 | Std. Dev. 0.004 | Min. 0.019 | Max. 0.045 | Median 0.028" → 91,300 ly (the list mixes the main body with Sgr globular clusters and stream debris).
  - Discovery-era value in NED-D: "16.99 ; ; 0.025 ; CMD ; 1994Nat...370..194I" → 81,600 ly.
  - **Verdict:** the draft "태양에서 약 8만 광년" matches only the 1994 discovery value; the modern TRGB value is 26.3 kpc. **Proposed: "태양에서 약 8만 6천 광년"** (or, more coarsely, "약 9만 광년").
- **D25:** **not in RC3** (the RC3 query within 1 deg returned only ESO 458-10, 3375″ away; Sgr was discovered after RC3). NED has no diameters (`n_ddf = 0`). HyperLEDA (`o=Sgr dSph` → "SAGITTARIUS_DWARF_SPHEROIDAL", pgc 4689212) lists "logd25 | 3.65 ± 0.02 | log(0.1 arcmin)" (446.7′), but the same page gives "bri25 | 26.46 | mag/arcsec2 | Mean surface brightness within isophote 25", i.e. fainter than the 25 mag isophote itself, so this is not a normal isophotal D25. **Recommendation: treat as no D25 → RadiusLy 0.** (If the HyperLEDA value were used: 5,600 ly.)
- **Type:** SIMBAD "Morphological type: dSph D 2012AJ....144....4M"; McC12 "dSph"; LEDA "type | E-S0", "t | -3.0 ± 2.0". Kind "Dwarf" fits. Not disputed.
- **Feature "우리은하 곁을 돌며 길게 흩어진 별의 흐름을 남긴다": supported.** Majewski et al. 2003, ApJ 599, 1082 (**2003ApJ...599.1082M**), SREF abstract: "A striking trailing tidal tail is found to extend from the Sgr center and arc across the south Galactic hemisphere … A prominent leading debris arm extends from the Sgr center northward of the Galactic plane", and "… as the former dwarf spiral/irregular satellite undergoes catastrophic disruption during its last orbits." Keep.
- **Printed:** `DistanceText` "태양에서 약 8만 ~ 9만 광년"; `FeatureText` "우리은하 곁을 돌며 길게 흩어진 별의 흐름을 남긴다".

### gal_sculptor_dsph — Sculptor Dwarf Spheroidal
- **NED preferred name:** `ESO 351- G 030` — NED-OL (name "Sculptor Dwarf Spheroidal"): `"Interpreted": {"Name": "Sculptor Dwarf Spheroidal"}`, `"Preferred": {"Name": "ESO 351- G 030"}`. NED-D heading: "Distance Results for Sculptor Dwarf Spheroidal".
- **l, b = 287.536545, −83.156693** — NED-TAP: `58157,"ESO 351- G 030","G",15.03875,-33.709,287.536545116916,-83.1566934176577,10,35,19`. SIMBAD (`Ident=ESO 351-30` → "NAME Sculptor Dwarf Galaxy"): "Coordinates(Gal,ep=J2000,eq=2000): 287.5334  -83.1568".
- **Distance:**
  - Pietrzyński et al. 2008, AJ 135, 1993 (**2008AJ....135.1993P**), SREF abstract: "obtain a true, reddening-corrected distance modulus of 19.67±0.02 (statistical) ±0.12 (systematic) mag for Sculptor" → 85.9 kpc → **280,200 ly**. NED-D row: "19.67 ; 0.02 ; 0.086 ; RR Lyrae ; 2008AJ....135.1993P ; 78, JK".
  - McC12: "Sculptor … 19.67 0.14 0.14 86 6 6 … D(MW) 86" → 260,900–300,100 ly (±6 kpc); Galactocentric distance equals heliocentric here.
  - NED-D (`?name=Sculptor Dwarf Spheroidal`): "35 Distances found in NED"; "Mean 0.082 | Std. Dev. 0.007 | Min. 0.062 | Max. 0.100 | Median 0.084" → 274,000 ly.
  - **Verdict:** "우리은하에서 약 29만 광년" (= 89 kpc) is about one sigma high. **Proposed: "우리은하에서 약 28만 광년".**
- **D25 = 39.81′** — RC3 row: "Sculptor | ESO 351- 30 | PGC 3589 | 287.53 | -83.16 | .E?.... | | | 2.60 | | 0.02 | 0.11". HyperLEDA lists "logd25 | 1.06 ± 0.08" (1.15′), which cannot describe this galaxy (McC12 half-light radius R1 = 11.30′); do not use it. **RadiusLy (RC3) = 1,600 ly.** (A dSph has no sharp 25-mag edge; McC12 gives central μV = 23.5 mag/arcsec².)
- **Type:** SIMBAD "dSph D 2012AJ....144....4M"; McC12 "dSph"; RC3 `.E?....`; LEDA "E", t −4.8 ± 0.5. Not disputed.
- **Feature "늙은 별이 대부분인 희미한 왜소은하": supported.** de Boer et al. 2012, A&A 539, A103 (**2012A&A...539A.103D**), SREF abstract: "The star formation history shows that Sculptor is dominated by old (>10Gyr), metal-poor stars, but that younger, more metal-rich populations are also present." Faintness: McC12 "Sculptor | dSph | 8.6 | 23.5 | 11.30 | 0.32 | -11.1" (M_V = −11.1). Keep.
- **Printed:** `DistanceText` "우리은하에서 약 28만 광년"; `FeatureText` "늙은 별이 대부분인 희미한 왜소은하".

### gal_fornax_dsph — Fornax Dwarf Spheroidal
- **NED preferred name:** `Fornax Dwarf SPHEROIDAL` — NED-OL (name "Fornax Dwarf Spheroidal"): `"Preferred": {"Name": "Fornax Dwarf SPHEROIDAL"}`.
- **l, b = 237.103952, −65.651440** — NED-TAP: `7463,"Fornax Dwarf SPHEROIDAL","G",39.997083,-34.449167,237.103952,-65.65144,12,78,41`. SIMBAD (`Ident=ESO 356-4`): "Coordinates(Gal,ep=J2000,eq=2000): 237.1038  -65.6515".
- **Distance:**
  - Pietrzyński et al. 2009, AJ 138, 459 (**2009AJ....138..459P**), SREF abstract: "for Fornax the same distance modulus of 20.84 mag was derived in both filters" → 147.2 kpc → **480,200 ly**. NED-D rows: "20.84 ; 0.03 ; 0.147 ; TRGB ; 2009AJ....138..459P ; J" and "… ; K".
  - McC12: "Fornax … 20.84 0.18 0.18 147 12 12 … D(MW) 149" → 440,300–518,600 ly.
  - NED-D (`?name=Fornax Dwarf Spheroidal`, second attempt): "78 Distances found in NED"; "Mean 0.140 | Std. Dev. 0.018 | Min. 0.048 | Max. 0.172 | Median 0.142" → 463,100 ly.
  - **Verdict:** "우리은하에서 약 47만 광년" **supported** (between the NED median, 46만, and the TRGB value, 48만).
- **D25 = 16.98′** — RC3 row: "Fornax | ESO 356- 4 | PGC10093 | 237.10 | -65.65 | dE.4... | -5.0 | 0.3 | 2.23 | | 0.16 | 0.13" (note the large error, ±0.16 dex). LEDA: "logd25 | 2.11 ± 0.03" → 12.88′. **RadiusLy (RC3) = 1,200 ly** (HyperLEDA: 900).
- **Type:** RC3 `dE.4...` = dE4, T = −5.0; SIMBAD "dSph D 2012AJ....144....4M"; McC12 "dSph"; LEDA "E", t −4.8 ± 0.5. Not disputed.
- **Feature "구상성단 여럿을 거느린 왜소은하": supported.** Pace et al. 2021, ApJ 923, 77 (**2021ApJ...923...77P**), SREF abstract: "The Fornax dwarf spheroidal galaxy has an anomalous number of globular clusters, five, for its stellar mass. … showing that this overdensity is indeed a star cluster and associated with the Fornax dSph." Also Mackey & Gilmore 2003, MNRAS 340, 175 (2003MNRAS.340..175M): "radial surface brightness profiles for all five globular clusters in the Fornax dwarf spheroidal galaxy". Keep.
- **Printed:** `DistanceText` "우리은하에서 약 47만 광년"; `FeatureText` "구상성단 여럿을 거느린 왜소은하".

### gal_ngc6822 — NGC 6822
- **NED preferred name:** `NGC 6822` — NED-OL: `"Preferred": {"Name": "NGC 6822"}`.
- **l, b = 25.339473, −18.398956** — NED-TAP: `58966,"NGC 6822","G",296.240416666667,-14.8033333333333,25.3394731388631,-18.3989559075369,6,127,62`. SIMBAD: "Coordinates(Gal,ep=J2000,eq=2000): 025.342467  -18.391181".
- **Distance:**
  - NED-D (`?name=NGC 6822`): "127 Distances found in NED"; "Mean 0.521 | Std. Dev. 0.176 | Min. 0.093 | Max. 1.700 | Median 0.479" → **1,562,300 ly**.
  - Gieren et al. 2006, ApJ 647, 1056 (**2006ApJ...647.1056G**), SREF abstract: "we obtain a distance to NGC 6822 of (m-M) 0 =23.312±0.021 (random error) mag, with an additional systematic uncertainty of ∼3%" → 459.6 kpc → 1,499,000 ly; McC12 "459 17 17". Cosmicflows-2 in NED-D: "23.41 ; 0.06 ; 0.480 ; Statistical ; 2013AJ....146...86T ; Mean of Cepheids and TRGB".
  - ESO potw1711a (https://www.eso.org/public/images/potw1711a/): "just 1.6 million light-years away"; ESA/Webb potm2307a (https://esawebb.org/images/potm2307a/): "NGC 6822 lies about 1.5 million light-years away".
  - **Verdict: "우리은하에서 약 160만 광년" supported** (modern values span 150만–160만).
- **D25 = 15.49′** — RC3: "NGC 6822 | MCG -2-50- 6 | PGC63616 | 25.34 | -18.40 | .IBS9.. | 10.0 | 0.3 | 2.19 | | 0.01 | 0.06". LEDA "logd25 | 2.19 ± 0.02" (same). **RadiusLy = 3,500 ly.**
- **Type:** RC3 `.IBS9..` = IB(s)m, T = 10.0; SIMBAD "IBm D 2013MNRAS.431.2006K"; LEDA "IB", t 9.8 ± 0.4; McC12 "dIrr". Not disputed.
- **Feature "막대 모양의 불규칙 왜소은하": supported in substance, reword.** ESO potw1711a: "NGC 6822 , a barred irregular dwarf galaxy". "막대 모양의" reads as "bar-shaped" (the whole galaxy); the fact is that it has a bar. **Proposed: "막대 구조를 지닌 불규칙 왜소은하".**
- **Printed:** `DistanceText` "우리은하에서 약 160만 광년"; `FeatureText` "막대 구조를 지닌 불규칙 왜소은하".

### gal_ic1613 — IC 1613 (not the Cetus dwarf)
- **NED preferred name:** `IC 1613` — NED-OL: `"Preferred": {"Name": "IC 1613"}`.
- **l, b = 129.737809, −60.577370** — NED-TAP: `2725,"IC 1613","G",16.19916666667,2.11772222222,129.737809070812,-60.5773700346125,7,106,82`. SIMBAD: "Coordinates(Gal,ep=J2000,eq=2000): 129.7425451  -60.5753451".
- **Distance:**
  - NED-D (`?name=IC 1613`, second attempt): "106 Distances found in NED"; "Mean 0.725 | Std. Dev. 0.069 | Min. 0.336 | Max. 0.859 | Median 0.728" → **2,374,400 ly** (±1σ: 2,139,600–2,589,700 ly).
  - McC12 "IC 1613 … 24.39 0.12 0.12 755 42 42" (Ref2 → Bernard et al. 2010, **2010ApJ...712.1259B**) → 2,462,500 ly. Cosmicflows-2 in NED-D: "24.36 ; 0.05 ; 0.740 ; Statistical ; 2013AJ....146...86T".
  - ESO eso1603 (https://www.eso.org/public/news/eso1603/): "IC 1613 itself lies just over 2.3 million light-years away from us."
  - **Verdict: "우리은하에서 약 240만 광년" supported.**
- **D25 = 16.22′** — RC3: "IC 1613 | UGC 668 | PGC 3844 | 129.79 | -60.56 | .IBS9.. | 10.0 | 0.3 | 2.21 | | 0.01 | 0.05". LEDA "logd25 | 2.26 ± 0.02" → 18.20′. **RadiusLy (RC3) = 5,600 ly.**
- **Type:** RC3 `.IBS9..` = IB(s)m, T = 10.0; SIMBAD "IB(s)m D 2015MNRAS.446..943V"; LEDA "I", t 9.9 ± 0.3; McC12 "dIrr". Not disputed.
- **Feature "별이 느리게 태어나는 불규칙 왜소은하": needs rewording.** The finding is a steady rate over the galaxy's whole life, not "slow". Skillman et al. 2014, ApJ 786, 44 (**2014ApJ...786...44S**), SREF abstract: "Our analysis shows that the SFH of the observed field in IC 1613 is consistent with being constant over the entire lifetime of the galaxy." **Proposed: "오랜 세월 한결같이 별을 낳아 온 불규칙 왜소은하"** (alternative supported by ESO eso1603, "IC 1613 contains very little cosmic dust": "먼지가 거의 없는 불규칙 왜소은하").
- **Printed:** `DistanceText` "우리은하에서 약 240만 광년"; `FeatureText` "오랜 세월 한결같이 별이 태어나 온 불규칙 왜소은하" (the passive, as NGC 253's "별이 활발히 태어나는" - the galaxy is not the one that begets; item 9's review).

### gal_m32 — M32
- **NED preferred name:** `Messier 032` — NED-OL: `"Interpreted": {"Name": "MESSIER 032"}`, `"Preferred": {"Name": "Messier 032"}`; NED-D: "M32 is also known as MESSIER 032".
- **l, b = 121.150093, −21.975988** — NED-TAP: `58140,"Messier 032","G",10.6743797,40.8653927,121.150093401247,-21.9759879511845,8,35,36`. SIMBAD: "Coordinates(Gal,ep=J2000,eq=2000): 121.15001697  -21.97633363".
- **Distance:** NED-D (`?name=M32`): "35 Distances found in NED"; "Mean 0.768 | Std. Dev. 0.058 | Min. 0.664 | Max. 0.925 | Median 0.780" → **2,544,000 ly** (±1σ: 2,315,700–2,694,100 ly). Same value as the Cosmicflows-2 mean in NED-D: "24.45 ; 0.08 ; 0.780 ; Statistical ; 2013AJ....146...86T ; Mean of SBF and TRGB" (2013AJ....146...86T = Tully et al., "Cosmicflows-2: the data", SREF). McC12: "M32 … 24.53 0.21 0.21 805 78 78" (Ref2 → 2010ApJ...708..817F, RR Lyrae). NASA Messier 32 page: "located 2.5 million light-years from Earth". **Verdict: "우리은하에서 약 250만 광년" supported.**
- **D25 = 8.71′** — RC3: "NGC 221 | UGC 452 | PGC 2555 | 121.15 | -21.98 | CE.2... | -6.0 | 0.3 | 1.94 | | 0.02 | 0.13". LEDA "logd25 | 1.89 ± 0.02" → 7.76′. **RadiusLy (RC3) = 3,200 ly.**
- **Type:** RC3 `CE.2...` = cE2, T = −6.0; SIMBAD "cE2 D 2013AJ....146...67B"; McC12 "cE"; LEDA "E". Kind "Dwarf" is acceptable for a low-mass galaxy, but the catalogued class is compact elliptical (cE), not dE. Not disputed.
- **Feature "안드로메다은하에 바짝 붙어 도는 작고 둥근 은하": supported.** NASA Hubble Messier 32 page, https://science.nasa.gov/mission/hubble/science/explore-the-night-sky/hubble-messier-catalog/messier-32/ : "Messier 32 (M32) is a compact elliptical galaxy … It is one of two satellite galaxies orbiting the Andromeda Galaxy (M31)." Closeness: Choi, Guhathakurta & Johnston 2002, AJ 124, 310 (**2002AJ....124..310C**), title "Tidal interaction of M32 and NGC 205 with M31", abstract: "A hint of excess light in the outer parts of M32 noted in earlier studies is confirmed". Keep (optional softer form: "안드로메다은하 가까이에서 그 둘레를 도는 작고 둥근 은하").
- **Printed:** `DistanceText` "우리은하에서 약 250만 광년"; `FeatureText` "안드로메다은하에 바짝 붙어 도는 작고 둥근 은하".

### gal_m110 — M110 (NGC 205)
- **NED preferred name:** `Messier 110` — NED-OL: `"Interpreted": {"Name": "MESSIER 110"}`, `"Preferred": {"Name": "Messier 110"}`; NED-D: "M110 is also known as MESSIER 110".
- **l, b = 120.716296, −21.138598** — NED-TAP: `59129,"Messier 110","G",10.0919524,41.6853935,120.716296261823,-21.1385975241693,8,32,32`. SIMBAD: "Coordinates(Gal,ep=J2000,eq=2000): 120.7162794892624  -21.1386993067892".
- **Distance:**
  - McConnachie et al. 2005, MNRAS 356, 979 (**2005MNRAS.356..979M**, TRGB; SREF title "Distances and metallicities for 17 Local Group galaxies"); NED-D row "24.58 ; 0.07 ; 0.824 ; TRGB ; 2005MNRAS.356..979M"; McC12 "NGC 205 … 24.58 0.07 0.07 824 27 27" → **2,687,600 ly** (2,599,500–2,775,600 ly).
  - NED-D (`?name=M110`): "32 Distances found in NED"; "Mean 0.821 | Std. Dev. 0.065 | Min. 0.700 | Max. 1.000 | Median 0.814" (parsed median of the list: 0.8135) → 2,653,300 ly.
  - NASA Messier 110 page: "M110 is approximately 2,690,000 light-years away from Earth".
  - **Verdict: "우리은하에서 약 270만 광년" supported.**
- **D25 = 21.88′** — RC3: "NGC 205 | UGC 426 | PGC 2429 | 120.72 | -21.14 | .E.5.P. | -5.0 | 0.3 | 2.34 | | 0.01 | 0.30". LEDA "logd25 | 2.21 ± 0.02" → 16.22′. **RadiusLy (RC3, 824 kpc) = 8,600 ly** (with the NED median: 8,400).
- **Type:** RC3 `.E.5.P.` = E5 pec, T = −5.0; SIMBAD "E5pec D 2013AJ....146...67B"; McC12 "dE/dSph"; LEDA "E". Not disputed (dE vs E5 pec is naming only).
- **Feature "안드로메다은하를 도는 왜소타원은하": supported.** NASA Hubble Messier 110 page, https://science.nasa.gov/mission/hubble/science/explore-the-night-sky/hubble-messier-catalog/messier-110/ : "It is a satellite galaxy of the Andromeda galaxy (M31)" and "M110 is an elliptical galaxy"; McC12 type "dE/dSph". Keep.
- **Printed:** `DistanceText` "우리은하에서 약 270만 광년"; `FeatureText` "안드로메다은하를 도는 왜소타원은하".

### gal_ngc253 — NGC 253
- **NED preferred name:** `NGC 0253` — NED-OL (name "NGC 253"): `"Interpreted": {"Name": "NGC 0253"}`, `"Preferred": {"Name": "NGC 0253"}`; NED-D: "NGC 253 is also known as NGC 0253".
- **l, b = 97.371777, −87.963936** — NED-TAP: `58143,"NGC 0253","G",11.8880416666667,-25.2880833333333,97.3717772200208,-87.9639356003772,22,32,53`. SIMBAD: "Coordinates(Gal,ep=J2000,eq=2000): 097.363851  -87.964547".
- **Distance:**
  - Radburn-Smith et al. 2011, ApJS 195, 18 (**2011ApJS..195...18R**, SREF title "The GHOSTS survey. I. Hubble Space Telescope Advanced Camera for Surveys data"); NED-D row "27.70 ; 0.07 ; 3.500 ; TRGB ; 2011ApJS..195...18R" → **11,415,600 ly** (central used here). Other TRGB rows: 3.40–3.47 (2009ApJS..183...67D), 3.66 (2009AJ....138..332J) → 11.1–11.9 million ly; Cosmicflows-2 "27.76 ; 0.08 ; 3.560 ; Statistical ; 2013AJ....146...86T".
  - NED-D (`?name=NGC 253`, second attempt): "32 Distances found in NED"; "Mean 3.215 | Std. Dev. 0.495 | Min. 2.060 | Max. 4.270 | Median 3.095" → 10,094,700 ly (pulled down by older and Tully-Fisher estimates).
  - ESO eso1152 (https://www.eso.org/public/news/eso1152/): "NGC 253 gleams about eleven and a half million light-years away".
  - **Verdict: "우리은하에서 약 1,100만 광년" supported.**
- **D25 = 27.54′** — RC3: "NGC 253 | ESO 474- 29 | PGC 2789 | 97.36 | -87.96 | .SXS5.. | 5.0 | 0.3 | 2.44 | | 0.01 | 0.61". LEDA "logd25 | 2.43 ± 0.01" → 26.92′. **RadiusLy (RC3, 3.50 Mpc) = 45,700 ly.**
- **Type:** RC3 `.SXS5..` = SAB(s)c, T = 5.0; LEDA "SABc", t 5.1 ± 0.4; SIMBAD "SAB(s)c D 2015MNRAS.446..943V". Not disputed.
- **Feature "별이 활발히 태어나는 나선은하": supported.** ESO eso1152: "Astronomers have noted the widespread active star formation in NGC 253 and labelled it a 'starburst' galaxy". Keep.
- **Printed:** `DistanceText` "우리은하에서 약 1,100만 광년"; `FeatureText` "별이 활발히 태어나는 나선은하".

### gal_m81 — M81
- **NED preferred name:** `SN 1993J HOST` — NED-OL (name "M81"): `"Interpreted": {"Name": "MESSIER 081"}`, `"Preferred": {"Name": "SN 1993J HOST"}`. The legacy NED-D page says "M81 is also known as MESSIER 081". (NED's current preferred name is the supernova-host designation.)
- **l, b = 142.091841, +40.900141** — NED-TAP: `58480,"SN 1993J HOST","G",148.8882210375,69.0652946122222,142.091840699829,40.9001409769856,8,99,117`. SIMBAD: "Coordinates(Gal,ep=J2000,eq=2000): 142.0918223605216  +40.9000561069047".
- **Distance:**
  - NED-D (`?name=M81`): "99 Distances found in NED"; "Mean 3.675 | Std. Dev. 0.483 | Min. 1.400 | Max. 5.000 | Median 3.630" → **11,839,600 ly**.
  - Freedman et al. 1994, ApJ 427, 628 (**1994ApJ...427..628F**, SREF title "The Hubble Space Telescope extragalactic distance scale key project. I. The discovery of cepheids and a new distance to M 81"); NED-D row "27.80 ; 0.08 ; 3.630 ; Cepheids ; 1994ApJ...427..628F". TRGB rows: 3.44 (2009AJ....138..332J) – 3.60 Mpc (2009ApJS..183...67D, 2011ApJS..195...18R) → 11.2–11.7 million ly.
  - NASA Messier 81 page: "It is located 11.6 million light-years from Earth".
  - **Verdict: "우리은하에서 약 1,200만 광년" supported.**
- **D25 = 26.92′** — RC3: "NGC 3031 | UGC 5318 | IRAS09514+6918 | PGC28630 | .SAS2.. | R | 2.0 | 0.3 | … | 2.43 | | 0.01 | 0.28". NED-DIAM row: "Frequency targeted=RC3 D_25, R_25 (blue) | Refcode=1991RC3.9.C...0000d | Major Axis=2.43 | Major Axis Unit=log(2a) + 1 | … | NED Major Axis=1614.90" (arcsec = 26.9′). LEDA "logd25 | 2.34 ± 0.01" → 21.88′. **RadiusLy (RC3) = 46,300 ly** (HyperLEDA: 37,700).
- **Type:** RC3 `.SAS2..` = SA(s)ab, T = 2.0; LEDA "Sab", t 2.4 ± 0.6; SIMBAD "SA C 2019MNRAS.488..590B". Not disputed.
- **Feature "선명한 나선팔을 지닌 나선은하, M82와 서로 끌어당긴다": supported.** NASA Messier 82 page, https://science.nasa.gov/mission/hubble/science/explore-the-night-sky/hubble-messier-catalog/messier-82/ : "The Cigar galaxy experiences gravitational interactions with its galactic neighbor, M81". NASA Messier 81 page, https://science.nasa.gov/mission/hubble/science/explore-the-night-sky/hubble-messier-catalog/messier-81/ : "The galaxy's spiral arms, which wind all the way down into its nucleus, are made up of young, bluish, hot stars". Keep.
- **Printed:** `DistanceText` "우리은하에서 약 1,200만 광년"; `FeatureText` "선명한 나선팔을 지닌 나선은하, M82와 서로 끌어당긴다".

### gal_m82 — M82
- **NED preferred name:** `Messier 082` — NED-OL: `"Interpreted": {"Name": "MESSIER 082"}`, `"Preferred": {"Name": "Messier 082"}`; NED-D: "M82 is also known as MESSIER 082".
- **l, b = 141.409175, +40.567750** — NED-TAP: `58481,"Messier 082","G",148.9705,69.67948333333,141.409174811254,40.5677501940947,13,21,40`. SIMBAD: "Coordinates(Gal,ep=J2000,eq=2000): 141.409467  +40.566962".
- **Distance:**
  - NED-D (`?name=M82`, second attempt): "21 Distances found in NED"; "Mean 3.835 | Std. Dev. 0.730 | Min. 2.400 | Max. 5.500 | Median 3.700" → **12,067,900 ly**.
  - TRGB rows: "27.73 ; 0.02 ; 3.520 ; TRGB ; 2009AJ....138..332J", "27.74 ; 0.04 ; 3.530 ; TRGB ; 2009ApJS..183...67D", "27.74 ; 0.08 ; 3.530 ; TRGB ; 2013AJ....146...86T" → 11.5 million ly. LEDA "mod0 | 27.79 ± 0.01" (3.61 Mpc).
  - NASA Messier 82 page: "Located 12 million light-years from Earth".
  - **Verdict: "우리은하에서 약 1,200만 광년" supported** (11.5–12.1 million ly).
- **D25 = 11.22′** — RC3: "NGC 3034 | UGC 5322 | PGC28655 | 141.40 | 40.57 | .I.0../ | 90.0 | | 2.05 | | 0.01 | 0.42". LEDA "logd25 | 2.04 ± 0.02" → 10.96′. **RadiusLy (RC3, NED median) = 19,700 ly** (with the TRGB 3.53 Mpc: 18,800).
- **Type:** RC3 `.I.0../` = I0, seen edge-on (T code 90 = I0); LEDA "type | S?", "t | 7.2 ± 4.1"; SIMBAD "Morphological type: S C 2019MNRAS.488..590B".
- **Dispute: yes.** Mayya, Carrasco & Luna 2005, ApJ 628, L33 (**2005ApJ...628L..33M**), SREF abstract: "We report the discovery of two symmetric spiral arms in the near-infrared (NIR) images of the starburst galaxy M82. … The arms emerge from the ends of the NIR bar". The catalogue class is irregular (RC3 I0); newer imaging reads it as an edge-on barred disc with spiral arms. "Irregular" is defensible as the catalogued class.
- **Feature "별이 폭발적으로 태어나는 은하, 중심에서 가스가 뿜어져 나온다": supported.** NASA Messier 82 page: "M82 or the Cigar galaxy, shines brightly at infrared wavelengths and is remarkable for its star formation activity … — a starburst." and "Radiation and energetic particles from these newborn stars carve into the surrounding gas, and the resulting galactic wind compresses enough gas to make millions of more stars." and "Plumes of gas and dust … blown from the central regions of M82 into the outer 'suburbs.'" Keep.
- **Printed:** `DistanceText` "우리은하에서 약 1,200만 광년"; `FeatureText` "별이 폭발적으로 태어나는 은하, 중심에서 가스가 뿜어져 나온다 (분류는 논쟁 중)".

### gal_cen_a — Centaurus A (NGC 5128)
- **NED preferred name:** `NGC 5128` — NED-OL (name "Centaurus A"): `"Interpreted": {"Name": "Centaurus A"}`, `"Preferred": {"Name": "NGC 5128"}`.
- **l, b = 309.515874, +19.417325** — NED-TAP: `58785,"NGC 5128","G",201.3650625,-43.0191127777,309.515873570429,19.4173245053707,22,54,65`. SIMBAD (`Ident=NGC 5128` → "NAME Centaurus A"): "Coordinates(Gal,ep=J2000,eq=2000): 309.51590291596  +19.41727090758".
- **Distance:**
  - Harris, Rejkuba & Harris 2010, PASA 27, 457 (**2010PASA...27..457H**, SREF title "The distance to NGC 5128 (Centaurus A)"); abstract from arXiv (https://arxiv.org/abs/0911.3180, via the arXiv API): "we derive a best-estimate distance of 3.8 +- 0.1 Mpc to NGC 5128" → **12,394,100 ly** (12,067,900–12,720,200 ly).
  - NED-D (`?name=NGC 5128`, second attempt): "54 Distances found in NED"; "Mean 3.812 | Std. Dev. 0.837 | Min. 2.980 | Max. 8.950 | Median 3.660" → 11,937,500 ly. Cosmicflows-2 row: "27.82 ; 0.06 ; 3.660 ; Statistical ; 2013AJ....146...86T ; Mean of Cepheids, SBF, and TRGB".
  - ESO eso1222 (https://www.eso.org/public/news/eso1222/), note [2]: "Centaurus A lies about 12 million light-years away".
  - **Verdict: "우리은하에서 약 1,200만 ~ 1,300만 광년" supported.**
- **D25 = 25.70′** — RC3: "NGC 5128 | ESO 270- 9 | PGC46957 | 309.52 | 19.42 | .L...P. | -2.0 | 0.3 | 2.41 | | 0.01 | 0.11". LEDA "logd25 | 2.41 ± 0.01" (same). **RadiusLy (Harris distance) = 46,300 ly.**
- **Type:** RC3 `.L...P.` = S0 pec (L = lenticular), T = −2.0; LEDA "S0", t −2.1 ± 0.6; SIMBAD "S0pec D 2013MNRAS.435.2274W".
- **Dispute: yes.** ESO eso1222 calls it "a massive elliptical radio galaxy" and "the result of a collision between a giant elliptical galaxy, and a smaller spiral galaxy whose remains form the dusty band", while the catalogues class it lenticular (S0 pec). Kind "Elliptical" follows the institutional description; "lenticular" follows the catalogues.
- **Feature "먼지 띠가 가로지르는 거대한 은하, 강한 전파를 낸다": supported.** ESO eso1222: "Centaurus A [1] is a massive elliptical radio galaxy — a galaxy which emits strong radio waves" and "In visible light, a characteristic feature of the galaxy is the dark band that obscures its centre … This dust lane harbours large amounts of gas, dust and young stars." Keep.
- **Printed:** `DistanceText` "우리은하에서 약 1,200만 ~ 1,300만 광년"; `FeatureText` "먼지 띠가 가로지르는 거대한 은하, 강한 전파를 낸다 (분류는 논쟁 중)".

### gal_m101 — M101
- **NED preferred name:** `Messier 101` — NED-OL: `"Interpreted": {"Name": "MESSIER 101"}`, `"Preferred": {"Name": "Messier 101"}`; NED-D: "M101 is also known as MESSIER 101".
- **l, b = 102.037326, +59.771382** — NED-TAP: `55877,"Messier 101","G",210.80226708333,54.34895,102.037326208619,59.7713817495056,13,116,132`. SIMBAD: "Coordinates(Gal,ep=J2000,eq=2000): 102.036992  +59.771413".
- **Distance:**
  - NED-D (`?name=M101`): "116 Distances found in NED"; "Mean 6.644 | Std. Dev. 0.936 | Min. 2.600 | Max. 8.990 | Median 6.730" → **21,950,600 ly**.
  - TRGB rows run from "29.05 ; 0.06 ; 6.460 ; TRGB ; 2011ApJ...733..124S" to "29.30 ; 0.01 ; 7.240 ; TRGB ; 2012ApJ...760L..14L" → 21.1–23.6 million ly. Cosmicflows-2: "29.21 ; 0.06 ; 6.950 ; Statistical ; 2013AJ....146...86T ; Mean of Cepheids and TRGB" → 22.7 million ly.
  - NASA Messier 101 page: "Located 25 million light-years away from Earth".
  - **Verdict:** "우리은하에서 약 2,100만 광년" is at the low end; the NED median rounds to 2,200만. **Proposed: "우리은하에서 약 2,200만 광년".**
- **D25 = 28.84′** — RC3: "NGC 5457 | UGC 8981 | PGC50063 | 102.04 | 59.77 | .SXT6.. | 6.0 | 0.3 | 2.46 | | 0.01 | 0.03". LEDA "logd25 | 2.38 ± 0.02" → 23.99′. **RadiusLy (RC3, NED median) = 92,100 ly** (at 2,200만 ly exactly: 92,300).
- **Type:** RC3 `.SXT6..` = SAB(rs)cd, T = 6.0; LEDA "SABc", t 5.9 ± 0.3; SIMBAD "S_AB C 2019MNRAS.488..590B". Not disputed; "GrandSpiral" (a large spiral) fits NASA's "giant spiral disk".
- **Feature "넓게 펼쳐진 나선팔": supported.** NASA Hubble Messier 101 page, https://science.nasa.gov/mission/hubble/science/explore-the-night-sky/hubble-messier-catalog/messier-101/ : "The giant spiral disk of stars, dust, and gas is 170,000 light-years across — nearly twice the diameter of our galaxy, the Milky Way." and "The galaxy's spiral arms are sprinkled with large regions of star-forming nebulas." Keep (the line is better than empty).
- **Printed:** `DistanceText` "우리은하에서 약 2,200만 광년"; `FeatureText` "넓게 펼쳐진 나선팔을 지닌 은하".

### gal_m51 — M51 (NGC 5194)
- **NED preferred name:** `SN 1994I HOST` for NGC 5194 — NED-OL (name "NGC 5194"): `"Interpreted": {"Name": "NGC 5194"}`, `"Preferred": {"Name": "SN 1994I HOST"}`. The name "M51" resolves to the pair: `"Interpreted": {"Name": "MESSIER 051"}`, `"Preferred": {"Name": "Messier 051"}`, `"ObjType": {"Value": "GPair"}`.
- **l, b = 104.851396, +68.560809** (NGC 5194) — NED-TAP: `55781,"SN 1994I HOST","G",202.469625833333,47.1952073611111,104.851395872305,68.5608088868517,13,53,78`. (Pair entry: `55780,"Messier 051","GPair",…,104.866002782287,68.5244977225169,…`.) SIMBAD (`Ident=M51` → "M 51"): "Coordinates(Gal,ep=J2000,eq=2000): 104.851585  +68.560702".
- **Distance:**
  - McQuinn et al. 2016, ApJ 826, 21 (**2016ApJ...826...21M**, SREF title "The distance to M51"), abstract: "We measure the distance modulus to be 8.58±0.10 Mpc (statistical), corresponding to a distance modulus of 29.67±0.02 mag." → **27,984,500 ly**. NED-D row "29.67 ; 0.02 ; 8.580 ; TRGB ; 2016ApJ...826...21M ; syst. +/- 0.05".
  - NED-D (`?name=NGC 5194`): "53 Distances found in NED"; "Mean 7.225 | Std. Dev. 2.126 | Min. 2.450 | Max. 12.200 | Median 7.800" → 25,440,500 ly.
  - NASA Messier 51 page: "M51 is located 31 million light-years from Earth".
  - **Verdict: "우리은하에서 약 2,300만 ~ 3,100만 광년" supported** (optional single value from the TRGB: "약 2,800만 광년").
- **D25 = 11.22′** (NGC 5194) — RC3: "NGC 5194 | UGC 8493 | PGC47404 | 104.85 | 68.56 | .SAS4P. | 4.0 | 0.3 | 2.05 | | 0.01 | 0.21". LEDA "logd25 | 2.14 ± 0.02" → 13.80′. **RadiusLy (RC3, TRGB distance) = 45,700 ly** (HyperLEDA: 56,200).
- **Type:** RC3 `.SAS4P.` = SA(s)bc pec, T = 4.0; LEDA "SABb", t 4.0 ± 0.3; SIMBAD "SA C 2019MNRAS.488..590B". Companion NGC 5195: RC3 `.I.0.P.` (I0 pec). Not disputed.
- **Feature "작은 동반 은하와 맞닿은 나선은하": needs rewording.** The companion is not in contact; it passes behind. NASA Hubble Messier 51 page, https://science.nasa.gov/mission/hubble/science/explore-the-night-sky/hubble-messier-catalog/messier-51/ : "a close encounter with NGC 5195, the small, yellowish galaxy at the outermost tip of one of the arms. The compact galaxy appears to be tugging on the arm … Hubble's clear view shows that NGC 5195 is passing behind M51." **Proposed: "작은 동반 은하와 서로 끌어당기는 나선은하".**
- **Printed:** `DistanceText` "우리은하에서 약 2,300만 ~ 3,100만 광년"; `FeatureText` "작은 동반 은하와 서로 끌어당기는 나선은하".

### gal_m104 — M104 (Sombrero)
- **NED preferred name:** `Messier 104` — NED-OL: `"Interpreted": {"Name": "MESSIER 104"}`, `"Preferred": {"Name": "Messier 104"}`; NED-D: "M104 is also known as MESSIER 104".
- **l, b = 298.460553, +51.149256** — NED-TAP: `58678,"Messier 104","G",189.9976312,-11.62308847,298.460553267788,51.1492564097722,7,31,43`. SIMBAD: "Coordinates(Gal,ep=J2000,eq=2000): 298.46058621087  +51.14928244354".
- **Distance:**
  - McQuinn et al. 2016, AJ 152, 144 (**2016AJ....152..144M**, SREF title "The distance to M104"), abstract: "Our measurement yields the distance to M104 to be 9.55 ± 0.13 ± 0.31 Mpc" → **31,148,300 ly**. NED-D row "29.90 ; 0.03 ; 9.550 ; TRGB ; 2016AJ....152..144M ; F814W, Syst. +/- 0.07 mag".
  - PNLF rows: "29.74 ; 0.06 ; 8.870 ; PNLF ; 1996ApJ...458..455F", "29.79 ; 0.07 ; 9.080 ; PNLF ; 1993ApJ...419..479C" → 28.9–29.6 million ly.
  - NED-D (`?name=M104`): "31 Distances found in NED"; "Mean 11.219 | Std. Dev. 3.592 | Min. 6.220 | Max. 21.700 | Median 9.950" → 32,452,900 ly (raised by Tully-Fisher values, which McQuinn et al. call "likely inappropriate for M104").
  - NASA Messier 104 page: "M104 is located 28 million light-years away".
  - **Verdict: "우리은하에서 약 2,900만 ~ 3,100만 광년" supported** (PNLF to TRGB; the TRGB value is the upper end).
- **D25 = 8.71′** — RC3: "NGC 4594 | MCG -2-32- 20 | PGC42407 | 298.46 | 51.15 | .SAS1./ | 1.0 | 0.3 | 1.94 | | 0.01 | 0.39". LEDA "logd25 | 1.93 ± 0.02" → 8.51′. **RadiusLy (RC3, TRGB) = 39,500 ly.**
- **Type:** RC3 `.SAS1./` = SA(s)a, seen edge-on, T = 1.0; LEDA "Sa", t 1.1 ± 0.4; SIMBAD "Morphological type: 1 D 2018MNRAS.479.4136K" (T-type 1 = Sa).
- **Dispute: yes.** McQuinn et al. 2016 (2016AJ....152..144M) abstract: "M104 (NGC 4594; the Sombrero galaxy) is a nearby, well-studied elliptical galaxy". Gadotti & Sánchez-Janssen 2012, MNRAS 423, 877 (**2012MNRAS.423..877G**, SREF title "Surprises in image decomposition of edge-on galaxies: does Sombrero have a (classical) bulge?"): "Finally, we discuss the possibility that Sombrero formed as an elliptical galaxy but accreted a massive disc". NASA keeps "a spiral galaxy seen nearly edge-on". "Spiral" follows the catalogues.
- **Feature "밝은 중심부를 먼지 띠가 가로지르는 은하": supported.** NASA Hubble Messier 104 page, https://science.nasa.gov/mission/hubble/science/explore-the-night-sky/hubble-messier-catalog/messier-104/ : "The hallmark of the nearly edge-on galaxy is a brilliant, white, bulbous core encircled by thick dust lanes comprising the spiral structure of the galaxy." Keep.
- **Printed:** `DistanceText` "우리은하에서 약 2,900만 ~ 3,100만 광년"; `FeatureText` "밝은 중심부를 먼지 띠가 가로지르는 은하 (분류는 논쟁 중)".

## Not retrieved

1. **NED "Classifications" tab — all 19 objects.** The new NED page returns an empty template to scripted fetches, and the legacy object page (`/cgi-bin/objsearch`) returned HTTP 503. Types were taken from RC3 (VizieR VII/155), HyperLEDA and SIMBAD instead.
2. **NED "Diameters" tab — all objects except M81.** Only M81 was read from NED's legacy diameter search. All other D25 values were read from RC3 itself (VizieR VII/155), the catalogue NED quotes.
3. **D25 for gal_sgr_dsph from RC3 or NED.** Sgr is not in RC3; NED holds no diameter data for it (`n_ddf = 0`). Only HyperLEDA's homogenized value (446.7′) exists, which is not a normal 25-mag isophote.
4. **ADS abstract pages (HTTP 405) — every bibcode.** Bibcodes were verified on SIMBAD reference pages instead.
5. **Abstracts of 1994Natur.370..194I (Ibata et al., Sgr discovery), 1994Natur.372..530Y (Yun et al., M81 group H I), 1998A&ARv...8..237I (Israel, Centaurus A review) and 2010PASA...27..457H (Harris et al.).** SIMBAD shows the titles only. The Centaurus A distance (3.8 ± 0.1 Mpc) was read from the arXiv abstract of the same paper (arXiv:0911.3180) instead. The other three are cited only by title.
6. **The current distance texts of the five existing rows.** These were not supplied, so only the reference values are given and no verdict is made.
7. **NED distance data ingested after January 2026.** The legacy distance service cannot access it (NED banner), so newer measurements, if any, are missing from the NED statistics.
