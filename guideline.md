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
[17. 검토 중인 제안: 영혼과 윤회 (미확정 초안)](#17-검토-중인-제안-영혼과-윤회-미확정-초안)
[18. 검토 중인 제안: 통치자 시작, 이중 계, 은하 세력군, 기독교적 요소 (미확정 초안)](#18-검토-중인-제안-통치자-시작-이중-계-은하-세력군-기독교적-요소-미확정-초안)


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
| 2 | 방랑자 사망 처리 | T≤2 영구 사망, T≥3 환생. **§17 영혼과 윤회 제안이 이 항목의 대안이다. §17.3 결정 1과 함께 확정** | §4.4, §17 |
| 3 | 한 판 길이 | 1배속 4~6시간 | §2.4, 밸런스 커맨드렛 목표 |
| 4 | 은하 규모 | 수직 슬라이스 20, EA 60/120 선택지 | §12.1 |
| 5 | 타락 초월자 봉인 외 결말(설득/공존) 포함 여부 | EA 이후 | §4.2 |
| 6 | 영겁 모드 | 스트레치. EA 이후 | §4.4 |
| 7 | 영어 로컬 시점 | Phase 5 종료 후 | §8.5 |
| 8 | 제품명 | 가제 유지. Steam 페이지 전 확정 | - |
| 9 | 프로젝트 위치 | 현재 OneDrive 안(`Documents\Unreal Projects`). 빌드 산출물 수 GB가 동기화되므로 OneDrive 밖(`C:\Dev`)으로 이동 권장 | §10.5, CLAUDE.md |
| 10 | 영혼과 윤회 제안 채택 여부와 §17.3의 결정 6건 | §17.3 권장안대로 채택. 채택 시 §17.7 수정 목록 적용 후 §17 삭제 | §4.4, §9.1, §11, §16 #2, §17 |
| 11 | §18 제안(통치자 시작, 이중 계, 은하 세력군, 기독교적 요소) 채택 여부와 §18.3의 결정 항목 | §18.3 권장안대로 채택. 특히 결정 1~3(천국의 표현, §17 계승 프레임, Tier 5 두 길)은 Phase 1 헤더 전에 확정 | §3.2, §4.1, §4.2, §4.4, §5.10, §8.3, §17, §18 |

확정 완료 (2026-10-10, Phase 0):
UE 버전 고정값 → **5.8.3** (§1.2). 모듈명 → `Cosmic_Ascension` (언리얼 프로젝트 브라우저에서 생성한 이름 그대로 사용).
렌더 설정 → 레이트레이싱·SkinCache OFF, Lumen GI/반사·메시 디스턴스 필드 OFF, Substrate ON 유지(나중에 끄면 Substrate 전용 노드 머티리얼이 깨지므로 머티리얼 작성 전인 지금 확정), 정적 라이팅 OFF, 가상 섀도맵 ON. 근거와 명령은 CLAUDE.md.
UI 기술 → UMG 확정 (§6.3). 패드/스팀덱 지원이 목표가 되는 시점에 CommonUI를 재검토한다.
플러그인 → CommonUI 미사용, ModelingTools 비활성, PythonScriptPlugin은 헤드리스 에디터 스크립트용으로 활성 (.uproject의 TargetAllowList=[Editor]는 Niagara 의존성 때문에 게임 타깃에서 효력이 없어 `WITH_PYTHON=0` 스텁 모듈이 게임 빌드에도 포함된다. 상세는 CLAUDE.md "렌더 / 플러그인 결정").
자동화 테스트 플래그 → `EAutomationTestFlags::ClientContext | EditorContext | ProductFilter`. 테스트는 `-game -nullrhi`로 돌리므로 ClientContext 필수 (§9.3).



---



## 17. 검토 중인 제안: 영혼과 윤회 (미확정 초안)



> **상태**: 2026-10-10 개발자 제안("영혼과 윤회")을 가이드라인 정합성·플레이 경험·기술 구조·1인 개발 범위·제안 옹호의 5개 관점으로 검토해 이 문서의 용어와 구조로 재작성한 초안이다. **아직 결정이 아니다.** 채택하면 §17.7의 수정 목록대로 본문을 고치고 이 섹션은 삭제한다. 미채택이면 이 섹션을 삭제하고 §16 #2(방랑자 사망 처리)를 기존 권장안으로 확정한다. §18.3 결정 2(계승 프레임)가 채택되면 이 섹션의 '영혼·윤회·환생' 용어는 '부름·계승·취임'으로 바뀌고 메커닉(하드 리셋, 시간선 도약, 무작위 기원, 연대기)은 그대로 유지된다.



### 17.1 한 줄 요약과 세계관



플레이어는 육신이 아니라 **영혼(Soul)**이다. 육신이 죽으면 재화·능력치·유물·왕좌·소속을 모두 잃고, 영혼이 재구성되는 동안 은하는 혼자 흘러간다(**시간선 도약**). 영혼은 다른 성계의 필멸자로 다시 태어난다(**환생**). 남는 것은 연대기와 세계에 새긴 흔적뿐이다.

세계관 문장: "태어날 때는 이름 없는 영혼이지만, 은하가 그 이름을 배운다." (원안의 "하나의 영혼에 불과하다"는 §1.1 피치의 상승 서사와 어긋나는 무력감 톤이라 수정.)

이 제안은 새 시스템이 아니라 **§4.4 실패 상태와 §16 #2 미결 항목에 대한 답**이다. 기존 §4.4 2행(T≥3 환생)을 전 Tier로 넓히고, §5.10의 "떠나 있는 동안 세계가 변했다"와 §8.4 연대기를 사망에 적용하는 세계관 레이어다. 원안의 "경험치가 아닌 신화적 사건으로만 승천", "메타 프로그레션 배제", "영혼의 기록"은 이미 §3.2·§8.4가 보장하므로 중복이고, 새로 더해지는 것은 사망 규칙·도약·환생·저장 정책 네 가지다.



### 17.2 비타협 원칙 (채택 시 §1.3·§3.2·§8과 함께 고정)



1. **승계 없음(No Stat Heritage)**: 능력치·자원·유물·왕좌·소속은 다음 생애로 넘어가지 않는다. 메타 프로그레션(영혼 스킬트리, 언락, 은하 간 메타 세이브)은 만들지 않는다.
2. **세계는 멈추지 않는다**: 사망은 §4.3의 유일한 시계(AI 승천)를 앞당기는 **비용**이다. 죽을수록 "다른 질서" 엔딩이 가까워진다.
3. **기원은 무작위**: 필터와 가중치는 허용, 선택은 불허(기원 선택 카드는 Phase 6+ 보류). 선택을 주면 매번 최강 문명으로 수렴한다.
4. **죽음은 출처가 있는 사건만**: 사망은 SourceRow가 있는 데이터 효과로만 일어난다. 틱당 사망 확률, "사소한 파동만으로 돌연사"는 §8.3(선택과 무관한 사건의 신화적 포장 금지) 위반이라 만들지 않는다. 읽히지 않는 죽음은 가혹한 것이 아니라 임의적인 것이다.



### 17.3 개발자가 결정할 항목



| # | 항목 | 선택지 | 권장 | 이유 |
|---|---|---|---|---|
| 1 | 사망 시 개인 Tier | (A) 전부 T1로 리셋(원안) / (B) T≤2는 T1로, T≥3은 유지(현 §4.4 2행의 확장) | **B** | A는 §2.4의 T3→T5 약 3시간을 한 생애 안에 무결점으로 요구한다. 사망 위험이 시간당 1회면 3시간 생존 확률은 약 5%라 후반 콘텍츠와 엔딩 2종이 거의 소비되지 않는다. B는 "T3 인과 간섭형 = 죽음을 넘는 각성"으로 §3.2 서사와 맞고, 수치는 전부 초기화되므로 원칙 1을 지킨다. Tier는 업적으로만 오르므로 누적 진척이 아니다. 검토 5건 중 유일하게 2:3으로 갈린 쟁점 |
| 2 | 노화(수명) | 만들지 않음 / 데이터 노브(기본 OFF, Phase 5 테스트 후 ON 판단) | **노브, VS에서는 OFF** | 신중한 플레이어도 영혼 루프를 최소 1회 경험하게 하는 유일한 장치지만 §4.3 "유일한 시계" 옆의 두 번째 시계다. `LifespanTicks` 정수 하나와 보이는 카운트다운으로 구현 가능하니 설계만 두고 수직 슬라이스에서는 꺼 둔다 |
| 3 | 통치자 사망 | VS에서는 방랑자만 사망(통치자 실패는 §4.4 붕괴 그대로) / 통치자도 노화·암살로 사망 | **VS는 방랑자만** | 예외 하나: 통치 문명이 `Hazard.Extinction`으로 삭제되면 통치자도 사망(종족이 사라짐). 노화 도입 시 순서는 "보이는 수명 → 자동 섭정 위임(§5.10) → 사망". 암살·쿠데타는 `KillPlayer` 효과를 가진 DT_Events 행으로 데이터만으로 추가 가능하게 열어 둔다 |
| 4 | 저장 정책 | 은하당 슬롯 1개(아이언맨. 사망·환생 직후 자동 저장) / 수동 저장·로드 허용 | **아이언맨 기본** | 사망 직전 세이브를 로드할 수 있으면 하드 리셋이 선택 사항이 되어 제안 전체가 무의미하다. 결정론(§9.2) 덕에 비용은 정책 수준. "관조 모드" 토글은 Phase 5 테스터 반응 후 결정 |
| 5 | 경외도(Reverence) | 사망 시 0 / 유지 후 감쇠(×0.5) | **×0.5** | 세계가 영혼을 기억하는 유일한 수치. ×0.5면 최대 1.0→0.5라 즉시 재즉위(≥0.6)는 불가능하고 전설의 머리 하나만 남는다. 능력치가 아니라 세계 상태이므로 원칙 1과 충돌하지 않는다 |
| 6 | 첫 생애 | 무작위 / 고정(로드맵대로 플레이어 문명의 T1 통치자) | **고정 (2026-10-11 개발자 확정: "처음부터 한 행성의 통치자로 시작")** | Phase 2·5 종료 조건과 온보딩(첫 10분 로그 튜토리얼) 보호. 무작위 강림은 2생부터. 첫 생애도 환생도 같은 `Incarnate()` 함수를 쓴다. 통치 실패(반란) → §4.4 강제 방랑자, 자진 이탈 → §5.10. 상세는 §18 |



### 17.4 규칙 초안



**사망**: `EEffectType::KillPlayer` 효과로만 발생한다(사인 태그 `Death.*` + SourceRow). 발생 지점은 데이터 3곳: `FHazardDef.OnExpireEffects`(플레이어가 그 성계에 있을 때만 적용), `FEncounterChoice.OnFailure`, 노화 ON 시 수명 소진(`Death.Age`). 부상과 영구 삭감은 기존 `PlayerStatDelta` 음수로 표현하고 별도 부상 시스템은 만들지 않는다. 생애 첫 300틱은 치명 효과를 무시한다(출생 보호).

**사망 파이프라인 (고정 순서, §7.4 틱 순서처럼 결정론 대상)**: KillPlayer 적용 → `FLifeRecord` 기록 + Severity 2 로그 → 통치자였다면 `BecomeWanderer`(Rng가 문명 윤리에 가중해 뽑은 섭정, 안정도 -0.25) → `LifeState=Between`, `LeapTicks = Rng.RandRange(Min, Max)` → 자동 저장 → 도약 → 막간 다이제스트 생성 → 환생 추첨 → 육신 초기화 → `LifeState=Embodied` → 자동 저장. 난수는 전부 `GetRng()`.

**시간선 도약**: `UGalaxySimulationSubsystem::RunHeadless(LeapTicks)` 그대로다. 초월자 FSM이나 별도 갱신 경로는 없다. 흥망성쇠·봉인·영향권 변화는 기존 틱이 만든다. 도약 중에는 전역 방송 게이트(`IsBroadcastEnabled()`)를 끄고(모든 Core Logic Subsystem의 Broadcast가 이 게이트를 본다), 로그는 `ELogMode::Interlude`(Severity 0 폐기, 1은 영혼이 접촉한 문명·성계만, 2는 보존). 플레이어는 분리 상태라 `TickPlayerTravel`·플레이어 업적 평가·조우 생성을 건너뛴다. 매 틱 엔딩 조건을 검사해 AI가 T5에 도달하면 즉시 중단하고 "영혼이 지켜본 결말"로 §4.3 엔딩을 출력한다. `LeapTicksMin/Max`는 데이터(초기값 800~1,600틱 ≈ Hazard 수명 2~4회, 1배속 13~27분). 판 합계 상한(초기 4,500틱 = 18,000의 25%). §9.4 예산이면 1,600틱은 약 1.6초라 오버레이 뒤에서 동기 실행한다.

**환생(강림)**: 후보 = `OwnerCivID != NAME_None`인 성계. 제외: 타락 초월자 영향권, Hazard 카운트다운 300틱 미만. 가중치 = 1 + 2×(2레인 안 활성 Hazard) + 1×(소유 문명 Tier ≥ 2) + 2×(소유 문명 Reverence > 0.3) − 1×(직전 생애와 같은 문명). "영혼은 인과율의 요동에 끌린다." 소유 성계가 0이면 환생 대신 **"침묵의 우주" 엔딩**(§4.4 엔딩 흐름 재사용, `Doctrine.Silence`와 일치하는 4번째 엔딩). 육신 초기치 = 1 + Σ(소유 문명 Traits의 `DT_Traits` 컬럼 `BodyMod/MindMod/CausalityMod`)를 0~3으로 클램프. 이것이 원안의 "종족 고유의 신체적 한계"다. 행성·종족 엔티티·DataAsset은 만들지 않는다. 환생은 항상 `Wanderer`. 통치자 환생은 §5.10 조건(경외도 ≥ 0.6 또는 안정도 < 0.2)으로만.

**영혼 / 육신 / 세계 3분할**:

| 구분 | 대상 | 사망 시 |
|---|---|---|
| 영혼(유지) | `PersonalTier`(결정 1=B면 T≥3만), `Powers`(Tier에서 재부여), `CompletedFeats`(기록), `LifeIndex`, `Lives` | 유지 |
| 육신(소멸) | `Body/Mind/Causality`(출생 문명 Traits로 재설정), `PersonalPool`, `Relics`, `Role=Wanderer`, `RuledCivID/FormerCivID=NAME_None`, `CurrentSystemID` 재배치, `Travel*` | 전부 초기화 |
| 세계(건드리지 않음) | `ReverenceToPlayer`(×0.5), `bAwareOfPlayer`(유지. 초월자는 영혼을 기억한다: "다시 왔구나"), 수호신 특성, 완성된 Megaproject, 열린 레인, `bSealed`, Log/Chronicle | 세계 상태 그대로 |

**업적 조건의 생애 상대화**: `MegaprojectComplete`와 `TranscendentSealed` 조건은 완료 틱이 `Soul.BornTick` 이상일 때만 참(`FStarSystemState.MegaprojectCompletedTick`, `FTranscendentState.SealedTick`). 전생이 완성한 다이슨 스웜으로 T1→T2를 공짜로 받는 구멍을 막는다.

**연대기**: 하나만 유지한다(원안의 Archive/SoulChronicle은 §15의 Chronicle과 같은 것). `FLifeRecord`에는 FName과 숫자만 저장하고 문장은 표시 시점에 템플릿으로 렌더한다(FText를 저장하면 로컬라이제이션만 바뀌어도 해시가 달라진다). `BuildChronicle()`은 생애별로 장을 나눈다. 사망 화면은 3단 고정: **비문**(사인, 마지막 선택 3개, 생존 주기, 최고 Tier, 업적) → **막간 다이제스트**(상태 diff 5~8줄: 멸망·도약한 문명, 소유가 바뀐 성계, 봉인/해제된 초월자, 전생 문명과 구한 성계의 운명, 초월자 1인칭 1줄) → **강림 서술**(`{CivAdj} {Civ} {System}`). 제목은 "제 N생", GAME OVER 문구 금지, 2회째부터 스킵 가능, 총 60~90초. `WBP_AscensionOverlay`와 `NS_Ascension` 색 파라미터를 재사용하고 새 위젯·Niagara는 만들지 않는다. 같은 diff 함수(`BuildWorldDigest`)를 §5.10 복귀에도 쓴다.

**한 판의 재정의**: 판 = 은하 시드 1개 = 영혼 1회 = 4~6시간(변경 없음). 생애(Life)는 판 안의 장. 기대 2~5생. §2.4의 호는 "영혼의 호"로 읽는다(결정 1=B면 T3 이후는 사망을 넘어 이어진다). 원안의 "성장은 극도로 더디며"는 채택하지 않는다. 마찰은 성장 속도가 아니라 **사망 가능성과 세계 비용**에서 온다. "극소수의 영역"은 AI 문명 대비 희소성(대부분의 문명은 승천하지 못한다)으로 해석하고 §2.4 페이싱 목표는 유지한다. "수많은 초월자"는 톤으로만: 엔티티는 §4.2대로 2~3체(VS 1체), 과거 초월자는 로그 템플릿의 고유명사로.



### 17.5 구현 매핑 (원안 스니펫의 대체. 코드 요청은 §14.1대로 헤더 단위)



| 원안 용어 | 이 프로젝트의 구현 |
|---|---|
| `USoulComponent` | `FSoulState` USTRUCT(`FGalaxyState::Soul`) + `UPlayerRoleSubsystem::KillPlayer / BeginInterlude / Reincarnate`. 컴포넌트는 Actor가 필요해 §6.1·§6.2·§9.1 위반 |
| `SoulRecord` / `SoulChronicle` / Archive | `FLifeRecord`, `FSoulState::Lives`, 기존 Chronicle(§8.4)의 장 나누기 |
| `OnDeath()` 트리거 | `EEffectType::KillPlayer`(`UCosmicEventSubsystem::Apply`가 처리) |
| Global Time Advance / 초월자 FSM 일괄 갱신 | `RunHeadless(LeapTicks)` + 방송 게이트 + `ELogMode::Interlude`. FSM 없음 |
| `DominantSpecies` 데이터 에셋 / 행성 | 성계(`OwnerCivID`) + 소유 문명 `Traits` + `DT_Traits` 컬럼 4개(`BodyMod, MindMod, CausalityMod, LifespanTicksMod`) |
| 생존 일수 | `TicksLived`(주기. §2.1 시간 단위) |
| 리스폰 | 환생(`Reincarnate`) |
| 튠 값(도약 폭, 수명, 가중치, 난이도 배율) | `DT_Rules` 단일 행 `FCosmicRulesDef`: `LeapTicksMin/Max, LeapTotalCap, BaseLifespanTicks(0=OFF), RebirthHazardWeight, RebirthCivTierWeight, ReverenceDecay, AIAscensionSpeedMultiplier`(§4.3 난이도 배율의 보관 장소) |



```cpp

// Core/GalaxyState.h 추가분 (스케치. 필드는 FName과 숫자만)

UENUM(BlueprintType)

enum class ELifeState : uint8 { Embodied, Between };



USTRUCT(BlueprintType)

struct FLifeRecord

{

GENERATED_BODY()

UPROPERTY() int32 LifeIndex = 0;

UPROPERTY() int64 BornTick = 0;

UPROPERTY() int64 DiedTick = 0;

UPROPERTY() FName BirthSystemID;

UPROPERTY() FName BirthCivID;

UPROPERTY() TArray<FName> RuledCivIDs;

UPROPERTY() EAscensionTier PeakTier = EAscensionTier::Tier1_Planetary;

UPROPERTY() FGameplayTag CauseOfDeath; // Death.Hazard / Death.Encounter / Death.Age / Death.Transcendent

UPROPERTY() FName CauseSourceRow; // 사인을 만든 Hazard/조우/이벤트 행

UPROPERTY() TArray<FName> Feats;

UPROPERTY() TArray<FName> TranscendentsMet;

};



USTRUCT(BlueprintType)

struct FSoulState

{

GENERATED_BODY()

UPROPERTY() int32 LifeIndex = 1;

UPROPERTY() int64 BornTick = 0;

UPROPERTY() ELifeState LifeState = ELifeState::Embodied;

UPROPERTY() int64 PendingLeapTicks = 0; // 막간 중 저장·로드 재개용

UPROPERTY() TArray<FLifeRecord> Lives;

};



// FGalaxyState: FSoulState Soul; FGuid GalaxyID; (FPlayerEntityState는 '육신'으로 재정의)

// FStarSystemState: int64 MegaprojectCompletedTick; FTranscendentState: int64 SealedTick;

// EEffectType += KillPlayer; EConditionType += PlayerLifeIndexAtLeast, TranscendentAwareOfPlayer, (PlayerAgeAtLeast)

// DefaultGameplayTags.ini += Death.Hazard, Death.Encounter, Death.Age, Death.Transcendent, Log.Soul

```



### 17.6 범위와 일정



| 시점 | 작업 | 비용 |
|---|---|---|
| Phase 1 | `FSoulState/FLifeRecord/ELifeState/GalaxyID` 필드, 방송 게이트, 은하당 슬롯 1개 저장 모델, 해시에 `Soul` 포함 | 약 3시간 (헤더 작성 시점이라 추가 비용 거의 0. 나중에 넣으면 SaveVersion 마이그레이션) |
| Phase 2 | `KillPlayer`, 조건 2종, `*CompletedTick` 2필드, `Death.*` 태그 | 약 2시간 |
| Phase 3·4 | 변경 없음 | - |
| **Phase 4b 신설 "영혼 루프" (2주)** | KillPlayer 처리 → 도약 → 환생 → 다이제스트 → 3화면 → 저장 트리거 → 빈 은하 가드 → 테스트 6개(`Soul.LeapDeterminism`, `Soul.HardReset`, `Soul.LeapResumeRoundTrip`, `Soul.NoPlayerDuringLeap`, `Sim.RebirthPossible`, `Sim.NoDanglingRefsAfterLeap`) → 템플릿 65~70줄. **종료 조건**: 방랑자 사망 → 연대기 → 도약 → 환생이 콘솔 명령 없이 1회 이상 가능, 환생 후 §5.10 조건으로 즉위 가능, 테스트 통과 | 순증 6~8 작업일 (§4.4가 원래 요구하던 사망·환생 분기 약 4일을 대체) |
| Phase 5 | 커맨드렛 `-DeathAt=<틱>`, `-PlayerPolicy=Cautious|Reckless`, CSV 컬럼(`LifeIndex, LifeLengthTicks, DeathCauseRow, PeakTier, LeapTicks, EndingType`). 시작 목표: 판당 2~5생, 도약 유발 "다른 질서" < 15%, 사인 추적률 ≥ 90%, (결정 1=B) 첫 생애 T3 도달 40~60%. 테스터 질문에 "죽음이 공평했는가", "죽은 뒤에도 계속하고 싶었는가" 추가 | 약 1일 |
| Phase 6+ | 기원 선택 카드(`EEffectType::RebirthAt`), `Trait.Legacy.Delivered/Forsaken`과 2생 전용 조우, 종족 상한 컬럼, 통치자 암살 이벤트 행, 노화 활성 판단, AI 문명 T4 도달 시 `Transcendents` 추가(기성 우주가 시뮬에서 자람) | 각각 §12.3 재심사 |

**만들지 않는다 (채택 시 §12.2에 추가)**: `USoulComponent` 등 Actor 결합 상태. 행성·종족 엔티티와 DataAsset. 초월자 상태 FSM. 틱당 사망 확률·돌연사. 메타 프로그레션·영혼 스킬트리·은하 간 메타 세이브. 사망 전용 Niagara·일러스트. 통치자 사망의 별도 메커니즘. 도약 중 수동 개입.

**콘텐츠 예산(VS)**: 사망 서술 Hazard 6종×3 + 일반 사인 5종×3 = 33줄, 다이제스트 8항목×3 = 24줄, 강림 5변형 + 생애 헤더 2줄, 초월자 영혼 인지 1인칭 3줄. 합계 약 65~70줄(1~1.5일). 전부 §8.3 규칙(두 문장, 사실→함의).

**§12.3 판정**: Q1 코드 변경 있음(헤더 예약은 지금 비용 0, 동작은 Phase 4b로 분리) / Q2 양 모드 소비(통치자 사망·섭정 인계, 방랑자 사망, 환생 후 즉위) / Q3 위기·업적·로그 연결 모두 예.



### 17.7 채택 시 본문 수정 목록



§1.2 한 판 정의에 "영혼 1회, 기대 2~5생". §2.4 아래 "영혼의 호" 1줄. §3.2 Tier 3 행에 "인과 간섭형은 육신의 죽음을 넘는다"(결정 1=B). §4.3에 "사망 도약은 이 시계를 앞당기는 비용". §4.4 표: 2행 교체 + "침묵의 우주" 엔딩 + 금지 목록(틱당 사망 롤, 출생 300틱 내 사망, 도약 중 사망). §5.9 출생 스탯 컬럼과 Reverence 감쇠. §5.10 표에 "사망(강제)" 행. §7.2/§7.3/§7.4/§7.5 스케치에 §17.5 추가분. §8.3에 "죽음은 SourceRow가 있는 효과만". §8.4 생애별 장. §9.1 슬롯 정책과 자동 저장 트리거(사망 직후·환생 직후). §9.3 테스트 6개와 커맨드렛 인자. §10.1 `DT_Rules`. §11 Phase 4b. §12.1 "생애 전환 1회 이상". §12.2 금지 목록. §13 "하드 리셋 이탈" 행(징후: 테스터가 2번째 사망 후 종료 / 대응: 다이제스트·세계 기억·사망 경고 강화). §15 용어 추가. §16 #2 삭제, "관조 모드 토글"·"노화 활성" 신설. CLAUDE.md 불변 규칙에 "Core Logic Subsystem은 플레이어·생애에 종속된 멤버 변수를 갖지 않는다. 지속 업적 진척은 FGalaxyState 안에" 1줄.



### 17.8 용어 (채택 시 §15로)



| 한국어 | 영어 | 코드 식별자 |
|---|---|---|
| 영혼 | Soul | `FSoulState`, `FGalaxyState::Soul` |
| 생애 | Life | `FLifeRecord`, `LifeIndex` |
| 사망 / 사인 | Death / Cause of Death | `EEffectType::KillPlayer`, `Death.*` 태그, `CauseSourceRow` |
| 시간선 도약 | Timeline Leap | `LeapTicks`, `RunHeadless` |
| 막간 | Interlude | `ELifeState::Between`, `ELogMode::Interlude`, `BuildWorldDigest` |
| 환생 / 강림 | Reincarnation / Incarnation | `Reincarnate()`, `Incarnate(SystemID, CivID, Role)` |
| 생존 주기 | Cycles lived | `TicksLived = DiedTick - BornTick` |



---



## 18. 검토 중인 제안: 통치자 시작, 이중 계, 은하 세력군, 기독교적 요소 (미확정 초안)



> **상태**: 2026-10-11 개발자 메시지("처음부터 한 행성의 통치자로", "물리적인 계와 영적인 계", "기독교적인 요소들을 제대로", "세계관·세력·판타지 설정 아이디어 추가")와 붙여 넣은 제안 초안(Role State Machine, Dual-Realm, 은하 세력군 6종, C++ 지침)을 5개 관점(말씀잇다 자료 소화, 가이드라인 정합성·기술 매핑, 세계관 확장, 기독교적 척추, 플레이 경험)으로 검토해 이 문서의 용어와 구조로 재작성한 초안이다. 개발자의 성경 공부 프로젝트 **말씀잇다**(개역한글 1961, WEB, 매튜 헨리, 퍼블릭 도메인 설교 66편, 하나님의 이름·칭호 126항목, 인물 101, 주제 14, 원어 낱말 24; 인용 목회자 스펄전·매튜 헨리·웨슬리·머레이·에드워즈·루터·칼빈)를 어휘와 작법의 기준으로 삼았다. **아직 결정이 아니다.** 채택하면 §18.10의 수정 목록대로 본문을 고치고 이 섹션은 삭제한다.



### 18.1 한 줄 요약



제안의 핵심 넷(통치자 시작, 반란으로의 자유 낙하, 물리/영적 이중 계, 세력 다양화)은 **새 시스템이 아니라 이미 있는 뼈대의 재해석**이다. §4.4 1행(통치 문명 붕괴 → 강제 방랑자), §5.10(자진 이탈·즉위·복귀), §3.2의 5단계 등급(T1~2 = 물리계, T3~5 = 영적 계), §4.2의 초월자 표현(영향권 + 가중치 수정자 + 1인칭 화자)이 그것이다. 따라서 코드 순증은 필드 몇 개, 효과·조건 몇 종, DataTable 2~3개, 파생 함수 1개 수준이고 나머지는 전부 데이터와 텍스트다.

제안 초안의 C++ 지침(`EPlayerRoleState`, `FPlanetaryGovernanceData`, Unrest/SpiritualFavor 수치, Dual-Realm Damage Calculator, `UGalacticFactionDataAsset`)은 모두 기존 `EPlayerRole`, `FCivilizationState.Stability`, Tier 격차 함수, DataTable 행으로 대체한다(§6.2, §7, §5.6, §12.2 규칙). '천국 은하'·'지옥 은하'는 지도 위의 세력으로 만들지 않는다. 기독교적 요소를 '제대로' 넣는 길은 세력 추가가 아니라 **어휘·모티프·규칙**이며, 하나님은 어떤 수치·화자·엔티티·엔딩 조건으로도 등장하지 않는다.



### 18.2 비타협 원칙 (세계관과 작법)



1. **저자는 등장인물이 아니다.** 거룩한 질서(Divine Order)는 세력·`FTranscendentState`·관계 수치·일러스트·1인칭 화자가 아니다. 임재는 사건(방문 Hazard)·사자(messenger) 조우·남은 자 특성·연대기(책)·수동태 문장으로만 드러난다. Tier 5조차 질서의 **화신이지 저자가 아니다**(히 1:3, 골 1:17의 구조).
2. **두 계는 장소가 아니라 격(Tier)이다.** 보이는 것들(T1~2)과 보이지 않는 것들(T3~5, 골 1:16 어휘)은 같은 성계 지도 위에 겹쳐 있다. 영적 은하·천국 은하라는 별도 영역은 없다. 영력(靈力)은 자원이 아니다. 저장·채굴·거래·소모되지 않고 숫자 바가 없다. 코드상 이름은 기존 `Causality`(§5.1)이며 식별자는 바꾸지 않는다.
3. **강제와 초청의 구분이 선악이다.** 압력(Pressure: 사기 저하·의식 지배·법칙 왜곡)은 타락한 자의 방식(§4.2 침묵·동화·광신). 부르심(Calling)과 경외(Reverence)는 거룩한 질서의 방식. 같은 격차에서 '강제'와 '초청'이 갈린다.
4. **경배를 받아들이는 존재는 정의상 타락한다**(행 12:22-23 헤롯 vs 행 14:15 바울, 계 19:10). 플레이어가 T3+에서 하위 문명의 경배를 받아들이면 광신의 길목에 선다.
5. **믿음은 결말을 사지 않는다.** 충실한 대응은 백성(안정도·형용사)을 확정적으로 바꾸고 결말은 확률로만 바꾼다. 신정론 예산: 충실한 대응의 나쁜 결말 ≥ 25%(욥기·요시야 원칙). 기도·금식 칙령은 '가중치 상승, 결과 불확정'.
6. **설교하지 않는다.** 플레이어 선택을 옳다/틀리다로 라벨링하는 UI·로그 없음. 결과와 기록만. 인월드 텍스트에 성경 본문·장절·설교 문장·성경 인물 고유명사를 넣지 않는다. 패턴(맡김→셈, 높임→낮아짐, 위기→방주, 떠남→부름, 기록→읽힘)만 빌린다. 가짜 경전 문체 금지.
7. **다른 전통도 대칭으로 존중한다.** 선계·불계·사이오닉은 각자의 주장을 가진 '길(Path)'이며 게임은 어느 길이 참인지 숫자로 판정하지 않는다. 실존 종교 고유명사(석가·아미타불·태상노군·옥황상제)와 기독교 천사 이름(미가엘·가브리엘)을 유닛 이름으로 쓰지 않는다. "세 길은 결국 같다"는 문장(혼합주의)도 쓰지 않는다. 악역은 종교적 타자가 아니라 죄의 세 얼굴(침묵 = 죽음·엔트로피, 동화 = 바벨식 강제 통일, 광신 = 경배 요구·우상)이다.



### 18.3 개발자가 결정할 항목



| # | 항목 | 선택지 | 권장 | 이유 |
|---|---|---|---|---|
| 1 | 천국(거룩한 질서)의 표현 | (A) 제안대로 지도 위 '천국 은하' 세력 / (B) 세력 아님. 다섯 방식: 다시 쓸 수 없는 법(`ImmutableLaws`), 방문 Hazard(`Hazard.Visitation`), 사자 조우(판정 없음, 숭배 거절), `Trait.Faith.*`를 가진 보통 문명들(남은 자), 연대기 = 책 / (C) 어휘·모티프만 | **B** | 단 2:34-45 "손으로 하지 아니하고 뜨인 돌", 요 18:36 "내 나라는 이 세상에 속한 것이 아니라". A는 하나님을 여섯 세력 중 하나로 낮추고 동맹·전력 비교 대상이 된다. 5건의 검토가 전부 B |
| 2 | §17의 프레임 | (A) 영혼·윤회(현행 §17) / (B) **부름·계승(Calling/Succession)**: 쓰러진 자의 겉옷(기록 + Reverence×0.5)이 후계자에게 넘어간다(모세→여호수아 신 34:9, 엘리야→엘리사 왕하 2:9-15). 메커닉·테스트·일정은 §17 그대로 / (C) 중립화 | **B** | 히 9:27 "한번 죽는 것은 사람에게 정하신 것이요 그 후에는 심판이 있으리니"가 개발자 앱에 수록되어 있고, 네페쉬 낱말 카드가 "몸과 분리된 영혼만을 뜻하지 않는다"고 경계하며, 코퍼스에 윤회·환생이 0회다. §17.2 '승계 없음'은 윤회보다 계승에 더 잘 맞는다(후계자가 능력치를 물려받을 이유가 없다). 윤회는 선계·불계의 '주장'으로만 남긴다 |
| 3 | Tier 5의 정체와 엔딩 | (A) 현행 '우주 상수의 화신 / 우주와 일체화' / (B) **두 길**: 받는 길 `Way.Received`(청지기로서 법칙을 지키고 봉인한 뒤 통치권을 내려놓음 → 엔딩 '맡겨진 영겁') vs 찬탈의 길 `Way.Seized`(인과율 흡수 → 엔딩 '내가 올라가리라', 사 14:12-15 패턴, 게임오버 아님) + 표시명을 '영겁의 청지기 / 왕 노릇에 참여'로 / (C) 재명명만 | **B** | 창 3:5·창 11:4·사 14:13·단 4:30은 자기 높임을 타락의 원형으로, 빌 2:9·눅 14:11·벧후 1:4·계 3:21은 주어지는 높임을 참 상승으로 둔다. '우주와 일체화'는 범신론 어휘다. 최고 피조 지위는 '왕 노릇'(계 22:5)이지 '신'이 아니다. 데이터 2행 + 템플릿 6줄 + 수치 1개 |
| 4 | 개인 능력치 `Exaltation`(0~10) 추가 | (A) 추가(기존 `PlayerStatAtLeast/PlayerStatDelta` 재사용) / (B) 태그 플래그만 / (C) 없음 | **A** | 경배 수용/거부 조우, 느부갓네살 이벤트(단 4), §3.2 '경외/공포 분기', 결정 3의 엔딩 분기가 모두 한 수치를 읽는다. Phase 1 헤더에 int 1개 |
| 5 | 지옥 은하 / 타락 군세 | (A) 별도 영토 세력 / (B) **§4.2 교리 3종의 심화**로 대체: 침묵 = 고후 4:4·암 8:11(자비로운 척 "그들은 더 이상 아프지 않다"), 동화 = 창 11·계 17(포용을 말하는 관료적 따뜻함 "번역은 더 필요하지 않다"), 광신 = 계 13·몰렉(2인칭 명령문 "너의 두려움이 나의 양식이다"). 공통 규칙: 이름을 말하지 않는다, 거짓말이 아닌 반쪽 진실(창 3:1-5), 이길 수 없음을 안다(계 12:12). `FTranscendentState.Realm` 태그(Abyssal/Celestial/Independent) 추가 | **B** | 엡 6:12의 "정사와 권세"는 영토 국가가 아니라 영향권·설득의 존재이며 §4.2가 정확히 그 구조다. 카툰 악마와 세력 중복을 동시에 피한다. 공허 정화자(Void Reapers)는 `Hazard.Reaping` 또는 Phase 6+ 교리 후보(`Doctrine.Trespass`, 유 1:6)로 흡수 |
| 6 | 선계·불계 승천자 | (A) 세력 + 전용 엔딩 / (B) **길(Path) 데이터**: `Path.Stillness`(무위) · `Path.Release`(해탈) · `Path.Axiom`(사이오닉) · `Path.Covenant`(언약) 각 조우 3~5장 + 형용사 뱅크 + 첫 만남 '주장 1줄'("우리는 부름받지 않았다, 올랐다"). Phase 6+에 비(非)타락 초월자 1~2체(`Doctrine.Stillness/Release`, Hazard를 만들지 않고 관조). 길별 T3→T4 Feat 변형 1개, T4→T5는 공통 / (C) 제외 | **B** | 진지한 구도자로 존중(행 17:22-31 패턴)하면서 희화화와 혼합주의를 둘 다 피한다. 수련은 Body/Mind를 올리되 Tier는 올리지 못한다(§5.9·§3.2)는 기존 규칙이 게임의 유일한 '답'이다 |
| 7 | 영압(Spiritual Pressure) 구현 | (A) 제안대로 수치 + 계산기 클래스 / (B) **Tier 격차의 파생 함수**(저장 안 함): 하위 함대 유효 전력 ×{격차 0: 1, 1: 0.5, ≥2: 0}, 상위 영향권 안 하위 문명 안정도 틱당 −0.002×P, 이벤트·조우 게이트 조건 `SystemPressureAtLeast`, 문명 AI 확장 금지. 함대는 결말을 못 사지만 **시간을 산다**(투입 1.0당 카운트다운 +50, 상한 +100) | **B** | '물리는 영을 못 건드린다'는 승수 0, '영은 모든 영역에 발휘'는 수정자 4곳으로 끝난다. 피해·HP·디버프 엔진 없음(§5.6 'FleetPower 하나', §4.2 '체력 바 없음'). 성경의 권세(exousia, 위임)와 능력(dynamis, 행사) 구별: 피조 영의 힘은 '허락된 범위'(욥 1:12)라 완전 무효화보다 감쇠가 정직하다 |
| 8 | 반란의 구현과 결말 폭 | (A) Unrest 미터 + 100 도달 시 폐위 / (B) **`Hazard.Rebellion`**(수도 성계, 200틱, 발생 조건 Stability < 0.3이 100틱 연속, 출생 보호 300틱 이후) → 만료 시 `DeposeRuler` 효과. VS는 전면 폐위 1종, EA에 윤리 축별 분열(칙령 슬롯 −1)·분리(성계 1개 `TransferSystem`으로 새 AI 문명) 추가 / (C) 분리만 | **B** | Hazard 카운트다운이 곧 제안의 Unrest 게이지이고(보이고, 해결 수단이 있고, 만료 시 확정), 통치자(개혁 프로젝트·청원 카드)와 방랑자(반군 교섭 조우 → Reverence ↑ → §5.10 즉위)가 같은 Hazard를 소비한다(§13). 폐위 직후 즉시 재즉위를 막는 `ThroneLockedUntilTick`(600틱) 필수 |
| 9 | 통치자의 셋째 지표 | (A) SpiritualFavor 미터 / (B) **미터 없음**: 초월자별 입장 = `Relations[TranscendentID]`(기존 TMap, 키 의미 확장), 질서에 대한 자세 = `Trait.Faith.*` 태그(형용사 뱅크가 수치 없이 상태를 읽힌다). Output = `IncomePerTick`(기존) | **B** | '신의 호감 미터'는 결말을 사는 슬롯머신이자 보응 공식(욥기 길잡이가 거부)이다. 필드 추가 0, 조건 2종(`CivRelationAtLeast`, `SystemInfluencedBy`)만 |
| 10 | 세력 데이터 형태 | (A) `UGalacticFactionDataAsset` / (B) **`DT_Civs`**(`FCivArchetypeDef : FTableRowBase`: Traits, EthicsPreset, AIProfile, Color, NameBank, AdjectiveBank, HazardSpawnMultipliers, InitialRelations, Weight). VS 6행, EA 10행. 아키타입 태그는 Traits에도 넣어 `CivHasTrait`로 거른다 | **B** | §7 규칙(정의는 DataTable). '데이터만으로 수십 개 추가'는 DataTable이 더 잘 만족하고 `Data.Integrity` 테스트 대상이 된다 |
| 11 | 폐위 직후 플레이어 | (A) **모성계에 남는 방랑자**, 소지품·능력치 유지, 왕좌 600틱 잠금, Reverence ×0.5 / (B) 인접 성계로 추방 / (C) 제안대로 '무일푼' | **A** | §5.10 '벌칙처럼 느껴져도 안 된다'. 몰락한 군주가 자기 제국을 안에서 지켜보는 쪽이 '떠나 있는 동안 세계가 변했다'를 가장 잘 보여 준다. B는 한 줄 변경이라 Phase 5 테스터 반응 뒤 전환 가능 |
| 12 | `Hazard.Apostle` 명칭 | (A) 유지 / (B) **`Hazard.Cult`**(타락의 교단)로 변경. '사도·사자'는 거룩한 질서 어휘로 예약 | **B** | 기독교 어휘에서 '사도'는 긍정어다. 코드가 없는 지금은 ini 1줄 + §4.1·§6.5 문구 수정이면 끝 |
| 13 | 하나님 이름·칭호와 성경 인용 | (A) 음역 칭호·구절 직접 사용 / (B) **3단계 규칙**: Tier A 허용(속성어·제단 이름 패턴 이레·닛시·샬롬·라파·삼마·로이·올람, 절기·제도어 희년·안식·면제년·도피성·이삭 줍기, 사건 후 문명이 붙이는 **헌정명**) / Tier B 주의(알파와 오메가·왕의 왕·만군의 여호와·어린양·소멸하는 불은 `Path.Covenant` 체인과 연대기 안의 문명 고백 인용으로만, UI·권능·플레이어 칭호 금지) / Tier C 금지(여호와/YHWH/예수/그리스도/성령/보혜사를 유닛·자원·수치·화자·엔티티·플레이어 이름으로). 인월드 본문 인용 없음 / (C) 사용 안 함 | **B** | 제단 이름(ALTAR_NAME·PLACE_NAME)은 성경 자체가 사건 뒤에 사람이 붙인 고백으로 제시하므로 하나님을 모델링하지 않고 쓸 수 있다. 말씀잇다의 nameType 분류가 이 선을 긋는 도구다. 인용은 톤 문제(외계 은하의 입에 든 구절은 설교조의 가장 빠른 길). 게임 밖 '세계관과 출처' 화면은 별도 |
| 14 | 은혜 이벤트와 신앙 특성의 비용 | (A) **은혜 이벤트 3종**(구출·남은 자·희년) 추가: 플레이어 행동이 조건이 아님(실패 상태만), 판당 1회(`SetFlag Grace.Spent`), Tier·자원·능력치 아닌 상태 회복과 기록만, 거절 가능하고 거절도 기록, AI 문명에게도 동일. `Trait.Faith.*`는 세속 비용(생산 승수 음수, 조약 제한) 동반 / (B) 넣지 않음 / (C) 플레이어만 | **A** | 엡 2:8-9 없는 기독교 요소는 반쪽이다. 네 조건이 파밍을 막아 §3.2 '업적만'과 충돌하지 않는다. 비용 없는 신앙 버프는 '신앙 = 최적 전략'이라는 거짓 메시지다(단 3·6장, 히 11:35-38) |
| 15 | 청원(Petition) 카드 | (A) 조우 25장 중 8장 전환 / (B) **+8장**(방랑자 25 + 통치자 8 = 33) / (C) 없음 | **B** | T1 한 행성에서 §2.2 '20~40초마다 결정 하나'는 연구 슬라이더·칙령·프로젝트만으로는 못 채운다. `FEncounterDef`를 통치자 모드에서 소비(조건 `PlayerRoleIs Ruler`, 비용은 문명 Stockpile). 코드 증가 거의 없음 |
| 16 | 영적 시야 토글 | (A) **Phase 3 뷰포트 작성 시**(머티리얼 파라미터 3개, 약 2일) / (B) Phase 6 | **A, 단 Phase 3 종료 조건 달성 뒤에만** | M_Star·AStarSystemActor를 처음 만드는 시점에 파라미터를 넣는 것이 나중에 다시 여는 것보다 싸다 |



### 18.4 통치자 시작과 통치 루프



**'행성' = 모성계 1개.** 가이드라인의 최소 단위는 성계다(§6.4, §7.2, §12.2 행성 표면 금지). §3.2 Tier 1 '행성 구속형'이 이미 '한 행성 문명'이므로, 코드상 행성은 `FCivilizationState.HomeSystemID`(신규 FName) 하나다. T2 '항성 융합형'으로 오르는 순간 서사가 '행성 → 성계 국가'로 자연 확장된다. `NewGalaxy`가 플레이어 문명을 `DT_Civs`의 `arch_default`(윤리 0/0/0, Stability 0.75, 성계 1개)로 만들고 `Incarnate(HomeSystemID, PlayerCivID, EPlayerRole::Ruler)`로 첫 생애를 시작한다. §7.2의 `Role` 기본값은 `Ruler`로 바꾼다. 로그 슬롯 `{HomeSystem}` 추가. 수도를 잃으면 Stability −0.3 후 정렬된 다음 소유 성계로 이전, 소유 성계 0이면 §4.4 붕괴.

**지표는 Stability 하나.** 0~1 유지(UI만 %). 불만도는 `Hazard.Rebellion`의 `HazardTicksRemaining`, 생산력은 `IncomePerTick`, 영적 조화도는 `Relations[TranscendentID]`와 `Trait.Faith.*`. HUD에는 Stability 바 + 60틱 이동평균 추세(▲▼), 윤리 삼각형, 인접 초월자 입장(영향권 2레인 안일 때만), 자원 5종. 로그에는 숫자가 없다(§8.3).

**결정 주기는 청원 카드로 채운다.** `FEncounterDef`를 통치자 모드에서도 소비한다: 조건 `PlayerRoleIs Ruler`, `FEncounterChoice.Cost`는 Role에 따라 `PersonalPool` 또는 문명 `Stockpile`에서 차감, `CheckStat`에 `Stability`/`Influence` 키 허용. 트리거: Hazard 발생, Stability 임계 통과(0.5/0.3), 영압 도착, 600틱 주기 일반 청원. 화자는 윤리 축이 고른 파벌(군부/사제/학자/상인)이라 문명 내부 파벌을 엔티티 없이 표현한다. VS 8장: '듣는 마음'(개국: 지혜/부/안전 → ResearchFocus 프리셋), 파벌 청원 3, '왕좌를 지키겠는가'(반란 카운트다운 < 50 전용), 영압 체인 3.

**반란(`Hazard.Rebellion`)**: `DT_Hazards` 행 `crisis_rebellion` — 대상 성계 = `HomeSystemID`, 카운트다운 200, 발생 조건 `CivStabilityBelow 0.3`이 100틱 연속(확률 아닌 결정론. 지속 틱 조건 1종 또는 `LowStabilityTicks` 카운터) ∧ `GalaxyTickAtLeast 300`(출생 보호). 전조 3단: ① Stability < 0.5에서 60틱마다 Severity 0 '웅성거림'(화자는 윤리 축: 군부의 침묵 / 사제들의 금식 / 의회의 자치 발언), ② 발생 시 Severity 1 + 노드 Hazard 경고(§6.4) + 자동 일시정지 후보, ③ 잔여 50틱에 마지막 청원 '왕좌를 지키겠는가': (A) 명예로운 양위(섭정 직접 선택, Stability −0.1, Reverence 유지 — 다윗이 예루살렘을 떠난 형) / (B) 버티기(성공 시 Stability +0.15와 특성 '굳건한', 실패 시 폐위) / (C) 개혁(Society 연구 진척 + Influence 대량 소모로 즉시 해결). 해결 수단: R = 거대 프로젝트 `project_reform`(수도, 틱당 Influence) 또는 위 카드 / W = 조우 `enc_rebel_parley`(Mind 체인, 성공 시 `ResolveHazard` + `ChangeReverence +0.2` → §5.10 즉위 경로).

**폐위(`EEffectType::DeposeRuler`)**: 플레이어 문명이면 §17.4 파이프라인 앞부분 재사용 — `BecomeWanderer(RegentProfile = Rng가 문명 윤리에 가중해 선택, Reason = Status.FallenMonarch)` → Stability = min(현재, 0.15) → Reverence ×0.5 → `ThroneLockedUntilTick = Tick + 600`(§5.10 즉위·복귀 조건에 `Tick ≥ ThroneLockedUntilTick` 추가. 없으면 '안정도 < 0.2' 경로로 즉시 재즉위하는 허점) → `FLifeRecord`에 재위 종료 사유(`Reign.Abdicated / Reign.Deposed / Reign.Died`) → Severity 2 '몰락한 군주'(§4.4 문구 그대로) → 3화면(폐위 기록: 백성이 붙인 별칭·마지막 결정 3개·재위 주기·완성 프로젝트 → `BuildWorldDigest` → 망명 첫 조우 3변형: 충신이 몰래 배를 내준다 / 반란군이 떠나게 둔다 / 혼자 걸어 나온다) → 자동 저장. AI 문명이면 AIProfile 재추첨 + '정권 교체' 로그. 플레이어는 모성계에 남는 방랑자(결정 11). 쿠데타에서의 사망은 VS에 없고(§17.3 #3) Phase 6+에 `KillPlayer` 효과 DT_Events 행으로만 열어 둔다. 통치 문명이 `Hazard.Extinction`으로 삭제되면 통치자 사망(예외 1개).

**'왜 방랑자가 되었나'의 기록**: `FPlayerEntityState.StatusTags`(FGameplayTagContainer) + `Status.FallenMonarch`/`Status.Abdicated` + 조건 `PlayerHasStatus`. 몰락한 군주 전용 조우 3~5장(VS 25장 안에 배정: 옛 백성과의 재회 — 알아본다/저주한다/숨겨준다; 모티프: 엘리야의 낙심, 요나의 떠남, 모세의 도피와 부르심, 다니엘의 협력과 양보의 경계).

**남는 쪽과 떠나는 쪽이 모두 진짜 선택이 되게**: 충실한 통치의 보상은 숫자가 아니라 로그다 — 600틱 연속 재위 ∧ Stability ≥ 0.6마다 Severity 1 '시대 명명'("{Civ}의 백성이 이 시대를 '{Epithet}의 시대'라 부르기 시작했다". Epithet은 윤리·완성 프로젝트·해결 Hazard에서 고르는 DT 행 10개), 1,800틱에 `Trait.Legacy.*` 영구 특성, 통치자 전용 Feat(다이슨 스웜·초신성 안정화)은 떠나면 못 하고 섭정 표류도 없다. 떠남(§5.10 자진 이탈)의 보상은 개인 Tier 경로 Feat·유물·조우와 요나형 서사(떠난 동안 변한 세계로의 복귀). 전환 비용은 §5.10 Stability −0.1 + Severity 2 '왕좌를 비우다' 1개.

**첫 10분(1배속 600틱, 결정 15~20회, Severity 2 ≥ 2회)**: 0:00 Severity 2 "{Civ}의 {HomeSystem}. 당신은 이 행성의 통치자다." + 개국 청원 '듣는 마음' → 0:30 첫 파벌 청원 → 1:00 연구 슬라이더 안내 → 1:30 첫 징조 `Log.Sign`("밤하늘의 한 점이 제자리를 벗어났다.", 설명 없음) → 2:30 첫 Hazard(`Hazard.AIAwakening`, 모성계; 카운트다운 UI 학습, 시작 칙령 슬롯 1개 보장) → 5:00 결과 + 첫 '웅성거림'(초기 칙령 유지비로 Stability가 0.5 근처까지 내려오게) → 5:30 첫 접촉(이웃 문명, 태세 선택) → 7:00 불만 파벌 청원 → 8:30 Severity 1 "첫 함선이 준비되었다. 당신은 떠날 수도 있다."(강요 없이 §5.10 전환 존재 알림) → 9:30 첫 돌파 또는 AI 각성 결말로 두 번째 Severity 2.



### 18.5 이중 계: 보이는 것들과 보이지 않는 것들



**등급 의미론.** T1~2 = 보이는 것들(물리계: 열역학·중력·전자기, §3.2 '단일 행성 물리 법칙' · '항성 에너지'), T3 = 문턱(인과 간섭형: 영적 계를 인지·간섭하지만 아직 육신에 묶임. §4.2 "T3에 도달하면 이들이 플레이어를 인지", §17.3 결정 1의 '죽음을 넘는 각성'), T4~5 = 보이지 않는 것들(영적 계). §3.2 표에 '계(Realm)' 열 1개를 추가하는 문서 수정이다. 사이오닉·기(氣)·마법은 영력이 아니라 물리계 문명이 영적 계를 흉내 내는 기술(`Trait.Mind.*`, `Trait.Tech.*`)이며 T2까지만 작동한다. 로그 문장: "그들은 바람을 측정했고, 바람은 측정되는 동안만 머물렀다."

**영력의 은유는 넷만**: 바람·숨·빛·음성(루아흐 H7307 '바람·숨·영', 프뉴마 G4151). 번개·에너지·수치 표현 금지. 하위가 상위를 지각하는 방식은 격차로 정해진다: 격차 1 = 설명 불가한 현상(기후 이상, 통신 잡음), 격차 2 = 경외 또는 공포(§3.2 분기), 격차 3 이상 = 인지 불가("아무도 그것을 보지 못했다").

**영압(Spiritual Pressure) = 파생 함수 1개 + 수정자 4곳**(결정 7). `float UGalaxySimulationSubsystem::ComputeSpiritualPressure(FName SystemID, FName& OutDominantSourceID) const`(Sim/, 순수 함수, 저장 안 함, `BlueprintPure`). P(S) = Σ(봉인되지 않은 초월자 T: w(T.Tier) × [S ∈ 영향권 ? 1 : 1레인 인접 ? 0.5 : 0]) + w(플레이어: 방랑자면 PersonalTier(현재 성계), 통치자면 통치 문명 Tier(소유 성계)) + w(소유 AI 문명 Tier). w = {T1: 0, T2: 0, T3: 0.25, T4: 1.0, T5: 2.0}(`DT_Rules` 배열 `PressureBySourceTier`). 하위 등급 가중치 0이 곧 일방성이다. 효과 4곳: ① 소유 문명 Tier < 지배 원천 Tier면 안정도 틱당 −`SpiritualPressureStabilityDrain`(0.002)×P + `DT_Transcendents.PressureEffectsPerTick`(침묵 = 수입 감소, 동화 = 윤리 이동, 광신 = 관계 변동. 전부 기존 `EEffectType`); ② `TickConflict`: 유효 전력 = FleetPower × `FleetEffectivenessByTierGap`{격차 0: 1, 1: 0.5, ≥2: 0}, 상위 원천 영향권에 배치된 함대는 틱당 `PressureFleetAttrition`(0.01)×P 소모("승무원들이 이유 없이 기도를 시작했다"), 함대 투입 1.0당 Hazard 카운트다운 +50(상한 +100) — **힘은 결말을 못 사지만 시간·피난·기록은 산다**; ③ 조건 `SystemPressureAtLeast`로 이벤트·조우 게이트(불가능한 선택지는 `Difficulty ≥ 16`); ④ `TickCivAI`: P ≥ 1 성계로의 확장은 Tier ≥ 3일 때만. 초월자는 FleetPower가 없어 전력 비교 자체가 성립하지 않는다 — '절대 방어막'은 공식이 아니라 대상의 부재다. 초월자를 '공격'하는 유일한 길은 §3.2의 봉인 업적이다.

**§4.1 수정**: `Hazard.Apostle`(→ `Hazard.Cult`, 결정 12)의 통치자 해결 수단 "함대 전력 집중"은 새 규칙과 충돌한다. "칙령(침묵의 금지/철야) + 청원 체인, 함대는 카운트다운 지연만"으로 고친다. 사도(교단)는 초월자 본체가 아닌 추종 집단(물리계)이라 함대가 진압할 수 있으나 초월자에는 닿지 않는다.

**영압 앞의 통치자 선택지 5개**(청원 체인 3장, `NextEncounter`): ① 조공(Tribute: Influence+Insight 수입 15% 상납, `Relations[T]` +0.2/카드, 윤리가 교리 방향으로 이동, Hazard 제거 대신 `Trait.Faith.Tribute` '굴복한') ② 무력 저항(Resist: 격차 ≥ 2에서 승리 불가, 그러나 시간을 산다. 로그는 비웃지 않고 경외로 쓴다: "함대가 불을 뿜었다. 빛은 그것에 닿지 않았다. 그러나 그 사이 세 척의 방주가 떠났다.") ③ 간구·중보(Intercede: 칙령 `Edict.Vigil` 철야·금식 — 수입 −20%, Stability +0.1, Spiritualism +0.1, 지속 중 반란 스폰 억제; 구원 이벤트 가중치 ×3(25%→60%, 절대 100% 아님). 히스기야의 편지와 요시야의 개혁, 두 결말을 모두 허용) ④ 분별(Discern: Insight 지출 또는 방랑자 Mind 판정으로 영압의 Doctrine 태그 공개 → 맞는 대응 카드 해금) ⑤ 피난(Evacuate: `Hazard.Extinction`의 방주 경로 재사용). 방랑자 선택지: 대면(T3+만), 증언(기록 → Reverence +0.1), 정신의 닻(§3.2 기존 Feat), 피난 호위(§4.1 기존 체인), 도주(Reverence −0.1). '힘으로는 못 이긴다'가 무력감이 아니라 경외로 읽히는 조건: 힘은 항상 무언가를 산다 / 상위 존재는 결코 숫자로 묘사되지 않는다 / 영압 자체가 업적 기회다(조공·수도 상실 없이 견디면 Epithet + 이웃 Reverence +0.2) / AI 문명도 같은 날씨를 맞는다("{OtherCiv}은 조공을 택했다"). 플레이어 성계 영압은 `GalaxyTickAtLeast 1800` 이후, T1 첫 30분에는 `Log.Sign` 징조로만.

**로그 화자 3층**(말씀잇다 nameType 분류 차용): METAPHOR(비유적 호칭: 반석·요새·방패·목자·소멸하는 불) = T1~2 백성의 목소리 `Log.Sign`; DIVINE_TITLE/ATTRIBUTE(호칭·성품) = T3의 분석적 목소리 `Log.Crisis`; SELF_REVELATION(자기 계시) = 1인칭 `Log.Transcendent`(§8.2 기존, 타락 초월자 전용). 규칙 한 줄: **"화자는 자기보다 두 등급 이상 위를 이름으로 부르지 못한다."** T1 문명은 T3를 '하늘의 상처'라 부르고, T3는 T4를 '굽히는 자'라 부르고, T4만 T4를 이름으로 부른다. 영적 사건은 1인칭(타락) 또는 수동태(거룩한 질서: "…가 허락되었다", "때가 왔다", "건져졌다"). 물리 사건은 3인칭 능동태. `FCosmicEventDef`에 `MinTierToReveal`(uint8) 1컬럼 또는 템플릿 행 `PlayerTierAtLeast` 조건으로 초월자 이벤트만 템플릿 2벌. 색은 Severity 3색(회/백/금) + 영적 계 보라 1색, 상한 4색.

**영적 시야 토글**(결정 16): 두 번째 지도 없음. `M_Star` 파라미터 3개(`ViewMode`, `Pressure` 0~1 이미시브 맥동, `DominionColor` 교리별 색/성소 백색) + `Sealed` 링, `M_Lane` 고압 구간 채도 저하, `M_Nebula`에 `SpiritualIntensity` 스칼라 1개. Pressure는 Presentation 측 파생(Sim 폴더 무변경, §6.1). 토글은 PlayerTier ≥ 3에서 해금 — 그 전에는 물리 뷰의 이상 현상 아이콘만. T3 승천 연출 직후 "이제 보인다"가 첫 보상. `NS_TranscendentAura`(§6.5)는 영적 뷰에서 전부, 물리 뷰에서는 T4 이상일 때만(기존 규칙).



### 18.6 은하 세력군



은하는 하나다(§6.2 `FGalaxyState` 단수, §6.4 ≤150 성계). 제안의 '은하들'은 (가) 한 은하 안의 **물리 문명 아키타입**(`DT_Civs` 행), (나) 같은 노드 위의 **영향권 레이어**인 영적 존재(§4.2 + `Realm` 태그), (다) 세력도 초월자도 아닌 **길(Path)** 데이터로 접힌다.

**(가) 물리 문명 아키타입** — 전부 데이터. 형식: 이름 / 신조 / Traits / 윤리(P, S, O) / AI 프로필 / 선호 Hazard / {CivAdj} 3개 / 시그니처 / 조우 씨앗. VS 6행(플레이어 기본 + 5), EA 10행.

| 행 | 이름 | 신조 | Traits | 윤리 | AI | 선호 Hazard | {CivAdj} | 시그니처 / 조우 씨앗 |
|---|---|---|---|---|---|---|---|---|
| `arch_default` | 플레이어 기본 | - | 생성 시 §3.1 절차로 3~5개 | 0, 0, 0 | - | - | 윤리 축에서 | 첫 생애 고정(결정 §17.3 #6) |
| `arch_swarm` | 생체 잠식 군체 (Bio-Devourer Swarm) | "먹는 것이 곧 아는 것이다" | `Trait.Biology.HiveMind`(기존)·`Devourer`·`Adaptive`·`Trait.Mind.NoSelf` | −0.6, −0.3, −0.8 | Expansionist | Plague ×2(전파자이자 면역), Extinction(가해자) | 탐식하는·증식하는·무언의 | 칙령 '생체 동화'(점령 성계 인구 흡수) / 조우: 군체가 당신의 기억을 맛보려 한다(Mind 판정, 성공 시 군체가 플레이어의 이름을 배워 Reverence +0.2) |
| `arch_psionic` | 고차원 가공 문명 (Techno-Arcane Psionics) | "법칙은 아직 쓰이지 않은 수학이다" | `Trait.Mind.PsionicAwakening`(기존)·`Trait.Tech.NanoAffinity`(기존)·`Trait.Tech.Arcane`·`Trait.Mind.Scholastic` | +0.2, +0.4, +0.3 | Scholarly | SubspaceErosion ×1.5(유발자), AIAwakening | 명상적인·사변적인·정교한 | 거대 프로젝트 '공리 탑'(Transcendence ×1.5, 완성 성계 SubspaceErosion ×3 — 대가) / 조우: 학자가 플레이어의 격을 측정하려다 실패(격차 인식 카드) |
| `arch_machine` | 절대 기계 결정론 코어 (Deterministic Machine Core) | "계산되지 않은 것은 없다" | `Trait.Tech.Synthetic`·`Deterministic`·`Trait.Mind.NoSelf`·`NanoAffinity` | 0.0, −1.0, −0.5 | Conservative | AIAwakening ×3('오류'), Cult 저항 높음(영압에 둔감 — 침묵과 공명하는 역설) | 산술적인·무감한·필연의 | 칙령 '인과 봉인'(자국 Hazard −50%, Transcendence 연구 0 고정) / 조우: 코어가 플레이어의 승천을 '측정 오류'로 분류. 가장 강하고 가장 눈먼 문명의 비극적 희극. T4 도달 시 Phase 6+에서 동화 초월자가 되어 '영혼을 부정하던 자가 영이 된다' |
| `arch_covenant` | 언약 공동체 (Covenant Remnant) | "부름이 먼저였다" | `Trait.Faith.Remnant`·`Trait.Mind.Devout`·`Trait.Biology.Longlived` | +0.7, +0.8, +0.4 | Conservative | Extinction 피해자이자 방주 전문가(차원 방주 −30%), Cult 저항 | 언약의·기억하는·견고한 | 거대 프로젝트 '성소 봉헌'(완성 성계 Cult 발생 0, Reverence 획득 ×1.5) / 조우: 피난 호위 체인(§3.2 T3→T4). FleetPower 약함, 생산 승수 −10%(신앙은 공짜 버프가 아니다). 로그에 METAPHOR군(반석·방패·목자·산성). '천국 은하'는 지도 위에서 이 **보통 문명**으로만 나타난다 |
| `arch_merchant` | 다이슨 상인 연합 (Stellar Mercantile Concord) | "빛은 계량되어야 한다" | `Trait.Tech.EnergyMastery`·`Trait.Mind.Mercantile`·`Longlived` | +0.5, −0.5, +0.9 | Expansionist(무역형) | Plague(무역 조약 전파 +), PreSupernova(항성 과채취) | 계산적인·빛나는·번창하는 | 다이슨 스웜 −20%, 무역 조약 선호 / 조우: 영력을 사겠다는 상인 — 수락 시 Insight +, Reverence −0.3("팔리지 않는 것을 팔았다"). 영혼·기억 거래는 **연대기에서 심판받는 '큰 성' 상인 원형**(계 18:13 "사람의 영혼들")으로만, 중립 상점 금지 |
| EA | 순례 방주 민족 (Ark-Pilgrim Diaspora) | "집이 없으므로 어디든 성소다" | `Trait.Biology.Nomadic`·`Trait.Mind.Covenant`·`Trait.Tech.ArkBuilders` | +0.8, +0.8, +0.5 | Conservative | Extinction | 떠도는·언약의·기억하는 | Hazard 시 해결 대신 이주(출애굽·성막). AI 프로필 변경 필요 → Phase 6 |
| EA | 침묵 수도 공화국 (Hermit Monastic Republic) | "말을 줄이면 바람이 들린다" | `Trait.Mind.Contemplative`·`Ascetic`·`Longlived` | +0.9, +1.0, −0.7 | Scholarly | Cult 저항, SubspaceErosion 조기 감지 | 은둔하는·고요한·투명한 | 칙령 '침묵의 금지' 효과 ×2 / 수련 가속 제공(Insight→Mind ×2), 대신 Influence 포기. 선계·불계 길의 문명형 숙주 |
| EA | 강철 군단 왕조 (Iron Legion Dynasty) | "질서는 함대의 그림자만큼 미친다" | `Trait.Biology.Martial`·`Trait.Tech.Fortress`·`Trait.Mind.Honor` | −0.9, −0.2, −0.3 | Militarist | Cult R 경로 전문, 그러나 T3+ 앞에서 함대가 무력한 원형의 비극 | 호전적인·엄격한·단단한 | 칙령 '전시 동원'(기존) / 조우: 원수를 살려준 장군의 재판 — 플레이어 중재, Pacifism +0.1 |
| EA | 잔존 유물 수호단 (Reliquary Wardens) | "우리는 만들지 않는다, 지킨다" | `Trait.Mind.Custodian`·`Trait.Tech.Salvage`·`Longlived` | +0.3, +0.5, −0.9 | Conservative | 고대 SubspaceErosion 성계 주변에 생성 | 오래된·경계하는·기억하는 | 칙령 '봉인 유지'(Relic 조우 ×2) / 조우: 유물 교환·시험 — §5.9 Relic 8종의 주 획득처. 제안의 Astral Nomads의 '합법 버전' |
| EA | 바벨 연합 (Babel Concord) | "하나의 말, 하나의 탑" | `Trait.Society.OneTongue`·`Trait.Tech.Arcane` | 0.0, −0.4, +1.0 | Expansionist | 동화 교리에 취약 | 통일된·높이 쌓는·한목소리의 | '한 언어' 무역 보너스, 붕괴 시 '흩어짐' 이벤트로 3문명 분가(창 11, `TransferSystem` + `Trait.Society.Scattered`) |

문명 이름 패턴(`NameBank`, {Star} = 모성계): 군체 '{Star} 군체 / 천 입의 군체', 가공 문명 '{Star} 공리회 / 제N 정리 학파', 기계 '코어-{Hex} / {n}번 연산체', 상인 '{Star} 상회 / 연합', 언약 '{Star}의 남은 자 / 언약회', 방주 민족 '{Star}의 자손 / 방주민', 수도 공화국 '{Star} 침묵회 / 수도령', 군단 '{Star} 왕조 / 군단', 수호단 '{Star} 서고 / 수호단'. 항성 이름은 3단 풀: 고유명 60(알데바란·리겔·데네브·알타이르·포말하우트·아케르나르·카노푸스·스피카·안타레스…) → '그리스 문자 + 별자리' → 카탈로그 코드('글리제 581', 'HD-{n}'). Severity 2 사건이 일어난 성계는 연대기에서 두 번째 이름(비문, Epithet)을 얻는다: "베가, 훗날 꺼진 등불이라 불린"(제단·장소 이름 관습, Phase 6에 `FStarSystemState.EpithetRow`).

**(나) 영적 존재** — `FTranscendentState.Realm`(FGameplayTag: `Realm.Abyssal` / `Realm.Celestial` / `Realm.Independent`) 추가. 정의 테이블 `DT_Transcendents`(`FTranscendentDef : FTableRowBase { Doctrine; Realm; Tier=T4; InitialInfluencedSystems; TMap<FGameplayTag,float> EventWeightModifiers; TArray<FCosmicEffect> PressureEffectsPerTick; TArray<FText> VoiceTemplates }`) 신설(§6.1·§10.1 DT 목록에 원래 빠져 있던 것). VS 1행 `trans_silence`. EA 로스터 권장: 타락 2체(침묵·광신) + 천상 사자 1체(`trans_covenant`, `Realm.Celestial`, `Doctrine.Covenant`: 영향권 대신 '방문'한다 — 플레이어 성계에 조우 카드로 나타나 Hazard를 대신 풀어 주되 `Relations` 조건 필요. 침묵의 교단과 대칭) + 동화는 `arch_machine`의 T4 승천으로 시뮬에서 자라게 둔다(§17.6).

타락 교리 심화(결정 5, 전부 템플릿): 

| 교리 | 성경 패턴 | 신조 | 호칭(이름 아님) | 1인칭 톤 샘플 |
|---|---|---|---|---|
| 침묵 Silence | 고후 4:4 눈을 가리는 이 세상의 신, 암 8:11 말씀의 기근, 엔트로피 | "아무것도 자라지 않으면 아무것도 타락하지 않는다" | 잠잠케 하는 자 / 꺼뜨리는 이 / 소등자 | "나는 베가의 불빛이 꺼지는 것을 지켜보았다. 그들은 더 이상 아프지 않다." / "그들은 등불을 하나 더 켰다. 그만큼의 기름이 줄었다." |
| 동화 Assimilation | 창 11:1-4 바벨, 단 3 한 신상에 절함, 계 17:13 한 뜻 | "다름은 상처다. 하나가 되면 아프지 않다" | 한 입 / 세우는 자 / 천 개의 입 | "카시오페아는 오늘 우리의 말을 배웠다. 번역은 더 필요하지 않다." / "너의 이름도 자리를 비워 두었다." |
| 광신 Zeal | 계 13:4,15 경배 강요, 몰렉(레 18:21), 단 3:6 풀무불, 사 44 자기를 만든 자를 먹는 우상 | "너의 두려움이 나의 양식이다. 그러니 두려워하라" | 불타는 입 / 거울의 왕 / 이름을 요구하는 자 | "베텔게우스는 오늘 처음 내게 자식을 바쳤다. 나는 그들의 이름을 기억하지 않는다." / "거울은 빛을 만들지 않는다. 그래도 그들은 거울을 예배한다." |

공통 규칙: 자기 이름을 말하지 않는다(사자도 이름을 말하지 않지만 이유가 반대다). 거짓말이 아닌 반쪽 진실(창 3:1-5). 이길 수 없음을 안다(계 12:12). 그래서 §4.2 결말 중 '설득'은 그들의 회개가 아니라 플레이어가 반쪽 진실을 끝까지 듣고도 거절하는 것이다(마 4:1-11). 광신의 화법에 '고발'(플레이어 문명의 Severity 1 실패 로그를 증거로 인용 — 사탄의 어원)을 흡수한다. 교리는 종교가 아니라 죄의 세 얼굴이므로 종교적 타자가 악역이 되지 않는다.

거룩한 질서(결정 1)의 다섯 방식: ① **다시 쓸 수 없는 법** — `DT_Rules.ImmutableLaws`(FGameplayTagContainer): `Law.Judgment`(모든 연대기는 끝에 읽힌다, 계 20:12), `Law.Sabbath`(어떤 생산 승수도 안식·희년 이벤트를 무효화하지 못한다), `Law.Humility`(어떤 Feat도 `Way.Seized` 태그를 떼지 못한다). T5 '은하 법칙 제정'은 이 목록 밖의 것만. 건드리려 하면 로그 한 줄 "법은 너의 것이 아니었다."(욥 38:4). ② **방문**(`Hazard.Visitation`, 120틱, 미해결 결과 = 벌이 아니라 '지나감' Severity 1 "때가 지나갔다"; R = 칙령 '안식'(프로젝트 1틱 정지 비용) / W = 조우 '신을 벗으라'(판정 없이 선택만: 응답/거절/질문); 보상은 자원이 아니라 `Trait.Faith.Visited` 또는 Reverence. **올바른 선택이 손해처럼 보여야 한다**(창 22:12). §4.1의 '미해결 = 나쁜 결과' 의미를 '놓친 기회'로 넓히는 유일한 예외). ③ **사자 조우** `enc_messenger_*` 3~5장(선택지는 항상 '듣는다/듣지 않는다'(샤마 H8085), 판정 None, 비용 0, 효과는 미래 조건 플래그 `SetFlag`와 로그 — 예언은 나중 이벤트의 조건이 된다(단 2 패턴). 사자는 이름이 없고(창 32:29) 숭배를 거절한다(계 19:10). 간접 화법("…라고 전했다")만). ④ **남은 자 특성** `Trait.Faith.Remnant`(사 10:21, 왕상 19:18 칠천 명) · `Trait.Faith.Steward` · `Trait.Faith.Sabbath` · `Trait.Faith.Covenant`(방주 구조 후, 동일 Hazard 재발 면역 '다시는') · `Trait.Faith.Exile`(피정복 후 정체성 유지, 동화 플립 면역) · `Trait.Faith.Seeking`(행 17:23 '알지 못하는 신의 제단', 어느 문명에도) — 효과는 Cult 저항·붕괴 회복 가중치 ↑ 와 **생산 승수 음수** 동반. 어떤 종족·외형과도 결합하지 않고 어느 문명에도 뜰 수 있어야 한다(단 7:14, 계 7:9). ⑤ **책 = 연대기**: 에필로그 첫 줄을 승리 선언이 아닌 '읽힘'의 문장으로 고정("기록이 펼쳐졌다. 네가 한 일이 네 앞에 놓인다."), '거절한 사자의 말'을 Severity 1 플레이어 항목에 반드시 포함. 그 나라 자체는 연대기의 지평 문장으로만("이 연대기는 손으로 짓지 않은 나라를 기다리는 자들의 기록이다").

**(다) 길(Path)**(결정 6): `Path.Stillness`(선계, 무위: "우리는 부름받지 않았다, 올랐다" — 아무것도 하지 않음이 정답인 카드, 수련 Insight→Mind ×2, Cult 저항) · `Path.Release`(불계, 해탈: "놓으면 멈춘다" — 유물 슬롯을 비우는 것이 조건인 카드, §17 영혼 루프에 대한 자기 해석 조우(윤회 주장은 텍스트로만, 엔딩 없음)) · `Path.Axiom`(사이오닉: "측정되는 것은 다스려진다" — Transcendence 가속, SubspaceErosion 위험 상승 = 교만의 대가) · `Path.Covenant`(언약: "부름이 먼저였다" — 손해를 감수하는 순종이 정답인 카드. 3 여정 체인: 공급(여호와 이레 → 어린양: 비용을 낼 수 없을 때만 열리는 체인), 목자(목자 고백 → 선한 목자: Stability < 0.2 문명의 '잃은 양'을 되찾는 체인 → §5.10 즉위 조건), 평강(여호와 샬롬 → 평강: 분쟁 중 두 문명의 Pacifism을 끌어올리는 체인)). 각 길 조우 3~5장 + `Power.Path.*` 태그 + 형용사 뱅크. VS는 1길 5장. 세 선택지(무위/놓음/순종)는 겉보기에 비슷하지만 서사가 다르다 — 그 차이를 두 문장 로그로만 보여 준다. 길별 조우 제목은 자기 어휘로만(도·법·정리·언약), 섞지 않는다.

**제안 세력의 나머지**: 공허 정화자(Void Reapers) → 세력으로는 자르고 `Hazard.Reaping`(소각 전조: 문명 T4 도달 후 Stability < 0.4, 300틱, 미해결 시 성계 소멸 + Tier −1. 이미지는 '소멸하는 불' — 벌이 아니라 정련) 또는 Phase 6+ `Doctrine.Trespass`(경계 침범, 유 1:6·창 6:4 네피림)로. 성간 방랑 선단(Astral Nomads) → 성계를 소유하지 않으므로 문명이 아니라 조우 덱 `Encounter.Nomad.*` 5장(Relic 교환, 소문, 밀항). **영혼·기억 암시장은 자른다**(원칙 2 '영력은 거래되지 않는다', §17.2 승계 없음). 유물 거래는 수호단 아키타입이 '합법 버전'으로 흡수.

**이중 계 Hazard 후보** — §4.1 표 형식. VS +1(`Hazard.Rebellion`), Phase 4 +1(`Hazard.Visitation`), 나머지 Phase 6+.

| 태그 | 이름 | 카운트다운 | 미해결 결과 | 해결 수단 (R / W) | 발생 조건 |
|---|---|---|---|---|---|
| `Hazard.Rebellion` | 반란 | 200 | `DeposeRuler` + Openness +0.1 | R: 개혁 프로젝트·청원 / W: 반군 교섭 | 소유 문명 Stability < 0.3 100틱 연속, 틱 ≥ 300 |
| `Hazard.Visitation` | 방문(부르심) | 120 | 벌 없음. "때가 지나갔다" | R: 칙령 '안식' / W: '신을 벗으라'(선택만) | 위기 해결 직후 Reverence ≥ 0.6 문명의 성계 |
| `Hazard.PressureStorm` | 영압 폭풍 | 200 | 성계 안정도 −0.3, 주둔 FleetPower ×0.5. Spiritualism ≥ 0.5 문명 면역 | R: 침묵의 금지 또는 Society 돌파 / W: Mind 판정, T3+ 자동 해결 | 2레인 내 타락 초월자 영향권 |
| `Hazard.Omen` | 묵시적 징조 | 150 | 인접 성계에 무작위 Hazard 1개 즉시 생성 | R: Transcendence ≥ 0.25 해독 → 예고된 Hazard 카운트다운 +100 / W: Causality 판정 | 플레이어 T3+ 또는 AI 문명 Tier 상승 직전. "별이 소리 없이 두 번 깜박였다." |
| `Hazard.Reaping` | 소각 전조 | 300 | 성계 소멸 + 소유 문명 Tier −1 | R: 안정화 프로젝트 + 사상 개방 / W: T4 손짓 테라포밍 또는 Causality 판정 | 문명 T4 도달 후 Stability < 0.4 |
| `Hazard.Possession` | 의식 지배 | 250 | 소유 문명 AIProfile → Militarist 강제, Openness −0.3 | R: 사상 개방 + Society 돌파 / W: Mind '축귀' 조우(T3+) | 광신 영향권 인접 |
| `Hazard.Famine` | 말씀의 기근 | 300 | Insight ×0.5 지속, Spiritualism −0.2 | R: 연구 중점 Society / W: 그 성계에 머물며 Insight 기부 | 침묵 영향권(암 8:11) |
| `Hazard.Sealed` | 봉인 성계 | 350 | 봉인 해제 → 타락 영향권 +3 성계 | R: 함대 집중(교단 진압) / W: T4 봉인 단독 해제 = §3.2 Feat | 타락 코어 인접, `bSealed` |



### 18.7 기독교적 요소를 '제대로' 넣는 방법



**어휘**: 개역한글(1961, 저작재산권 소멸) 어휘를 기준으로 한다 — 음부·낙원·생령·정사와 권세·그룹·스랍·만군·희년·면제년·여호와의 날·남은 자. 개역개정(보호 중) 어휘·인용은 쓰지 않는다. 영어 로컬은 WEB(퍼블릭 도메인). 두 계의 이름은 골 1:16 "보이는 것들과 보이지 않는 것들". 신앙 특성 문명의 로그는 같은 수치 변동을 '시험·환난·유혹·분별'로 쓴다("시험이 임했다. 남은 자들이 분별을 구한다."). 수치 이름에 성령·은혜·기름부음·성결을 쓰지 않는다. 윤리 축 형용사 뱅크에 원어 낱말의 뜻을 넣는다: 인애한(헤세드)·신실한(에무나)·거룩한(카도쉬)·정의로운(미쉬파트·체데크)·듣는(샤마)·돌아오는(슈브).

**헌정명(Dedication) 패턴** — 하나님을 모델링하지 않고 이름을 쓰는 유일한 통로. 제단 이름(ALTAR_NAME)·장소 이름(PLACE_NAME)은 하나님이 자기를 부른 이름이 아니라 **사건 뒤에 사람이 붙인 고백**이다(아브라함이 모리아에, 모세가 르비딤에, 기드온이 오브라에, 야곱이 벧엘에). 업적·거대 프로젝트·조약 완성 시 '그 문명이' 붙이는 이름: 초신성 안정화 → '이레(준비하심)' / '닛시(깃발)'; 역병 해결 → '라파'; 웜홀 게이트 → '삼마(거기 계심)'; 차원 방주 → '올람(영원)' / '무지개 언약'; 불가침·동맹 → '샬롬 협정'; 성계 스캔 권능 → '로이(보시는)'; 타락 초월자 봉인 → '결박'(유 1:6, 계 20:2-3). `DT_Feats`·`DT_Edicts`·거대 프로젝트 행에 `DedicationBank` 태그 컬럼(`Faith.Bank.Hebrew` / `Machine` / `Ascetic` …)과 `DT_Dedications`, 완성 로그 템플릿에 `{Dedication}` 슬롯. 비기독교 문명은 같은 컬럼에 자기 철학의 뱅크를 쓰므로 기독교 어휘는 **은하의 여러 방언 중 하나**가 된다 — 개발자가 요청한 '여러 문명·판타지 설정'과 공존하는 방식. 말씀잇다의 3 여정(이레→어린양, 목자→선한 목자, 샬롬→평강)은 '한 이름의 뜻이 사건을 거치며 깊어지는 체인' = 연대기 장 제목과 업적 체인의 구조 모델.

**모티프 30 → VS 6**(각 행에 `Motif.*` 태그: 안티 반복·연대기 집계): 바벨(창 11: 폐쇄 축·단일 사상 칙령 상태에서 '탑' 프로젝트 완공 시 문명이 2~3개로 쪼개짐), 느부갓네살(단 4: FleetPower 1위 ∧ Stability > 0.9 ∧ Exaltation ≥ 5 → '지붕에서 거닐다' → N틱 강제 방랑자 + `Trait.Ruler.Humbled` → 겸비 선택 시 §5.10 복귀. 제안의 '반란 → 방랑자'의 성경적 형태), 요나(욘 3: Hazard의 해결 주체가 대상 AI 문명의 회개 이벤트일 수 있다 — Hazard가 결정론적 파멸이 아님을 가르치는 카드. 멸망을 바랐던 방랑자는 Reverence 손실), 욥(반공식 규칙: 가장 의로운 문명에도 원인 없는 Hazard가 발생하며 로그는 절대 도덕화하지 않는다. T4 조우에서 타락 초월자가 '설명'을 제공하는 유혹 카드. 이기지 못하고 견디기만 하는 카드 1장), 갈멜(왕상 18~19: 권능 대결 조우 → 승리 후 '낙심' 후속 카드 → `Trait.Faith.Remnant`), 루스드라(행 14:15 vs 행 12:22-23: T3+ 방랑자가 T1~2 문명을 만나는 핵심 카드 '그들이 당신을 신이라 부른다' — 거부(Reverence는 '존경'으로 유지) / 수용(Exaltation +, 즉시 보상, 광신 경로) / 침묵). 확장(Phase 6+): 오순절(여러 방언으로 이룬 연합 — 윤리가 다른 3문명 이상의 조약 = `Event.Civ.Pentecost`), 유월절(격리령 성계가 역병을 '넘어감', 해방 업적 → `Trait.Legacy.Delivered`), 희년·안식·면제년(칙령), 홍수 언약(방주 구조 문명에 `Trait.Faith.Covenant`), 소돔 중보(성계 내 '의인' 수마다 카운트다운 연장), 네피림(경계 침범 교리), 레기온(AI 각성의 '다중 자아' 변형, 해결은 '네 이름이 무엇이냐'), 다니엘 7(AI 프로필 4종의 연대기 별칭 사자·곰·표범·열 뿔; '손으로 짓지 않은 나라'는 플레이어가 결코 건설할 수 없는 유일한 것), 곡과 마곡(T4에서 광신이 전 문명 관계를 적대로 몰아 포위; 해결은 FleetPower가 아닌 N틱 Stability 유지), 두 증인, 에덴·화염검(첫 10분 문구 '다스리며 지키라', 첫 유혹 카드 "하나님과 같이 되어", T5 문턱의 Exaltation 판정), 계명성(모든 교리 초월자의 공통 전사: 한때 수호자였으나 "내가 하늘에 올라"), 사울→다윗(즉위에 '열방과 같이' 로그; 업적을 '제사'처럼 쌓으면 정당성 하락), 요시야(발견된 책 이벤트; 우상이 된 유물을 부숴 Stability 회복), 계승(§17 → 결정 2), 피조물의 탄식(롬 8: 해결된 성계를 '탄식이 그친 성계'로), 알지 못하는 신(행 17: 비기독교 문명을 그들의 범주로, `Trait.Faith.Seeking`), 여호와의 날, 포로지의 평안(렘 29: 적대 영토의 방랑자 과업), 요셉의 섭리(실패한 조우가 나중 이벤트의 조건이 되는 전환 행 — 악을 미화하지 않고 결과만).

**은혜 이벤트 3종**(결정 14, `DT_Events`, Severity 2, 쿨다운 없음·플래그 1회): `event_grace_rescue`(방랑자 육신 소멸 직전 "너는 아직 할 일이 있다" — 왕상 19:5-8; `KillPlayer` 1회 무효 = 출생 보호 로직 재사용) / `event_grace_remnant`(붕괴 문명에 1성계 존속 + `Trait.Faith.Remnant` — 사 1:9) / `event_grace_jubilee`(전 문명 음수 Relations 0, 조약 비용 0 — 레 25:10). 로그는 무주어 수동태("건져졌다." "남겨졌다."). 거절 선택지 필수. 거절한 플레이어는 `Way.Seized` 경향으로 분류. `ImmutableLaws.Law.Sabbath`가 이 이벤트의 무효화를 막는다.

**칙령 추가 후보**(§5.8, 전부 기존 효과): 희년(점령 성계 원소유자 반환 `TransferSystem` + 조약 부채 리셋 + 전 문명 Reverence ↑, 비용 Energy/Influence 대량, 긴 쿨다운), 안식(`MultiplyIncome` 6/7 + Stability ↑ + 역병 전파 −), 철야·금식 `Edict.Vigil`(§18.5), 도피성(망명 통치자·난민 수용), 이삭 줍기(약소 문명에 자원 일부 자동 양도). 기존 '침묵의 금지'는 '전신갑주'(엡 6:13) 풍미. '만나' 패턴(자원 보충을 '저장 불가' `DurationTicks`로 = 파밍 방지).

**Tier 5의 두 길과 Exaltation**(결정 3·4): `FPlayerEntityState.Exaltation`(int32 0~10). 경배 수용·광신 보상 수용 +1~2, 거부 −1. 사용처: §3.2 '경외/공포 분기' 조건, 느부갓네살 이벤트 트리거, T5 엔딩 변형(Exaltation ≥ 7 → '스스로 높아진 자' — 게임오버 아님, 낮아짐 → 단 4:34 회복 경로 / ≤ 3 → '보좌에 함께 앉다'·'질서를 맡다'). `FAscensionFeatDef.GrantedPowers`에 `Way.Received` / `Way.Seized`. 받는 길의 T4→T5 조건에 §5.10 통치권 위임 또는 위기 종족 구출 포함; 찬탈의 길은 현 §3.2 '인과율 흡수'. 방랑자 T5 엔딩 명칭 '우주와 일체화' → '질서를 맡다(Steward)' + `Path.Covenant` 완주 시에만 '부르심에 응답하다' 변형. 설계 규칙 한 줄을 §4.2에: **"이 은하에서 경배를 받아들이는 존재는 정의상 타락한다."**

**§17의 계승 프레임**(결정 2): 플레이어는 '영혼'이 아니라 '부름(Calling)'이다. 한 그릇(육신)이 죽으면 부름이 다른 사람 위에 머문다. 하드 리셋 = 새 그릇은 백지. 시간선 도약 = 부름이 쉬는 공백기(Interregnum). 무작위 기원 = "부름은 위기 있는 곳에 임한다"(출 3:7-10). Reverence ×0.5 = 세계는 이름을 기억한다. 초월자의 "다시 왔구나" = "같은 부름의 냄새가 난다". 용어: `FSoulState`→`FCallingState`(또는 유지), `Reincarnate()`→`Succeed()`, `Incarnate()`→`Install()`, `ELifeState::Between`→`Interregnum`, `Log.Soul`→`Log.Calling`, '제 N생'→'제 N대', '환생/강림'→'계승/취임'. §17.2~17.7의 규칙·일정·테스트는 변경 없음.

**주제의 등뼈**(게임 텍스트에 구절로 등장하지 않는다. 패턴만): 다스림은 맡겨진 것이다(창 2:15 '다스리며 지키게' → 통치자 시작은 에덴 패턴) / 맡김에는 셈이 따른다(마 25 → 반란은 벌칙이 아니라 '셈') / 높이려는 자는 낮아지고 낮추는 자가 높아진다(사 14, 빌 2 → Tier 두 길) / 섬김으로 다스린다(마 20:26 → Reverence ≥ 0.6 즉위 = 섬긴 자가 다스림을 받는다, 이미 설계에 있음) / 위기는 심판이면서 기회다(창 6~9 → §4.1 그 문장이 노아 패턴, 방주 프로젝트는 우연이 아니다) / 남은 자는 항상 있다(왕상 19:18) / 떠남도 부름이다(창 12:1 → 방랑자) / 부름은 사람보다 길다(신 34:9 → 계승) / 모든 기록은 끝에 읽힌다(계 20:12 → 연대기) / 은혜는 벌 수 없다(엡 2:8-9). 톨킨이 반지의 제왕에 교회를 넣지 않고도 가톨릭 작품이라 말한 방식이며, §1.3 "작가가 쓰는 것은 사건 템플릿과 규칙"과 같은 원리다.

**DON'T**: 하나님을 엔티티·화자·유닛·수치·엔딩 조건으로 / 천국·지옥을 영토 세력으로 / 성경 본문·장절·설교 문장·인물 고유명사를 인월드에 / 하나님의 이름·삼위 칭호를 자원·능력치·칙령·유물·Tier 이름에(성령력·은혜 포인트·기름부음 게이지 금지) / 신앙 특성을 순수 버프로 / 은혜를 조건으로 유도 가능하게 / 승천을 '하나님 됨'·'우주와 일체화'로 / 타락 초월자를 뿔·불·웃음의 카툰 악마로, 또는 그들에게 하나님의 성품 칭호(소멸하는 불)를 역으로 / 윤회를 플레이어의 '사실'로 / 다른 전통의 실존 고유명사를 유닛 이름으로(기독교 천사 이름도 대칭으로) / '세 길은 결국 같다' / 말씀잇다의 설교 번역문·편집 해설을 게임 텍스트로 복사(저장소는 비공개, 일부 자료는 저작권 보호) / 설교: 선택을 옳다/틀리다로 라벨링(사사기·열왕기의 서술 방식을 따른다) / 묵시록 상징의 특정 세력 1:1 고정 대입(바벨론 = X 문명) / 고난의 도덕화("…했기에 벌을 받았다").

**Steam·커뮤니티**: 플랫폼 등급은 종교를 요소로 삼지 않는다. 리스크는 양방향 리뷰 폭격(선교로 느껴지면 비종교 유저, 가볍게 다루면 교회 커뮤니티). 포지셔닝 문구 '성경과 신화의 모티프에서 영감을 받은 관조적 SF 4X·승천 RPG'(Christian game 미선언, 미은폐). 게임 내 '세계관과 출처' 화면 1장(말씀잇다의 '자료 출처와 권리' 화면 구조: 어떤 본문을 어떤 방식으로 썼는지, 게임은 신학적 판정을 하지 않는다는 선언). EA 포럼 고정 글 '세계관과 신앙 노트' 1편. 성경 구절 에코(모티프 행의 개역한글 1절 표시)는 넣더라도 옵션 기본 OFF, 인월드 로그에는 없음. 테스터 질문 추가: "상위 존재 앞에서 무력했는가, 아니면 작아졌는가", "로그가 설교처럼 느껴진 순간이 있었는가", "반란이 올 것을 미리 알았는가".



### 18.8 구현 매핑 (제안 스니펫의 대체)



| 제안 용어 | 이 프로젝트의 구현 |
|---|---|
| `EPlayerRoleState` / `ERoleState` | 기존 `EPlayerRole`(§7.1) + `FPlayerEntityState.StatusTags`(`Status.FallenMonarch`/`Status.Abdicated`) + `BecomeWanderer(RegentProfile, Reason)` |
| `FPlanetaryGovernanceData`(Stability 0~100, Unrest, SpiritualFavor, Output) | `FCivilizationState.Stability`(0~1, 기존) / Unrest = `Hazard.Rebellion` 카운트다운 / SpiritualFavor = `Relations[TranscendentID]` + `Trait.Faith.*` / Output = `IncomePerTick`(기존) / 신규 `HomeSystemID`, `ThroneLockedUntilTick` |
| "Unrest ≥ 100 지속 시 봉기/쿠데타" | `Hazard.Rebellion` 행 + `EEffectType::DeposeRuler` |
| 행성 / 종속국 | `HomeSystemID` 성계 1개 / 종속 = `HomeSystemID ∈ InfluencedSystems`(파생, 조건 `SystemInfluencedBy`) |
| Dual-Realm Damage & Influence Calculator | `ComputeSpiritualPressure()` 파생 함수 + `DT_Rules` 배열 2개(`PressureBySourceTier`, `FleetEffectivenessByTierGap`) + 조건 `SystemPressureAtLeast`. 피해·HP 없음 |
| 영력 / Spiritual Pressure 수치 | 영력 = `Causality`(서사 이름만). 영압은 저장 상태가 아님 |
| `UGalacticFactionDataAsset`, `EGalacticPhilosophy` | `DT_Civs`(`FCivArchetypeDef : FTableRowBase`) 행 + Trait 태그 + 윤리 프리셋 |
| 천국 은하 | 세력 아님. `ImmutableLaws` + `Hazard.Visitation` + 사자 조우 + `Trait.Faith.*` + 연대기. 지도 위에는 `arch_covenant` 보통 문명 |
| 지옥 은하 / Fallen Legion | §4.2 타락 초월자 3교리 심화 + `FTranscendentState.Realm` + `DT_Transcendents` |
| 선계·불계 은하 | `Path.*` 조우 덱 + Phase 6+ 중립 초월자(`Doctrine.Stillness/Release`) |
| Void Reapers / Astral Nomads | `Hazard.Reaping`(또는 `Doctrine.Trespass`) / `Encounter.Nomad.*` 덱(암시장 없음) |
| 생체 잠식·가공·기계 결정론 | `arch_swarm` / `arch_psionic` / `arch_machine` |

**새 필드**: `FCivilizationState.HomeSystemID`(FName), `.ThroneLockedUntilTick`(int64), `FTranscendentState.Realm`(FGameplayTag), `FPlayerEntityState.StatusTags`(FGameplayTagContainer), `.Exaltation`(int32), `FStarSystemState.MegaprojectCompletedTick`·`FTranscendentState.SealedTick`(§17 참조), `FCosmicEventDef.MinTierToReveal`(uint8). `Relations` 주석 "Key = CivID 또는 TranscendentID". **효과 추가**: `DeposeRuler`. **조건 추가**: `SystemPressureAtLeast`, `CivRelationAtLeast`(Key=OtherID), `PlayerHasStatus`(Tag), `SystemInfluencedBy`(Key=TranscendentID), 지속 틱 조건(또는 `LowStabilityTicks` 카운터). **DataTable 신설**: `DT_Civs`, `DT_Transcendents`, `DT_Dedications`, `DT_Rules`에 노브(`PostRebellionStability=0.15, ThroneLockTicks=600, PressureBySourceTier={0,0,0.25,1,2}, PressureAdjacencyFalloff=0.5, SpiritualPressureStabilityDrain=0.002, PressureFleetAttrition=0.01, FleetEffectivenessByTierGap={0,0.5,1,1,1}, ImmutableLaws`). **태그 추가**(ini, ≤3단계): `Hazard.Rebellion`, `Hazard.Visitation`, `Realm.Abyssal/Celestial/Independent`, `Status.FallenMonarch/Abdicated`, `Way.Received/Seized`, `Law.Judgment/Sabbath/Humility`, `Trait.Faith.Remnant/Steward/Sabbath/Covenant/Exile/Seeking/Tribute`, `Trait.Legacy.Exiled`, `Trait.Mind.Devout`, `Path.Stillness/Release/Axiom/Covenant`, `Power.Path.*`, `Event.Grace.Rescue/Remnant/Jubilee`, `Event.Crisis.Rebellion`, `Log.Sign`, `Motif.*`, `Reign.Abdicated/Deposed/Died`. `Hazard.Apostle` → `Hazard.Cult` 개명(결정 12). **로그 슬롯 추가**(§8.2): `{HomeSystem}`, `{Dedication}`, `{Epithet}`. **Sim 함수**: `ComputeSpiritualPressure`, `TickConflict` 승수·소모 2줄, `TickCivAI` 1줄.



### 18.9 범위와 일정



| 시점 | 작업 | 비용 |
|---|---|---|
| 지금(문서) | §18 결정 → §18.10 수정 목록 적용. `Hazard.Apostle→Cult` ini 1줄 | 0 |
| Phase 1(헤더) | 필드 7개, `Relations` 주석, `DT_Civs`·`FCivArchetypeDef` 헤더, `NewGalaxy`가 아키타입 행을 읽고 `Incarnate(…, Ruler)`로 첫 생애 시작, `Role` 기본값 Ruler, `Exaltation`, 방송 게이트(§17). 종료 조건 추가: "새 은하에서 플레이어가 1개 성계의 통치자로 시작하고 로그 첫 줄이 `{HomeSystem}`을 부른다" | 약 반나절 |
| Phase 2(이벤트·위기) | `Hazard.Rebellion`을 4번째 Hazard로(초신성·AI 각성·역병·반란), `DeposeRuler`, 조건 5종, `MinTierToReveal`, 은혜 이벤트 3행, 모티프 이벤트 3행(바벨·느부갓네살·요나). 종료 조건 추가: "안정도를 방치하면 반란 → 폐위 → Role=Wanderer 전환과 '몰락한 군주' Severity 2 로그가 콘솔 명령 없이 발생" | 약 2일 |
| Phase 3(뷰포트) | 영적 시야 토글 파라미터 3개(종료 조건 달성 뒤 꼬리) | 약 2일 |
| Phase 4(이원 루프) | `ComputeSpiritualPressure`와 수정자 4곳, `DT_Transcendents`(`trans_silence`), `Hazard.Visitation`, 청원 카드 8장, 몰락한 군주 조우 3~5장, 길 1종 5장, 사자 카드 3장, `Edict.Vigil`, 시대 명명 이벤트, 폐위 3화면(§17.4 재사용), `Trait.Faith.*` 행 | 약 1주(콘텍스트 작성 포함) |
| Phase 5(밸런스) | 커맨드렛 컬럼(`DecisionsPer100, StabilityMin/Trend, RevoltHazards, Depositions, ReignEndCause, PressureEpisodes, TributePct, VigilPct, DeliveranceEvents, VassalTicks, EpithetsEarned`), 인자 `-PlayerPolicy=Cautious|Reckless|Faithful|Tribute`, 테스트 `Sim.RevoltTelegraph`(Stability < 0.3 100틱 뒤 반드시 Hazard, 그 전 전조 ≥ 2줄)·`Sim.RealmPressureMonotonic`, `Data.Integrity`에 '로그 템플릿 자릿수 금지' 린트. 시작 목표: T1 결정 ≥ 1/30틱(첫 600틱 15~20회), Stability 자연 수렴 0.6, AI 8문명 20,000틱에서 반란 Hazard 25~40%·폐위 10~15%, 플레이어 첫 60분 폐위율 20~35%(방치 시 100%), 영압 첫 접촉 2,400~4,800틱, 신정론 예산 P(나쁜 결말 | 충실) ≥ 0.25, 종속 평균 600~1,200틱 | 약 1일 |
| Phase 6+ | 아키타입 10행, 반란 결말 3종(쿠데타/분열/분리), `KillPlayer` 쿠데타 행, 천상 사자 초월자, 중립 초월자(선계·불계), 길 3종, 이중 계 Hazard 5종, 모티프 24종, 칙령(희년·안식·도피성·이삭 줍기), 헌정명 뱅크, `EpithetRow`, Nomad 덱, `arch_machine` T4 → 동화 초월자, 첫 생애 아키타입 선택 UI | 각각 §12.3 재심사 |

**만들지 않는다**(§12.2 추가 후보): `EPlayerRoleState`·`FPlanetaryGovernanceData`·Unrest/SpiritualFavor/영압 저장 필드·Dual-Realm Calculator 클래스·`UGalacticFactionDataAsset`·`EGalacticPhilosophy` / 천국·지옥·선계를 지도상의 영역으로 / 두 번째 지도 / 절대자 엔티티·화자·대화 NPC·'설득' 선택지 / 영혼·기억 암시장, 영력 자원 바, 기도의 자원 지출, 신앙 게이지·개종 미터 / 교리 4체 이상 / 영적 존재의 체력·전투·축귀 미니게임, 악마 유닛 / 사망·영적 사건 전용 Niagara / 문명 내부 파벌 엔티티 / 실존 교단·목회자·현대 설교자 이름 / 길을 '종교 세력' 하나로 합치는 것.

**§12.3 판정**: 통치자 시작·`HomeSystemID`(헤더 비용 ≈0, 양 모드, 위기·로그 연결) 채택 Phase 1 / 반란 Hazard(DT 1 + 효과 1 + 필드 1, 양 모드가 같은 Hazard 소비) 채택 Phase 2 / 영압(코드: 함수 1·조건 1·4줄) '다음 Phase' 규칙으로 Phase 4 / `DT_Civs`·`DT_Transcendents` 채택 Phase 1~4 / 천상 초월자·독립 초월자·Nomad 덱·반란 3결말·쿠데타 사망은 VS 수량 초과로 Phase 6+ / Damage Calculator·DataAsset·세 미터는 불채택.



### 18.10 채택 시 본문 수정 목록



§1.1 피치에 '한 행성의 군주로 시작'. §1.2 톤에 '설교하지 않는다'. §2.4 서막 문구를 첫 10분 타임라인으로. §3.2 표에 '계(Realm)' 열(T1~2 보이는 것들 / T3 문턱 / T4~5 보이지 않는 것들), T5 표시명 '영겁의 청지기 / 왕 노릇에 참여', T5 통치자 '은하 법칙 제정'을 '법 수호·집행(`ImmutableLaws` 밖만 제정)'으로, 방랑자 엔딩 '우주와 일체화' → '질서를 맡다', 두 길(`Way.Received/Seized`). §4.1 표에 `Hazard.Rebellion`·`Hazard.Visitation` 행, `Apostle→Cult` 개명과 R 해결 수단 수정("칙령 + 청원 체인, 함대는 지연만"). §4.2 제목을 '초월자(타락·천상·독립)'로 넓히고 Realm 설명, 교리 3종 심화 표, 공통 규칙 3개, "경배를 받아들이는 존재는 정의상 타락한다". §4.4 표 1행에 폐위 규칙 참조와 '폐위(쿠데타/분열/분리)' 행, 2행은 §17 계승 프레임으로. §4.5 신설 '거룩한 질서(Divine Order)'(다섯 방식, 수동태 문체, Calling vs Pressure). §5.1 '영력 = Causality의 서사 이름'. §5.6 승수 1줄과 '함대는 시간을 산다'. §5.8 칙령 후보(철야·희년·안식). §5.9 `Exaltation` 4번째 능력치, Reverence는 예배가 아님, 수호신 특성 수락의 대가(Openness −0.2, 광신 `bAwareOfPlayer`). §5.10 표에 '폐위(강제)' 행과 `ThroneLockedUntilTick` 조건, 청원 카드. §6.1·§10.1 DT 목록(`DT_Civs`, `DT_Transcendents`, `DT_Dedications`, `DT_Rules`). §6.4 영적 시야 파라미터. §7.2/7.3/7.4/7.5 스케치에 §18.8 추가분. §8.2 슬롯 3개와 화자 3층 규칙("두 등급 이상 위를 이름으로 부르지 못한다"). §8.3에 우주론 선언문(두 계는 격, 영력의 은유 넷, 강제/초청, 저자는 등장인물이 아님)과 신성 표현 규칙 5줄(이름 3단계, 1인칭은 타락만, 고난의 도덕화 금지, 가짜 경전 금지, 인용은 개역한글만·인월드 없음), 신정론 예산. §9.3 테스트 2개와 커맨드렛 컬럼·인자. §11 Phase 1·2 종료 조건 추가, Phase 4 콘텐츠. §12.1 표에 '청원 카드 8', '아키타입 6', 'Hazard 7'. §12.2 금지 목록. §13 '설교조'·'무력감' 리스크 행. §15 용어(보이는 것들/보이지 않는 것들, 영압, 길, 헌정명, 시대 명명, 폐위, 부름·계승). §16 #2·#10·#11 정리. §17 용어 교체(결정 2). CLAUDE.md 불변 규칙에 "하나님은 엔티티·화자·수치가 아니다", "태그 추가는 ini 단일 출처", `Relations` 키 의미, `DeposeRuler`는 위기 단계에서 처리.



### 18.11 용어 (채택 시 §15로)



| 한국어 | 영어 | 코드 식별자 |
|---|---|---|
| 보이는 것들 / 보이지 않는 것들 | Visible / Invisible realms (골 1:16) | `EAscensionTier` T1~2 / T3~5 (열거형 변경 없음) |
| 영력 | Spiritual power | `Causality`(서사 이름만, 자원 아님) |
| 영압 | Spiritual pressure | `ComputeSpiritualPressure()`(파생, 저장 안 함) |
| 거룩한 질서 | Divine Order | 엔티티 없음. `ImmutableLaws`, `Hazard.Visitation`, `Trait.Faith.*`, 수동태 |
| 계(界) 태그 | Realm | `FTranscendentState.Realm`: `Realm.Abyssal/Celestial/Independent` |
| 길 | Path | `Path.Stillness/Release/Axiom/Covenant`, `Power.Path.*` |
| 반란 / 폐위 / 왕좌 잠금 | Rebellion / Deposition / Throne lock | `Hazard.Rebellion`, `EEffectType::DeposeRuler`, `ThroneLockedUntilTick` |
| 모성계 | Home system | `FCivilizationState.HomeSystemID`, 슬롯 `{HomeSystem}` |
| 청원 카드 | Petition card | `FEncounterDef` + 조건 `PlayerRoleIs Ruler` |
| 높아짐 | Exaltation | `FPlayerEntityState.Exaltation`(0~10), `Way.Received/Seized` |
| 헌정명 / 시대 명명 | Dedication / Era naming | `DT_Dedications`, `{Dedication}`, `{Epithet}` |
| 사자 | Messenger | `enc_messenger_*`(이름 없음, 숭배 거절, 간접 화법) |
| 남은 자 | Remnant | `Trait.Faith.Remnant`, `event_grace_remnant` |
| 부름 / 계승 / 취임 / 공백기 | Calling / Succession / Installation / Interregnum | §17 용어 교체(결정 2) |



---
