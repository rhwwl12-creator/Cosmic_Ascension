import sys
sys.stdout.reconfigure(encoding='utf-8')
p='docs/60_DECISIONS/DECISIONS_LOG.md'
s=open(p,'rb').read().decode('utf-8')
lines=s.split('\n')
idx=[i for i,l in enumerate(lines) if l.startswith('- `[B-6b2b]`(2026-11-09')]
assert len(idx)==1
new=("- `[B-6b2c]`(2026-11-09 — 결정 12~18·23의 장치, 헤더 §2.5·§2.6·§2.10·§3.1·§3.2·§3.4·§4의 건설 단추, §3.3 규칙 7과 규칙 8의 `log_fallen_bound` 몫): "
 "`[B-6b2]`을 나눈 셋째이자 마지막이다 — 이것으로 헤더 §8 `[B-6b2]` 행의 장치가 모두 코드에 있다. **결박**: 새 단일 창구 `CosmicSeal`(`BindTargetOf` — `Transcendents` 차례로 처음인, 봉인되지 않은 타락; `Bind` — `bSealed`·`SealedTick`을 쓰는 하나뿐인 곳이고 한 번 묶인 봉인은 옮기지 않는다), "
 "저장되지 않는 시작 거절 `NoFallen`(`RowTier` 뒤·칸 앞 — 패널의 키 표 둘과 ST_CA 끝의 두 키), 건설 행 `project_binding`(넷째 칸, 틱마다 Energy 4·Matter 4 — 지금 가장 비싼 건설의 두 배로, 성계 넷인 넷째 칸 백성의 수입(배수 3, 약 12)에서 둘째 칸 백성에게 안정화가 차지하는 몫과 같은 셋째 몫쯤; 진척 1,800은 노트 §2 ⓒ의 시작값; 인과율 0; 화면 이름 '공명 기관'과 완성 줄은 `[B-6b5]`의 감사가 정하는 초안 — 값의 마지막은 `[B-6b7]`). "
 "`Advance`의 건설 칸이 값을 치르기 전마다 대상을 묻고(같은 패스의 앞선 결박이 방금 묶었어도 — 대상을 패스나 틱에 한 번만 읽지 않는다) 없으면 결과 없이 끝나며, 다 차면 기록을 적은 뒤 `Bind`; 완성 줄 바로 뒤에 묶임의 줄 `log_fallen_bound`(`Log.Civ`, 완성 줄의 초점과 등급, `{System}`만 — 행위자 없는 수동 '…그늘이 묶였다', 말하기 한 수; 범주는 헤더가 정하지 않아 완성 줄과 같은 것으로 했다). "
 "**제동**: `CosmicAscent::FourthRungOpenOf`(그 은하의 넷째 칸 이상 백성 — 다스리는 백성 포함 — 와 닿지 않은 건넘의 합 < 그 은하 행의 `AIFourthRungCap`; 행이 없거나 0 이하면 제동 없음)를 `JudgePeople`이 다스리지 않는 셋째 칸 백성의 행을 보기 전에 묻는다. "
 "**지연**: 노브 `AIUpperRungProgressMultiplier` 1(건설 칸·다스리지 않는 백성·셋째 칸 이상 행만, 대응 칸에는 없음; 0 이하·수 아님은 에러 한 줄과 1). **지킴 상한**: 노브 `FleetDelayCountdownShare` 0과 `CosmicConflict::GuardDelayCapOf`(min(`FleetDelayCap`, 몫 × 행의 카운트다운)) — `FleetDelayOf`가 둘의 작은 쪽을 내고 신정론 불변식이 행마다 읽는다(규칙을 베끼지 않는다). "
 "건설 단추는 그 칸이 다음 칸일 때부터 선다(표현만 — `UI.TopBarAndPanel`의 단추 목록도 같은 거름을 옮겼다). 스키마: 규칙 7(프로젝트의 인과율 비용 0 — 검사기가 자원 풀의 한 값 `Pool.Member`를 짚게 했다), 규칙 8의 `log_fallen_bound` 몫(`{System}`만), 지연 노브의 0 초과, 몫의 0~1. "
 "`Data.Integrity`: 방주의 여유(다스리는 이를 죽이는 위기의 답 프로젝트가 AI 배율로 카운트다운 안에 끝난다 — 테스트의 절멸 250·방주 150 사본을 게임의 규칙으로 보아 지금 문다)와 결박이 어느 위기의 답도 아님(헤더 밖에서 더한 것 — 대응 칸의 완성은 봉인하지 않는다). "
 "게임 데이터에 결박을 부르는 업적 행도 넷째 칸 백성도 없고 지연 1·몫 0이라 기준값 다섯 줄과 해시 판 둘이 비트 그대로다.")
lines.insert(idx[0]+1, new)
open(p,'wb').write('\n'.join(lines).encode('utf-8'))
print(len(new.encode('utf-8')))
