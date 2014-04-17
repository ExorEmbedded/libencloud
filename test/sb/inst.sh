#!/bin/sh -x
#
# Installs SB simulator environment on fresh Ubuntu 12.04 - might work on other variants.
#
# Notes: 
#   - not a real system installation, creates symolic links to current working tree to
#   speed up development.

CONTRIB=../../contrib
DESTDIR=/test
CONFDIR=${DESTDIR}/etc
CADIR=demoCA

CA1=ca1
CA2=ca2
CA1DIR="${CONFDIR}/${CA1}"
CA2DIR="${CONFDIR}/${CA2}"

WS1=ws1
WS2=ws2
WS1CN=initialization-host
WS2CN=operation-host
WS1DIR="${CONFDIR}/${WS1}"
WS2DIR="${CONFDIR}/${WS2}"

VPNDIR="${CONFDIR}/vpnsrv"
VPNCN="vpnsrv"

#LIBENCLOUDDIR="${CONFDIR}/encloud"
LIBENCLOUDDIR="/etc/encloud"
LIBENCLOUDSERIAL="123"
LIBENCLOUDPOI="94c97e4b-ab8c-4dd6-b06b-ef3e18ed2d83"

EXTRAARGS="-policy policy_anything"

DOCROOT=/var/www

# give current user permissions on DESTDIR
sudo mkdir ${DESTDIR}
sudo chown ${USER}:${USER} ${DESTDIR}

mkdir -p "${CONFDIR}"

# produce CA1
rm -rf "${CA1DIR}"
"${CONTRIB}/produce.sh" -newca -cn "${CA1}" ${EXTRAARGS}
mv "${CADIR}" "${CA1DIR}"

# produce CA2
rm -rf "${CA2DIR}"
"${CONTRIB}/produce.sh" -newca -cn "${CA2}" ${EXTRAARGS}
mv "${CADIR}" "${CA2DIR}"

# produce SB web service 1 (Initialization)
rm -rf "${WS1DIR}"
mkdir -p "${WS1DIR}"
"${CONTRIB}/produce.sh" -cn "${WS1CN}" -cadir "${CA1DIR}" ${EXTRAARGS}
mv *.pem "${WS1DIR}"

# produce SB web service 2 (Operation)
rm -rf "${WS2DIR}"
mkdir -p "${WS2DIR}"
"${CONTRIB}/produce.sh" -cn "${WS2CN}" -cadir "${CA2DIR}" ${EXTRAARGS}
mv *.pem "${WS2DIR}"

# produce OpenVPN server
rm -rf "${VPNDIR}"
mkdir -p "${VPNDIR}"
"${CONTRIB}/produce.sh" -cn "${VPNCN}" -cadir "${CA2DIR}" -exts server ${EXTRAARGS}
openssl dhparam -out dh.pem 1024
mv *.pem "${VPNDIR}"

# produce SECE (Initialization)
#rm -rf "${LIBENCLOUDDIR}"
#mkdir -p "${LIBENCLOUDDIR}"
#"${CONTRIB}/produce.sh" -cn "SECE" -cadir "${CA1DIR}" ${EXTRAARGS}
#mv key.pem "${LIBENCLOUDDIR}/init_key.pem"
#mv cert.pem "${LIBENCLOUDDIR}/init_cert.pem"
#mv ca.pem "${LIBENCLOUDDIR}/init_ca.pem"

# produce LIBENCLOUD (Initialization)
mkdir -p "${LIBENCLOUDDIR}"
"${CONTRIB}/produce.sh" -cn "${LIBENCLOUDSERIAL}" -cadir "${CA1DIR}" ${EXTRAARGS}
mv key.pem "${LIBENCLOUDDIR}/init_key.pem"
mv cert.pem "${LIBENCLOUDDIR}/init_cert.pem"
mv ca.pem "${LIBENCLOUDDIR}/init_ca.pem"
echo "${LIBENCLOUDSERIAL}" > "${LIBENCLOUDDIR}/serial"
echo "${LIBENCLOUDPOI}" > "${LIBENCLOUDDIR}/poi"
sudo chmod 600 "${LIBENCLOUDDIR}/"*
sudo chown -R "${USER}" "${LIBENCLOUDDIR}"

# install deps
sudo apt-get install apache2 libapache2-mod-python

# install deps
sudo apt-get install apache2 libapache2-mod-python
sudo a2enmod ssl python

# install apache config
sudo rm -f /etc/apache2/sites-available/sbsim
sudo ln -s "${PWD}"/apache-sbsim /etc/apache2/sites-available/sbsim
sudo rm -f /etc/apache2/sites-enabled/sbsim
sudo ln -s /etc/apache2/sites-available/sbsim /etc/apache2/sites-enabled/sbsim
sudo rm -f /var/www/sbsim.py
sudo ln -s "${PWD}"/sbsim.py /var/www/sbsim.py

sudo mkdir -p "${DOCROOT}/${WS1}"
sudo mkdir -p "${DOCROOT}/${WS2}"

# we copy CA2 into WS1 scope so it can be used to create Operation certs
sudo rm -rf "${DOCROOT}/${WS1}/demoCA"
sudo cp -a "${CA2DIR}" "${DOCROOT}/${WS1}/demoCA"
sudo chown -R www-data:www-data "${DOCROOT}"

sudo service apache2 restart
