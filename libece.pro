include(common.pri)

TEMPLATE = subdirs

SUBDIRS += include
SUBDIRS += src
SUBDIRS += etc
SUBDIRS += test

# tests depend on build
check.depends = all

# generate tar package
TARFILE = $${PKGNAME}-$${VERSION}.tar.gz
dist.target = dist
dist.depends = all
dist.commands = @( \
                echo "Making tar package $${TARFILE}"; \
                if [ -e .svn ]; then \
                    echo "found VCS: SVN"; \
                    find . -type f -not -iwholename '*.svn*' | \
                    xargs svn status -v 2>/dev/null | grep -v "^[?I]" | \
                    sed 's,^\\ ,-,' | sed 's,A*+,,' | \
                    tr -s "[:space:]" | cut -d '\\ ' -f 5 | \
                    xargs tar czf .$${TARFILE}; \
                elif [ -e .git ]; then \
                    echo "found VCS: GIT"; \
                    git ls-files | xargs tar czf .$${TARFILE}; \
                else \
                    echo "[warn] no VCS found"; \
                    # not under VCS \
                    find . -type f -not -name Makefile | \
                    xargs tar czf .$${TARFILE}; \
                fi ; \
                rm -rf $${PKGNAME}-$${VERSION}; \
                mkdir $${PKGNAME}-$${VERSION}; \
                tar xzf .$${TARFILE} -C $${PKGNAME}-$${VERSION}; \
                tar czvf $${TARFILE} $${PKGNAME}-$${VERSION}; \
                md5sum $${TARFILE} > $${TARFILE}.md5; \
                rm -rf .$${TARFILE}; \
                )
QMAKE_EXTRA_TARGETS += dist

# override distclean so we can remove extra files
redirect_distclean.target = distclean
redirect_distclean.commands = rm -rf Makefile $${PKGNAME}-$${VERSION}*
QMAKE_EXTRA_TARGETS += redirect_distclean
