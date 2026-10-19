"""Phase 2 document consolidation for Cosmic_Ascension (mechanical, verbatim moves).

Usage: python3 -I migrate.py <repo_root> <heads_dir>
Writes docs/ tree; does not delete or git-mv anything (the shell does that after).
"""
import os, re, sys, json

R = sys.argv[1]; H = sys.argv[2]
D = os.path.join(R, 'docs')

def read_lines(path):
    with open(path, encoding='utf-8') as f:
        return f.read().split('\n')

G = read_lines(os.path.join(R, 'guideline.md'))
C = read_lines(os.path.join(R, 'CLAUDE.md'))
W = read_lines(os.path.join(R, 'WORKPLAN.md'))
RU = read_lines(os.path.join(R, 'v08_rulings.md'))

def L(src, a, b):
    """1-indexed inclusive range -> list of lines, collapsing blank runs."""
    out = []; blank = 0
    for ln in src[a-1:b]:
        if ln.strip() == '':
            blank += 1
            if blank > 1: continue
        else:
            blank = 0
        out.append(ln)
    return out

def head(name):
    with open(os.path.join(H, name), encoding='utf-8') as f:
        return f.read().rstrip('\n').split('\n')

BANNER = ('> 원문: guideline.md v0.8 {sec} (2026-10-18 docs/ 재편으로 이관, 내용 무변경). '
          '표지 `(v0.8 — 단계 N부터)`·`(v0.7 — …)`·`Phase N`이 붙은 문장은 **예정**(아직 코드에 없음), '
          '표지 없는 문장은 **현행 규칙**이다. 열린 물음은 `docs/60_DECISIONS/OPEN_QUESTIONS.md`, 확정 기록은 '
          '`docs/60_DECISIONS/DECISIONS_LOG.md`. 절 번호(§N)는 원문 그대로 두었다 — 위치는 `docs/SECTION_MAP.md`.')

def write(rel, lines):
    p = os.path.join(D, rel); os.makedirs(os.path.dirname(p), exist_ok=True)
    txt = '\n'.join(lines).rstrip('\n') + '\n'
    with open(p, 'w', encoding='utf-8', newline='\n') as f: f.write(txt)
    return len(txt.encode('utf-8'))

# ---- drift edits applied on moved copies (originals untouched; archive keeps them) ----
sec4 = L(G, 600, 977)
# note after the hazard table that contains the AI awakening row (original 625)
for i, ln in enumerate(sec4):
    if ln.startswith('| `Hazard.AIAwakening`'):
        j = i
        while j + 1 < len(sec4) and sec4[j + 1].startswith('|'): j += 1
        sec4.insert(j + 1, '')
        sec4.insert(j + 2, '※ 현행 데이터(2026-10-18): 표의 결과 열은 VS 목표다. `Tools/data/DT_Hazards.json`이 원본이고 Phase 2의 축소형은 `crisis_ai_awakening` 만료 → Energy/Matter +30 또는 Stability −0.15(분기), `crisis_plague` → 이웃 효과 없음이다(아래 §4.1의 \'잠정 값\' 문단 참조). Cult·Extinction·SubspaceErosion·Visitation은 태그만 있고 행이 없다.')
        break
sec7 = L(G, 1618, 3210)
for i, ln in enumerate(sec7):
    if '인자 -TellAll은 Phase 5' in ln:
        sec7[i] = ln.replace('인자 -TellAll은 Phase 5', '인자 `-TellAll=0|1`은 Phase 2 종료 리뷰에서 구현됨 — 2026-10-18 정정')
        break
sec10 = L(G, 3731, 3850)

