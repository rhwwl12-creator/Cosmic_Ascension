# A3 — 문서 통폐합·Single Source of Truth

## 0. 결론과 전제 교정
사용자는 "설정값이 확립되지 않았다"고 했지만, 저장소를 열어 보면 반대다. 설계 문서 7개 1.7MB, `DT_Rules` 노브 46개, `schema.json` 규칙 45개(why 49개)가 있고 코드 주석이 guideline 절을 574회 인용한다. 문제는 **없음**이 아니라 (1) 현재와 예정이 한 문장에 섞임(`v0.8 — 단계` 162회, `가칭` 63회, `Phase 4` 276회), (2) 같은 규칙이 3~4곳에 복제되어 이미 어긋남, (3) 변경 이력·리뷰 통계가 본문에 박힘(guideline:3922 한 줄 65KB), (4) 한 줄 2KB 넘는 줄 94개라 diff·검색이 안 됨이다. SSoT는 선언(guideline:16)만 있고 구조는 아니다.

## (a) 문서 지도
| 문서 | 분량 | 선언된 역할 | 실제 내용 | 갱신 | 문제 |
|---|---|---|---|---|---|
| guideline.md | 4,598줄 1.1MB, §0~§16 | 설계 SSoT(:16) | 비전+명세+헤더 사본(§7)+테스트 카탈로그+로드맵 일지+미결+이력 | 작업마다 | 이력 25KB(:20-92)와 §0이 본문 앞; §11 상태 줄 65KB; `(§N)` 참조 1,353 |
| v08_rulings.md | 1,147줄 261KB, A~E | 단계 1 판정 기록 | A~D는 결정 근거, E(:709-1147)는 적용 끝난 패치 스크립트 | 1회(동결) | 행 번호가 편집 전 4,789행 기준(:3, '행)' 81회) |
| CLAUDE.md | 279줄 118KB | 작업 규칙 | 불변 규칙 26KB + 빌드 21KB + 엔진 헤더 표 27KB + 작업 일지 33KB | 세션마다 | 일지가 29%; Windows 경로 전제 |
| WORKPLAN.md | 216줄 140KB | effort·ultracode 지시서(:3) | §3·부록 A가 110KB 일지 | 작업마다 | 역할 선언과 내용 불일치, 부록 A가 중복임을 자인(:204) |
| cosmic_species_codex.md | 336줄 48KB | 세계관 원천 | 작가 노트(차용 고유명) | 고치지 않음 | 게임 어휘 규칙과 충돌, 매핑표는 rulings B.2에만 |
| code_artifact.md | 290줄 36KB | 옛 판 | 현행과 334줄 diff | 동결 | v0.8 정경과 어긋난 채 루트 |
| v08_agent_directive.md | 62줄 7KB | 개발자 지시서 | §17 답변 | 동결 | §17 삭제로 앵커 소실(§17 9회) |
| schema.json + ini DevComment + 헤더 주석 | — | 데이터 규칙 | 사실상 4번째 명세 | 코드와 함께 | 규칙 의미가 4곳에 산다 |

