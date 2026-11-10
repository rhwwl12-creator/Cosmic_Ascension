import sys
P = sys.argv[1]
t = open(P, 'rb').read().decode('utf-8')
R = [
 ("- **이 헤더의 낱말.** 설계서의 '존재'는 여기서 늘 **백성**",
  "- **이 헤더의 낱말.** 원안의 '존재'는 여기서 늘 **백성**"),
 ("여섯 갈래로 센 것(설계서의 시간 벡터)",
  "여섯 갈래로 센 것(원안 동결본 §4.1의 시간 벡터)"),
 ("설계서가 이름한 `Tools/audit_content.py`와 `FPlayerCalling`은 없다",
  "원안(동결본 §5의 프롬프트)이 이름한 `Tools/audit_content.py`와 `FPlayerCalling`은 없다"),
 ("### 1.3 설계서와 부딪는 곳 (표는 노트 §1·§2)",
  "### 1.3 설계서 원안과 부딪는 곳 (표는 노트 §1·§2; 원안은 동결본 §2 축 1~5의 예순 블록)"),
 ("*왜 개발자인가*: 설계서의 머릿기능들이다.",
  "*왜 개발자인가*: 원안의 머릿기능들이다."),
 ("- **비전과의 거리.** 설계서가 그린 '별을 끌고 다니는 존재'는",
  "- **비전과의 거리.** 원안(동결본 §2 축 3의 `SystemDragger`)이 그린 '별을 끌고 다니는 존재'는"),
]
for o, n in R:
    assert t.count(o) == 1, o[:60]
    t = t.replace(o, n)
b = t.encode('utf-8'); assert b'\r' not in b
open(P, 'wb').write(b); print(len(b))
