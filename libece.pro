include(common.pri)

TEMPLATE = subdirs

SUBDIRS += src
SUBDIRS += test

# header installation
headers.path = $$INCDIR
headers.files += include/ece.h
INSTALLS += headers

# configuration file installation
headers.path = $$INCDIR
conf.path = $$CONFDIR
conf.files += etc/*.json
conf.files += etc/*.pem
conf.files += etc/*.tmpl
INSTALLS += conf

# generate tar package
TARFILE = $${PKGNAME}-$${VERSION}.tar.gz
dist.target = dist
dist.commands = @( \
                echo Making tar package $$TARFILE; \
                if [ -e .svn ]; then \
                    # under VCS \
                    cd .. ; \
                    find $$PKGNAME -type f -not -iwholename '*.svn*' | \
                    xargs svn status -v | grep -v "^[?I]" | \
                    sed 's,^\\ ,-,' | sed 's,A*+,,' | \
                    tr -s "[:space:]" | cut -d '\\ ' -f 5 | \
                    xargs tar czvf $$PKGNAME/$$TARFILE; \
                else \
                    # not under VCS \
                    cd .. ; \
                    find $$PKGNAME -type f -not -name Makefile | \
                    xargs tar czvf $$PKGNAME/$$TARFILE; \
                fi)

QMAKE_EXTRA_TARGETS = dist
