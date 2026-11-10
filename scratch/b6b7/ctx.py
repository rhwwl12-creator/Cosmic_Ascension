import sys,re
path, pat = sys.argv[1], sys.argv[2]
w = int(sys.argv[3]) if len(sys.argv)>3 else 300
for n,l in enumerate(open(path,encoding='utf-8'),1):
    for m in re.finditer(pat,l):
        print(f"{path}:{n}@{m.start()}: ...{l[max(0,m.start()-w):m.end()+w]}...\n")
