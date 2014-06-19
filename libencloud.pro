include(common.pri)

TEMPLATE = subdirs

SUBDIRS += include
SUBDIRS += about
SUBDIRS += src
SUBDIRS += etc

!notest:SUBDIRS += test

# tests depend on build
check.depends = all

# generate tar package
TARFILE = $${PKGNAME}-$${VERSION}.tar
TARZIPFILE = $${TARFILE}.gz
dist.target = dist
dist.commands = @( \
                echo "Making tar package $${TARZIPFILE}"; \
                if [ -e .svn ]; then \
                    echo "found VCS: SVN"; \
                    find . -type f -not -iwholename '*.svn*' | \
                    xargs svn status -v 2>/dev/null | grep -v "^[?I]" | \
                    sed 's,^\\ ,-,' | sed 's,A*+,,' | \
                    tr -s "[:space:]" | cut -d '\\ ' -f 5 | \
                    xargs tar czf .$${TARZIPFILE}; \
                elif [ -e .git ]; then \
                    echo "found VCS: GIT"; \
                    git rev-parse --short HEAD > .revision; \
                    tar cf .$${TARFILE} .revision; \
                    git ls-files | xargs tar rf .$${TARFILE}; \
                    gzip .$${TARFILE}; \
                else \
                    echo "[warn] no VCS found"; \
                    # not under VCS \
                    find . -type f -not -name Makefile | \
                    xargs tar czf .$${TARZIPFILE}; \
                fi ; \
                rm -rf $${PKGNAME}-$${VERSION}; \
                mkdir $${PKGNAME}-$${VERSION}; \
                tar xzf .$${TARZIPFILE} -C $${PKGNAME}-$${VERSION}; \
                tar czvf $${TARZIPFILE} $${PKGNAME}-$${VERSION}; \
                md5sum $${TARZIPFILE} > $${TARZIPFILE}.md5; \
                rm -rf .$${TARZIPFILE}; \
                )
QMAKE_EXTRA_TARGETS += dist

# override distclean so we can remove extra files
redirect_distclean.target = distclean
redirect_distclean.commands = rm -rf Makefile $${PKGNAME}-$${VERSION}*
QMAKE_EXTRA_TARGETS += redirect_distclean
