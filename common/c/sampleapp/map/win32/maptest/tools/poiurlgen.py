#
# (C) Copyright 2006 by Networks In Motion, Inc.
#
# The information contained herein is confidential, proprietary
# to Networks In Motion, Inc., and considered a trade secret as
# defined in section 499C of the penal code of the State of
# California. Use of this information by anyone other than
# authorized employees of Networks In Motion is granted only
# under a written non-disclosure agreement, expressly
# prescribing the scope and manner of such use.
#
# poiurlgen.py: created 2006/12/18 by Mark Goddard.
#

from optparse import OptionParser
import urllib
import sys

def generate_url(line, commands):
    fields = line.strip().split('|')

    query = {}
    query['version'] = '1.0'
    query['commands'] = commands
    query['immediate'] = 'F'
    query['name'] = fields[6]
    query['street'] = fields[9]
    query['city'] = fields[14]
    query['state'] = fields[15]
    query['postal'] = fields[16]
    query['country'] = fields[17]
    query['catcode'] = fields[1].split(',')[0]
    query['phone'] = fields[10].replace('-', '')
    query['lon'] = fields[18]
    query['lat'] = fields[19]
    return "vznavigator:app2app?" + urllib.urlencode(query).strip()

def main(argv=None):
    usage = "usage: %prog [options]"
    parser = OptionParser(usage)

    parser.add_option('-r', '--recents', action="store", type="int", dest="recents", default=0, help='Number of recents to add')
    parser.add_option('-f', '--favorites', action="store", type="int", dest="favorites", default=0, help='Number of favorites to add')
    parser.add_option('-i', '--input', action="store", dest="input", default=None, help='Input filename')
    parser.add_option('-o', '--output', action="store", dest="output", default=None, help='Output filename')

    (options, _) = parser.parse_args()

    if options.input is None:
        parser.error('Input file (-i) must be specified.')

    if options.output is None:
        parser.error('Output file (-o) must be specified.')

    recents = options.recents
    favorites = options.favorites

    output = open(options.output, "w")

    print "Will generate %d recents and %d favorites." % (recents, favorites)

    nl = False
    for line in open(options.input, "r"):
        if (recents or favorites) and nl:
            print >> output, ""
        if recents > 0:
            recents = recents - 1
            url = generate_url(line, 'r')
            output.write(url)
            nl = True
        elif favorites > 0:
            favorites = favorites - 1
            url = generate_url(line, 'f')
            output.write(url)
            nl = True
        else:
            print "Finished."
            return

if __name__ == '__main__':
    sys.exit(main())
