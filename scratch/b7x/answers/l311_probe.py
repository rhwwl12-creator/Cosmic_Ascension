import os,re,sys
old=re.compile(r"kessler|planck|tapestry|abyssal ?maw|star ?shepherd|hermit ?monolith|dyson ?brain|unseen ?law|silence ?feed|quiet ?feed|quiet tick", re.I)
new=re.compile(r"kessler|planck|tapestry|abyssal[ _-]?maw|star[ _-]?shepherd|hermit[ _-]?monolith|dyson[ _-]?brain|unseen[ _-]?law|silence[ _-]?feed|quiet[ _-]?feed|quiet[ _-]tick", re.I)
for s in ["unseen_law","b7x2_unseen_law_x.patch","star-shepherd","quiet_tick","Brick.Form.UnseenLaw","folded_small"]:
    print(s, bool(old.search(s)), bool(new.search(s)))
for root in sys.argv[1:]:
    n=0
    for top in ("Source","Config","Tools"):
        for dp,dn,fn in os.walk(os.path.join(root,top)):
            dn[:]=[d for d in dn if d not in(".git","Intermediate","Binaries","Saved","__pycache__")]
            for f in fn:
                p=os.path.join(dp,f); rel=os.path.relpath(p,root).replace("\\","/")
                if new.search(rel): print("PATH",rel); n+=1
                try: t=open(p,encoding="utf-8").read()
                except Exception: continue
                a=set(m.group(0) for m in old.finditer(t)); b=set(m.group(0) for m in new.finditer(t))
                if b-a: print("NEWHIT",rel,b-a); n+=1
    print(root,"extra hits",n)
