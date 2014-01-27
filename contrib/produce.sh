#!/bin/bash
#
# Helper script to automate CA creation and credential generation.
#

# defaults
TMPDIR=/tmp/ece
DESTDIR=.
KEY=key.pem
KEY_SZ=2048
REQ=req.pem
CN="SECE"
CERT=cert.pem
CA_DST=ca.pem
CADIR=./demoCA
ENDDATE=""
CMD="newcreds"  # make credentials by default
POLICY=""

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
    echo "      -newca        create a new CA (can combine with: -cadir, -subj)"
    echo "      -cadir  DIR   specify CA directory"
    echo "      -subj   STR   Subject Name (for CSR)"
    echo "      -cn     STR   Common Name part of Subject Name (for CSR)"
    echo "      -key    FN    Output key file"
    echo "      -cert   FN    Output certificate file"
    echo "      -exts   exts  Specify OpenSSL extensions"
    echo "      -policy P     Specify CA policy"
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
            -newca)
                CMD="newca"
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
            -exts)
                shift
                [ -z $VALUE ] && die "-exts requires extensions argument!"
                EXTS=$VALUE
                ;;
            -policy)
                shift
                [ -z $VALUE ] && die "-policy requires policy argument!"
                POLICY=$VALUE
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
                ;;
        esac
        shift
    done

    CA=${CADIR}/cacert.pem
    SUBJ="/C=IT/O=efw/CN=${CN}"

    msg "DESTDIR=${DESTDIR}"
    msg "CADIR=${CADIR}"
    msg "CA=${CA}"
    msg "KEY=${KEY}"
    msg "KEY_SZ=${KEY_SZ}"
    msg "SUBJ=${SUBJ}"
    msg "CN=${CN}"
    msg "CERT=${CERT}"
    msg "EXTS=${EXTS}"
    [ "${ENDDATE}" != "" ] && msg "ENDDATE=${ENDDATE}"
}

cmd_newcreds()
{
    cd "${TMPDIR}"

    ln -s "${CADIR}" demoCA

    GENRSA_ARGS="-out ${KEY} ${KEY_SZ}"
    wrap openssl genrsa ${GENRSA_ARGS}

    REQ_ARGS="-batch -new -key ${KEY} -out ${REQ} -subj ${SUBJ}"
    [ "${EXTS}" != "" ] && REQ_ARGS="${REQ_ARGS} -reqexts ${EXTS}"
    wrap openssl req ${REQ_ARGS}

    CA_ARGS="-batch -in ${REQ} -out ${CERT}"
    [ "${ENDDATE}" != "" ] && CA_ARGS="${CA_ARGS} -enddate ${ENDDATE}"
    [ "${EXTS}" != "" ] && CA_ARGS="${CA_ARGS} -extensions ${EXTS}"
    [ "${POLICY}" != "" ] && CA_ARGS="${CA_ARGS} -policy ${POLICY}"
    wrap openssl ca ${CA_ARGS}

    wrap cp "${CA}" "${CA_DST}"

    cd -

    wrap mv "${TMPDIR}/${KEY}" "${DESTDIR}"
    wrap mv "${TMPDIR}/${CERT}" "${DESTDIR}"
    wrap cp "${TMPDIR}/${CA_DST}" "${DESTDIR}"
}

cmd_newca()
{
    cd "${TMPDIR}"

    wrap mkdir "${CADIR}"
    wrap mkdir "${CADIR}/certs"
    wrap mkdir "${CADIR}/crl"
    wrap mkdir "${CADIR}/newcerts"
    wrap mkdir "${CADIR}/private"
    wrap touch "${CADIR}/index.txt"

    GENRSA_ARGS="-out ${CADIR}/private/cakey.pem ${KEY_SZ}"
    wrap openssl genrsa ${GENRSA_ARGS}

    REQ_ARGS="-batch -new -key ${CADIR}/private/cakey.pem -out ${REQ} -subj ${SUBJ}"
    wrap openssl req ${REQ_ARGS}

    CA_ARGS="-batch -in ${REQ} -keyfile ${CADIR}/private/cakey.pem -out ${CADIR}/cacert.pem"
    CA_ARGS="${CA_ARGS} -create_serial -selfsign -extensions v3_ca"
    [ "${POLICY}" != "" ] && CA_ARGS="${CA_ARGS} -policy ${POLICY}"
    [ "${ENDDATE}" != "" ] && CA_ARGS="${CA_ARGS} -enddate ${ENDDATE}"
    wrap openssl ca ${CA_ARGS}

    cd -

    wrap rm -rf "${DESTDIR}/${CADIR}"
    wrap mv "${TMPDIR}/${CADIR}" "${DESTDIR}"
}

mkdir "${TMPDIR}"
trap cleanup 0 1 2 3 4 6 7 8 9 11 13 15

parse_args $@

cmd_"${CMD}"
