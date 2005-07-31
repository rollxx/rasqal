# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Rasqal utilities
#
# $Id$
#
# Copyright (C) 2005, David Beckett http://purl.org/net/dajobe/
# Institute for Learning and Research Technology http://www.ilrt.bristol.ac.uk/
# University of Bristol, UK http://www.bristol.ac.uk/
# 
# This package is Free Software and part of Redland http://librdf.org/
# 
# It is licensed under the following three licenses as alternatives:
#   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
#   2. GNU General Public License (GPL) V2 or any newer version
#   3. Apache License, V2.0 or any newer version
# 
# You may not use this file except in compliance with at least one of
# the above three licenses.
# 
# See LICENSE.html or LICENSE.txt at the top of this package for the
# complete terms and further detail along with the license texts for
# the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
# 

bin_PROGRAMS = roqet

man_MANS = roqet.1

roqet_SOURCES = roqet.c
if GETOPT
roqet_SOURCES += getopt.c rasqal_getopt.h
endif
roqet_LDADD = $(top_builddir)/src/librasqal.la @RASQAL_EXTERNAL_LIBS@ $(MEM_LIBS)

EXTRA_DIST= \
$(man_MANS) \
roqet.html

if MAINTAINER_MODE
roqet.html: $(srcdir)/roqet.1 $(srcdir)/../fix-groff-xhtml
	-groff -man -Thtml -P-l $< | tidy -asxml -wrap 1000 2>/dev/null | perl $(srcdir)/../fix-groff-xhtml $@

endif

$(top_builddir)/src/librasqal.la:
	cd $(top_builddir)/src && $(MAKE) librasqal.la