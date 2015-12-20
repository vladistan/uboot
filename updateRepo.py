import boto
import sys

PKG = sys.argv[1]
BUILD = sys.argv[2]


print "PKG %s" % PKG
print "Build version %s" % BUILD

from boto.s3.key import Key

c = boto.connect_s3()
b = c.get_bucket('pplansbuildtools')

k = Key(b)
k.key = "%s/defaultStage/defaultJob/1.0.%s/" % (PKG, BUILD)
k.set_metadata('user','vlad')
k.set_metadata('traceback_url','http://www.google.com')
k.set_metadata('completed','yes')
k.set_contents_from_string('')