sizes = {}
# ---- 00_VISION ----
sizes['00_VISION.md'] = write('00_VISION.md', head('vision_head.md') + [''] + L(G, 160, 347))
# ---- 10_DESIGN ----
sizes['10_DESIGN/pillars.md'] = write('10_DESIGN/pillars.md', ['# 핵심 게임플레이 3대 기둥 (옛 §3)', '', BANNER.format(sec='§3'), ''] + L(G, 348, 599))
sizes['10_DESIGN/crisis_and_transcendents.md'] = write('10_DESIGN/crisis_and_transcendents.md', ['# 긴장과 위기: 위기 생성기·초월자·엔딩·거룩한 질서 (옛 §4)', '', BANNER.format(sec='§4'), ''] + sec4)
sizes['10_DESIGN/systems.md'] = write('10_DESIGN/systems.md', ['# 게임 시스템 상세: 자원·윤리·특성·연구·외교·분쟁·거대 프로젝트·칙령·방랑자·역할 전환·아키타입 (옛 §5)', '', BANNER.format(sec='§5'), '', '※ 2026-10-18 현행 코드: §5.5 외교·§5.6 분쟁은 틱 단계가 빈 본문(`TickDiplomacy`·`TickConflict`)이고 Phase 4에서 채운다. 칙령(`DT_Edicts`)·조우/청원(`DT_Encounters` 데이터)·유물(`DT_Relics`)·특성(`DT_Traits`)은 아직 데이터가 없다.', ''] + L(G, 978, 1459))
# ---- 20_ARCHITECTURE ----
arch = ['# 아키텍처: 모듈 층·Subsystem·상태·틱·난수·해시·저장·성능 (옛 §6·§9.1·§9.2·§9.4 + 불변 규칙 전문)', '', BANNER.format(sec='§6, §9.1, §9.2, §9.4 및 옛 CLAUDE.md 불변 규칙'), '']
arch += L(G, 1460, 1617) + ['', '---', ''] + L(G, 3473, 3549) + ['', '---', ''] + L(G, 3702, 3730)
arch += ['', '---', '', '# 부록: 아키텍처 불변 규칙 전문 (옛 CLAUDE.md, v0.8 — 2026-10-18 이관, 내용 무변경)', '', '> `CLAUDE.md`의 14항목 요약이 이 전문의 색인이다. 전문의 괄호 `(v0.7 — …)`·`(v0.8 — 단계 N부터)`는 예정 표지다.', ''] + L(C, 13, 48)
sizes['20_ARCHITECTURE.md'] = write('20_ARCHITECTURE.md', arch)
sizes['21_DATA_MODEL_REFERENCE.md'] = write('21_DATA_MODEL_REFERENCE.md', ['# 데이터 모델·C++ 설계안 사본 (옛 §7)', '', '> **코드가 원본이다.** 이 문서는 guideline.md v0.8 §7(공통 타입·런타임 상태·정의 데이터·시뮬레이션 Subsystem·인터페이스 요약)을 2026-10-18에 그대로 옮긴 사본이다. `Source/Cosmic_Ascension/Core`·`Data`·`Sim`·`Events`·`Log`·`Player`·`Save`의 헤더와 다르면 코드가 이긴다. 설계 결정(규칙)의 출처로만 읽고, 필드 목록·시그니처는 헤더를 본다. 예정 표지와 §번호의 뜻은 다른 문서와 같다(`docs/SECTION_MAP.md`).', ''] + sec7)
# ---- 70_CONTENT_GUIDE ----
sizes['70_CONTENT_GUIDE.md'] = write('70_CONTENT_GUIDE.md', ['# 콘텐츠 가이드: 로그 항목·템플릿 문법·서사 규칙·연대기·로컬라이제이션·헌정명 (옛 §8)', '', BANNER.format(sec='§8'), ''] + L(G, 3211, 3472))
# ---- 15_GLOSSARY ----
sizes['15_GLOSSARY.md'] = write('15_GLOSSARY.md', ['# 용어집 (옛 §15)', '', BANNER.format(sec='§15'), '', '※ 같은 낱말의 두 뜻은 이렇게 고정한다: \'칸\' = Tier(거대 프로젝트의 자리는 \'건설/대응 슬롯\'), \'부르심\' = 서막 끝의 소명 사건, \'문턱\' = T3(칸 이름) / `enc_threshold`(T5 카드), \'정착률\'은 \'주장 만료율\'로 바꿔 부른다.', ''] + L(G, 4271, 4405))
# ---- 40_DEV_GUIDE ----
dev = head('dev_guide_head.md') + ['', '## 옛 §10 프로젝트 구조 및 코딩 규약', ''] + sec10
dev += ['', '## 옛 §14 Claude Code 협업 규칙 (§14.3 CLAUDE.md 템플릿은 중복이라 뺐다 — 현행 CLAUDE.md가 원본)', ''] + L(G, 4154, 4176) + [''] + L(G, 4255, 4270)
dev += ['', '## 옛 §9.3 테스트와 1인 밸런싱', ''] + L(G, 3550, 3701)
dev += ['', '## 빌드 · 실행 · 테스트 — 명령 전문 (옛 CLAUDE.md, Windows·UE 5.8.3 소스로 검증한 명령)', ''] + L(C, 61, 148)
sizes['40_DEV_GUIDE.md'] = write('40_DEV_GUIDE.md', dev)
# ---- 45_ENGINE_NOTES ----
sizes['45_ENGINE_NOTES.md'] = write('45_ENGINE_NOTES.md', ['# 엔진 노트: UE 5.8.3 소스로 검증한 헤더 경로·UBT 사실 (옛 CLAUDE.md)', '', '> 2026-10-18 CLAUDE.md에서 옮김(내용 무변경). 낯선 엔진 API는 로컬 엔진 소스 `C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Source`에서 선언 헤더를 확인한 뒤 여기에 한 줄을 더한다.', '', '## .uproject의 EngineAssociation', '', next(ln for ln in C if 'EngineAssociation' in ln), ''] + L(C, 159, 237))
# ---- 50_ROADMAP ----
road = head('roadmap_head.md') + ['']
road += L(W, 96, 114) + [''] + L(W, 162, 181) + [''] + L(W, 189, 200)
road += ['', '---', '', '# 옛 guideline §11 로드맵 · §12 범위 관리 · §13 리스크 (2026-10-18 이관 — §11의 기간 열과 상태 줄은 역사 기록이며 상태 줄은 docs/CHANGELOG.md로 옮겼다)', '', BANNER.format(sec='§11~§13'), '']
status_lines = {3876, 3887, 3901, 3922, 3935}
sec11 = [ln for i, ln in enumerate(G, start=1) if 3851 <= i <= 4153 and i not in status_lines]
# collapse blanks of sec11
tmp = []; blank = 0
for ln in sec11:
    if ln.strip() == '':
        blank += 1
        if blank > 1: continue
    else: blank = 0
    tmp.append(ln)
