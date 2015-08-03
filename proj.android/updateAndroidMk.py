#!/usr/bin/python

import os
import os.path
import argparse
import glob

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-o', '--output', default='Android.mk')
    ap.add_argument('-e', '--ext', nargs='+', default=['.cpp', '.cc'])
    ap.add_argument('-F', '--exclude', nargs='*', default=[])
    ap.add_argument('-t', '--tpl', required=True)
    ap.add_argument('-b', '--basedir', default='./')
    ap.add_argument('-n', '--dryrun', action='store_true')
    ap.add_argument('src_dirs', nargs='+')
    args = ap.parse_args()

    src_files = []
    src_dirs = []
    for d in args.src_dirs:
        src_dirs.append(d)
        for r, ds, fs in os.walk(d):
            for f in fs:
                if os.path.splitext(f.lower())[1] in args.ext and not has_match_fn(args.exclude, os.path.join(r, f)):
                    src_files.append(os.path.join(r, f))
            for sd in ds:
                src_dirs.append(os.path.join(r, sd))

    src_files = [os.path.relpath(p, args.basedir)  for p in src_files]
    src_files.sort()

    src_dirs = ['$(LOCAL_PATH)/' + os.path.relpath(p, args.basedir) for p in src_dirs]
    src_dirs.sort();

    with open(args.tpl) as p:
        tpl = p.read()

    mk = tpl.format(src_files=' \\\n'.join(src_files), src_dirs=' '.join(src_dirs))

    if args.dryrun:
        print mk
    else:
        with open(args.output, 'w') as p:
            p.write(mk)

def has_match_fn(wilds, fn):
    for w in wilds:
        if glob.fnmatch.fnmatch(fn, w):
            return True
    return False

if __name__ == '__main__':
    main()
