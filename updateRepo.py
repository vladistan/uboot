import boto
import sys
import os

PKG = sys.argv[1]
BUILD = sys.argv[2]
URL = os.environ['BUILD_URL']


print "PKG %s" % PKG
print "Build version %s" % BUILD
print "Traceback URL %s" % URL

from boto.s3.key import Key

c = boto.connect_s3()
b = c.get_bucket('pplansbuildtools')

k = Key(b)
k.key = "%s/defaultStage/defaultJob/1.0.%s/" % (PKG, BUILD)
k.set_metadata('user', 'vlad')
k.set_metadata('traceback_url', URL)
k.set_metadata('completed', 'yes')
k.set_contents_from_string('')
