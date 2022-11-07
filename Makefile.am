# $Id: Makefile.am,v 1.1.1.1 2004/08/05 21:47:57 caso Exp $

AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
CCOPTS= -DNO_FORK
SUBDIRS= src arena
EXTRA_DIST = configure.ac

noinst_PROGRAMS = hid async

hid_SOURCES= arena/hid.c
async_SOURCES= arena/async.c

LDADD= $(top_builddir)/src/libnomad.la

if BUILDING_SLACK

slack:
	make install DESTDIR=$(abs_builddir)/pkg.${build_cpu}
	$(MKDIR_P) $(abs_builddir)/pkg.${build_cpu}/install
	cp ${top_srcdir}/slack-desc $(abs_builddir)/pkg.${build_cpu}/install
	find $(abs_builddir)/pkg.${build_cpu}/ -name "*.la" | xargs rm
	cd $(abs_builddir)/pkg.${build_cpu}; ${SLACKBUILD} -l y -c n $(abs_builddir)/${PACKAGE}-${VERSION}-${build_cpu}-${SVN_BUILD}.txz
#	cp $(abs_builddir)/../../${build_cpu}/${PACKAGE}-${VERSION}-${build_cpu}-${SVN_BUILD}.txz /var/cache/packages/${build_cpu}/extra/${PACKAGE}-${VERSION}-${build_cpu}-${SVN_BUILD}.txz

else

slack:
	echo "No slackware package requested ${PACKAGE}-${VERSION}-${SVN_BUILD}"
endif


