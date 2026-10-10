# Cosmic Ascension (코스믹 어센션, 가제) — 프로젝트 가이드라인 v0.3



> "우주 질서를 유지하는 초월자의 길, 혹은 별들을 거느리는 제국의 군주로"

> 1인 개발에 최적화된 거시 우주 시뮬레이션 & 샌드박스 승천 RPG



| 항목 | 내용 |
|---|---|
| 문서 상태 | 설계 가이드라인. 초안 v0.1을 검토하고 보완한 v0.2 → Phase 0 셋업 결과를 반영한 v0.3 (UE 5.8.3 고정, C++ 모듈명 `Cosmic_Ascension`, 실제 빌드 경로) |
| 최종 수정 | 2026-10-10 |
| 대상 독자 | 개발자 본인, 그리고 코드 작성을 돕는 Claude Code 세션 |
| 원칙 | 이 문서 하나가 설계의 단일 기준(Single Source of Truth)이다. 결정이 바뀌면 이 문서를 먼저 고친다. |



---



## 목차



[0. 초안(v0.1) 검토 결과 및 변경 요약](#0-초안v01-검토-결과-및-변경-요약)
[1. 프로젝트 비전 및 핵심 철학](#1-프로젝트-비전-및-핵심-철학)

[2. 플레이어 경험: 코어 루프](#2-플레이어-경험-코어-루프)
[3. 핵심 게임플레이 3대 기둥](#3-핵심-게임플레이-3대-기둥)
[4. 긴장과 위기: 샌드박스를 '게임'으로 만드는 장치](#4-긴장과-위기-샌드박스를-게임으로-만드는-장치)
[5. 게임 시스템 상세 (린 설계)](#5-게임-시스템-상세-린-설계)
[6. UE5 기술 아키텍처](#6-ue5-기술-아키텍처)
[7. 데이터 모델 및 C++ 설계안](#7-데이터-모델-및-c-설계안)
[8. 프로시저럴 텍스트 / 로그 시스템](#8-프로시저럴-텍스트--로그-시스템)
[9. 저장, 결정론, 테스트, 밸런스](#9-저장-결정론-테스트-밸런스)
[10. 프로젝트 구조 및 코딩 규약](#10-프로젝트-구조-및-코딩-규약)
[11. 로드맵 (현실화)](#11-로드맵-현실화)
[12. 범위 관리: 수직 슬라이스와 미구현 목록](#12-범위-관리-수직-슬라이스와-미구현-목록)
[13. 리스크와 대응](#13-리스크와-대응)
[14. Claude Code 협업 규칙](#14-claude-code-협업-규칙)
[15. 용어집](#15-용어집)
[16. 미결 사항 (결정 필요)](#16-미결-사항-결정-필요)


---



## 0. 초안(v0.1) 검토 결과 및 변경 요약



초안의 뼈대(린 철학, 3대 기둥, 5단계 존재 등급, 이원화 플레이, 단계별 로드맵)는 그대로 유지했다. 아래는 검토에서 발견한 공백과 이 문서에서 보완한 내용이다.



| # | 초안의 문제 | v0.2에서의 보완 | 위치 |
|---|---|---|---|
| 1 | 시스템 목록은 있으나 플레이어가 "10분 동안 무엇을 하는지"가 없음 | 두 모드의 분 단위 루프, 결정 주기, 한 판 길이 목표 정의 | §2 |
| 2 | 승천 업적이 시뮬레이션과 연결되지 않아 스크립트 이벤트처럼 보임 | 위기(Crisis) 생성기가 업적 기회를 만들어 Pillar 1과 2를 연결 | §4.1, §3 |
| 3 | 적대 요소·실패 상태가 없어 '스프레드시트 감상'이 될 위험 | 타락 초월자, 경쟁 승천 문명, 위기 카운트다운, 실패 상태, 엔딩 2종 | §4 |
| 4 | 방랑자 모드가 "함선으로 유랑"이라고만 되어 있음. 3D 비행은 린 철학과 충돌 | 노드 이동 + 조우 카드(텍스트 선택지) 방식으로 확정 | §5.9 |
| 5 | 통치자↔방랑자 전환 규칙이 없음 | 섭정 위임, 경외도, 전환 비용과 조건 정의 | §5.10 |
| 6 | Tier 5 도달 이후가 비어 있음 | 집단 승천 / 단독 승천 엔딩 + 연대기 에필로그 | §4.4 |
| 7 | 저장/로드, 시간 배속, 결정론이 전혀 언급되지 않음 | Phase 1부터 설계에 포함. 상태를 USTRUCT로 모아 USaveGame에 저장 | §9 |
| 8 | 데이터 구조에서 '정의(Def)'와 '상태(State)'가 섞임. 특성이 FString | Def/State 분리. 특성·카테고리는 FGameplayTag, 행 ID는 FName | §7 |
| 9 | 이벤트 시스템의 데이터 구조가 없음 | 조건·가중치·효과·로그 템플릿을 가진 이벤트 정의 구조체 추가 | §7.3 |
| 10 | CommonUI를 기본으로 채택 | 싱글 PC 텍스트 중심 게임에는 UMG가 더 가볍다. CommonUI는 패드 지원 시 재검토 | §6.3 |
| 11 | 로드맵 총합 약 10주는 UE5 1인 개발로는 비현실적 | Phase 0(셋업)·Phase 5(폴리시) 추가, 각 Phase에 종료 조건, EA까지 현실 일정 | §11 |
| 12 | C++ 소스에 한글 DisplayName 하드코딩 | 소스는 영문, 표시 텍스트는 DataTable/String Table(FText)로 | §10.3 |
| 13 | 테스트·밸런스 전략 부재 | 헤드리스 시뮬 커맨드렛 + 자동화 테스트로 1인 밸런싱 | §9.3 |
| 14 | "무엇을 만들지 않는가"가 철학 수준에만 있음 | 수직 슬라이스 정의와 명시적 미구현 목록 | §12 |
| 15 | 폴더 구조, 명명 규칙, 용어집 없음 (Claude Code 협업 시 일관성 저하) | 프로젝트 구조, 규약, 용어집, CLAUDE.md 템플릿 추가 | §10, §14, §15 |



---



## 1. 프로젝트 비전 및 핵심 철학



### 1.1 한 줄 피치



> 은하 문명들이 스스로 흥망하는 텍스트 기반 시뮬레이션 위에서, 플레이어는 제국의 군주 또는 홀로 떠도는 방랑자로서 우주적 사건에 개입해 존재의 격을 다섯 단계로 끌어올린다.



### 1.2 게임 개요



| 항목 | 내용 |
|---|---|
| 장르 | 4X 대전략 시뮬레이션 + 샌드박스 스페이스 RPG 하이브리드 |
| 엔진 | Unreal Engine **5.8.3** (C++ 중심 + Data-Driven). 2026-10-10 Phase 0에서 고정. EA까지 엔진 버전을 올리지 않는다 |
| 플랫폼 | PC (Steam), 싱글 플레이어 |
| 한 판 길이 목표 | 4~6시간 (1배속 기준). 배속 사용 시 2~3시간 |
| 톤 | 신화적, 관조적, 간결한 SF 서사. 유머보다는 경외 |
| 언어 | 한국어 우선, 영어 2차 |



### 1.3 1인 개발 최적화 철학 (Lean Simulation)



**미시적 공장 건축 배제**

개별 기계, 컨베이어, 작업자 3D 모델링을 만들지 않는다.
산업·물류·연구는 수치(Tick)와 프로시저럴 텍스트 로그로 완전 자동화한다.


**시각 리소스의 선택과 집중**

외계인 3D 캐릭터 모델을 만들지 않는다. 문명은 아이콘 + 색 + 텍스트로 표현한다.
그래픽 예산은 세 곳에만 쓴다: 은하 지도(3D 노드), 초월적 순간의 Niagara 이펙트, 깔끔한 SF UI.


**규칙 기반 세계관 (Emergent Gameplay)**

일자형 스토리를 쓰지 않는다. 문명의 특성, 위기 생성기, 초월자의 법칙이 부딪쳐 사건이 발생한다.
작가가 쓰는 것은 '사건 템플릿'과 '규칙'이며, 서사는 플레이 중에 조립된다.


**데이터가 코드보다 많아야 한다**

새 특성, 이벤트, 업적, 로그 문장은 C++ 재컴파일 없이 DataTable 행 추가로 끝나야 한다.


### 1.4 참조 게임과 차용 요소



| 게임 | 차용하는 것 | 차용하지 않는 것 |
|---|---|---|
| Stellaris | 거시 4X 틱 경제, 조건 기반 이벤트, 승천 경로 | 수백 노드 기술 트리, 행성 타일 관리, 실시간 함대 전투 |
| Sunless Skies / FTL | 노드 이동 + 텍스트 조우 카드, 선택지와 자원 트레이드오프 | 실시간 조작, 선박 내부 관리 |
| Crusader Kings 3 | 개인(캐릭터) 중심 이벤트, 즉위·위임 구조 | 가계도, 봉건 계약 |
| Dwarf Fortress / RimWorld | 프로시저럴 서사 로그, '스토리텔러' 식 위기 페이싱 | 개체 단위 시뮬레이션 |
| Caves of Qud / Cultist Simulator | 신화적·시적 텍스트 톤, 텍스트 자체가 보상인 설계 | 로그라이크 전투 |



**차별점**: (1) 문명 통치자와 단독 방랑자를 한 세이브 안에서 오갈 수 있고, (2) 승천이 경험치가 아닌 '세계에 흔적을 남기는 사건'으로만 일어난다.



---



## 2. 플레이어 경험: 코어 루프



### 2.1 시간 모델



시뮬레이션은 **고정 스텝 틱**으로 돈다. 1틱 = 1 은하 주기(Cycle). 1배속에서 1초 = 1틱.
배속: 일시정지 / 1x / 2x / 5x / 10x. 중요 이벤트(위기 발생, 업적 기회, 조우)는 자동 일시정지 옵션.
한 판 목표 길이 약 5시간은 1배속 기준 약 18,000틱. 등급별 페이싱은 §3.2 참조.


### 2.2 통치자(Ruler) 10분 루프



```

시간 흐름 ─▶ 로그 피드에 사건 누적 ─▶ (자동 일시정지) ─▶ 결정

│

┌────────────────────────────────────────────────────┘

▼

연구 중점 조정 • 외교 태세/조약 • 칙령(Edict) 발동
거대 프로젝트 착수 • 함대 전력 배치 • 위기 대응 선택
│

▼

재개 ─▶ 결과가 로그로 돌아옴 ─▶ 위기 발생 ─▶ 업적(Feat) 기회 ─▶ 승천

```



1배속 기준 **20~40초마다 의미 있는 결정 하나**가 발생해야 한다. 그보다 뜸하면 위기 생성 빈도를 올린다.
결정은 모두 '수치 하나를 바꾸거나, 프로젝트 하나를 켜는' 수준이다. 미세 관리가 없다.


### 2.3 방랑자(Wanderer) 10분 루프



```

은하 지도에서 목적 성계 선택 ─▶ 이동 (레인당 N틱 소모, 도중 조우 확률)

│

▼

조우 카드 등장 (선택지 2~3개, 능력치 판정 + 자원 비용)

│

▼

보상/손실 ─▶ 유물·권능·개인 자원 ─▶ 성계 탐사(스캔) ─▶ 위기 성계 발견

│

▼

개입 선택 ─▶ 신화적 업적 조건 충족 ─▶ 승천 (또는 실패와 그 흔적)

```



방랑자의 '전투'는 없다. 모든 갈등은 조우 카드의 판정과 서술로 해결된다.
수련(Insight 소모 → 개인 능력치)은 성계에 머무르는 동안 틱 단위로 자동 진행된다.


### 2.4 한 판의 호(Arc)



| 구간 | 플레이 시간(1x) | 플레이어가 느껴야 할 것 |
|---|---|---|
| 서막 (Tier 1) | ~30분 | 작은 세계, 첫 위기, 첫 선택 |
| 성간 시대 (Tier 2) | ~60분 | 지도가 넓어짐, 다른 문명과의 마찰, 첫 거대 프로젝트 |
| 인과의 시대 (Tier 3) | ~90분 | 타락 초월자의 존재 인지, 경쟁 승천 문명 등장 |
| 초월 (Tier 4) | ~90분 | 세계를 바꿀 수 있는 힘과 그 대가 |
| 영겁 (Tier 5) | 엔딩 + 연대기 | 지금까지의 로그가 신화로 재편집되어 돌아옴 |



---



## 3. 핵심 게임플레이 3대 기둥



```

[ Cosmic Architecture ]

│

┌──────────────┼──────────────┐

▼ ▼ ▼

[Pillar 1] [Pillar 2] [Pillar 3]

문명 자동화 존재 등급과 이원화된 플레이

(Text/Data) 초월자(승천) (통치자 vs 방랑자)

│ ▲ │

└── 위기 생성 ──┘── 개입 수단 ──┘

```



세 기둥은 **위기(Crisis)**로 연결된다. Pillar 1의 시뮬레이션이 위기를 낳고, Pillar 3의 두 역할이 각자의 수단으로 개입하며, 그 결과가 Pillar 2의 업적이 된다.



### 3.1 Pillar 1: 추상화된 문명 및 경제 시뮬레이션 (Data Engine)



**문명 다양성 (Procedural Traits)**

윤리 3축(평화↔군사, 영성↔물질, 폐쇄↔개방), 생물학적 특성, 모행성 환경에서 고유 특성(Trait)이 조합된다.
특성은 `FGameplayTag`로 표현한다. 예: `Trait.Biology.HiveMind`, `Trait.Tech.NanoAffinity`, `Trait.Mind.PsionicAwakening`.


**자동화된 발전과 상호작용**

성계 단위로 자원 생산, 인구, 연구 진척이 백그라운드 틱으로 계산된다.
문명 AI는 단순 정책 프로필(보수/확장/학구/군사)로 행동한다. 복잡한 계획 AI는 만들지 않는다.
결과는 텍스트 피드로 전달된다.
"베텔게우스 성계의 광물 채굴 인공지능이 자아를 획득하여 생산량이 150% 증가했습니다."
"오리온 성운의 문명이 초광속 무역로를 개척하여 인근 3개 성계와 상업 연합을 결성했습니다."


### 3.2 Pillar 2: 존재 등급(Ascension Tier)과 초월자 시스템



우주 질서는 물리 법칙뿐 아니라 임계점을 넘은 개체들의 의지로 유지되고 재편된다.



**5단계 존재 등급**



| 등급 | 명칭 | 특성 및 권능 | 문명(통치자) 관점에서 열리는 행동 | 개인(방랑자) 관점에서 열리는 행동 |
|---|---|---|---|---|
| Tier 1 | 행성 구속형 (Planetary) | 단일 행성 물리 법칙에 종속 | 모성계 개발, 기초 연구 중점 | 모성계 내 탐사, 첫 함선 확보 |
| Tier 2 | 항성 융합형 (Stellar) | 항성 에너지 운용, 성간 항해 | 다이슨 스웜 프로젝트, 성계 확장, 무역 조약 | 레인 이동, 조우, 유물 수집 |
| Tier 3 | 인과 간섭형 (Causal) | 시공간 왜곡, 아공간 물리 | 웜홀 게이트(레인 추가), 시간 왜곡 연구, 자원 '인과(Causality)' 해금 | 아공간 유물 조작, 성계 간 즉시 도약(쿨다운), 위기 직접 개입 |
| Tier 4 | 초월자 (Transcendent) | 성계 물리 법칙을 단독 의지로 재편 | 집단 정신 승천, 인과율 병기, 칙령 범위 확대 | 손짓 테라포밍(성계 Hazard 제거), 타락 초월자와 직접 대면 |
| Tier 5 | 우주 질서 (Eternal Order) | 우주 상수의 화신 | 은하 법칙 제정, 영겁의 문명 (엔딩) | 우주와 일체화 (엔딩) |



**승천의 대가**: 모든 등급 상승은 무언가를 바꾼다. 예: Tier 3→4 집단 정신 승천 시 문명의 '개방' 축이 폐쇄 쪽으로 이동하고 일부 개인성 특성이 사라진다. 방랑자가 Tier 4가 되면 Tier 1~2 문명과의 조우 카드가 "경외/공포" 분기로 바뀐다.



**신화적 업적 (Mythic Feats)**



경험치 누적으로는 절대 승천하지 않는다. 세계에 거대한 파동을 남기는 사건을 달성해야 한다. 각 등급 구간에 통치자 경로와 방랑자 경로가 최소 하나씩 있고, 대부분은 §4.1의 위기와 연결된다.



| 구간 | 통치자 경로 (Ruler Feat) | 방랑자 경로 (Wanderer Feat) | 연결되는 위기/상태 |
|---|---|---|---|
| T1→T2 | 다이슨 스웜 1단계 완성 (거대 프로젝트, 에너지 투입 + 물리 연구 50%) | 항성 코로나 잠수 (조우 체인 3단계, 육신 판정) | 없음 (초입이므로 자력 달성) |
| T2→T3 | 붕괴 직전 초신성 안정화 (위기 대응 프로젝트, 카운트다운 내 완료) | 초신성 에너지 단독 흡수 (해당 성계에 존재 + 인과 판정) | `Hazard.PreSupernova` |
| T3→T4 | 절멸 위기 종족을 차원 방주로 이주 (프로젝트 + 외교 조약) | 아공간 균열 단독 봉합, 또는 승천 중인 문명의 '정신의 닻' 되기 | `Hazard.Extinction`, `Hazard.SubspaceErosion` |
| T4→T5 | 은하 법칙 3개를 N틱 이상 유지 + 타락 초월자 1체 봉인 | 고대 타락 초월자의 코어 봉인을 단독 해제하고 인과율 흡수 | 타락 초월자 (§4.2) |



업적 정의는 데이터(`DT_Feats`)이며, 조건은 §7.3의 조건 구조체로 표현한다. 위 표는 수직 슬라이스용 최소 8개이고, 콘텐츠 확장 단계에서 등급당 3~4개로 늘린다.



### 3.3 Pillar 3: 이원화된 플레이어 경험 (Dual Role Loop)



```

[ 플레이어의 진로 ]

│

┌──────────┴──────────┐

▼ ▼

[문명 통치자] [단독 방랑자]

(Ruler Mode) (Wanderer Mode)

│ │

연구/외교/칙령 • 이동/조우/탐사
거대 프로젝트 • 수련/유물/개입
제국 전체의 승천 • 개인의 승천
│ │

└────< 상호 전환 >─────┘

방랑자 → 경외도 높은 문명의 수호신/군주로 즉위

통치자 → 섭정에게 제국을 맡기고 영겁의 방랑 시작

```



두 모드는 **같은 은하, 같은 이벤트 풀, 같은 위기**를 공유한다. 콘텐츠는 한 번 만들고 두 시점에서 소비한다.
개인 등급(PersonalTier)과 문명 등급(Civ Tier)은 별개의 값이다. 통치자 모드에서는 문명 등급이, 방랑자 모드에서는 개인 등급이 전면에 나온다.
전환 규칙은 §5.10.


---



## 4. 긴장과 위기: 샌드박스를 '게임'으로 만드는 장치



초안에는 적대 요소가 없었다. 이 장이 없으면 게임은 '예쁜 로그 감상기'가 된다.



### 4.1 위기 생성기 (Crisis Generator)



매 틱, 각 성계는 항성 유형·나이·소유 문명 특성에 따라 작은 확률로 **Hazard**를 얻는다.
Hazard는 **카운트다운**을 가진다. 기한 내 해결되지 않으면 결과가 확정된다(성계 소멸, 문명 붕괴, 레인 차단 등).
플레이어와 AI 문명 모두 해결을 시도할 수 있다. 해결 주체가 업적 조건을 만족하면 승천 기회가 된다.
즉, **위기는 문명에게는 위협, 플레이어에게는 기회**다. 이것이 이 게임의 핵심 긴장이다.


수직 슬라이스용 Hazard 6종:



| 태그 | 이름 | 카운트다운 | 미해결 결과 | 해결 수단 (R=통치자, W=방랑자) |
|---|---|---|---|---|
| `Hazard.PreSupernova` | 초신성 전조 | 300틱 | 성계 소멸, 인접 성계 피해 | R: 안정화 프로젝트 / W: 현장 개입 판정 |
| `Hazard.AIAwakening` | 채굴 AI 각성 | 150틱 | 50%: 생산 +150% / 50%: 성계 반란 | R: 칙령 '기계 권리' / W: 조우 대화 |
| `Hazard.Plague` | 성간 역병 | 200틱 | 인구 -40%, 인접 전파 | R: 생명 연구 + 격리 조약 / W: 유물 사용 |
| `Hazard.SubspaceErosion` | 아공간 침식 | 400틱 | 레인 영구 차단 | R: 인과 연구 프로젝트 / W: Tier 3 봉합 |
| `Hazard.Extinction` | 절멸 위기 | 250틱 | 문명 삭제 | R: 방주 프로젝트 / W: 피난 호위 조우 체인 |
| `Hazard.Apostle` | 타락 초월자의 사도 출현 | 350틱 | 성계가 타락 초월자 영향권으로 편입 | R: 함대 전력 집중 / W: Tier 3+ 대면 |



### 4.2 타락 초월자 (Fallen Transcendents)



은하에는 플레이어 외에 2~3체의 **Tier 4 존재**가 데이터로 정의되어 있다. 각자 교리(Doctrine)가 있다.
침묵(Silence): 문명의 성장을 억제한다. 엔트로피의 화신.
동화(Assimilation): 문명을 흡수해 하나로 만든다.
광신(Zeal): 자신을 숭배하는 문명만 남긴다.
이들은 3D 모델이 아니다. **영향권(성계 집합) + 이벤트 가중치 수정자 + 로그 화자**로만 존재한다. 시각적으로는 Niagara 오라와 일러스트 1장.
플레이어가 Tier 3에 도달하면 이들이 플레이어를 '인지'하고, 플레이어 관련 이벤트 가중치가 바뀐다. Tier 4에서는 직접 대면 조우가 열린다.
이들은 최종 '보스' 층이지만 체력 바가 없다. 봉인·설득·대체·공존 중 어느 결말이든 업적 조건으로 표현된다.


### 4.3 경쟁 승천 문명과 시계



AI 문명도 같은 업적 조건(단순화된 판정)으로 승천한다. 한 AI 문명이 Tier 5에 먼저 도달하면 **"다른 질서"** 엔딩이 열린다. 패배라기보다 "당신이 아닌 누군가가 우주를 정의했다"는 결말이다.
이것이 샌드박스에 시계를 넣는 유일한 장치다. 난이도는 AI 승천 속도 배율로 조절한다.


### 4.4 실패 상태와 엔딩



| 상황 | 결과 |
|---|---|
| 통치 중인 문명이 붕괴 | 게임 오버가 아니다. 플레이어는 **강제로 방랑자가 된다.** 로그에 "몰락한 군주" 기록 |
| 방랑자의 육신 소멸 | 개인 Tier ≤ 2: 게임 오버(연대기 출력). Tier ≥ 3: 능력치 손실 후 환생 |
| 문명 Tier 5 도달 (통치자) | **집단 승천 엔딩** "영겁의 문명" |
| 개인 Tier 5 도달 (방랑자) | **단독 승천 엔딩** "우주와 일체화" |
| AI 문명이 먼저 Tier 5 | **다른 질서 엔딩** |
| 엔딩 이후 | 연대기(Chronicle) 출력. 선택 시 '영겁 모드'(관전/신 모드 샌드박스)로 계속. 영겁 모드는 Phase 6 이후 스트레치 |



---



## 5. 게임 시스템 상세 (린 설계)



각 시스템은 "데이터 몇 줄로 정의 가능한가"를 기준으로 깎았다.



### 5.1 자원 (5종)



| 코드 | 이름 | 생산 | 소비 | 해금 |
|---|---|---|---|---|
| Energy | 에너지 | 성계 항성 등급 × 소유 | 프로젝트, 함대 유지 | T1 |
| Matter | 물질 | 성계 자원 풍부도 | 프로젝트, 확장 | T1 |
| Insight | 지식 | 인구 × 영성 축 | 연구, 수련 | T1 |
| Influence | 영향력 | 인구 × 개방 축 × 조약 수 | 외교, 칙령 | T1 |
| Causality | 인과 | T3 연구 완료 후 극소량 | 웜홀, 시간 왜곡, T3+ 권능 | T3 |



### 5.2 문명 윤리 3축



각 축은 -1.0 ~ +1.0의 float. 생산·외교·이벤트 가중치에 승수로 작용한다.



**Pacifism** (-1 군사 ↔ +1 평화)
**Spiritualism** (-1 물질 ↔ +1 영성). 초안의 "영지"는 이 축으로 해석했다.
**Openness** (-1 폐쇄 ↔ +1 개방)


윤리는 고정이 아니다. 위기 결과와 승천의 대가로 서서히 이동한다.



### 5.3 특성 (Traits)



`FGameplayTagContainer`. 문명 생성 시 3~5개 부여. 각 특성은 `DT_Traits` 행에 효과(자원 승수, 이벤트 가중치, 로그 형용사 뱅크)를 갖는다.
수직 슬라이스 목표 20개. 확장 시 60개.


### 5.4 연구 (4개 영역, 트리 없음)



| 영역 | 해금하는 것 |
|---|---|
| Physics (물리) | 다이슨 스웜, 초신성 안정화, 함대 전력 승수 |
| Society (사회) | 조약 종류, 안정도 회복, 칙령 슬롯 |
| Engineering (공학) | 확장 비용 감소, 방주, 웜홀 게이트 |
| Transcendence (초월) | Causality 해금, 집단 정신 승천, 인과율 병기 |



각 영역은 0.0~1.0 진척도 하나. 임계점(0.25/0.5/0.75/1.0)마다 '돌파(Breakthrough)' 1개가 열리고 로그에 뜬다.
통치자의 유일한 조작은 **4개 영역 간 중점 비율 슬라이더**다.


### 5.5 외교 (태세 + 조약 3종)



문명 간 관계는 -1~+1 스칼라 하나. 윤리 거리, 국경 접촉, 이벤트로 변한다.
조약: 무역(Influence +, 역병 전파 +), 불가침, 동맹(함대 전력 공유).
통치자의 조작: 문명별 태세(우호/중립/적대) + 조약 제안. 승낙 여부는 관계 스칼라와 Influence 비용으로 즉시 판정.


### 5.6 분쟁 (완전 추상화)



함대는 유닛이 아니라 **FleetPower(float) 하나**다. 통치자는 전력을 성계에 비율로 배치한다.
적대 문명이 접촉한 성계에서는 틱마다 전력 비교 + 난수로 소유권 변동 확률을 계산한다.
결과는 로그 한 줄이다. "카시오페아 연합이 베가 성계의 궤도 방어선을 돌파했습니다. 성계 주권이 이양됩니다."


### 5.7 거대 프로젝트 (Megaprojects)



통치자 경로 업적의 실체다. 성계 하나를 지정하고 자원을 틱당 투입하면 진척도가 오른다.
수직 슬라이스 4종: 다이슨 스웜, 초신성 안정화, 차원 방주, 웜홀 게이트.
진행 중 프로젝트는 성계 노드에 링 이펙트로 표시된다 (Niagara 아님, 머티리얼 파라미터).


### 5.8 칙령 (Edicts)



제국 전체 또는 (T5) 은하 전체에 걸리는 수정자. 슬롯 수는 Society 연구로 증가.
수직 슬라이스 6종: 기계 권리, 전시 동원, 사상 개방, 격리령, 무역 자유화, 침묵의 금지(타락 초월자 영향 저항).
T5 통치자 업적 "은하 법칙 3개 유지"는 칙령 범위가 은하로 확장된 상태를 가리킨다.


### 5.9 방랑자 시스템



**개인 능력치 3종**: 육신(Body), 정신(Mind), 인과(Causality). 각 0~10. 수련과 유물로 상승.



**이동**: 레인 하나 통과에 N틱(기본 20). 이동 중 조우 확률. T3부터 '도약' 권능(쿨다운 있는 즉시 이동).



**조우 카드 (Encounter)**

제목, 서술, 선택지 2~3개. 각 선택지는 `능력치 판정(기준치)` + `자원 비용` + `결과 효과 + 로그`.
판정: `능력치 + 0~5 난수 ≥ 기준치`. 결정론적 RNG 사용 (§9.2).
조우도 `DT_Encounters` 데이터다. 조건(현재 성계 Hazard, 소유 문명 윤리, 개인 Tier)으로 필터링 후 가중치 추첨.
수직 슬라이스 25장. 확장 시 100장+.


**유물 (Relics)**: 소지 슬롯 3개. 각 유물은 능력치 보정 + 특정 조우 선택지 해금. 수직 슬라이스 8개.



**경외도 (Reverence)**: 각 문명이 플레이어에게 갖는 0~1 값. 그 문명의 위기를 해결하면 상승. 통치자 전환 조건.



### 5.10 역할 전환 규칙



| 전환 | 조건 | 즉시 효과 | 지속 효과 |
|---|---|---|---|
| 통치자 → 방랑자 | 언제나 가능 | 섭정 프로필 선택(보수/확장/학구/군사). 문명 안정도 -0.1 | 문명은 AI로 운영. 윤리가 섭정 성향으로 서서히 이동(섭정 표류) |
| 방랑자 → 통치자 | 대상 문명의 경외도 ≥ 0.6, 또는 안정도 < 0.2(지도자 공백) | 즉위. 개인 Tier > 문명 Tier이면 문명에 '수호신' 특성 부여 | 개인 Tier는 유지되고 숨겨짐. 방랑자로 돌아가면 다시 드러남 |
| 복귀 (전 제국으로) | 전 제국이 존속 중 | 섭정 해임. 섭정 표류로 바뀐 윤리는 되돌아오지 않음 | - |



전환은 공짜가 아니어야 하지만 벌칙처럼 느껴져도 안 된다. 핵심은 "떠나 있는 동안 세계가 변했다"를 로그로 보여주는 것이다.



---



## 6. UE5 기술 아키텍처



### 6.1 모듈 계층 구조



```

[ Presentation Layer (UI & Visuals) ]

├── UMG 위젯 (은하 지도 HUD, 성계 정보 패널, 로그 피드, 조우 카드, 상단 자원/시간 바)

├── Minimal 3D Viewport (성계 노드 액터, 레인 라인, 성운 배경 셰이더)

└── Niagara FX (승천 연출, 차원 균열, 초월자 오라) — 3종만

▲ (델리게이트 구독만. UI는 시뮬 상태를 읽기만 하고 명령은 Subsystem 함수 호출)

[ Core Logic Layer (GameInstance Subsystems) ]

├── UGalaxySimulationSubsystem 성계/문명 고정 스텝 틱, 경제·연구·외교·분쟁·위기·AI

├── UCosmicEventSubsystem 조건 평가, 가중치 추첨, 효과 적용, 쿨다운

├── UAscensionSubsystem 업적 조건 감시, 등급 상승, 권능 부여, 승천 연출 트리거

├── UCosmicLogSubsystem 템플릿 치환, 로그 큐, 연대기 집계

├── UPlayerRoleSubsystem 통치자/방랑자 전환, 이동, 조우 진행, 명령 API

└── UCosmicSaveSubsystem FGalaxyState ↔ USaveGame 직렬화, 슬롯 관리

▲

[ Data Layer ]

├── 정의(Definition, 읽기 전용, DataTable): DT_Traits, DT_Events, DT_Feats, DT_Encounters, DT_Relics, DT_Edicts, DT_Hazards, DT_LogTemplates

├── 상태(State, 런타임, 저장 대상, USTRUCT): FGalaxyState { Systems, Civs, Player, Transcendents, Log, Rng }

└── 문자열: String Table ST_CA (UI), DataTable의 FText 컬럼 (서사)

```



**원칙**

시뮬레이션 코드는 UI와 월드(Actor)를 모른다. Core Logic Layer 폴더(`Sim/`, `Events/`, `Ascension/`, `Log/`, `Player/`, `Save/`) 안의 코드가 `UUserWidget`이나 `AActor`를 include하면 설계 위반이다.
UI는 Subsystem의 델리게이트를 구독하고 상태를 읽는다. 상태를 직접 수정하지 않는다.
뷰포트의 성계 액터는 상태의 '시각 표현'일 뿐이다. 액터가 없어도 시뮬은 돌아야 한다 (헤드리스 테스트의 전제).


### 6.2 Subsystem 선택 근거



모두 `UGameInstanceSubsystem`. 레벨 전환(메뉴 ↔ 은하 맵)에도 상태가 유지된다.
틱은 `FTickableGameObject`를 함께 상속해 받는다. 월드 타이머에 의존하지 않는다.
상태는 Subsystem이 들고 있는 `FGalaxyState` 구조체 하나에 모은다. UObject 그래프로 만들지 않는다. 저장이 단순해지고 결정론 검증(해시)이 가능하다.


### 6.3 UI 기술 선택: UMG 우선



초안은 CommonUI였다. CommonUI의 강점(입력 라우팅, 플랫폼별 액션 바, 패드 포커스)은 PC 싱글 텍스트 게임에서는 거의 쓰이지 않고, 학습 비용과 셋업 비용이 있다.
**결정: UMG + C++ 위젯 베이스 클래스**로 시작한다. 스팀덱/패드 지원을 목표에 넣는 시점에 CommonUI 전환을 재검토한다 (§16).
로그 피드는 `UListView`로 가상화한다. 수천 줄을 Text Block으로 쌓지 않는다.


### 6.4 3D 뷰포트 (최소)



`AStarSystemActor`: 스피어 메시 1개 + 이미시브 머티리얼(항성 색/크기 파라미터) + 선택 콜리전. 성계 수 ≤ 150이면 개별 액터로 충분하다.
레인: `ULineBatchComponent` 또는 얇은 스플라인 메시. 수직 슬라이스에서는 라인 배치로 충분.
카메라: 스프링 암을 가진 폰. 궤도 회전 + 줌 + 더블 클릭 포커스. 클릭은 라인 트레이스로 성계 선택.
배경: 스카이스피어에 성운 노이즈 머티리얼 1장.
성계 상태 표시(소유 문명 색, Hazard 경고, 프로젝트 링)는 모두 머티리얼 파라미터와 작은 월드 위젯으로 처리한다.


### 6.5 Niagara 예산 (3종)



| 시스템 | 트리거 | 비고 |
|---|---|---|
| NS_Ascension | 등급 상승 | 화면 중앙, 3~5초, 사운드 + 서사 텍스트 동반 |
| NS_Rift | `Hazard.SubspaceErosion` 발생/봉합 | 성계 노드 위치 |
| NS_TranscendentAura | 타락 초월자 영향권 성계, 플레이어 T4 이상 | 루프, 저비용 |



### 6.6 모듈 의존성 (Build.cs)



```csharp

PublicDependencyModuleNames.AddRange(new string[] {

"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",

"UMG", "Slate", "SlateCore",

"GameplayTags", "Niagara"

});

// CommonUI는 넣지 않는다 (§6.3). 필요 시 재검토.

```



---



## 7. 데이터 모델 및 C++ 설계안



**규칙**: 정의(Def)는 `FTableRowBase` 파생 USTRUCT로 DataTable에, 상태(State)는 일반 USTRUCT로 `FGalaxyState` 안에. 식별자는 `FName`, 분류·특성은 `FGameplayTag`. 사용자에게 보이는 문자열은 `FText`.



아래 코드는 설계 스케치다. 실제 구현 시 Claude Code에 "이 스케치를 기준으로 컴파일되는 헤더/소스 작성"을 요청한다.



### 7.1 공통 타입 (Core/CosmicTypes.h)



```cpp

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "CosmicTypes.generated.h"



UENUM(BlueprintType)

enum class EAscensionTier : uint8

{

Tier1_Planetary UMETA(DisplayName = "Tier 1 - Planetary"),

Tier2_Stellar UMETA(DisplayName = "Tier 2 - Stellar"),

Tier3_Causal UMETA(DisplayName = "Tier 3 - Causal"),

Tier4_Transcendent UMETA(DisplayName = "Tier 4 - Transcendent"),

Tier5_EternalOrder UMETA(DisplayName = "Tier 5 - Eternal Order")

};



UENUM(BlueprintType)

enum class EPlayerRole : uint8 { Ruler, Wanderer };



UENUM(BlueprintType)

enum class ECosmicResource : uint8 { Energy, Matter, Insight, Influence, Causality };



USTRUCT(BlueprintType)

struct FResourcePool

{

GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite) float Energy = 0.f;

UPROPERTY(EditAnywhere, BlueprintReadWrite) float Matter = 0.f;

UPROPERTY(EditAnywhere, BlueprintReadWrite) float Insight = 0.f;

UPROPERTY(EditAnywhere, BlueprintReadWrite) float Influence = 0.f;

UPROPERTY(EditAnywhere, BlueprintReadWrite) float Causality = 0.f; // Tier 3+



float Get(ECosmicResource R) const;

void Add(ECosmicResource R, float Amount);

};



USTRUCT(BlueprintType)

struct FEthicsAxes

{

GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin=-1, ClampMax=1)) float Pacifism = 0.f; // -1 군사 ~ +1 평화

UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin=-1, ClampMax=1)) float Spiritualism = 0.f; // -1 물질 ~ +1 영성

UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin=-1, ClampMax=1)) float Openness = 0.f; // -1 폐쇄 ~ +1 개방

};

```



### 7.2 런타임 상태 (Core/GalaxyState.h)



```cpp

USTRUCT(BlueprintType)

struct FStarSystemState

{

GENERATED_BODY()

UPROPERTY(BlueprintReadOnly) FName SystemID;

UPROPERTY(BlueprintReadOnly) FText DisplayName;

UPROPERTY(BlueprintReadOnly) FVector Position = FVector::ZeroVector; // 뷰포트 배치용

UPROPERTY(BlueprintReadOnly) TArray<FName> Lanes; // 인접 성계

UPROPERTY(BlueprintReadOnly) FGameplayTag StarClass; // Star.Class.G 등

UPROPERTY(BlueprintReadOnly) float ResourceRichness = 1.f;

UPROPERTY(BlueprintReadOnly) FName OwnerCivID; // NAME_None = 무주

UPROPERTY(BlueprintReadOnly) FGameplayTag ActiveHazard; // 없으면 빈 태그

UPROPERTY(BlueprintReadOnly) int32 HazardTicksRemaining = 0;

UPROPERTY(BlueprintReadOnly) FName ActiveMegaproject;

UPROPERTY(BlueprintReadOnly) float MegaprojectProgress = 0.f;

};



USTRUCT(BlueprintType)

struct FCivilizationState

{

GENERATED_BODY()

UPROPERTY(BlueprintReadOnly) FName CivID;

UPROPERTY(BlueprintReadOnly) FText CivName;

UPROPERTY(BlueprintReadOnly) FLinearColor Color = FLinearColor::White;

UPROPERTY(BlueprintReadOnly) EAscensionTier Tier = EAscensionTier::Tier1_Planetary;

UPROPERTY(BlueprintReadOnly) FEthicsAxes Ethics;

UPROPERTY(BlueprintReadOnly) FGameplayTagContainer Traits;

UPROPERTY(BlueprintReadOnly) FResourcePool Stockpile;

UPROPERTY(BlueprintReadOnly) FResourcePool IncomePerTick; // 캐시, 매 틱 재계산

UPROPERTY(BlueprintReadOnly) float Population = 1.f; // 추상 단위

UPROPERTY(BlueprintReadOnly) TMap<FName, float> Research; // "Physics" 등 -> 0~1

UPROPERTY(BlueprintReadOnly) TMap<FName, float> ResearchFocus; // 합계 1.0

UPROPERTY(BlueprintReadOnly) float FleetPower = 0.f;

UPROPERTY(BlueprintReadOnly) float Stability = 0.75f;

UPROPERTY(BlueprintReadOnly) TArray<FName> OwnedSystems;

UPROPERTY(BlueprintReadOnly) TMap<FName, float> Relations; // CivID -> -1~1

UPROPERTY(BlueprintReadOnly) TArray<FName> ActiveEdicts;

UPROPERTY(BlueprintReadOnly) FGameplayTag AIProfile; // AI.Profile.Expansionist 등

UPROPERTY(BlueprintReadOnly) float ReverenceToPlayer = 0.f; // 0~1, 방랑자→군주 전환 조건

UPROPERTY(BlueprintReadOnly) bool bIsPlayerRuled = false;

};



USTRUCT(BlueprintType)

struct FPlayerEntityState

{

GENERATED_BODY()

UPROPERTY(BlueprintReadOnly) EPlayerRole Role = EPlayerRole::Wanderer;

UPROPERTY(BlueprintReadOnly) EAscensionTier PersonalTier = EAscensionTier::Tier1_Planetary;

UPROPERTY(BlueprintReadOnly) FName CurrentSystemID;

UPROPERTY(BlueprintReadOnly) FName TravelTargetSystemID;

UPROPERTY(BlueprintReadOnly) int32 TravelTicksRemaining = 0;

UPROPERTY(BlueprintReadOnly) FName RuledCivID; // Ruler일 때만

UPROPERTY(BlueprintReadOnly) FName FormerCivID; // 섭정에게 맡긴 제국

UPROPERTY(BlueprintReadOnly) int32 Body = 1, Mind = 1, Causality = 0; // 0~10

UPROPERTY(BlueprintReadOnly) FResourcePool PersonalPool;

UPROPERTY(BlueprintReadOnly) TArray<FName> Relics; // 최대 3

UPROPERTY(BlueprintReadOnly) FGameplayTagContainer Powers;

UPROPERTY(BlueprintReadOnly) TArray<FName> CompletedFeats;

};



USTRUCT(BlueprintType)

struct FTranscendentState

{

GENERATED_BODY()

UPROPERTY(BlueprintReadOnly) FName TranscendentID;

UPROPERTY(BlueprintReadOnly) FGameplayTag Doctrine; // Doctrine.Silence 등

UPROPERTY(BlueprintReadOnly) TArray<FName> InfluencedSystems;

UPROPERTY(BlueprintReadOnly) bool bAwareOfPlayer = false;

UPROPERTY(BlueprintReadOnly) bool bSealed = false;

};



USTRUCT(BlueprintType)

struct FGalaxyState

{

GENERATED_BODY()

UPROPERTY() int32 SaveVersion = 1;

UPROPERTY() int32 Seed = 0;

UPROPERTY() int64 Tick = 0;

UPROPERTY() int32 RngState = 0; // FRandomStream 복원용

UPROPERTY() TArray<FStarSystemState> Systems;

UPROPERTY() TArray<FCivilizationState> Civs;

UPROPERTY() TArray<FTranscendentState> Transcendents;

UPROPERTY() FPlayerEntityState Player;

UPROPERTY() TMap<FName, int64> EventCooldownUntilTick;

UPROPERTY() TArray<FCosmicLogEntry> Log; // 상한 유지 (§8.4)

};

```



### 7.3 정의 데이터 (Data/*.h)



```cpp

UENUM(BlueprintType)

enum class EConditionType : uint8

{

CivTierAtLeast, CivHasTrait, CivResourceAtLeast, CivStabilityBelow, CivEthicsAxisAbove,

SystemHasHazard, SystemOwnedBy, MegaprojectComplete,

PlayerTierAtLeast, PlayerInSystemWithHazard, PlayerStatAtLeast, PlayerHasRelic, PlayerRoleIs,

GalaxyTickAtLeast, TranscendentSealed

};



USTRUCT(BlueprintType)

struct FCosmicCondition

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) EConditionType Type = EConditionType::GalaxyTickAtLeast;

UPROPERTY(EditAnywhere) FGameplayTag Tag; // 특성/Hazard/교리 등

UPROPERTY(EditAnywhere) FName Key; // 연구 영역, 자원 이름, 능력치 이름 등

UPROPERTY(EditAnywhere) float Value = 0.f; // 임계값

};



UENUM(BlueprintType)

enum class EEffectType : uint8

{

AddResource, MultiplyIncome, ShiftEthics, AddTrait, RemoveTrait, ChangeStability, ChangeRelation,

SpawnHazard, ResolveHazard, TransferSystem, ChangeReverence, GrantPower, PlayerStatDelta, SetFlag

};



USTRUCT(BlueprintType)

struct FCosmicEffect

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) EEffectType Type = EEffectType::AddResource;

UPROPERTY(EditAnywhere) FGameplayTag Tag;

UPROPERTY(EditAnywhere) FName Key;

UPROPERTY(EditAnywhere) float Value = 0.f;

UPROPERTY(EditAnywhere) int32 DurationTicks = 0; // 0 = 즉시/영구

};



// DT_Events

USTRUCT(BlueprintType)

struct FCosmicEventDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FGameplayTag Category; // Event.Civ.Economy, Event.Civ.Society, Event.Crisis.Supernova, Event.Crisis.Transcendent (DefaultGameplayTags.ini의 Event.* 트리와 동일하게 유지)

UPROPERTY(EditAnywhere) TArray<FCosmicCondition> Conditions; // AND

UPROPERTY(EditAnywhere) float BaseWeight = 1.f;

UPROPERTY(EditAnywhere) int32 CooldownTicks = 200;

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> Effects;

UPROPERTY(EditAnywhere) TArray<FText> LogTemplates; // 3개 이상 권장. {Civ} {System} {Trait} {Value}

UPROPERTY(EditAnywhere) uint8 Severity = 0; // 0 일반, 1 주목, 2 신화적

};



// DT_Hazards

USTRUCT(BlueprintType)

struct FHazardDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FGameplayTag HazardTag;

UPROPERTY(EditAnywhere) FText DisplayName;

UPROPERTY(EditAnywhere) int32 CountdownTicks = 300;

UPROPERTY(EditAnywhere) float SpawnChancePerTick = 0.0005f;

UPROPERTY(EditAnywhere) TArray<FCosmicCondition> SpawnConditions;

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnExpireEffects;

UPROPERTY(EditAnywhere) FName RulerResolutionMegaproject; // 통치자 해결 수단

UPROPERTY(EditAnywhere) FName WandererResolutionEncounter; // 방랑자 해결 수단

};



// DT_Feats

USTRUCT(BlueprintType)

struct FAscensionFeatDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FText Title;

UPROPERTY(EditAnywhere) FText MythicNarrative; // 승천 연출 중 표시되는 서사

UPROPERTY(EditAnywhere) EAscensionTier RequiredTier = EAscensionTier::Tier1_Planetary;

UPROPERTY(EditAnywhere) EAscensionTier TargetTier = EAscensionTier::Tier2_Stellar;

UPROPERTY(EditAnywhere) EPlayerRole Path = EPlayerRole::Ruler;

UPROPERTY(EditAnywhere) TArray<FCosmicCondition> CompletionConditions;

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> AscensionCost; // 승천의 대가 (§3.2)

UPROPERTY(EditAnywhere) FGameplayTagContainer GrantedPowers;

UPROPERTY(EditAnywhere) TSoftObjectPtr<class UNiagaraSystem> FX;

UPROPERTY(EditAnywhere) TSoftObjectPtr<class USoundBase> Sound;

};



// DT_Encounters

USTRUCT(BlueprintType)

struct FEncounterChoice

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FText Label;

UPROPERTY(EditAnywhere) FName CheckStat; // Body / Mind / Causality / None

UPROPERTY(EditAnywhere) int32 Difficulty = 0;

UPROPERTY(EditAnywhere) FResourcePool Cost;

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnSuccess;

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnFailure;

UPROPERTY(EditAnywhere) FText SuccessText;

UPROPERTY(EditAnywhere) FText FailureText;

UPROPERTY(EditAnywhere) FName NextEncounter; // 체인용

};



USTRUCT(BlueprintType)

struct FEncounterDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FText Title;

UPROPERTY(EditAnywhere) FText Body;

UPROPERTY(EditAnywhere) TArray<FCosmicCondition> Conditions;

UPROPERTY(EditAnywhere) float BaseWeight = 1.f;

UPROPERTY(EditAnywhere) TArray<FEncounterChoice> Choices; // 2~3

UPROPERTY(EditAnywhere) TSoftObjectPtr<class UTexture2D> Illustration; // 선택

};

```



### 7.4 시뮬레이션 Subsystem (Sim/GalaxySimulationSubsystem.h)



```cpp

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSimTick, int64, Tick);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGalaxyGenerated, int32, Seed);



UCLASS()

class UGalaxySimulationSubsystem : public UGameInstanceSubsystem, public FTickableGameObject

{

GENERATED_BODY()

public:

// UE 5.8 Tickable.h 규칙: UObject와 함께 쓸 때는 Never로 생성하고 Initialize 끝에서 켠다. Deinitialize에서 다시 Never.

UGalaxySimulationSubsystem() : FTickableGameObject(ETickableTickType::Never) {}

virtual void Initialize(FSubsystemCollectionBase& Collection) override; // 끝에서 SetTickableTickType(ETickableTickType::Conditional)

virtual void Deinitialize() override; // 처음에 SetTickableTickType(ETickableTickType::Never)



// FTickableGameObject (IsAllowedToTick은 5.5에서 deprecated. 오버라이드하지 않는다)

virtual void Tick(float DeltaTime) override;

virtual bool IsTickable() const override { return bRunning && TimeScale > 0.f; }

virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UGalaxySimulationSubsystem, STATGROUP_Tickables); }



UFUNCTION(BlueprintCallable) void NewGalaxy(int32 Seed, int32 SystemCount, int32 CivCount);

UFUNCTION(BlueprintCallable) void SetTimeScale(float Scale); // 0,1,2,5,10

UFUNCTION(BlueprintCallable) void StepOnce(); // 디버그/테스트

UFUNCTION(BlueprintCallable) void RunHeadless(int64 Ticks); // 테스트/밸런스용, 델리게이트 억제



UFUNCTION(BlueprintPure) const FGalaxyState& GetState() const { return State; }

FGalaxyState& GetMutableState() { return State; }

FRandomStream& GetRng() { return Rng; }



UPROPERTY(BlueprintAssignable) FOnSimTick OnSimTick;

UPROPERTY(BlueprintAssignable) FOnGalaxyGenerated OnGalaxyGenerated;



// 저장/로드에서 호출

void LoadState(const FGalaxyState& In);

uint32 ComputeStateHash() const; // 결정론 테스트용



private:

void SimulateTick(); // 고정 순서: 경제 → 연구 → 외교 → 분쟁 → 위기 → AI → 플레이어 이동

void TickEconomy();

void TickResearch();

void TickDiplomacy();

void TickConflict();

void TickHazards();

void TickCivAI();

void TickPlayerTravel();



UPROPERTY() FGalaxyState State;

FRandomStream Rng;

float Accumulator = 0.f;

float TimeScale = 1.f;

bool bRunning = false;

static constexpr float SecondsPerTick = 1.0f;

};

```



### 7.5 승천·이벤트·로그 Subsystem 인터페이스 요약



```cpp

// UCosmicEventSubsystem

// - void EvaluateAndFire(FGalaxyState&, FRandomStream&) 매 틱 호출. 조건 통과 이벤트 중 가중치 추첨, 쿨다운 기록

// - bool Check(const FCosmicCondition&, const FEvalContext&) 조건 1개 평가 (업적/조우도 같은 함수 사용)

// - void Apply(const FCosmicEffect&, FEvalContext&) 효과 1개 적용

// - FOnCosmicEvent OnEventFired(FName EventRow, FEvalContext)



// UAscensionSubsystem

// - 매 틱 + 주요 이벤트 후 EvaluateFeats()

// - FOnTierAscended OnTierAscended(EAscensionTier NewTier, FName FeatRow, EPlayerRole Path)

// - 승천 시: 대가 적용 → 권능 부여 → 로그 Severity 2 → FX/사운드 요청 (Presentation이 구독)



// UCosmicLogSubsystem

// - void Push(FGameplayTag Category, uint8 Severity, const FText& Template, const FFormatNamedArguments&, FName FocusSystem)

// - FOnLogEntry OnLogEntry(const FCosmicLogEntry&)

// - TArray<FCosmicLogEntry> BuildChronicle() const Severity 2 + 선택된 1 항목을 엔딩용으로 집계



// UPlayerRoleSubsystem

// - BecomeRuler(FName CivID) / BecomeWanderer(FGameplayTag RegentProfile) / ReturnToFormerCiv()

// - TravelTo(FName SystemID) / ScanCurrentSystem() / Train(FName Stat)

// - 통치자 명령: SetResearchFocus, SetStance, ProposeTreaty, EnactEdict, StartMegaproject, AllocateFleet

// - 조우: FOnEncounterPresented / ResolveEncounterChoice(int32 Index)

```



### 7.6 초안 코드와의 차이



| 초안 | v0.2 | 이유 |
|---|---|---|
| `TArray<FString> ActiveTraits` | `FGameplayTagContainer Traits` | 오타 방지, 계층 질의, 에디터 검증. 초안 자체의 "FName 위주" 원칙과도 일치 |
| `FCivilizationData` 하나 | `FCivilizationState`(런타임) + `DT_Traits` 등 정의 | 저장 대상과 읽기 전용 데이터 분리 |
| `FAscensionFeat`에 `FeatID` 멤버 | 행 이름이 ID. 구조체는 `FTableRowBase` 파생 | DataTable 관례 |
| 조건이 없음 | `FCosmicCondition` 배열 | 업적·이벤트·조우가 같은 조건 평가기를 공유 |
| 한글 DisplayName | 영문 DisplayName | §10.3 |



---



## 8. 프로시저럴 텍스트 / 로그 시스템



이 게임에서 텍스트는 장식이 아니라 **주 보상**이다. 로그가 지루하면 게임이 지루하다.



### 8.1 로그 항목



```cpp

USTRUCT(BlueprintType)

struct FCosmicLogEntry

{

GENERATED_BODY()

UPROPERTY(BlueprintReadOnly) int64 Tick = 0;

UPROPERTY(BlueprintReadOnly) FGameplayTag Category; // Log.Civ / Log.Crisis / Log.Conflict / Log.Discovery / Log.Ascension / Log.Transcendent

UPROPERTY(BlueprintReadOnly) uint8 Severity = 0; // 0 일반(회색), 1 주목(흰색), 2 신화적(금색 + 자동 일시정지 후보)

UPROPERTY(BlueprintReadOnly) FText Message;

UPROPERTY(BlueprintReadOnly) FName FocusSystemID; // 클릭 시 카메라 이동

UPROPERTY(BlueprintReadOnly) FName SourceRow; // 디버그/연대기 집계용

};

```



### 8.2 템플릿 문법



`FText::Format` + `FFormatNamedArguments`. 슬롯: `{Civ}`, `{System}`, `{OtherCiv}`, `{Trait}`, `{Value}`, `{Tier}`, `{Hazard}`, `{Player}`.
이벤트마다 템플릿 **3개 이상**. 같은 문장이 10분 안에 두 번 보이면 실패다.
문명별 **형용사 뱅크**: 윤리 축에서 뽑는다. 군사 → "호전적인", 영성 → "명상적인", 폐쇄 → "은둔하는". 템플릿의 `{CivAdj}` 슬롯에 들어간다.
타락 초월자 관련 로그는 **1인칭 화자**로 쓴다. 다른 로그와 톤이 구분되어야 한다. "나는 베가의 불빛이 꺼지는 것을 지켜보았다."


### 8.3 서사 가이드 (작성 규칙)



한 줄은 두 문장 이내. 첫 문장은 사실, 둘째 문장은 함의.
숫자는 꼭 필요할 때만. "생산량이 150% 증가"는 좋고 "에너지 +37.2"는 나쁘다.
Severity 2 로그는 반드시 플레이어 또는 문명의 선택과 연결된 결과여야 한다. 랜덤 사건을 신화적으로 포장하지 않는다.


### 8.4 상한과 연대기



`FGalaxyState.Log`는 최근 2,000개를 유지한다. 넘치는 항목 중 Severity 0은 버리고, 1·2는 `Chronicle` 배열로 옮긴다.
엔딩 시 `BuildChronicle()`이 Severity 2 전부 + Severity 1 중 플레이어 관련 항목을 시간순으로 엮어 에필로그 페이지를 만든다. 플레이어의 한 판이 '신화'로 재편집되어 돌아오는 순간이다.


### 8.5 로컬라이제이션



사용자에게 보이는 모든 문자열은 `FText`. C++ 안의 UI 문자열은 `LOCTEXT`/`NSLOCTEXT("Cosmic_Ascension", ...)`.
서사 템플릿은 DataTable의 FText 컬럼, UI 라벨은 String Table `ST_CA`.
한국어로 먼저 쓰고, 영어는 Localization Dashboard로 Phase 5 이후에 붙인다.


---



## 9. 저장, 결정론, 테스트, 밸런스



초안에 없던 장이지만 Phase 1부터 들어가야 한다. 나중에 붙이면 전체 리팩터링이 된다.



### 9.1 저장/로드



`UCosmicSaveGame : USaveGame { UPROPERTY() FGalaxyState State; }` 하나.
상태가 전부 USTRUCT이므로 `UGameplayStatics::SaveGameToSlot`로 끝난다. Actor나 위젯 상태는 저장하지 않는다. 로드 후 Presentation이 상태를 보고 다시 그린다.
`SaveVersion`을 두고, 구조체 필드 추가 시 기본값으로 호환한다. 필드 삭제·의미 변경은 버전 올리고 마이그레이션 함수를 쓴다.
자동 저장: 승천 직후, 역할 전환 직후, 10분마다.


### 9.2 결정론



모든 난수는 `UGalaxySimulationSubsystem::Rng`(`FRandomStream`) 하나를 통한다. `FMath::Rand`를 시뮬 코드에서 쓰면 설계 위반.
틱 내부 처리 순서는 고정(§7.4 주석). `TMap` 순회로 결과가 달라지는 코드를 쓰지 않는다. 순서가 중요한 곳은 `TArray`를 정렬해서 쓴다.
같은 시드 + 같은 플레이어 입력 시퀀스 → 같은 `ComputeStateHash()`. 이것이 테스트의 기준이다.
결정론의 보상: 버그 재현, 리플레이, 헤드리스 밸런스 측정이 모두 가능해진다.


### 9.3 테스트와 1인 밸런싱



**자동화 테스트** (`Source/Cosmic_Ascension/Tests/`)



| 테스트 | 검증 내용 |
|---|---|
| `Cosmic_Ascension.Sim.Determinism` | 시드 42로 3,000틱 두 번 → 해시 동일 |
| `Cosmic_Ascension.Sim.NoNaN` | 10,000틱 후 모든 float 유한 |
| `Cosmic_Ascension.Sim.SaveRoundTrip` | 저장 → 로드 → 해시 동일 |
| `Cosmic_Ascension.Data.Integrity` | 모든 DataTable 행의 태그/FName 참조가 존재 |
| `Cosmic_Ascension.Feats.Reachable` | AI만으로 20,000틱 돌려 최소 1개 문명이 T3 도달 |



**밸런스 커맨드렛**



```

UnrealEditor-Cmd Cosmic_Ascension.uproject -run=CosmicBalance -Seed=42 -Ticks=20000 -Systems=60 -Civs=8 -Out=balance.csv

```



100틱마다 문명별 자원·Tier·안정도·Hazard 수를 CSV로 떨어뜨린다. 스프레드시트에서 그래프를 본다.
1인 개발에서 "플레이해 보고 느낌으로 조절"은 시간이 가장 많이 드는 일이다. 커맨드렛이 그 시간을 10분의 1로 줄인다.


**커맨드렛과 Subsystem (2026-10-10 결정)**: `-run=CosmicBalance`로 뜨는 `UCosmicBalanceCommandlet : UCommandlet`(`Tests/`)은 UEditorEngine 아래에서 실행되며 UGameInstance가 없다. 따라서 §6.2의 Subsystem들이 자동으로 생기지 않는다. `Main()`에서 GameInstance를 직접 만든다: `UGameMapsSettings`의 GameInstanceClass(없으면 `UGameInstance::StaticClass()`)로 `NewObject<UGameInstance>(GEngine, Class)` → `AddToRoot()` → `InitializeStandalone()` → `GetSubsystem<UGalaxySimulationSubsystem>()`로 `NewGalaxy`/`RunHeadless`(§7.4) 호출 → 종료 시 `Shutdown()`, `RemoveFromRoot()`. 자동화 테스트(§9.3 표)는 `-game` 프로세스에서 돌아 GameInstance가 있으므로 이 부트스트랩이 필요 없다.


### 9.4 성능 예산



| 항목 | 예산 |
|---|---|
| 시뮬 1틱 (성계 120, 문명 12) | < 1 ms |
| 10x 배속 | 프레임당 최대 10틱 처리, 초과 시 틱을 다음 프레임으로 이월 (렌더 60fps 유지) |
| 로그 UI | `UListView` 가상화, 표시 상한 500 |
| 성계 액터 | ≤ 150개, 개별 Draw Call 허용 |



---



## 10. 프로젝트 구조 및 코딩 규약



### 10.1 폴더 구조



```

Cosmic_Ascension/

├── CLAUDE.md (§14.3 템플릿)

├── guideline.md (이 문서. 설계의 단일 기준)

├── .gitignore / .gitattributes (§10.5)

├── Tools/ 에디터 Python 스크립트 (make_galaxy_level.py 등. 헤드리스 자동화용, 게임 빌드에 포함되지 않음)

├── Cosmic_Ascension.uproject

├── Config/

│ └── DefaultGameplayTags.ini (태그는 ini에 선언, 코드에서 FGameplayTag::RequestGameplayTag)

├── Source/Cosmic_Ascension/

│ ├── Cosmic_Ascension.Build.cs

│ ├── Core/ 타입, 열거형, FGalaxyState, 상수

│ ├── Data/ FTableRowBase 파생 정의 구조체

│ ├── Sim/ UGalaxySimulationSubsystem, 경제/연구/외교/분쟁/위기/AI 틱 (UI·Actor include 금지)

│ ├── Events/ UCosmicEventSubsystem, 조건 평가기, 효과 적용기

│ ├── Ascension/ UAscensionSubsystem

│ ├── Log/ UCosmicLogSubsystem, 템플릿 유틸

│ ├── Player/ UPlayerRoleSubsystem, 조우 진행

│ ├── Save/ UCosmicSaveGame, UCosmicSaveSubsystem

│ ├── World/ AStarSystemActor, 카메라 폰, 선택 처리

│ ├── UI/ C++ 위젯 베이스 (로직 노출용, 레이아웃은 WBP에서)

│ └── Tests/ 자동화 테스트, 밸런스 커맨드렛

└── Content/CA/

├── Data/ DT_Traits, DT_Events, DT_Hazards, DT_Feats, DT_Encounters, DT_Relics, DT_Edicts, ST_CA

├── UI/ WBP_GalaxyHUD, WBP_SystemPanel, WBP_LogFeed, WBP_EncounterCard, WBP_AscensionOverlay

├── FX/ NS_Ascension, NS_Rift, NS_TranscendentAura

├── Materials/ M_Star, M_Lane, M_Nebula

└── Maps/ L_MainMenu, L_Galaxy

```



### 10.2 C++ / Blueprint 분담



| 영역 | C++ | Blueprint |
|---|---|---|
| 시뮬·이벤트·승천·저장 | 전부 | 없음 |
| 위젯 | 베이스 클래스(바인딩용 함수, 델리게이트 핸들러) | 레이아웃, 애니메이션, 스타일 |
| 성계 액터, 카메라 | 베이스 클래스 | 메시·머티리얼 할당 |
| Niagara/사운드 트리거 | 델리게이트 발행 | 구독해서 재생 |



### 10.3 소스 텍스트 규칙



C++ 소스 파일은 ASCII만 쓴다. `UMETA(DisplayName)`도 영문. 한글은 DataTable·String Table에만 둔다.
이유: 리뷰·diff 가독성, 로컬라이제이션 경로 단일화(사용자에게 보이는 문자열은 FText 하나로). UE 5.8 UBT는 `/utf-8 /wd4819`로 컴파일하므로 인코딩 경고(C4819)는 이유가 아니고 BOM도 필요 없다.
주석은 한글 허용(UTF-8 저장, BOM 불필요). 단, 코드 식별자와 문자열 리터럴은 영문.


### 10.4 명명



UE 표준 접두어(U/A/F/E) 외 프로젝트 접두어는 쓰지 않는다. 대신 이름에 `Cosmic`/`Galaxy`/`Ascension` 등 도메인 단어를 넣는다.
DataTable 행 이름: `snake_case` FName. 예: `crisis_pre_supernova`, `feat_dyson_swarm_1`.
GameplayTag: `Domain.Sub.Leaf`. 최대 3단계. 예: `Hazard.PreSupernova`, `Trait.Biology.HiveMind`.
에셋 접두어: `DT_`, `ST_`, `WBP_`, `NS_`, `M_`, `MI_`, `L_`.


### 10.5 버전 관리



Git + Git LFS(`.uasset`, `.umap`, 텍스처, 오디오). `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/` 제외.
커밋 단위는 "컴파일되는 상태". 헤더만 추가하고 커밋하지 않는다.


---



## 11. 로드맵 (현실화)



초안의 "총 10주"는 프로토타입 기준으로는 맞지만 UE5 1인 개발의 실제 달력으로는 비현실적이다. 아래는 각 Phase에 **종료 조건(Exit Criteria)**을 두고, 셋업과 폴리시 단계를 추가한 것이다. 기간은 주당 25~30시간 투입 기준이다.



```

[P0 셋업] ─▶ [P1 데이터 엔진] ─▶ [P2 이벤트·승천] ─▶ [P3 은하 뷰포트] ─▶ [P4 이원 루프] ─▶ [P5 수직 슬라이스 폴리시] ─▶ [P6+ 콘텐츠 → EA]

1주 2주 3주 4주 6주 4주 6~12개월

```



### Phase 0: 프로젝트 셋업 (1주)



UE 버전 고정, C++ 프로젝트 생성, 모듈 의존성(§6.6), 폴더 구조(§10.1), Git LFS, CLAUDE.md.
GameplayTags ini에 초기 태그 트리. 빈 Subsystem 6개가 컴파일되고 에디터가 뜨는 상태.
**종료 조건**: 커맨드라인 빌드 성공, 빈 L_Galaxy 맵 실행, 로그 1줄 출력.
**상태 (2026-10-10)**: 종료 조건 달성. 에디터·게임 타깃 빌드 성공, `Tools/make_galaxy_level.py`로 L_Galaxy 생성, `-game -nullrhi` 스모크 실행에서 Subsystem 6개의 LogCosmic 로그 확인. Git 초기화만 남음(Git/LFS 미설치).


### Phase 1: 데이터 엔진 프로토타입 (2주)



그래픽 없이 UMG 텍스트 패널 1개. 성계 5개를 코드로 생성. 1초마다 자원·연구가 쌓이고 로그가 흐른다.
포함: 고정 스텝 틱, 시간 배속, `FRandomStream`, 로그 템플릿 치환, **저장/로드 왕복**, 결정론 테스트, 밸런스 커맨드렛 골격.
**종료 조건**: 1,000틱 실행 후 저장 → 재시작 → 로드 → 해시 동일. 로그에 같은 문장이 연속 2회 뜨지 않음.


### Phase 2: 이벤트·위기·승천 (3주)



`UCosmicEventSubsystem` 조건/효과 평가기. Hazard 6종 중 3종(초신성, AI 각성, 역병). 업적 T1→T2, T2→T3 통치자 경로.
승천 연출: NS_Ascension + 사운드 + MythicNarrative 오버레이.
**종료 조건**: AI만 돌려도 Hazard가 발생·만료·해결되는 로그가 나온다. 플레이어가 초신성 위기에 프로젝트로 대응해 T3로 승천하는 플레이가 콘솔 명령 없이 가능하다.


### Phase 3: 은하 맵 뷰포트 (4주)



성계 20개 3D 노드, 레인, 카메라, 클릭 선택 → 우측 정보 패널. 소유 색/Hazard 경고/프로젝트 링 표시.
상단 바(자원 5종, 틱, 배속), 하단 로그 피드(ListView, 클릭 시 카메라 포커스).
**종료 조건**: 마우스만으로 Phase 2의 플레이가 가능. 60fps 유지.


### Phase 4: 방랑자 vs 통치자 루프 (6주)



방랑자: 이동, 조우 카드 UI, 능력치 판정, 유물, 수련, 경외도. 조우 25장.
통치자: 연구 중점 슬라이더, 외교 태세/조약, 칙령 6종, 거대 프로젝트 4종, 함대 배치.
전환: 섭정 위임, 즉위, 복귀. 타락 초월자 1체(침묵).
**종료 조건**: 새 게임에서 30분 동안 양쪽 모드를 각각 한 번 이상 경험하며 T1→T3 도달이 가능하다.


### Phase 5: 수직 슬라이스 폴리시 (4주)



밸런스 커맨드렛으로 등급별 페이싱(§2.4)을 ±30% 안에 맞춘다.
온보딩: 첫 10분을 안내하는 로그 기반 튜토리얼(별도 UI 없음).
외부 테스터 5~10명. 피드백 수집 → 단일 개선 목록.
**종료 조건**: 테스터 과반이 "다음에 뭘 할지 알겠다"와 "로그를 읽는 게 재미있다"에 동의.


### Phase 6+: 콘텐츠 확장과 Early Access (6~12개월)



이벤트 150+, 조우 100+, 특성 60, 업적 등급당 3~4, 타락 초월자 3체, 엔딩 3종, 영어 로컬.
성계 60/120 규모, 영겁 모드(스트레치).
Steam 페이지, 데모 빌드, EA 출시.


**현실 일정 요약**: 수직 슬라이스까지 약 5개월, EA까지 12~18개월. 초안의 10주는 "Phase 1~4의 프로토타입 핵심 코드만" 기준으로 읽어야 한다.



---



## 12. 범위 관리: 수직 슬라이스와 미구현 목록



### 12.1 수직 슬라이스 정의 (Phase 5 종료 시점)



| 항목 | 수량/상태 |
|---|---|
| 성계 | 20 |
| 문명 | 6 (플레이어 포함) |
| 타락 초월자 | 1 (침묵) |
| 도달 가능 등급 | T1 → T3 (양 경로) |
| Hazard | 6 |
| 이벤트 | 40 |
| 조우 | 25 |
| 업적 | 4 (T1→T2, T2→T3 각 2경로) |
| 거대 프로젝트 / 칙령 / 유물 | 4 / 6 / 8 |
| Niagara | 1 (NS_Ascension) |
| 저장/로드, 배속, 결정론 테스트 | 완료 |
| 언어 | 한국어 |



### 12.2 명시적 미구현 목록 (Cut List)



아래는 **만들지 않는다**. 아이디어가 떠올라도 이 목록을 먼저 본다.



실시간 3D 함선 조작·전투. 방랑자 이동은 노드 간 이동이고, 분쟁은 수치 비교다.
행성 표면·식민지 건설 UI, 타일, 건물.
개별 유닛·함선 설계기.
수백 노드 기술 트리. 영역 4개 + 임계점 돌파만.
외계인·초월자 3D 캐릭터 모델. 일러스트 1장 + Niagara 오라.
멀티플레이어, 모딩 API(데이터 테이블 공개로 사실상 대체), 성우.
한국어·영어 외 언어 (EA 이후 판단).
영겁 모드(엔딩 후 샌드박스)는 EA 이후.


### 12.3 범위 추가 절차



새 기능을 넣고 싶으면 다음 세 질문에 모두 "예"여야 한다.

DataTable 행 추가로 구현되는가? (코드 변경이 필요하면 다음 Phase로)
두 모드 모두에서 소비되는가?
위기·업적·로그 중 하나와 연결되는가?


---



## 13. 리스크와 대응



| 리스크 | 징후 | 대응 |
|---|---|---|
| 범위 폭발 (4X + RPG) | Phase 종료 조건이 두 번 연속 미달 | §12 Cut List 재확인, 수직 슬라이스 항목 수 축소. 기능 추가는 §12.3 절차 |
| '스프레드시트 감상' 느낌 | 테스터가 로그를 읽지 않음 | 템플릿 변형 수 증가, Severity 2 빈도 조정, 자동 일시정지로 결정 유도, 승천 연출 강화 |
| 두 모드가 콘텐츠 비용을 2배로 | 조우와 이벤트가 따로 늘어남 | 같은 Hazard를 양 모드가 해결하는 구조 유지. 한쪽에만 있는 콘텐츠는 보류 |
| UE5 C++ 반복 속도 | 헤더 수정 → 전체 재빌드 반복 | 헤더 안정화 후 데이터로 작업. Live Coding은 .cpp만. 로직을 DataTable 쪽으로 밀기 |
| 저장/로드 후반 도입 | 상태가 Actor·위젯에 흩어짐 | Phase 1에서 왕복 테스트 통과를 종료 조건으로 고정 (이미 반영) |
| 밸런스 불가 (1인) | 플레이 1판 5시간씩 소요 | 헤드리스 커맨드렛 + CSV. 페이싱은 그래프로 맞춘다 |
| 텍스트 품질 편차 | 후반 이벤트 문장이 초반보다 성의 없음 | §8.3 작성 규칙 체크리스트. 10개 쓰고 다시 읽고 3개 버리기 |
| 번아웃 | 2주 이상 커밋 없음 | Phase를 더 작은 종료 조건으로 쪼갠다. 매주 '플레이 가능한 상태' 유지 |



---



## 14. Claude Code 협업 규칙



### 14.1 요청 단위



한 번에 **클래스 하나, 또는 헤더 하나**. "게임 전체" "Subsystem 전부" 같은 요청은 하지 않는다.
순서: **헤더 → 검토 → 소스 → 컴파일 → 다음 클래스**. 헤더에서 인터페이스가 확정되면 소스는 빠르다.
요청에 이 문서의 섹션 번호를 함께 적는다. 예: "§7.4 스케치를 기준으로 `UGalaxySimulationSubsystem` 헤더를 작성해줘. Tick 순서는 §7.4 주석대로."
요청에 포함할 제약을 매번 반복한다: "3D 에셋 참조 금지, FName/FGameplayTag/FText 사용, Sim 폴더에서 UI·Actor include 금지, 난수는 Rng 스트림만."


### 14.2 검증



각 클래스가 컴파일되는지 확인한 뒤 다음을 요청한다. 빌드 명령은 CLAUDE.md에 둔다.
시뮬 로직은 반드시 **자동화 테스트와 함께** 요청한다. "함수 작성 + Determinism 테스트 갱신."
Claude Code가 UE API를 추측하면 틀릴 수 있다. 낯선 API가 나오면 헤더 파일 경로를 확인하게 한다. ("이 함수가 선언된 엔진 헤더 경로를 알려줘")


### 14.3 CLAUDE.md 템플릿 (프로젝트 루트)



```markdown

# Cosmic Ascension — Claude Code 작업 규칙



## 프로젝트

UE 5.8.3 (C:\Program Files\Epic Games\UE_5.8), C++ 게임 모듈 `Cosmic_Ascension`. 설계 기준 문서: guideline.md (섹션 번호로 참조)
1인 개발. 린 시뮬레이션. 3D 에셋·캐릭터 모델 참조 금지. 데이터는 DataTable(FTableRowBase), 식별자 FName, 분류 FGameplayTag, 표시 문자열 FText.


## 아키텍처 불변 규칙

Source/Cosmic_Ascension/Sim, Events, Ascension, Log, Player, Save 는 UUserWidget/AActor를 include하지 않는다.
모든 런타임 상태는 FGalaxyState 안의 USTRUCT. UObject 그래프로 상태를 만들지 않는다.
난수는 UGalaxySimulationSubsystem::GetRng() 의 FRandomStream만. FMath::Rand* 금지.
틱 처리 순서: 경제 → 연구 → 외교 → 분쟁 → 위기 → AI → 플레이어 이동. 변경 시 가이드라인 §7.4 먼저 수정.
소스 파일은 ASCII. UMETA DisplayName 영문. 한글은 DataTable/String Table.


## 작업 방식

클래스 하나씩. 헤더 먼저 제시하고 승인 후 소스.
시뮬 코드는 Tests/ 의 자동화 테스트와 함께 작성.
낯선 엔진 API는 선언된 헤더 경로를 확인한 뒤 사용.


## 빌드

UE_ROOT = C:\Program Files\Epic Games\UE_5.8
PROJECT = <프로젝트 루트>\Cosmic_Ascension.uproject
에디터 빌드(에디터 닫고): "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" Cosmic_AscensionEditor Win64 Development -Project="%PROJECT%" -WaitMutex
게임 빌드(컴파일 검증): "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" Cosmic_Ascension Win64 Development -Project="%PROJECT%" -WaitMutex
프로젝트 파일 재생성: "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="%PROJECT%" -game -rocket -progress
테스트: "%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PROJECT%" -game -nullrhi -nosound -unattended -nosplash -ExecCmds="Automation RunTests Cosmic_Ascension; Quit" -TestExit="Automation Test Queue Empty"
로그: <프로젝트 루트>\Saved\Logs\Cosmic_Ascension.log (UBT: %LOCALAPPDATA%\UnrealBuildTool\Log.txt)
실제 전체 명령과 PowerShell 주의점은 프로젝트 루트 CLAUDE.md에 유지한다.
```



### 14.4 좋은 요청 예시



> "§7.2의 `FGalaxyState`와 §7.1 타입들을 `Source/Cosmic_Ascension/Core/CosmicTypes.h`, `GalaxyState.h`로 작성해줘. `FCosmicLogEntry`는 §8.1. 모든 USTRUCT는 BlueprintType, 필드는 UPROPERTY. 아직 Subsystem은 만들지 말고, 이 두 헤더만 컴파일되는지 확인할 수 있게 빈 .cpp도 추가해줘."



> "§7.4 `UGalaxySimulationSubsystem`에서 `TickEconomy()`만 구현해줘. 공식은 §5.1 표 기준: Energy = Σ(소유 성계 richness) × tier 승수(T1 1.0, T2 1.5, T3 2.2, T4 3.0) × (1 + 0.2×Spiritualism의 음수 부분). 완료 후 `Cosmic_Ascension.Sim.Determinism` 테스트가 통과하는지 확인."



---



## 15. 용어집



| 한국어 | 영어 | 코드 식별자 | 설명 |
|---|---|---|---|
| 존재 등급 | Ascension Tier | `EAscensionTier` | 5단계. 문명과 개인이 각각 가짐 |
| 신화적 업적 | Mythic Feat | `FAscensionFeatDef`, `DT_Feats` | 승천의 유일한 조건 |
| 승천 | Ascension | `UAscensionSubsystem::OnTierAscended` | 등급 상승 사건 |
| 위기 | Crisis / Hazard | `FHazardDef`, `Hazard.*` 태그 | 카운트다운이 있는 성계 상태 |
| 타락 초월자 | Fallen Transcendent | `FTranscendentState`, `Doctrine.*` | Tier 4 AI 존재. 침묵/동화/광신 |
| 통치자 | Ruler | `EPlayerRole::Ruler` | 문명 지도자 모드 |
| 방랑자 | Wanderer | `EPlayerRole::Wanderer` | 단독 개체 모드 |
| 섭정 | Regent | `AI.Profile.*` 태그 | 통치자 이탈 시 문명을 맡는 AI 프로필 |
| 경외도 | Reverence | `FCivilizationState::ReverenceToPlayer` | 방랑자 → 통치자 전환 조건 |
| 조우 | Encounter | `FEncounterDef`, `DT_Encounters` | 방랑자 텍스트 선택 카드 |
| 유물 | Relic | `DT_Relics` | 방랑자 소지품, 슬롯 3 |
| 거대 프로젝트 | Megaproject | `FStarSystemState::ActiveMegaproject` | 통치자 경로 업적의 실체 |
| 칙령 | Edict | `DT_Edicts` | 제국/은하 범위 수정자 |
| 특성 | Trait | `Trait.*` 태그, `DT_Traits` | 문명 고유 성격 |
| 윤리 축 | Ethics Axes | `FEthicsAxes` | 평화/영성/개방 3축 |
| 인과 | Causality | `ECosmicResource::Causality` | Tier 3 해금 자원. 개인 능력치 이름과 동일 |
| 틱 / 주기 | Tick / Cycle | `FGalaxyState::Tick` | 고정 스텝 시뮬 단위 |
| 레인 | Lane | `FStarSystemState::Lanes` | 성계 간 이동 경로 |
| 연대기 | Chronicle | `UCosmicLogSubsystem::BuildChronicle` | 엔딩용 로그 재편집 |
| 수직 슬라이스 | Vertical Slice | - | §12.1 정의 |



---



## 16. 미결 사항 (결정 필요)



이 문서를 코드로 옮기기 전에 개발자가 확정해야 하는 항목이다. 결정되면 해당 섹션을 갱신하고 이 표에서 지운다.



| # | 결정 사항 | 권장안 | 영향 |
|---|---|---|---|
| 1 | 개인 Tier와 문명 Tier의 관계 | 독립. 단, T4→T5 통치자 업적은 개인 Tier ≥ 3 요구 | §3.2, §3.3 |
| 2 | 방랑자 사망 처리 | T≤2 영구 사망, T≥3 환생 | §4.4 |
| 3 | 한 판 길이 | 1배속 4~6시간 | §2.4, 밸런스 커맨드렛 목표 |
| 4 | 은하 규모 | 수직 슬라이스 20, EA 60/120 선택지 | §12.1 |
| 5 | 타락 초월자 봉인 외 결말(설득/공존) 포함 여부 | EA 이후 | §4.2 |
| 6 | 영겁 모드 | 스트레치. EA 이후 | §4.4 |
| 7 | 영어 로컬 시점 | Phase 5 종료 후 | §8.5 |
| 8 | 제품명 | 가제 유지. Steam 페이지 전 확정 | - |
| 9 | 프로젝트 위치 | 현재 OneDrive 안(`Documents\Unreal Projects`). 빌드 산출물 수 GB가 동기화되므로 OneDrive 밖(`C:\Dev`)으로 이동 권장 | §10.5, CLAUDE.md |

확정 완료 (2026-10-10, Phase 0):
UE 버전 고정값 → **5.8.3** (§1.2). 모듈명 → `Cosmic_Ascension` (언리얼 프로젝트 브라우저에서 생성한 이름 그대로 사용).
렌더 설정 → 레이트레이싱·SkinCache OFF, Lumen GI/반사·메시 디스턴스 필드 OFF, Substrate ON 유지(나중에 끄면 Substrate 전용 노드 머티리얼이 깨지므로 머티리얼 작성 전인 지금 확정), 정적 라이팅 OFF, 가상 섀도맵 ON. 근거와 명령은 CLAUDE.md.
UI 기술 → UMG 확정 (§6.3). 패드/스팀덱 지원이 목표가 되는 시점에 CommonUI를 재검토한다.
플러그인 → CommonUI 미사용, ModelingTools 비활성, PythonScriptPlugin은 헤드리스 에디터 스크립트용으로 활성 (.uproject의 TargetAllowList=[Editor]는 Niagara 의존성 때문에 게임 타깃에서 효력이 없어 `WITH_PYTHON=0` 스텁 모듈이 게임 빌드에도 포함된다. 상세는 CLAUDE.md "렌더 / 플러그인 결정").
자동화 테스트 플래그 → `EAutomationTestFlags::ClientContext | EditorContext | ProductFilter`. 테스트는 `-game -nullrhi`로 돌리므로 ClientContext 필수 (§9.3).



---
