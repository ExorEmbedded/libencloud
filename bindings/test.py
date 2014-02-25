import encloud
import time

print 'Runnings libencloud (v%s rev: %s) tests' % (encloud.version(), encloud.revision())

e = encloud.create(0, None)

encloud.start(e)
time.sleep(3)
encloud.stop(e)
time.sleep(2)
encloud.start(e)
time.sleep(10)

if False:   
    # SECE only
    encloud.set_license(e, '{a8098c1a-f86e-11da-bd1a-00112444be1e}')

    encloud.retr_sb_info(e)
    encloud.retr_sb_cert(e)
    c = encloud.retr_sb_conf(e)

    print "ip: " + encloud.sb_conf_get_vpn_ip(c)
    print "port: " + str(encloud.sb_conf_get_vpn_port(c))
    print "proto: " + encloud.sb_conf_get_vpn_proto(c)
    print "type: " + encloud.sb_conf_get_vpn_type(c)

encloud.destroy(e)

print 'All tests passed'
