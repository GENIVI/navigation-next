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
import urllib2
import sys
from xml.sax.saxutils import escape

def xmlescape(s):
    return escape(s, {r'"':r'&quot;'})

def generate_request(line, mdn, msgnum):
    fields = line.strip().split('|')

    params = {}
    params['msgnum'] = msgnum
    params['mdn'] = xmlescape(mdn)
    params['name'] = xmlescape(fields[6])
    params['streetnum'] = xmlescape(fields[8])
    params['street'] = xmlescape(fields[9])
    params['city'] = xmlescape(fields[14])
    params['state'] = xmlescape(fields[15])
    params['postal'] = xmlescape(fields[16])
    params['country'] = xmlescape(fields[17])
    params['catcode'] = xmlescape(fields[1].split(',')[0])
    params['phone'] = xmlescape(fields[10].replace('-', ''))
    params['lon'] = xmlescape(fields[18])
    params['lat'] = xmlescape(fields[19])

    request =  '<?xml version="1.0" encoding="UTF-8"?>\n'
    request += '<XNB version="1.0.0" language="en">\n'
    request += '<Request clientID="myclientid" clientPassword="password" requestID="%(msgnum)s">\n'
    request += '<SendPlaceRequest targetAppID="abc123" targetMSID="%(mdn)s" from="NIM QA" saveToInbox="True" launchApp="True">\n'
    request += '<Message>Test Message %(msgnum)s</Message>\n'
    request += '<Signature>Powered by Message Test Tool</Signature>\n'
    request += '<Place>\n'
    request += '<Name>%(name)s</Name>\n'
    request += '<Address countryCode="%(country)s">\n'
    request += '<Street>%(streetnum)s %(street)s</Street>\n'
    request += '<City>%(city)s</City>\n'
    request += '<State>%(state)s</State>\n'
    request += '<PostalCode>%(postal)s</PostalCode>\n'
    request += '</Address>\n'
    request += '<PhoneNumber>%(phone)s</PhoneNumber>\n'
    request += '<Category codetype="NIM" code="%(catcode)s"/>\n'
    request += '<Position>\n'
    request += '<Point>%(lat)s %(lon)s</Point>\n'
    request += '</Position>\n'
    request += '</Place>\n'
    request += '</SendPlaceRequest>\n'
    request += '</Request>\n'
    request += '</XNB>\n'

    return request % params

def main(argv=None):
    usage = "usage: %prog [options]"
    parser = OptionParser(usage)

    parser.add_option('-m', '--mdn', action="store", dest="mdn", default=None, help='Target MDN.')
    parser.add_option('-n', '--num', action="store", type="int", default=1, dest="num", help='Number of messages to send.')
    parser.add_option('-u', '--url', action="store", dest="url", default="http://192.168.168.127:8100/meet/sendplace", help='Target URL')
    parser.add_option('-i', '--input', action="store", dest="input", default=None, help='Input file.')
    parser.add_option('-s', '--skip', action="store", type="int", dest="skip", default=0, help='Number of lines to skip.')

    (options, _) = parser.parse_args()

    if not options.mdn:
        parser.error("Target MDN must be specified.")

    if not options.input:
        parser.error("Input file must be specified.")

    print "Will send %d messages to %s using '%s'." % (options.num, options.mdn, options.url)
    print "Skipping %d lines." % options.skip

    sent = 0
    skip = options.skip

    for line in open(options.input, "r"):
        if skip > 0:
            skip = skip - 1
            continue
        if sent < options.num:
            req_txt = generate_request(line, options.mdn, sent+1)
            print "Request:"
            print req_txt
            open('req.xml', 'w').write(req_txt)
            req = urllib2.Request(options.url, req_txt)
            print "Response:"
            res = urllib2.urlopen(req).read()
            print res
            sent=sent+1
        else:
            print "Finished."
            return

if __name__ == '__main__':
    sys.exit(main())
