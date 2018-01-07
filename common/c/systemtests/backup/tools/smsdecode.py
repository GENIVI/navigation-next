import sys
import hmac
import sha
import operator
from itertools import *

def obfuscate(s, key):
    return imap(chr, imap(operator.xor, cycle(key), imap(ord, s)))

def itake(iterable, i):
    """Yield the first i elements of the iterable."""
    return islice(iterable, 0, i)

sms = sys.argv[1]

bhdr = sms[0:7]
clsid = sms[7:15]
hm = sms[16:36]
msgenc = sms[36:]

smskey = 'AykaNSVCEcrnhiqacLY+F5aeisd7pmiATyTGMuRS'

msg = ''.join(obfuscate(msgenc.decode('base64'), imap(ord, smskey)))

print "CLSID: \"%s\"" % clsid
print "Message: \"%s\"" % msg
