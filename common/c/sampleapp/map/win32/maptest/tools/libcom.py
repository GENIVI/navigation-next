#
# libcom.py: created 2004/12/02 by Dima Dorfman.
#
# Copyright (C) 2004 Networks In Motion, Inc. All rights reserved.
#
# The information contained herein is confidential and proprietary to
# Networks In Motion, Inc., and is considered a trade secret as
# defined in section 499C of the California Penal Code. Use of this
# information by anyone other than authorized employees of Networks
# In Motion is granted only under a written non-disclosure agreement
# expressly prescribing the scope and manner of such use.
#

"""TPS template library compiler."""

__all__ = ['Error', 'comyt', 'comfile', 'main']
__revision__ = '$Id: libcom.py,v 1.19 2004/12/20 12:12:25 dima Exp $'

import os.path, sha
import optparse
import yaml

class Error(Exception): pass

def comyt(yt):
    def gencaps():
        caps = yt.get('capabilities', {})
        def gen():
            for k in sorted(caps):
                yield str(k)
                yield str(caps[k])
            yield ''
        yield bepack(len(caps), 4) + '\0'.join(gen())
    def gentemplates():
        for t in yt.get('templates', []):
            name, attrs = t.iteritems().next()
            attrs.sort()
            def gentpl():
                yield name
                yield '\0'
                for x in attrs:
                    yield x
                    yield '\0'
            tpl = ''.join(gentpl())
            yield bepack(len(tpl), 4)
            yield tpl
    caps = ''.join(gencaps())
    templates = ''.join(gentemplates())
    body = caps + templates
    id = sha.new(body).digest()
    return id, id + body

def comfile(fn):
    return comyt(yaml.loadFile(fn).next())

def generateTpl(destdir, outputfile, inputfile):
    id, data = comfile(inputfile)
    
    if outputfile:
        fname = os.path.join(destdir, outputfile)

        print "Input: %s" % inputfile
        print "TPSLIB: %s" % id.encode('hex')
        print "Writing output to %s" % fname 
        f = open(fname, 'wbc')
        try:
            f.write(data)
        finally:
            f.close()
    

def main():
    usage = """%prog [options] file ...

The specified files are compiled. For each file, an output file is
created using the hexlified template library ID as the name. Errors
are reported as Python exceptions (sorry).

"""
    parser = optparse.OptionParser(usage)
    parser.add_option('-d', type='string', dest='destdir', default='.',
                      help='destination directory')
    parser.add_option('-o', type='string', dest='outputfile', default=None,
                      help='output file')
    options, args = parser.parse_args()
    for fn in args:
        id, data = comfile(fn)
        if options.outputfile:
            fname = os.path.join(options.destdir, options.outputfile)
        else:
            fname = os.path.join(options.destdir, id.encode('hex'))
        print "Input: %s" % fn
        print "TPSLIB: %s" % id.encode('hex')
        print "Writing output to %s" % fname 
        f = open(fname, 'wbc')
        try:
            f.write(data)
        finally:
            f.close()

# Pasted from tesla.std to avoid the dependency
def bepack(v, n):
    """Pack big-endian integer into n bytes."""
    return ''.join(chr(v >> (n - i - 1) * 8 & 255) for i in xrange(n))

if __name__ == '__main__':
    main()
