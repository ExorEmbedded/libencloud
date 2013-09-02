#!/bin/sh
#
# SECE: Cert/Key are unique for software package
#       => run only once!        
# ECE: Subject will depend on PoI (TODO)
#
# Prerequisites:
#   - CA_default/dir setup in /usr/lib/ssl/openssl.cnf

TMPDIR=/tmp/ece
DESTDIR=/etc/ece

KEY=key1.pem
KEY_SZ=2048
REQ=req.pem
REQ_SUBJ="/C=IT/O=efw/CN=SECE"
CERT=cert1.pem
CACERT=cacert1.pem
CACERT_SRC=${HOME}/local/etc/ca1/demoCA/cacert.pem

wrap()
{
    echo "# Executing $@"
    $@
    rc=$?
    if [ ${rc} -ne 0 ]; then
        echo "Command '$@' returned ${rc} - exiting"
        exit 1
    fi
}

cleanup() 
{
    rm -rf ${TMPDIR}
}

mkdir ${TMPDIR}
trap cleanup 0 1 2 3 4 6 7 8 9 11 13 15

cd ${TMPDIR}

wrap openssl genrsa -out ${KEY} ${KEY_SZ}
wrap openssl req -batch -new -key ${KEY} -out ${REQ} -subj ${REQ_SUBJ}
wrap openssl ca -batch -in ${REQ} -out ${CERT}
wrap cp ${CACERT_SRC} ${CACERT}

cd -

wrap mv ${TMPDIR}/${KEY} ${DESTDIR}
wrap mv ${TMPDIR}/${CERT} ${DESTDIR}
wrap cp ${TMPDIR}/${CACERT} ${DESTDIR}
