#!/bin/bash
#
# Helper script to automate key, CSR and certificate generation.
#
# SECE: Cert/Key are unique for software package
#       => run only once!
# ECE: Subject will depend on PoI (TODO)

# defaults
TMPDIR=/tmp/ece
DESTDIR=.
KEY=key1.pem
KEY_SZ=2048
REQ=req.pem
CN="SECE"
CERT=cert1.pem
CACERT_DST=cacert1.pem
CADIR=./demoCA
ENDDATE=""

msg()
{
    echo "# $@"
}

die()
{
    msg "$@"
    exit 1
}

wrap()
{
    msg "Executing $@"
    $@
    rc=$?
    [ ${rc} -ne 0 ] && die "Command '$@' returned ${rc} - exiting"
}

cleanup() 
{
    rm -rf ${TMPDIR}
}

usage()
{
    echo "Usage: $0"
    echo
    echo "      -help         this help"
    echo "      -dest   DIR   specify destination directory for output files (default = '.')"
    echo "      -cadir  DIR   specify CA directory"
    echo "      -subj   STR   Subject Name (for CSR)"
    echo "      -cn     STR   Common Name part of Subject Name (for CSR)"
    echo "      -key    FN    Output key file"
    echo "      -cert   FN    Output certificate file"
    echo "      -test-expiry  Make certifcate expire in 3 minutes"
}

parse_args()
{
    while [ "$1" != "" ]
    do
        PARAM=$1
        VALUE=$2

        case $PARAM in
            -help)
                usage
                exit 0
                ;;
            -dest)
                shift
                [ -z $VALUE ] && die "-dest requires DIR parameter!"
                DESTDIR=$VALUE
                ;;
            -cadir)
                shift
                [ -z $VALUE ] && die "-cadir requires DIR parameter!"
                CADIR=$VALUE
                ;;
            -subj)
                shift
                [ -z $VALUE ] && die "-subj requires STR argument!"
                SUBJ=$VALUE
                ;;
            -cn)
                shift
                [ -z $VALUE ] && die "-cn requires STR argument!"
                CN=$VALUE
                ;;
            -key)
                shift
                [ -z $VALUE ] && die "-key requires filename argument!"
                KEY=$VALUE
                ;;
            -cert)
                shift
                [ -z $VALUE ] && die "-cert requires filename argument!"
                CERT=$VALUE
                ;;
            -test-expiry)
                # now + 3 minutes
                ENDDATE=$[ $(date +%Y%m%d%H%M%S) + 3*60 ]
                ENDDATE="${ENDDATE}Z"
                ;;
            :)
                echo "Option $OPTARG requires arguments"
                usage
                exit 1
        esac
        shift
    done

    CACERT=${CADIR}/cacert.pem
    SUBJ="/C=IT/O=efw/CN=${CN}"

    msg "DESTDIR=${DESTDIR}"
    msg "CADIR=${CADIR}"
    msg "CACERT=${CACERT}"
    msg "KEY=${KEY}"
    msg "KEY_SZ=${KEY_SZ}"
    msg "SUBJ=${SUBJ}"
    msg "CN=${CN}"
    msg "CERT=${CERT}"
    [ "${ENDDATE}" != "" ] && msg "ENDDATE=${ENDDATE}"
}

mkdir ${TMPDIR}
trap cleanup 0 1 2 3 4 6 7 8 9 11 13 15

parse_args $@

cd ${TMPDIR}

ln -s ${CADIR} demoCA

GENRSA_ARGS="-out ${KEY} ${KEY_SZ}"
wrap openssl genrsa ${GENRSA_ARGS}

REQ_ARGS="-batch -new -key ${KEY} -out ${REQ} -subj ${SUBJ}"
wrap openssl req ${REQ_ARGS}

CA_ARGS="-batch -in ${REQ} -out ${CERT}"
[ "${ENDDATE}" != "" ] && CA_ARGS="${CA_ARGS} -enddate ${ENDDATE}"
wrap openssl ca ${CA_ARGS}

wrap cp ${CACERT} ${CACERT_DST}

cd -

wrap mv ${TMPDIR}/${KEY} ${DESTDIR}
wrap mv ${TMPDIR}/${CERT} ${DESTDIR}
wrap cp ${TMPDIR}/${CACERT_DST} ${DESTDIR}
