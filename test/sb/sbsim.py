# *** EXPERIMENTAL/TESTING ONLY ***
#
# Tiny Switchboard simulator for libece communication testing
#
# Prerequisites:
#   * apache2 config:
#   * demoCA directory setup in /var/www/ws1, with key unprotected by password
#   * sudo chown -R www-data:www-data /var/www/
#       (allows $RANDFILE to be written in $HOME - could change settings in /usr/lib/ssl/openssl.cnf)

from mod_python import apache, util
import os
import time
import urllib2
import subprocess
import simplejson as json

def handler(req):

    #os.chdir("/tmp/sbsim")
    #os.chdir("/var/www/ws1")

    # force timeout (testing)
    #time.sleep(8)

    command = req.uri.strip("/")

    req.send_http_header()
    req.content_type = 'application/json'

    if command == "command.access.cloud.getInfo":
        return handler_info(req)
    elif command == "command.access.cloud.getCertificate":
        return handler_csr(req)
    elif command == "command.access.cloud.getConfiguration":
        return handler_conf(req)
    else:
        req.write("bad command: '" + command + "'")
        return apache.DECLINED

    return apache.OK

def handler_info(req):

    # received = req.read()
    #
    # [..] here SB:
    #   - parses/checks received message
    #   - checks validity of license 
    #   - grabs CSR template 

    day = 60*60*24;

    req.write(json.dumps({ \
                'valid' : True, \
                'expiry' : time.time() + 30*day, \
                'csr_tmpl' : { "DN" : {  "C" : "IT" } }, \
                'time' : time.time() \
                }))

    return apache.OK

def handler_csr(req):

    csrfn = "/tmp/sbsim-req.pem"
    csrf = open(csrfn, "w")

    js = urllib2.urlparse.parse_qs(req.read())["json"][0]
    jo = json.loads(js)

    # quick hack? URL decoding (correctly) changes "+" simbols to " ", breaking
    # base64 CSR data => replace them back
    csr = jo["certificate_request_data"].replace(' ', '+')
    # .. but rollback only start/end markers
    csr = csr.replace("--BEGIN+CERTIFICATE+REQUEST--", "--BEGIN CERTIFICATE REQUEST--")
    csr = csr.replace("--END+CERTIFICATE+REQUEST--", "--END CERTIFICATE REQUEST--")
    csrf.write(csr)
    csrf.close()

    # display request
    #sp = subprocess.Popen(["openssl", "req", "-text"], \
    #        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    #req.write("RECEIVED CSR: " + str(sp.communicate(csr)))

    # request certificate from CA
    #sp = subprocess.Popen(["strace", "openssl", "ca", "-in", csrfn ], \
    sp = subprocess.Popen(["openssl", "ca", "-in", csrfn ], \
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

#    req.write("XXX CA: " + str(sp.readline()))
    req.write("XXX CA: " + str(sp.communicate()))
#    req.write("XXX CA: " + os.popen("opnssl ca -in /tmp/sbsim-req.pem"))


    return apache.OK

def handler_conf(req):
    req.write("handler_conf")
    return apache.OK
