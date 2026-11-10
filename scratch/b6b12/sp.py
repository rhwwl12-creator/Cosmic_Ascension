import re,sys
sys.stdout.reconfigure(encoding='utf-8')
f,ln=sys.argv[1],int(sys.argv[2])
L=open(f,encoding='utf-8').read().split('\n')
s=L[ln-1]
parts=re.split(r'(?=[①-⑳㉑-㉟㊱-㊿])',s)
for p in parts: print('-',p[:int(sys.argv[3]) if len(sys.argv)>3 else 700])