road += tmp
sizes['50_ROADMAP.md'] = write('50_ROADMAP.md', road)
# ---- 60_DECISIONS ----
sizes['60_DECISIONS/OPEN_QUESTIONS.md'] = write('60_DECISIONS/OPEN_QUESTIONS.md', ['# 열린 물음 (옛 §16 미결 사항 — 번호는 그대로)', '', '> 2026-10-18 이관(내용 무변경). 결정되면 해당 설계 문서를 고치고 이 표에서 지운 뒤 `DECISIONS_LOG.md`에 날짜·근거를 적는다. MVP를 막는 물음은 `STATUS.md` Top 5에 올린다.', ''] + L(G, 4406, 4489))
dec = ['# 확정 기록 (DECISIONS LOG)', '', '> 2026-10-18 이관(내용 무변경). 위가 최신. 새 결정은 "확정 완료 (날짜, 작업 — 근거):" 꼴로 맨 위에 더한다.', '', '## ADR-0001 렌더 / 플러그인 결정 (옛 CLAUDE.md, 2026-10-10)', ''] + L(C, 240, 244) + ['', '## 옛 guideline §16의 확정 기록 블록', ''] + L(G, 4490, 4598)
sizes['60_DECISIONS/DECISIONS_LOG.md'] = write('60_DECISIONS/DECISIONS_LOG.md', dec)
# ---- 90_LORE ----
sizes['90_LORE/theology_and_bestiary_notes.md'] = write('90_LORE/theology_and_bestiary_notes.md', ['# 대도감 통합·신학의 조율 (옛 v08_rulings.md B·C장)', '', '> 2026-10-18 이관(내용 무변경). 원문 전체는 `docs/archive/v08_rulings.md`(행 번호는 편집 전 guideline 기준이라 무효). 대도감 원문은 같은 폴더의 `cosmic_species_codex.md`(고치지 않는다).', ''] + L(RU, 355, 612))
# ---- CHANGELOG ----
def phase_of(n):
    hdr = None
    for i in range(n, 3850, -1):
        if G[i-1].startswith('### '): hdr = G[i-1][4:]; break
    return hdr or '?'
