import hashlib, os, sys
A = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension\Saved\Balance\b6b_bundleC"
F = r"C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad\b6b11\bundleC"
def sha(p):
    h = hashlib.sha1()
    with open(p, 'rb') as f:
        for b in iter(lambda: f.read(1 << 20), b''): h.update(b)
    return h.hexdigest()
man = {}
for line in open(os.path.join(A, 'manifest.sha1'), encoding='utf-8'):
    line = line.rstrip('\n')
    if not line: continue
    h, name = line.split('  ', 1)
    man[name] = h
print('manifest entries', len(man))
arch = [n for n in os.listdir(A) if n != 'manifest.sha1']
print('archive files (excl manifest)', len(arch), 'logs', sum(n.endswith('.log') for n in arch))
bad_a = [n for n in arch if n not in man or sha(os.path.join(A, n)) != man[n]]
print('archive mismatches', len(bad_a), bad_a[:5])
folder = os.listdir(F)
print('folder files', len(folder))
missing = [n for n in man if n not in folder]
extra = [n for n in folder if n not in man]
bad_f = [n for n in man if n in folder and sha(os.path.join(F, n)) != man[n]]
print('manifest not in folder', len(missing), missing[:5], 'folder not in manifest', len(extra), extra[:5], 'folder mismatches', len(bad_f), bad_f[:5])
