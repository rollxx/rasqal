# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Rasqal SPARQL tests - examples
#
# Copyright (C) 2005-2009, David Beckett http://www.dajobe.org/
# Copyright (C) 2005-2005, University of Bristol, UK http://www.bristol.ac.uk/
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

SPARQL_MANIFEST_FILES= manifest.ttl

SPARQL_MODEL_FILES= \
survey-sample.ttl

SPARQL_TEST_FILES= \
query-survey-1.rq query-survey-2.rq query-survey-3.rq	\
query-survey-8.rq query-survey-9.rq query-survey-10.rq	\
query-survey-11.rq query-survey-12.rq query-survey-13.rq

SPARQL_RESULT_FILES= \
result-survey-1.ttl result-survey-2.ttl result-survey-3.ttl	\
result-survey-8.ttl result-survey-9.ttl result-survey-10.ttl	\
result-survey-11.ttl result-survey-12.ttl result-survey-13.ttl

EXPECTED_SPARQL_CORRECT= \
   survey-1 \
   survey-2 \
   survey-8 \
   survey-9 \
   survey-10 \
   survey-11 \
   survey-12 \
   survey-13

# Test failures
# survey-3
#   UNION
EXPECTED_SPARQL_CORRECT_FAILURES= \
survey-3

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
	for test in $(EXPECTED_SPARQL_CORRECT); do \
	  comment="sparql query $$test"; \
	  expect="PositiveTest"; \
	  t=`echo $$test`; \
	  $(ECHO) "  [ a t:$$expect; mf:name \"$$test\"; rdfs:comment \"$$comment\"; mf:action  \"$(PERL) $(srcdir)/../check-sparql -s $(srcdir) $$t\" ]"; \
	done; \
	for test in $(EXPECTED_SPARQL_CORRECT_FAILURES); do \
	  comment="sparql query $$test"; \
	  expect="NegativeTest"; \
	  t=`echo $$test`; \
	  $(ECHO) "  [ a t:$$expect; mf:name \"$$test\"; rdfs:comment \"$$comment\"; mf:action  \"$(PERL) $(srcdir)/../check-sparql -s $(srcdir) $$t\" ]"; \
	done; \
	$(ECHO) ")."
