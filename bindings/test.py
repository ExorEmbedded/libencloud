import ece

print 'Runnings libece tests'

e = ece.create(0, None)

ece.set_license(e, '{a8098c1a-f86e-11da-bd1a-00112444be1e}')
ece.retr_sb_info(e)
ece.retr_sb_cert(e)
ece.retr_sb_conf(e)

ece.destroy(e)

print 'All tests passed'
