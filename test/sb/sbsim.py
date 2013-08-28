# *** EXPERIMENTAL/TESTING ONLY ***
#
# Tiny Switchboard simulator for libece communication testing
#
# Prerequisites:
#   * apache2 config: sample in test/sb/default-ssl
#   * demoCA directory setup in /var/www/ws1, with key unprotected by password
#       Note: must point to CA2 (Operation)!
#   * sudo chown -R www-data:www-data /var/www/
#       (allows $RANDFILE to be written in $HOME - could change settings in /usr/lib/ssl/openssl.cnf)

from mod_python import apache, util
import os
import time
import urllib2
import subprocess
import simplejson as json

# testing
#force_error = True
force_error = False

def err (s):

    apache.log_error(s)

def handler (req):

    rc = apache.OK

    """
    # force timeout (testing)
    time.sleep(8)
    """

    command = req.uri.strip('/')

    req.send_http_header()
    req.content_type = 'application/json'

    if force_error:
        req.write(json.dumps({
                    'time' : time.time(),
                    'error' : 'test error message from SB'
                    }))
        return apache.OK

    if command == 'command.access.cloud.getInfo':
        rc = handler_info(req)
    elif command == 'command.access.cloud.getCertificate':
        rc = handler_csr(req)
    elif command == 'command.access.cloud.getConfiguration':
        rc = handler_conf(req)
    else:
        req.write('bad command: ' + command)
        rc = apache.DECLINED

    return rc

def handler_info (req):

    """
    received = req.read()
    
    [..] here SB:
       - parses/checks received message
       - checks validity of license 
       - grabs CSR template 
    """

    day = 60*60*24;

    req.write(json.dumps({
                'valid' : True,
                'expiry' : time.time() + 30*day,
                'csr_tmpl' :
                    { 'DN' :
                        {
                        'C' : 'AU',
                        'ST' : 'Some-State',
                        'O' : 'Internet Widgits Pty Ltd'
                        # CN added by ECE
                        }
                    },
                'time' : time.time()
                }))

    return apache.OK

def handler_csr (req):

    # enter demoCA context dir
    os.chdir('/var/www/ws1')

    csrfn = '/tmp/sbsim-csr.pem'
    certfn = '/tmp/sbsim-cert.pem'

    js = urllib2.urlparse.parse_qs(req.read())['json'][0]
    jo = json.loads(js)

    # quick hack? URL decoding (correctly) changes '+' simbols to ' ', breaking
    # base64 CSR data => replace them back
    csr = jo['certificate_request_data'].replace(' ', '+')
    # .. but rollback only start/end markers
    csr = csr.replace('--BEGIN+CERTIFICATE+REQUEST--', '--BEGIN CERTIFICATE REQUEST--')
    csr = csr.replace('--END+CERTIFICATE+REQUEST--', '--END CERTIFICATE REQUEST--')

    csrf = open(csrfn, 'w')
    csrf.write(csr)
    csrf.close()

    """
    # display request
    sp = subprocess.Popen(['openssl', 'req', '-text'], \
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    #req.write('received csr: ' + str(sp.communicate(csr)))
    """

    # request certificate from CA based on CSR and without prompts
    os.system('openssl ca -batch -in ' + csrfn + ' -out ' + certfn + ' 2>/dev/null');

    """
    upon duplicates 'openssl ca' returns 'failed to update database\nTXT_DB error number 2'
    or perhaps a lookup could be done with -status, anyhow SB will return appropriate error string
    """
    # read the newly-generated certificate
    certf = open(certfn, 'r')
    cert = certf.read()
    certf.close()

    req.write(json.dumps({
                'certificate' : cert,
                'time' : time.time()
                }))
    
    # clear DB for testing - better way?
    os.system('rm -f demoCA/newcerts/*')
    os.system('> demoCA/index.txt')

    return apache.OK

def handler_conf (req):

    req.write(json.dumps({
                'vpn' : {
                    'ip' : '192.168.0.169', 
                    'port' : 443,
                    'proto' : 'tcp',
                    'type' : 'tap',
                    }
                }))
    
    return apache.OK