chg = ['# CHANGELOG — 변경 이력·Phase 상태 기록·작업 일지', '', '> 2026-10-18 재편: 본문에 섞여 있던 이력·상태 줄·일지·리뷰 통계를 여기로 옮겼다(내용 무변경, 위치만). 앞으로는 날짜·작업 접두어·바뀐 규칙·리뷰 통계를 여기에만 적고, 설계 본문과 STATUS.md에는 적지 않는다.', '']
chg += ['## 문서 상태 표 (옛 guideline.md 머리)', ''] + L(G, 11, 16)
chg += ['', '## guideline.md 변경 요약 v0.4 → v0.8 (옛 머리 20~99행)', ''] + L(G, 20, 99)
chg += ['', '## 옛 §0 초안(v0.1) 검토 결과 및 변경 요약', ''] + L(G, 128, 159)
chg += ['', '## Phase 상태 기록 (옛 guideline §11의 상태 줄)', '']
for n in sorted(status_lines):
    chg += ['### ' + phase_of(n) + ' — guideline.md:' + str(n), '', G[n-1], '']
chg += ['## 옛 CLAUDE.md "현재 단계" (2026-10-18 이관)', ''] + L(C, 247, 279)
chg += ['', '## 옛 WORKPLAN.md §3 작업 기록 (Phase 1·v0.7 예약·Phase 2·Phase 3)', ''] + L(W, 115, 161) + [''] + L(W, 182, 188)
chg += ['', '## 옛 WORKPLAN.md 부록 A. Phase 2 작업 기록', ''] + L(W, 202, 216)
sizes['CHANGELOG.md'] = write('CHANGELOG.md', chg)

# ---- SECTION_MAP ----
dest = [  # (start, end, file)
    (1, 19, 'CHANGELOG.md (문서 상태 표)'), (20, 99, 'CHANGELOG.md'), (100, 127, '(목차 — 삭제)'), (128, 159, 'CHANGELOG.md'),
    (160, 347, '00_VISION.md'), (348, 599, '10_DESIGN/pillars.md'), (600, 977, '10_DESIGN/crisis_and_transcendents.md'),
    (978, 1459, '10_DESIGN/systems.md'), (1460, 1617, '20_ARCHITECTURE.md'), (1618, 3210, '21_DATA_MODEL_REFERENCE.md'),
    (3211, 3472, '70_CONTENT_GUIDE.md'), (3473, 3549, '20_ARCHITECTURE.md'), (3550, 3701, '40_DEV_GUIDE.md (옛 §9.3)'),
    (3702, 3730, '20_ARCHITECTURE.md'), (3731, 3850, '40_DEV_GUIDE.md'), (3851, 4025, '50_ROADMAP.md (상태 줄은 CHANGELOG.md)'),
    (4026, 4113, '50_ROADMAP.md'), (4114, 4153, '50_ROADMAP.md'), (4154, 4176, '40_DEV_GUIDE.md'),
    (4177, 4254, '(§14.3 CLAUDE.md 템플릿 — 중복이라 삭제; 불변 규칙 전문은 20_ARCHITECTURE.md 부록)'),
    (4255, 4270, '40_DEV_GUIDE.md'), (4271, 4405, '15_GLOSSARY.md'), (4406, 4489, '60_DECISIONS/OPEN_QUESTIONS.md'),
    (4490, 4598, '60_DECISIONS/DECISIONS_LOG.md'),
]
def where(n):
    for a, b, f in dest:
        if a <= n <= b: return f
    return '?'
smap = ['# SECTION MAP — 옛 guideline.md v0.8 §번호 → 새 문서', '', '> 코드 주석의 `guideline.md section N`·문서의 `§N`은 이 표로 푼다. 원문(동결본)은 `docs/archive/guideline_v0.8_2026-10-18.md`이고 행 번호는 그 파일 기준이다. 절 번호는 재사용하지 않는다(새 절은 새 문서에 쓴다).', '', '| § | 제목 | 원문 행 | 새 위치 |', '|---|---|---|---|']
hdr_re = re.compile(r'^(##|###) (\d+(?:\.\d+)?)\.?\s+(.*)$')
for i, ln in enumerate(G, start=1):
    m = hdr_re.match(ln)
    if m:
        num, title = m.group(2), m.group(3)
        smap.append('| §' + num + ' | ' + title + ' | ' + str(i) + ' | ' + where(i) + ' |')