## (b) 중복 목록(파일 열어 확인)
| # | 무엇 | 어디 ↔ 어디 |
|---|---|---|
| 1 | 아키텍처 불변 규칙 36줄 26KB | CLAUDE.md:13-48 ≡ guideline:4196-4231(바이트 동일) |
| 2 | '다음' 문단 | CLAUDE.md:278 ≈ WORKPLAN:200(유사도 0.99) |
| 3 | Phase 2 작업 기록 | guideline:3922(65KB) ↔ WORKPLAN 부록 A(:202-216) ↔ CLAUDE.md:244-278; WORKPLAN:143↔:214(0.93) 같은 파일 안 |
| 4 | §16 새 행·확정 표 | rulings D(:613-705) ↔ guideline:4467-4490 |
| 5 | 편집 지침 전문 | rulings E ↔ guideline 본문(적용 완료) |
| 6 | 성역 규칙 | guideline:86,391,936,946,1095,1444,3319 ↔ CLAUDE.md:22,47 ↔ rulings:72-89,812,1093 ↔ directive:43-44 |
| 7 | 년 토글 | guideline:89,250,253,591,1536,3335 ↔ CLAUDE.md:35 ↔ WORKPLAN:176 ↔ directive:53 |
| 8 | arch_seeker 전환 괄호 | guideline §5 안 13회 + :87,328,407,530 ↔ WORKPLAN:65,194 |
| 9 | 추첨 규칙·틱 순서·핸들러 금지 | guideline §7.4/§7.5/§9.2 ↔ CLAUDE.md:14-15,41 ↔ §14.3 |
| 10 | 판정 어휘 7낱말 | schema.json verdict ↔ CLAUDE.md:32 ↔ guideline:3398 |
| 11 | Hazard 카운트다운·결과 | ini DevComment ↔ guideline §4.1 표 ↔ DT_Hazards.json |
| 12 | RealmCapByTier 1/3/4/7/10 | guideline 11곳 ↔ CLAUDE.md:253 ↔ DT_Rules.json ↔ 헤더 기본값 |
| 13 | 변경 이력 | guideline:20-92 ↔ §0 ↔ §16 확정 블록 24개(:4490-4598) |
| 14 | 대도감 두 판 | code_artifact.md ↔ cosmic_species_codex.md(약 70% 동일) |
| 15 | 빌드 명령 두 벌 | CLAUDE.md:63-101 ↔ :105-129; Tools 스크립트 docstring 4개 |
| 16 | 설계 낱말 사전 | CLAUDE.md:48 ↔ guideline §15(124행) ↔ rulings:22-33 ↔ WORKPLAN:65 |
| 17 | 헤더 주석 = 문서 사본 | GalaxyState.h·CosmicEventSubsystem.h 주석 ↔ guideline §7.2/§7.5 |

