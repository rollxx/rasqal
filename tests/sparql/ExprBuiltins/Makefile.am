# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Rasqal syntax SPARQL tests
#
# Copyright (C) 2005-2008, David Beckett http://www.dajobe.org/
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
data-builtin-1.ttl \
data-langMatches.ttl

SPARQL_TEST_FILES= \
q-blank-1.rq q-datatype-1.rq q-str-1.rq q-str-2.rq q-str-3.rq \
q-str-4.rq q-uri-1.rq \
q-langMatches-1.rq q-langMatches-2.rq q-langMatches-3.rq q-langMatches-4.rq \
q-iri-1.rq q-uri-1.rq

EXPECTED_SPARQL_CORRECT= \
  str-1 \
  str-2 \
  str-3 \
  str-4 \
  isBlank-1 \
  isURI-1 \
  isIRI-1 \
  LangMatches-1 \
  LangMatches-2 \
  LangMatches-3 \
  LangMatches-4

# datatype-1 - relies on lexical form of double being same as in .ttl
EXPECTED_SPARQL_CORRECT_FAILURES= \
datatype-1

SPARQL_RESULT_FILES= \
result-blank-1.ttl result-datatype-1.ttl result-str-1.ttl \
result-str-2.ttl result-str-3.ttl result-str-4.ttl result-uri-1.ttl \
result-langMatches-1.ttl result-langMatches-2.ttl \
result-langMatches-3.ttl result-langMatches-4.ttl \
result-iri-1.ttl result-uri-1.ttl

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
	@type=`echo $@ | sed -e 's/^.*sparql-//' -e 's/er$$//'`; \
	verb=`echo $$type`ing; \
	prog=sparql_`echo $$type`er_test; \
	$(ECHO) '@prefix rdfs:	<http://www.w3.org/2000/01/rdf-schema#> .'; \
	$(ECHO) '@prefix mf:     <http://www.w3.org/2001/sw/DataAccess/tests/test-manifest#> .'; \
	$(ECHO) '@prefix t:     <http://ns.librdf.org/2009/test-manifest#> .'; \
	$(ECHO) ' '; \
	$(ECHO) "<> a mf:Manifest; rdfs:comment \"SPARQL legal $$verb\"; mf:entries ("; \
	for test in $(SPARQL_TEST_FILES); do \
	  comment="sparql $$type of $$test"; \
	  $(ECHO) "  [ a t:PositiveTest; mf:name \"$$test\"; rdfs:comment \"$$comment\"; mf:action  \"$(top_builddir)/src/$$prog $(srcdir)/$$test\" ]"; \
	done; \
	$(ECHO) ")."

get-testsuite-sparql-query:
	@$(ECHO) '@prefix rdfs:	<http://www.w3.org/2000/01/rdf-schema#> .'; \
	$(ECHO) '@prefix mf:     <http://www.w3.org/2001/sw/DataAccess/tests/test-manifest#> .'; \
	$(ECHO) '@prefix t:     <http://ns.librdf.org/2009/test-manifest#> .'; \
	$(ECHO) ' '; \
	$(ECHO) "<> a mf:Manifest; rdfs:comment \"SPARQL Query Execution\"; t:path \"$(top_builddir)/utils:$(top_builddir)@RAPPER_PATH@:$$PATH\"; t:precondition \"$(PERL) -MXML::DOM -e 'exit 0'\"; mf:entries ("; \
	for test in $(EXPECTED_SPARQL_CORRECT); do \
	  comment="sparql query $$test"; \
	  $(ECHO) "  [ a t:PositiveTest; mf:name \"$$test\"; rdfs:comment \"$$comment\"; mf:action  \"$(PERL) $(srcdir)/../check-sparql -s $(srcdir) '$$test'\" ]"; \
	done; \
	for test in $(EXPECTED_SPARQL_CORRECT_FAILURES); do \
	  comment="sparql query $$test"; \
	  $(ECHO) "  [ a t:NegativeTest; mf:name \"$$test\"; rdfs:comment \"$$comment\"; mf:action  \"$(PERL) $(srcdir)/../check-sparql -s $(srcdir) '$$test'\" ]"; \
	done; \
	$(ECHO) ")."