smap += ['', '## 옛 CLAUDE.md 절 → 새 위치', '', '| 절 | 새 위치 |', '|---|---|',
         '| 프로젝트 | CLAUDE.md(요약), docs/45_ENGINE_NOTES.md(EngineAssociation) |',
         '| 아키텍처 불변 규칙 | CLAUDE.md(14항목 요약), docs/20_ARCHITECTURE.md 부록(전문) |',
         '| 작업 방식 | CLAUDE.md |', '| 빌드 / 실행 / 테스트 | CLAUDE.md(요약), docs/40_DEV_GUIDE.md(전문) |',
         '| 모듈 / UHT / 컴파일 규칙 | CLAUDE.md |', '| 검증된 엔진 헤더 경로 | docs/45_ENGINE_NOTES.md |',
         '| 렌더 / 플러그인 결정 | docs/60_DECISIONS/DECISIONS_LOG.md ADR-0001 |', '| 현재 단계 | STATUS.md(현재), docs/CHANGELOG.md(기록) |',
         '', '## 옛 WORKPLAN.md·루트 md → 새 위치', '', '| 원본 | 새 위치 |', '|---|---|',
         '| WORKPLAN §0~§2 | WORKPLAN.md(유지) |', '| WORKPLAN §3 작업 표(v0.8 단계·Phase 3·v0.8 예약·다음) | docs/50_ROADMAP.md |',
         '| WORKPLAN §3 작업 기록·부록 A | docs/CHANGELOG.md |', '| v08_rulings.md A·D·E | docs/archive/v08_rulings.md (결정은 DECISIONS_LOG의 v0.8 확정 블록에 이미 흡수됨) |',
         '| v08_rulings.md B·C | docs/90_LORE/theology_and_bestiary_notes.md |', '| v08_agent_directive.md | docs/archive/ (결정 15·13·손 확인은 DECISIONS_LOG v0.8 확정 블록) |',
         '| code_artifact.md | docs/archive/ (대도감 옛 판 — 원천 아님) |', '| cosmic_species_codex.md | docs/90_LORE/cosmic_species_codex.md |']
sizes['SECTION_MAP.md'] = write('SECTION_MAP.md', smap)

# ---- loss check: every original line (outside dropped ranges) appears in some new doc ----
def norm(s): return s.strip()
new_lines = set()
for root, _, files in os.walk(D):
    for fn in files:
        if fn.endswith('.md') and 'archive' not in root:
            for ln in read_lines(os.path.join(root, fn)): new_lines.add(norm(ln))
# CLAUDE/STATUS new files are not in docs; originals of CLAUDE 11-49 are in ARCHITECTURE, 61-148 in DEV_GUIDE, 159-237 ENGINE, 240-244 DECISIONS, 247-279 CHANGELOG
checks = []
drop_g = set(range(100, 128)) | set(range(4177, 4255)) | {3135} | set(range(1, 11)) | {17, 18, 19}
for i, ln in enumerate(G, start=1):
    if i in drop_g or not ln.strip(): continue
    checks.append(('guideline.md', i, ln))
for i in list(range(13, 49)) + list(range(61, 149)) + list(range(159, 238)) + list(range(240, 245)) + list(range(247, 280)):
    if C[i-1].strip(): checks.append(('CLAUDE.md', i, C[i-1]))
for i in list(range(96, 115)) + list(range(115, 162)) + list(range(162, 182)) + list(range(182, 189)) + list(range(189, 201)) + list(range(202, 217)):
    if W[i-1].strip(): checks.append(('WORKPLAN.md', i, W[i-1]))
for i in range(355, 613):
    if RU[i-1].strip(): checks.append(('v08_rulings.md', i, RU[i-1]))
missing = [(f, i, ln[:80]) for f, i, ln in checks if norm(ln) not in new_lines]
print('loss check: original lines checked', len(checks), '| missing', len(missing))
for m in missing[:20]: print('  MISSING', m)
print('sizes (bytes):'); tot = 0
for k, v in sorted(sizes.items()): print(f'  {v:>8}  {k}'); tot += v
print('  total docs (excluding archive):', tot)
