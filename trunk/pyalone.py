#!/usr/bin/env python

# Note:
# A lot of this code is taken from py2exe project
# (http://starship.python.net/crew/theller/py2exe/)

from os.path import join, split, isfile
from modulefinder import ModuleFinder, ReplacePackage
from sys import platform, version_info, prefix
from imp import get_suffixes, C_EXTENSION
from os import environ

ReplacePackage("_xmlplus", "xml")

# imports done from builtin modules in C code (untrackable by py2exe)
HIDDEN_IMPORTS =  {
    "time": ["_strptime"],
##  "datetime": ["time"],
    "cPickle": ["copy_reg"],
    "parser": ["copy_reg"],
    "codecs": ["encodings"],
    "cStringIO": ["copy_reg"],
    "_sre": ["copy", "string", "sre"],
}

def basic_module_find(files):
    '''Do a basic module find on all files'''
    mf = ModuleFinder()
    for fname in files:
        mf.run_script(fname)

    return mf

def add_hidden(mf):
    '''Add hidden dependecies'''
    for name in HIDDEN_IMPORTS:
        if name in mf.modules:
            for mod in HIDDEN_IMPORTS[name]:
                mf.import_hook(mod)

def tk_dirs(mf):
    '''Tk directories'''
    if "Tkinter" not in mf.modules:
        return []

    import Tkinter
    import _tkinter

    tk = _tkinter.create()
    tcl_dir = tk.call("info", "library")
    tcl_src_dir = split(tcl_dir)[0]

    dirs = [ join(tcl_src_dir, "tcl%s" % _tkinter.TCL_VERSION),
             join(tcl_src_dir, "tk%s" % _tkinter.TK_VERSION)]

    del tk, _tkinter, Tkinter

    return dirs


def winfiles():
    '''Windows extra files'''
    files = [join(prefix, "msvcp71.dll"), join(prefix, "msvcr71.dll")]
    dllbase = "python%d%d.dll" % version_info[:2]
    if isfile(join(prefix, dllbase)):
        files.append(join(prefix, dllbase))
    else:
        files.append(join(environ["WINDIR"], "system32", dllbase))

    return files

# FIXME: Do we want to add cygwin1.dll and friends?
def cygfiles():
    '''cygwin extra files'''
    dllbase = "libpython%d.%d.dll" % version_info[:2]
    return [join(prefix, "bin", dllbase)]

EXTRA = {
    "win32" : winfiles,
    "cygwin" : cygfiles
}

def extra_files():
    '''Platform specific extra files'''
    if platform in EXTRA:
        return EXTRA[platform]()

    print "warning: no extra files for platform %s" % platform
    return []

if __name__ == "__main__":
    from optparse import OptionParser
    from os.path import isdir, splitext
    from sys import path, executable
    from os import makedirs
    from shutil import copy


    p = OptionParser("usage: %prog [options] CONFIG_FILE")

    opts, args = p.parse_args()
    if len(args) != 1:
        p.error("wrong number of arguments") # Will exit

    outdir = "dist"
    run = join(path[0], "run.exe")
    if not isdir(outdir):
        try:
            makedirs(outdir)
        except OSError:
            raise SystemExit("error: can't create %s" % outdir)

    mf = basic_module_find(["hw.py"])
    add_hidden(mf)

    for m in mf.modules.values():
        fname = m.__file__
        if not fname:
            continue

        if not isfile(fname):
            print "warning: can't find %s" % fname
            continue

    
        print fname
        copy(fname, outdir)

    # FIXME: chmod +x on *nix
    copy(run, join(outdir, splitext("hw.py")[0]) + ".exe")
    if platform == "cygwin":
        exe = executable + ".exe"
    else:
        exe = executable
    copy(exe, outdir)
    for fname in extra_files():
        if not isfile(fname):
            print "warning: can't find %s" % fname
        else:
            print fname
            copy(fname, outdir)
