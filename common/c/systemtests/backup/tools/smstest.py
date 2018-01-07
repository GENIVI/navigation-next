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

msg = sys.argv[1]

clsid = '0103589c'
smskey = 'AykaNSVCEcrnhiqacLY+F5aeisd7pmiATyTGMuRS'
#smskey = 'YvXBwA9XZLVqE7aa9QnDkid7QwuYt2sBFyGaOWZI'

msgenc = ''.join(obfuscate(msg, imap(ord, smskey))).encode('base64')[:-1]

hm = hmac.new(smskey, msgenc, sha)

sms = '//BREW:' + clsid + ':' + hm.hexdigest()[:20] + msgenc

print "Secret: \"%s\"" % smskey
print "Message: \"%s\"" % msg
print "Encoded: \"%s\"" % msgenc
print "SMS: \"%s\"" % sms
