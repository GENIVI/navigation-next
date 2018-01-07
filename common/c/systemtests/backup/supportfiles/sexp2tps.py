import os
import sys
from tesla import tps
from tesla.tps import lib
from tesla.tps import io

""" sexp2tps.py

    This program takes in an S-expression file and generates a TPS file using
    the specified TPS library.

"""

def main(argv=None):
    if argv is None:
        argv = sys.argv
    progname = argv[0]
    if progname is None:
        progname = 'sexp2tps.py'
        
    if len(argv) < 4:
        print 'Usage: %s SExpFile TPSOutFile TPSLib' % progname
        return 2

    sexpfn = argv[1]
    outfn = argv[2]
    tpslib = argv[3]

    tl = tps.lib.TemplateLibrary(tpslib)
    
    print 'sexp file: ' + sexpfn
    print 'Creating TPS document: %s' % outfn

    if sexpfn == '-':
        f = sys.stdin
    else:
        f = open(sexpfn, 'r')
    try:
        sexp = f.read()
    finally:
        f.close()

    elt = tps.tpselt.fromsexp(sexp)

    tpsfile = open(outfn, 'wb')
    try:
        tpsfile.write(tps.io.pack(tl, elt))
    finally:
        tpsfile.close()

    return 0
    
if __name__ == '__main__':
    main()
