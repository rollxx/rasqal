# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Rasqal SPARQL tests
#
# Copyright (C) 2004-2008, David Beckett http://www.dajobe.org/
# Copyright (C) 2004-2004, University of Bristol, UK http://www.bristol.ac.uk/
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

SPARQL_MANIFEST_FILES= manifest.n3

SPARQL_MODEL_FILES= \
dawg-data-01.n3

SPARQL_TEST_FILES= \
dawg-query-001.rq dawg-query-002.rq dawg-query-003.rq dawg-query-004.rq

SPARQL_TEST_NAMES= \
dawg-opt-query-001 \
dawg-opt-query-002 \
dawg-opt-query-003 \
dawg-opt-query-004

EXPECTED_SPARQL_CORRECT_FAILURES= \
dawg-opt-query-001 \
dawg-opt-query-002 \
dawg-opt-query-003 \
dawg-opt-query-004

SPARQL_RESULT_FILES= \
dawg-result-001.n3 dawg-result-002.n3 dawg-result-003.n3 dawg-result-004.n3

EXTRA_DIST= \
$(SPARQL_MANIFEST_FILES) \
$(SPARQL_MODEL_FILES) \
$(SPARQL_TEST_FILES) \
$(SPARQL_RESULT_FILES)

CLEANFILES=diff.out roqet.err roqet.out roqet.tmp result.out

build-roqet:
	@(cd $(top_builddir)/utils ; $(MAKE) roqet)

build-sparql-lexer-test:
	@(cd $(top_builddir)/src ; $(MAKE) sparql_lexer_test)

build-sparql-parser-test:
	@(cd $(top_builddir)/src ; $(MAKE) sparql_parser_test)

check-local: build-roqet build-sparql-lexer-test build-sparql-parser-test
	@$(PERL) $(srcdir)/../../improve .

get-testsuites-list:
	@echo "sparql-lexer sparql-parser sparql-query"

get-testsuite-sparql-lexer get-testsuite-sparql-parser:
	@tests="$(SPARQL_TEST_FILES)"; \
	type=`echo $@ | sed -e 's/^.*sparql-//' -e 's/er$$//'`; \
	verb=`echo $$type`ing; \
	prog=sparql_`echo $$type`er_test; \
	$(ECHO) '@prefix rdfs:	<http://www.w3.org/2000/01/rdf-schema#> .'; \
	$(ECHO) '@prefix mf:     <http://www.w3.org/2001/sw/DataAccess/tests/test-manifest#> .'; \
	$(ECHO) '@prefix t:     <http://ns.librdf.org/2009/test-manifest#> .'; \
	$(ECHO) ' '; \
	$(ECHO) "<> a mf:Manifest; rdfs:comment \"SPARQL legal $$verb\"; mf:entries ("; \
	for test in $$tests; do \
	  comment="sparql $$type of $$test"; \
	  $(ECHO) "  [ a t:PositiveTest; mf:name \"$$test\"; rdfs:comment \"$$comment\"; mf:action  \"$(top_builddir)/src/$$prog $(srcdir)/$$test\" ]"; \
	done; \
	$(ECHO) ")."

get-testsuite-sparql-query:
	@tests="$(SPARQL_TEST_NAMES)"; \
	$(ECHO) '@prefix rdfs:	<http://www.w3.org/2000/01/rdf-schema#> .'; \
	$(ECHO) '@prefix mf:     <http://www.w3.org/2001/sw/DataAccess/tests/test-manifest#> .'; \
	$(ECHO) '@prefix t:     <http://ns.librdf.org/2009/test-manifest#> .'; \
	$(ECHO) ' '; \
	$(ECHO) "<> a mf:Manifest; rdfs:comment \"SPARQL Query Execution\"; t:path \"$(top_builddir)/utils:$(top_builddir)@RAPPER_PATH@:$$PATH\"; t:precondition \"$(PERL) -MXML::DOM -e 'exit 0'\"; mf:entries ("; \
	for test in $$tests; do \
	  comment="sparql query $$test"; \
	  expect="PositiveTest"; \
	  t=`echo $$test`; \
	  $(ECHO) "  [ a t:$$expect; mf:name \"$$test\"; rdfs:comment \"$$comment\"; mf:action  \"$(PERL) $(srcdir)/../check-sparql -s $(srcdir) $$t\" ]"; \
	done; \
	$(ECHO) ")."