## (b') 상충 목록과 택할 기준
| # | A | B | 택함 |
|---|---|---|---|
| 1 | 자동화 테스트 '12종'(guideline:3887, CLAUDE.md:251) | 실제 76(Tests grep) | 코드. 수치는 문서에서 뺀다 |
| 2 | 용도 상수 8(CLAUDE.md:254, guideline:3896) | 9(CosmicSeed.h:22-34, guideline:3531) | 9 |
| 3 | SaveRestart 1,000틱(guideline:3886) | 3,000(:3563, Tests) | 3,000; §11은 역사 표지 |
| 4 | AI 각성 '생산 +150%/성계 반란'(guideline:625, ini) | +30/−0.15(:652, DT_Hazards.json) | 데이터 기준, 표를 현행/EA 목표로 분리 |
| 5 | `-TellAll`은 Phase 5(guideline:3135) | 커맨드렛 구현됨(:3277, 커맨드렛 5건) | 구현 |
| 6 | #95 '세 파일'(rulings:658) | '네 파일'(guideline:4488) | guideline(최신) |
| 7 | 연대기 이관 Severity 1·2 전부(§8.4:3407 = 코드) | BuildChronicle 읽는 것만(#40 ⑦:4444) | 결정은 ⑦, 코드 미반영 → '예정' 표지 |
| 8 | 저장 '은하당'(guideline:22,4590) | '판당'(:3488) | 판당 |
| 9 | 주기는 해로 환산 안 함(:69) | 년 토글(:250) | :250(표시 전용) |
| 10 | 첫 생애 sol·arch_terran(DT_Rules.json, Tests, 코드) | sys_0·arch_seeker(v0.8 본문) | 의도된 지연 — '현행/목표' 두 열로 |
| 11 | '항성 안정화'(guideline 19곳) | DT '초신성 안정화' | 단계 5 예정 표지 |
| 12 | §10.1 WBP 레이아웃·M_Lane·L_MainMenu | 실제 C++ 위젯·LineBatch·L_Galaxy | 코드 기준으로 §10 재작성 |
| 13 | 사자가 책을 전달(directive:21,30) | 가리키기만(CLAUDE.md:23) | guideline/CLAUDE |
| 14 | Saved/Design 헤더 초안 참조(CLAUDE 4, WORKPLAN 5) | 이 체크아웃에 Saved/ 없음(.gitignore:6) | 설계 기록을 추적 폴더로 |
| 15 | '부르심'=서막(:327) | '부르심'=첫 구간(:253) | 용어집에서 하나로 |

## (c) 새 문서 체계와 매핑
digest가 제안한 docs/ 틀을 받되 1인 개발에 맞게 **파일 12개 상한**으로 자르고, 두 가지를 더한다: 규칙 ID 앵커(§번호 대신)와 '생성되는 문서'(노브 표는 손으로 쓰지 않는다).

| 새 문서 | 목차 핵심 | 원본 |
|---|---|---|
| CLAUDE.md(≤15KB) | 불변 규칙 20줄 요약·명령 5줄·포인터 | CLAUDE.md:3-48 압축; 빌드 상세→40, 헤더 표→docs/45_ENGINE_NOTES.md, 일지→50/CHANGELOG |
| docs/00_VISION.md | 피치·코어 루프·3기둥·정체성·신학적 토대 선언·톤 | guideline §1~§3.3, §4.5 머리, §13 포지셔닝 |
| docs/10_DESIGN/{time,economy,civ,crisis,transcendent,ascension,calling,knowing,text}.md | 시스템별: 규칙(현행)/예정/미결/테스트 이름 | §2.1, §5.1-5.6, §5.11, §4.1-4.3, §3.2+§4.2, §4.4, §8.1 |
| docs/20_ARCHITECTURE.md | 층·Subsystem·FGalaxyState·틱 열 단계·난수 셋·해시·저장·데이터 파이프라인 | §6, §7.1-7.3 요약, §7.4-7.5 규칙만, §9.1-9.2; 헤더 사본은 코드 링크로 대체 |
| docs/30_CONFIG_REFERENCE.md(생성) | DT_Rules 46노브: 값·범위·읽는 코드·Phase | DT_Rules.json+schema.json+헤더 주석 → Tools 스크립트 산출 |
| docs/40_DEV_GUIDE.md | 컨벤션·세션 절차·effort·헤더→리뷰→변이 파이프라인·빌드/테스트 | §10, §14.1-14.2, WORKPLAN §0-§2, CLAUDE 빌드 절 |
| docs/50_ROADMAP.md | 단일 번호 체계·단계별 상태 표(끝/최소형/미착수+커밋)·종료 조건 | §11(상태 줄 제외), §12, WORKPLAN §3 표, 'v0.8 단계 0~8' |
| docs/60_DECISIONS/ADR-nnnn.md + OPEN_QUESTIONS.md | 결정·근거·대안·영향; 열린 물음만 | §16 확정 24블록, rulings A·D, directive, Saved/Design 헤더 초안; §16 미결 58행 |
| docs/70_CONTENT_GUIDE.md | 템플릿 문법·화자 3층·사후 문장 ①~⑩·이름의 거름·고정 문안 표(ST_CA 1:1) | §8.2-8.3, §8.5-8.6, §5.11 이름, rulings B.12·C.2 문장 |
| docs/90_LORE/ | codex 원문·대도감 배정표·신학 노트(성경 근거 모음) | codex, rulings B.2·C, guideline 성경 인용 |
| docs/CHANGELOG.md | 날짜·커밋·바뀐 규칙 ID | guideline:20-92, §0, §11 상태 줄, CLAUDE 일지 |
| docs/archive/ | 동결본 | guideline_v0.8.md, v08_rulings.md, directive, code_artifact.md |

## (d) 가독성 비용과 규칙
비용: 길이 1,000B 넘는 줄 276개, 괄호 § 참조 1,353 + `§16 #` 221, 성경 장절 패턴 약 400건, '후보' 92·'인쇄 전' 14. 용어 대응표(§15)가 문서 끝(:4271)이라 앞 3,000줄을 매핑 없이 읽는다. '칸'(Tier/프로젝트 슬롯), '부르심', '문턱'(T3/T5), '정착률'(주장 만료율)이 두 뜻이다. 코드 주석이 §번호를 574회 인용해 재편 시 전부 깨진다.
규칙: ① 2중 표기 — 첫 등장 "인게임 낱말(코드 식별자)", 이후 10/20/30은 식별자, 00/70은 인게임 낱말. ② 한 줄 1문장·300자 이내, 괄호 1단, 번호 목록. ③ 규칙 단위에 안정 ID(`RULE-SIM-TICK-ORDER`)와 상태 표지 `[구현]`/`[예정 P4]`/`[미결 Q-71]`. ④ 근거(성경·채팅 인용·리뷰 통계)는 ADR/lore로. ⑤ §15는 3열(인게임/표준 영문/식별자)+상태 열의 **코드 사전**으로 재정의하고 금지·허용 어휘는 70으로 분리.

## (e) 이관 절차(2단계 실행안)
| 단계 | 일 | 검증 | 커밋 |
|---|---|---|---|
| 0 준비 | docs/SECTION_MAP.md(§N→새 파일#앵커), 규칙 ID 부여 스크립트 | §번호 전수 추출 1,353건 매핑 누락 0 | 1 |
| 1 이력 분리 | :20-92·§0·§11 상태 줄·CLAUDE 일지·WORKPLAN 부록 A → CHANGELOG; 확정 24블록 → ADR | 원문 글자 수 합 = 이관 후 합(손실 검사) | 1 |
| 2 CLAUDE 다이어트 | 불변 규칙 요약·포인터만 | 테스트 76종 2회 통과·데이터 불변(check_data) | 1 |
| 3 본문 분할 | §→10/20/70, 현행/예정 열 분리 | Python으로 `(§N)` 전부 링크 치환, 깨진 링크 0 | 절마다 1 |
| 4 코드 주석 | 'section N'→규칙 ID (574+73) | 빌드·테스트 2회 | 1 |
| 5 아카이브 | 옛 파일 → docs/archive, 루트엔 CLAUDE·README만 | git mv로 이력 보존 | 1 |
보존: 감사 기록은 CHANGELOG에 커밋 해시와 함께, 미결 번호는 Q-nn으로 재부여하되 옛 #N을 괄호로 1회 남긴다. 각 단계 뒤 `python Tools/check_data.py`와 테스트 두 번 — 문서 변경이 데이터 도장(generated.sha1)에 닿지 않음을 확인한다.

## 권장안
- [P0] **CLAUDE.md를 15KB 이하로 줄이고 불변 규칙의 단일 출처를 정한다** — CLAUDE.md:13-48과 guideline.md:4196-4231이 바이트 동일 사본이다. 한쪽(권장: docs/20_ARCHITECTURE·40_DEV_GUIDE의 규칙 ID)을 원본으로 두고 CLAUDE.md에는 20줄 요약과 포인터만 남긴다. 빌드 명령 두 벌(:63-101/:105-129)은 러너 스크립트 하나로, 엔진 헤더 표(:159-224, 27KB)는 docs/45_ENGINE_NOTES.md로, 작업 일지(:244-278)는 CHANGELOG·ROADMAP으로 옮긴다. 세션마다 읽히는 파일이 118KB인 것이 에이전트 비용과 드리프트의 1차 원인이다.
- [P0] **변경 이력·리뷰 통계·해시를 본문에서 떼어 docs/CHANGELOG.md로 보낸다** — guideline.md:20-92(25KB), §0(:128-159), §11 상태 줄(:3922 한 줄 65KB, :3935 9KB), §16 확정 블록 24개(:4490-4598), WORKPLAN 부록 A(:202-216), CLAUDE.md '현재 단계'가 같은 사건을 3~4번 기록한다. 커밋 해시·날짜·바뀐 규칙 ID만 남기는 한 파일로 모으고, 원문 글자 수 합계로 손실을 검사한 뒤 지운다. '옛 §17'처럼 번호가 재사용된 이력은 ADR 번호로 다시 가리킨다.
- [P0] **모든 규칙 문장에 [구현]/[예정 Phase N]/[미결 Q-nn] 상태 표지와 안정 규칙 ID를 붙인다** — 'v0.8 — 단계' 162회, '가칭' 63회, '그 전의 코드는' 16회가 보여주듯 현행과 목표가 한 문장에 공존한다. 10_DESIGN 각 절을 '현행 규칙 | 목표(Phase) | 미결' 3열로 쓰고 규칙마다 RULE-<영역>-<이름> ID를 둔다. 코드 주석이 'section N'을 비테스트 Source 574회·Tests 73회·Tools 8회 인용하므로 docs/SECTION_MAP.md(§N→파일#ID)를 먼저 만들고 코드 주석 치환은 마지막 단계에 한다.
- [P0] **로드맵 번호 체계를 하나로 통일하고 상태 표로 바꾼다** — Phase N(§11)·v0.8 단계 0~8(WORKPLAN §3)·WORKPLAN 작업 번호 1~12·§16 #1~#95·①~⑧이 한 문장에 섞인다(WORKPLAN:98 한 줄에 #참조 57개). docs/50_ROADMAP.md는 '마일스톤 → 작업 → 상태(미착수/최소형/끝+커밋)·종료 조건·선행 결정' 표 하나로 하고 주 단위 추정(Phase 2 '3주+' 계획이 5일 실행)은 뺀다. Playable MVP → 코어 시뮬 완성 → 폴리싱의 세 마일스톤을 이 표의 1열로 삼는다.
- [P0] **승인된 헤더 초안(Saved/Design)을 추적 폴더 docs/60_DECISIONS/headers/로 옮긴다** — CLAUDE.md:271과 WORKPLAN이 '직접 승인한 헤더 초안'으로 Saved/Design/phase3_*/header_draft.md를 가리키지만 Saved/는 .gitignore:6에 있고 이 체크아웃에는 폴더 자체가 없다. 설계 승인 기록이 한 PC에만 있는 상태라 SSoT 구축의 전제가 빠져 있다. 개발자 PC에서 복사해 커밋한 뒤 ADR로 흡수한다.
- [P1] **§15 용어집을 '인게임 낱말 / 표준 게임 용어 / 코드 식별자 / 상태' 4열 코드 사전으로 재정의한다** — 현재 124행 4열이지만 11행이 '식별자 없음'이고 OfferReturn·DT_Edicts처럼 코드에 없는 식별자가 표지 없이 적혀 있다(Source에 OfferReturn 0건). 두 뜻 낱말('칸'=Tier/프로젝트 슬롯, '부르심'=서막/첫 구간, '문턱'=T3/T5, '정착률'=주장 만료율)을 하나로 고정하고, 2중 표기 규칙(첫 등장 '인게임 낱말(식별자)', 기술 문서는 식별자 우선)을 문서 머리에 둔다. 금지·허용 어휘(판정 어휘·이름의 거름)는 70_CONTENT_GUIDE로 분리한다.
- [P1] **docs/30_CONFIG_REFERENCE.md를 손으로 쓰지 말고 DT_Rules.json+schema.json에서 생성한다** — 노브 값이 guideline 본문·헤더 초기화식·schema 범위·JSON 네 곳에 있고 FallenTranscendentCount(C++ 3/JSON 1)처럼 이미 어긋난다. Tools에 작은 Python 생성기를 두어 '노브·값·범위·읽는 코드 위치·Phase' 표를 만들고, 런타임이 읽지 않는 노브(Succession 6·Fleet 4·ImmutableLaws·Vigil/ReturnLure 등 18개)에 '미사용' 열을 자동으로 찍는다. 생성 파일은 generated.sha1 도장 대상에 넣는다.
- [P1] **guideline §4.1·§10·§12.1 같은 낡은 표를 코드·데이터 기준으로 먼저 고친다** — §4.1 AI 각성 '생산 +150%/성계 반란'(:625) vs 데이터 +30/−0.15, §10.1의 WBP_*·M_Lane·L_MainMenu vs 실제 C++ 위젯·LineBatch·L_Galaxy, §12.1의 'Niagara 1' vs 미룸, '테스트 12종'(:3887) vs 76. 분할 전에 이런 표를 '현행'으로 되돌려야 옮기는 과정에서 옛 값이 새 문서로 번지지 않는다.
- [P1] **v08_rulings E장·directive·code_artifact는 아카이브하고 A~D·§16 확정 24블록은 ADR로 흡수한다** — rulings E(:709-1147, 104KB)는 편집 전 4,789행 기준 행 번호를 81회 쓰는 적용 완료 패치이고 파일 머리(:3)가 '다시 적용하지 않는다, guideline이 이긴다'고 못박는다. directive는 삭제된 §17을 9회 가리킨다. code_artifact.md는 현행 codex와 334줄 diff이며 v0.8 정경(성역 밖 인류 없음)과 충돌한다. 셋을 docs/archive/로 git mv하고 결정·근거·대안만 ADR-nnnn 양식(출처 [개발자 확정]/[권장대로]/[고침] 유지)으로 옮긴다.
- [P1] **ini DevComment·헤더 주석·스키마 why에서 설계 수치를 걷어내고 한 줄 식별 설명만 남긴다** — Hazard.AIAwakening DevComment '50% production +150% / 50% revolt'는 데이터와 다르고, 헤더 주석 'RevelationTemplates Exactly 3'는 스키마(1..3)·데이터(1줄)와 다르다. 수치·규칙의 출처는 docs 10_DESIGN과 schema.json 둘로 제한하고, ini와 헤더는 '어디를 보라'는 규칙 ID만 적는다.
- [P2] **문서 린트 스크립트를 Tools에 둔다(줄 길이·괄호 깊이·금지 표지·깨진 링크)** — guideline 한 줄 1,000B 초과 276개, 2,000B 초과 94개, 괄호 중첩 3~4단이 재발을 막는 장치 없이 쌓였다. check_data.py와 같은 꼴로 docs/*.md를 검사해 300자 초과 줄, 괄호 2단, '가칭'·'인쇄 전' 표지의 상태 열 누락, SECTION_MAP에 없는 §참조를 보고한다. 이관 단계 3·4의 통과 조건으로 쓴다.
- [P2] **신학 근거와 대도감 배정표를 docs/90_LORE로 분리하고 00_VISION에 토대 1문단을 명시한다** — guideline 본문에 성경 장절 패턴이 약 400건 섞여 '왜'가 '무엇'보다 길다(§4.5 성역 밖 문단 2.3KB 전부 근거). 규칙 문장에는 근거를 두지 않고 90_LORE/theology_notes.md에 규칙 ID별로 모은다. 반대로 §1 비전에는 신학적 토대가 한 줄도 없으니 00_VISION에 '거룩한 질서는 저자이지 등장인물이 아니다' 수준의 선언 1문단을 둔다.
- [P2] **§9.3 테스트 표를 등록된 76종만 5열 카탈로그로 정규화하고 미래 17행은 로드맵으로 보낸다** — §9.3은 한 셀 16.7KB(:3582)에 리뷰·재감사 이력이 섞여 있고 93행 중 17행이 Phase 4b/5/6a 약속이다. '이름 / 목적 / 핵심 단언 / 실행 조건 / 소요' 다섯 열로 등록 테스트만 적고(생성 가능: IMPLEMENT_SIMPLE_AUTOMATION_TEST grep), 밸런스 목표·실측·테스터 질문은 50_ROADMAP과 10_DESIGN/balance로 나눈다.
- [P2] **DEV_GUIDE의 빌드·테스트 절을 플랫폼 중립으로 쓰고 경로는 변수로 뺀다** — CLAUDE.md:5-7·61은 Windows 절대 경로와 PowerShell 5.1을 전제하지만 저장소는 지금 Linux 경로에 있다. 40_DEV_GUIDE는 '두 번 연속 실행·변이로 한 번 깨뜨리기·헤더→리뷰→재감사' 절차를 본문으로, 명령은 Tools/run_tests.(ps1|sh) 한 쌍과 환경 변수(UE_ROOT, PROJECT_ROOT)로 둔다.

## claims
- A3-1 [factual/high] CLAUDE.md 13~48행(아키텍처 불변 규칙 36줄, 약 26KB)과 guideline.md 4196~4231행(§14.3 CLAUDE.md 템플릿)은 바이트 단위로 동일한 사본이다 — 불변 규칙이 두 파일에 산다.  (근거: diff <(sed -n '13,48p' CLAUDE.md) <(sed -n '4196,4231p' guideline.md) 결과 차이 없음; guideline.md:4181-4251 코드 펜스; CLAUDE.md:13-48)
- A3-2 [factual/high] Phase 2·3 작업 기록이 세 문서에 중복된다: guideline.md:3922는 한 줄 65,165바이트의 상태 줄이고, WORKPLAN.md:204가 부록 A가 CLAUDE.md '현재 단계'를 옮긴 것임을 자인하며, CLAUDE.md:278과 WORKPLAN.md:200의 '다음' 문단은 문자 유사도 0.99, WORKPLAN.md:143과 :214는 0.93이다.  (근거: guideline.md:3922(wc -c 65165); WORKPLAN.md:204; difflib SequenceMatcher: CLAUDE.md:278 vs WORKPLAN.md:200 = 0.99, WORKPLAN.md:143 vs :214 = 0.93)
- A3-3 [factual/high] v08_rulings.md의 E장(709~1147행)은 편집 전 guideline(4,789행) 기준 행 번호를 81회 쓰는 적용 완료 패치 스크립트이고, 적용은 커밋 9aba0e9로 끝나 현재 guideline은 4,598행·§17 없음이다 — 파일 머리가 스스로 '다시 적용하지 않는다, guideline이 이긴다'고 적는다.  (근거: v08_rulings.md:3, :714; grep -o '[0-9]\+행)' v08_rulings.md | wc -l → 81; git log --oneline(9aba0e9 '문서: v0.8 확정 — §17 흡수'); wc -l guideline.md → 4598; grep '^## ' guideline.md에 §17 없음)
- A3-4 [factual/medium] 문서 사이·문서와 코드 사이에 수치 상충이 남아 있다: 자동화 테스트 '12종'(guideline.md:3887, CLAUDE.md:251) vs 실제 76; CosmicSeed 용도 상수 '8'(CLAUDE.md:254, guideline.md:3896) vs 헤더 9개(guideline.md:3531도 9); '-TellAll은 Phase 5'(guideline.md:3135) vs 커맨드렛 구현(:3277); §16 #95 '세 파일'(v08_rulings.md:658) vs '네 파일'(guideline.md:4488); SaveRestart 1,000틱(:3886) vs 3,000(:3563, Tests).  (근거: grep -rc IMPLEMENT_SIMPLE_AUTOMATION_TEST Source/Cosmic_Ascension/Tests/*.cpp 합 76; Source/Cosmic_Ascension/Core/CosmicSeed.h:22-34(열거자 9); grep -c TellAll Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp → 5; Tests/CosmicSimTests.cpp 'RestartTicks = 3000')
- A3-5 [factual/high] 코드 주석이 guideline 절 번호('section N')를 비테스트 Source에서 574회, Tests에서 73회, Tools에서 8회 인용하고 guideline 본문 안의 괄호형 § 참조는 1,353개, '§16 #' 참조는 221개다 — 섹션 번호를 바꾸는 재편은 매핑표 없이는 코드·문서 참조를 전부 깨뜨린다.  (근거: grep -rhoE 'section [0-9]+(\.[0-9]+)?' Source/Cosmic_Ascension --exclude-dir=Tests | wc -l → 574; Tests → 73; Tools → 8; grep -o '(§[0-9]' guideline.md | wc -l → 1353; grep -o '§16 #' guideline.md | wc -l → 221)
- A3-6 [factual/medium] CLAUDE.md:271과 WORKPLAN.md가 '직접 승인한 헤더 초안'의 위치로 Saved/Design/phase3_*/header_draft.md를 가리키지만(참조 CLAUDE 4회·WORKPLAN 5회·guideline 1회), Saved/는 .gitignore 6행에 있고 이 체크아웃에는 Saved/ 폴더 자체가 없다 — Phase 3 설계 승인 기록이 저장소 밖에 있다.  (근거: grep -c 'Saved/Design' CLAUDE.md WORKPLAN.md guideline.md → 4/5/1; .gitignore:6 'Saved/'; ls Saved → 없음; CLAUDE.md:271)
- A3-7 [judgment/high] 사용자의 전제('기본/상세 설정값이 확립되지 않았다')는 절반만 맞다. 설정값은 부족한 것이 아니라 과잉이고 읽을 수 없다: DT_Rules 노브 46개 중 약 18개는 런타임이 읽지 않고, 규칙 문장 안에 'v0.8 — 단계' 표지 162회·'가칭' 63회가 섞여 '지금 참인 값'을 가려낼 수 없다. 통폐합의 핵심 작업은 새 설정값 작성이 아니라 현행/예정/미결의 분리와 복제 제거다.  (근거: Tools/data/DT_Rules.json(46키); grep -rl OfferReturn Source → 0, IsSanctuary → 0; grep -o 'v0.8 — 단계' guideline.md | wc -l → 162; grep -o '가칭' guideline.md | wc -l → 63; Core/GalaxyState.h:545(LeapTicks 주석만); digest c1/c8의 미사용 노브 목록)
- A3-8 [judgment/medium] 1인 개발에는 digest 제안의 docs/ 틀을 파일 12개 상한으로 줄이고 (1) 규칙 ID 앵커, (2) 생성형 CONFIG_REFERENCE, (3) CLAUDE.md 다이어트+CHANGELOG 분리를 1단계로 두는 순서가 유지 비용이 가장 낮다 — 분할을 먼저 하면 낡은 표(§4.1·§10·§12.1)가 새 문서로 번진다.  (근거: CLAUDE.md 섹션 바이트: 불변 규칙 26,294·빌드 21,066·헤더 표 27,261·일지 33,400(총 118,092); guideline.md 1,000B 초과 줄 276개·2,000B 초과 94개; guideline.md:625 vs Tools/data/DT_Hazards.json(AI 각성 효과); guideline.md:3795-3801 vs Content/CA/UI(WBP_LogEntry만))
