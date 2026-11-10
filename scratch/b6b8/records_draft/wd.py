import subprocess, sys, re
root = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
ctx = int(sys.argv[1])
for f in sys.argv[2:]:
    out = subprocess.run(["git","diff","-U0","--word-diff=plain","--",f],cwd=root,capture_output=True).stdout.decode("utf-8","replace")
    print("=====", f)
    for line in out.splitlines():
        if line.startswith("@@"):
            print(line); continue
        if not ("[-" in line or "{+" in line): continue
        for m in re.finditer(r"\[-.*?-\]|\{\+.*?\+\}", line, flags=re.S):
            s=max(0,m.start()-ctx); e=min(len(line),m.end()+ctx)
            print("   ...", line[s:e].replace("\n"," "), "...")
