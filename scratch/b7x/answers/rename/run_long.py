"""Runs a tree's Python check so that file calls reach paths past 260 characters (the scratch trees are deep in a temp
folder and this PC has no LongPathsEnabled). The script sees ordinary paths; only the file calls get the \\\\?\\ prefix.
Usage: python -I -X utf8 run_long.py <script> [args...]"""
import builtins
import io
import os
import runpy
import sys

LONG = "\\\\?\\"


def fix(p):
    if isinstance(p, str) and os.name == "nt" and not p.startswith(LONG) and os.path.isabs(p):
        return LONG + os.path.normpath(p)
    return p


_open, _exists, _isfile, _isdir, _getsize, _stat, _listdir, _walk = (builtins.open, os.path.exists, os.path.isfile,
                                                                     os.path.isdir, os.path.getsize, os.stat, os.listdir, os.walk)
builtins.open = io.open = lambda f, *a, **k: _open(fix(os.path.abspath(f)) if isinstance(f, str) else f, *a, **k)
os.path.exists = lambda p: _exists(fix(os.path.abspath(p)))
os.path.isfile = lambda p: _isfile(fix(os.path.abspath(p)))
os.path.isdir = lambda p: _isdir(fix(os.path.abspath(p)))
os.path.getsize = lambda p: _getsize(fix(os.path.abspath(p)))
os.stat = lambda p, *a, **k: _stat(fix(os.path.abspath(p)) if isinstance(p, str) else p, *a, **k)
os.listdir = lambda p=".": _listdir(fix(os.path.abspath(p)))


def walk(top, *a, **k):
    top_abs = os.path.normpath(os.path.abspath(top))
    for dirpath, dirnames, files in _walk(fix(top_abs), *a, **k):
        yield (dirpath[len(LONG):] if dirpath.startswith(LONG) else dirpath), dirnames, files


os.walk = walk
script = os.path.abspath(sys.argv[1])
sys.argv = [script] + sys.argv[2:]
runpy.run_path(script, run_name="__main__")
