import ece

print 'Runnings libece (v%s rev: %s) tests' % (ece.version(), ece.revision())

e = ece.create(0, None)

# SECE only
ece.set_license(e, '{a8098c1a-f86e-11da-bd1a-00112444be1e}')

ece.retr_sb_info(e)
ece.retr_sb_cert(e)
c = ece.retr_sb_conf(e)

print "ip: " + ece.sb_conf_get_vpn_ip(c)
print "port: " + str(ece.sb_conf_get_vpn_port(c))
print "proto: " + ece.sb_conf_get_vpn_proto(c)
print "type: " + ece.sb_conf_get_vpn_type(c)

ece.destroy(e)

print 'All tests passed'
