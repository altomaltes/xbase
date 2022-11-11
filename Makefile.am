# $Id: Makefile.am,v 1.1.1.1 2004/08/05 21:47:57 caso Exp $

AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
CCOPTS= -DNO_FORK
SUBDIRS= source 
EXTRA_DIST = configure.ac

check_PROGRAMS = ex1 ex2 ex3 ex4 ex5 ex6 ex7 ex8 seeker append bank capieg copydat2 copydata custlist d4test d4verify datainfo date1 deletion

ex1_SOURCES=examples/source/c/ex1.c 
ex2_SOURCES=examples/source/c/ex2.c 
ex3_SOURCES=examples/source/c/ex3.c 
ex4_SOURCES=examples/source/c/ex4.c 
ex5_SOURCES=examples/source/c/ex5.c 
ex6_SOURCES=examples/source/c/ex6.c 
ex7_SOURCES=examples/source/c/ex7.c 
ex8_SOURCES=examples/source/c/ex8.c 
seeker_SOURCES=examples/source/c/seeker.c 

append_SOURCES=examples/source/c/seeker.c 
bank_SOURCES=examples/source/c/bank.c 
capieg_SOURCES=examples/source/c/capieg.c 
copydat2_SOURCES=examples/source/c/copydat2.c 
copydata_SOURCES=examples/source/c/copydata.c 

custlist_SOURCES=examples/source/c/custlist.c 
d4test_SOURCES=examples/source/c/d4test.c 
d4verify_SOURCES=examples/source/c/d4verify.c 
datainfo_SOURCES=examples/source/c/datainfo.c 
date1_SOURCES=examples/source/c/date1.c 
deletion_SOURCES=examples/source/c/deletion.c 

LDADD= $(top_builddir)/source/libxbase.la

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


