import os
import shutil
import sys

"""
rename_tpslib.py
This program copies a TPS lib to new file with its hex id name.
"""

def main(argv=None):
    if argv is None:
        argv = sys.argv
    progname = argv[0]
    if progname is None:
        progname = 'rename_tpslib.py'

    if len(argv) < 2:
        print 'Usage: %s TPSLib' % progname
        return 2

    tpslib = argv[1]

    try:
        id = open(tpslib, 'rb').read(20).encode('hex')
    except:
        print("Exception when trying to open or read file: %s"%tpslib)
        print sys.exc_info()
        id = "invalid_tps.tsl"
    output_dir = os.path.join(os.path.curdir, 'tpslib')
    if not os.path.exists(output_dir):
        os.makedirs(output_dir, 0777)
    newfile = os.path.join(output_dir, id)

    print 'tpslib: %s' % id

    shutil.copyfile(tpslib, newfile)

    if len(argv) == 3:
        open(os.path.join(os.path.dirname(tpslib), argv[2]), 'w').write(id)

    return 0

if __name__ == '__main__':
    main()
