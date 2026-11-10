"""Dry run in a scratch copy (never the project): append the three [B-6b6] rows with N 1.5's texts and run check_data there.
usage: rows_dryrun.py <scratch copy root>"""
import json, os, subprocess, sys
root = sys.argv[1]
D = os.path.join(root, 'Tools', 'data')
C1 = {
    "Name": "crisis_species_extinction", "HazardTag": "Hazard.Extinction", "SourceDoctrine": "", "DisplayName": "절멸 위기",
    "CountdownTicks": 250, "SpawnRule": "Draw", "SpawnChancePerTick": 0.0001,
    "SpawnConditions": [{"Type": "CivTierAtLeast", "Tag": "", "Key": "None", "Value": 3.0}],
    "OnExpireEffects": [], "BranchChance": 0.0, "OnExpireBranchEffects": [], "OnExpireNeighbourEffects": [], "OnResolveEffects": [],
    "RulerResolutions": ["project_ark"], "WandererResolutions": [], "LogCategory": "Log.Crisis", "Severity": 1,
    "OutbreakTemplates": [
        "{System}의 대기가 {Civ}의 사람들에게 맞지 않게 바뀌어 갔다. {Civ}의 학자들은 그 변화를 늦출 방법을 찾기 시작했다.",
        "{System}의 흙에서 {Civ}의 사람들이 먹을 것이 잘 자라지 않게 되었다. {Civ}의 농부들은 밭마다 흙을 떠서 학자들에게 보냈다.",
        "{System}에서 {Civ}의 사람들의 몸이 그 땅의 변화를 따라가지 못했다. {Civ}의 의원들이 그 차이를 재기 시작했다."],
    "ExpireTemplates": [
        "{Civ}|hpp(이,가) 사라졌다. {System}의 산과 강을 그들의 말로 부르던 이름들은 잊혀 갔다.",
        "{Civ}|hpp(은,는) {Hazard} 속에 자취를 감추었다. 그들이 즐겨 먹던 음식은 이제 어느 밥상에도 오르지 않는다.",
        "{Civ}의 사람들은 {System}에서 다음 세대를 잇지 못했다. 그들의 달력은 그해의 장에서 멈추었다."],
    "ResolveTemplates": [
        "{System}의 위기가 그쳤다. 장이 서는 날이면 {Civ}의 장터가 붐볐다.",
        "{System}의 위기는 더 나아가지 않았다. {Civ}의 아이들은 그해에도 자랐다.",
        "{Civ}의 사람들은 {System}에 그대로 살았다. 그들은 그 위기의 일을 이웃들과 이야기했다."],
    "OutbreakRow": "None"}
C2 = {"Name": "project_ark", "DisplayName": "방주", "RequiredTier": "Tier3_Causal",
      "CostPerTick": {"Energy": 3.0, "Matter": 3.0, "Insight": 0.0, "Influence": 0.0, "Causality": 0.0}, "RequiredProgress": 150.0,
      "CompleteTemplates": ["{Civ}|hpp(이,가) {System}에서 {Project}|hpp(을,를) 완성했다.",
                            "{System}의 {Project}|hpp(이,가) 다 지어졌다. {Civ}의 기술자들이 마지막 점검표를 덮었다.",
                            "{System}에서 {Civ}의 {Project}|hpp(을,를) 쓸 수 있게 되었다."]}
C3 = {"Name": "feat_passage_by_ark", "Title": "방주와 조약", "MythicNarrative": "이 백성과 마주 앉는 이들은 말을 더 신중히 고른다.",
      "RequiredTier": "Tier3_Causal", "TargetTier": "Tier4_Transcendent", "Path": "Ruler",
      "CompletionConditions": [{"Type": "MegaprojectComplete", "Tag": "", "Key": "project_ark", "Value": 0.0},
                               {"Type": "CivHasTreaty", "Tag": "", "Key": "None", "Value": 1.0}],
      "AscensionCost": [{"Type": "ShiftEthics", "Tag": "", "Key": "Openness", "Value": -0.1, "DurationTicks": 0}],
      "GrantedPowers": "(GameplayTags=)", "DedicationBank": "", "FX": "", "Sound": "",
      "LogTemplates": ["{Civ}|hpp(은,는) 방주를 지어 위기를 건넜다. 다른 백성들의 사절이 전보다 자주 {HomeSystem}|hpp(을,를) 찾았다.",
                       "{Civ}|hpp(이,가) 넷째 칸에 서 있게 되었다. {HomeSystem}에 닿는 다른 백성들의 배가 먼저 인사를 보내왔다.",
                       "{Civ}|hpp(은,는) 넷째 칸에 서 있게 되었다. 다른 백성들은 저마다의 지도에서 {Civ}의 자리를 고쳐 그렸다."]}
for f, row in (('DT_Hazards.json', C1), ('DT_Megaprojects.json', C2), ('DT_Feats.json', C3)):
    p = os.path.join(D, f)
    data = json.load(open(p, encoding='utf-8'))
    first = data[0]
    missing = set(first) - set(row)
    extra = set(row) - set(first)
    print(f, 'columns missing vs first row:', sorted(missing), 'extra:', sorted(extra))
    data.append(row)
    json.dump(data, open(p, 'w', encoding='utf-8', newline='\n'), ensure_ascii=False, indent='\t')
for name in ('DT_Hazards.json', 'DT_Megaprojects.json', 'DT_Feats.json'):
    txt = open(os.path.join(D, name), encoding='utf-8').read()
r = subprocess.run([sys.executable, '-I', os.path.join(root, 'Tools', 'check_data.py')], cwd=root, capture_output=True, text=True, encoding='utf-8', errors='replace')
print('check_data exit', r.returncode)
print(r.stdout[-3000:], r.stderr[-2000:])
