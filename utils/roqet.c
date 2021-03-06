/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * roqet.c - Rasqal RDF Query utility
 *
 * Copyright (C) 2004-2010, David Beckett http://www.dajobe.org/
 * Copyright (C) 2004-2005, University of Bristol, UK http://www.bristol.ac.uk/
 * 
 * This package is Free Software and part of Redland http://librdf.org/
 * 
 * It is licensed under the following three licenses as alternatives:
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 * 
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * complete terms and further detail along with the license texts for
 * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
 * 
 * 
 */


#ifdef HAVE_CONFIG_H
#include <rasqal_config.h>
#endif

#ifdef WIN32
#include <win32_rasqal_config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
/* for access() and R_OK */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifndef HAVE_GETOPT
#include <rasqal_getopt.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* Rasqal includes */
#include <rasqal.h>
#ifdef RASQAL_INTERNAL
#include <rasqal_internal.h>
#endif


#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif

int main(int argc, char *argv[]);


static char *program=NULL;


#ifdef HAVE_GETOPT_LONG
#define HELP_TEXT(short, long, description) "  -" short ", --" long "  " description
#define HELP_TEXT_LONG(long, description) "      --" long "  " description
#define HELP_ARG(short, long) "--" #long
#define HELP_PAD "\n                            "
#else
#define HELP_TEXT(short, long, description) "  -" short "  " description
#define HELP_TEXT_LONG(long, description)
#define HELP_ARG(short, long) "-" #short
#define HELP_PAD "\n      "
#endif

#ifdef RASQAL_INTERNAL
/* add 'g:' */
#define GETOPT_STRING "cd:D:e:f:F:g:G:hi:np:r:qs:vw"
#else
#define GETOPT_STRING "cd:D:e:f:F:G:hi:np:r:qs:vw"
#endif

#ifdef HAVE_GETOPT_LONG

#ifdef RASQAL_INTERNAL
#define STORE_RESULTS_FLAG 0x100
#endif

static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"count", 0, 0, 'c'},
  {"dump-query", 1, 0, 'd'},
  {"data", 1, 0, 'D'},
  {"exec", 1, 0, 'e'},
  {"feature", 1, 0, 'f'},
  {"format", 1, 0, 'F'},
#ifdef RASQAL_INTERNAL
  {"engine", 1, 0, 'g'},
#endif
  {"named", 1, 0, 'G'},
  {"help", 0, 0, 'h'},
  {"input", 1, 0, 'i'},
  {"dryrun", 0, 0, 'n'},
  {"protocol", 0, 0, 'p'},
  {"quiet", 0, 0, 'q'},
  {"results", 1, 0, 'r'},
  {"source", 1, 0, 's'},
  {"version", 0, 0, 'v'},
  {"walk-query", 0, 0, 'w'},
#ifdef STORE_RESULTS_FLAG
  {"store-results", 1, 0, STORE_RESULTS_FLAG},
#endif
  {NULL, 0, 0, 0}
};
#endif


static int error_count = 0;

static const char *title_format_string = "Rasqal RDF query utility %s\n";

#ifdef BUFSIZ
#define FILE_READ_BUF_SIZE BUFSIZ
#else
#define FILE_READ_BUF_SIZE 1024
#endif

#define MAX_QUERY_ERROR_REPORT_LEN 512


#ifdef HAVE_RAPTOR2_API
static void
roqet_log_handler(void* user_data, raptor_log_message *message)
{
  /* Only interested in errors and more severe */
  if(message->level < RAPTOR_LOG_LEVEL_ERROR)
    return;

  fprintf(stderr, "%s: Error - ", program);
  raptor_locator_print(message->locator, stderr);
  fprintf(stderr, " - %s\n", message->text);

  error_count++;
}
#else
static void
roqet_error_handler(void *user_data, 
                    raptor_locator* locator, const char *message) 
{
  fprintf(stderr, "%s: Error - ", program);
  raptor_print_locator(stderr, locator);
  fprintf(stderr, " - %s\n", message);

  error_count++;
}
#endif

#define SPACES_LENGTH 80
static const char spaces[SPACES_LENGTH + 1] = "                                                                                ";

static void
roqet_write_indent(FILE *fh, int indent) 
{
  while(indent > 0) {
    int sp = (indent > SPACES_LENGTH) ? SPACES_LENGTH : indent;

    (void)fwrite(spaces, sizeof(char), sp, fh);
    indent -= sp;
  }
}

  

static void
roqet_graph_pattern_walk(rasqal_graph_pattern *gp, int gp_index,
                         FILE *fh, int indent) {
  int triple_index = 0;
  rasqal_graph_pattern_operator op;
  int seen;
  raptor_sequence *seq;
  int idx;
  rasqal_expression* expr;
  rasqal_variable* var;
  rasqal_literal* literal;
  
  op = rasqal_graph_pattern_get_operator(gp);
  
  roqet_write_indent(fh, indent);
  fprintf(fh, "%s graph pattern", 
          rasqal_graph_pattern_operator_as_string(op));
  idx = rasqal_graph_pattern_get_index(gp);

  if(idx >= 0)
    fprintf(fh, "[%d]", idx);

  if(gp_index >= 0)
    fprintf(fh, " #%d", gp_index);
  fputs(" {\n", fh);
  
  indent += 2;

  /* look for LET variable and value */
  var = rasqal_graph_pattern_get_variable(gp);
  if(var) {
    roqet_write_indent(fh, indent);
    fprintf(fh, "%s := ", var->name);
    rasqal_expression_print(var->expression, fh);
  }

  /* look for GRAPH literal */
  literal = rasqal_graph_pattern_get_origin(gp);
  if(literal) {
    roqet_write_indent(fh, indent);
    fputs("origin ", fh);
    rasqal_literal_print(literal, fh);
    fputc('\n', fh);
  }
  
  /* look for SERVICE literal */
  literal = rasqal_graph_pattern_get_service(gp);
  if(literal) {
    roqet_write_indent(fh, indent);
    rasqal_literal_print(literal, fh);
    fputc('\n', fh);
  }
  

  /* look for triples */
  seen = 0;
  while(1) {
    rasqal_triple* t;

    t = rasqal_graph_pattern_get_triple(gp, triple_index);
    if(!t)
      break;
    
    if(!seen) {
      roqet_write_indent(fh, indent);
      fputs("triples {\n", fh);
      seen = 1;
    }

    roqet_write_indent(fh, indent + 2);
    fprintf(fh, "triple #%d { ", triple_index);
    rasqal_triple_print(t, fh);
    fputs(" }\n", fh);

    triple_index++;
  }
  if(seen) {
    roqet_write_indent(fh, indent);
    fputs("}\n", fh);
  }


  /* look for sub-graph patterns */
  seq = rasqal_graph_pattern_get_sub_graph_pattern_sequence(gp);
  if(seq && raptor_sequence_size(seq) > 0) {
    roqet_write_indent(fh, indent);
    fprintf(fh, "sub-graph patterns (%d) {\n", raptor_sequence_size(seq));

    gp_index = 0;
    while(1) {
      rasqal_graph_pattern* sgp;
      sgp = rasqal_graph_pattern_get_sub_graph_pattern(gp, gp_index);
      if(!sgp)
        break;
      
      roqet_graph_pattern_walk(sgp, gp_index, fh, indent + 2);
      gp_index++;
    }

    roqet_write_indent(fh, indent);
    fputs("}\n", fh);
  }
  

  /* look for filter */
  expr = rasqal_graph_pattern_get_filter_expression(gp);
  if(expr) {
    roqet_write_indent(fh, indent);
    fputs("filter { ", fh);
    rasqal_expression_print(expr, fh);
    fputs("}\n", fh);
  }
  

  indent -= 2;
  
  roqet_write_indent(fh, indent);
  fputs("}\n", fh);
}


    
static void
roqet_query_write_variable(FILE* fh, rasqal_variable* v)
{
  fputs((const char*)v->name, fh);
  if(v->expression) {
    fputc('=', fh);
    rasqal_expression_print(v->expression, fh);
  }
}


static void
roqet_query_walk(rasqal_query *rq, FILE *fh, int indent) {
  rasqal_query_verb verb;
  int i;
  rasqal_graph_pattern* gp;
  raptor_sequence *seq;

  verb = rasqal_query_get_verb(rq);
  roqet_write_indent(fh, indent);
  fprintf(fh, "query verb: %s\n", rasqal_query_verb_as_string(verb));

  i = rasqal_query_get_distinct(rq);
  if(i != 0) {
    roqet_write_indent(fh, indent);
    fprintf(fh, "query asks for distinct results\n");
  }
  
  i = rasqal_query_get_limit(rq);
  if(i >= 0) {
    roqet_write_indent(fh, indent);
    fprintf(fh, "query asks for result limits %d\n", i);
  }
  
  i = rasqal_query_get_offset(rq);
  if(i >= 0) {
    roqet_write_indent(fh, indent);
    fprintf(fh, "query asks for result offset %d\n", i);
  }
  
  seq = rasqal_query_get_bound_variable_sequence(rq);
  if(seq && raptor_sequence_size(seq) > 0) {
    fprintf(fh, "query bound variables (%d): ", 
            raptor_sequence_size(seq));
    i = 0;
    while(1) {
      rasqal_variable* v = (rasqal_variable*)raptor_sequence_get_at(seq, i);
      if(!v)
        break;

      if(i > 0)
        fputs(", ", fh);

      roqet_query_write_variable(fh, v);
      i++;
    }
    fputc('\n', fh);
  }

  gp = rasqal_query_get_query_graph_pattern(rq);
  if(!gp)
    return;


  seq = rasqal_query_get_construct_triples_sequence(rq);
  if(seq && raptor_sequence_size(seq) > 0) {
    roqet_write_indent(fh, indent);
    fprintf(fh, "query construct triples (%d) {\n", 
            raptor_sequence_size(seq));
    i = 0;
    while(1) {
      rasqal_triple* t = rasqal_query_get_construct_triple(rq, i);
      if(!t)
        break;
    
      roqet_write_indent(fh, indent + 2);
      fprintf(fh, "triple #%d { ", i);
      rasqal_triple_print(t, fh);
      fputs(" }\n", fh);

      i++;
    }
    roqet_write_indent(fh, indent);
    fputs("}\n", fh);
  }

  /* look for binding rows */
  seq = rasqal_query_get_bindings_variables_sequence(rq);
  if(seq) {
    roqet_write_indent(fh, indent);
    fprintf(fh, "bindings variables (%d): ",  raptor_sequence_size(seq));
    
    i = 0;
    while(1) {
      rasqal_variable* v = rasqal_query_get_bindings_variable(rq, i);
      if(!v)
        break;

      if(i > 0)
        fputs(", ", fh);

      roqet_query_write_variable(fh, v);
      i++;
    }
    fputc('\n', fh);
    
    seq = rasqal_query_get_bindings_rows_sequence(rq);

    fprintf(fh, "bindings rows (%d) {\n", raptor_sequence_size(seq));
    i = 0;
    while(1) {
      rasqal_row* row;
      
      row = rasqal_query_get_bindings_row(rq, i);
      if(!row)
        break;
      
      roqet_write_indent(fh, indent + 2);
      fprintf(fh, "row #%d { ", i);
      rasqal_row_print(row, fh);
      fputs("}\n", fh);
      
      i++;
    }
  }


  fputs("query ", fh);
  roqet_graph_pattern_walk(gp, -1, fh, indent);
}


typedef enum {
  QUERY_OUTPUT_UNKNOWN,
  QUERY_OUTPUT_DEBUG,
  QUERY_OUTPUT_STRUCTURE,
  QUERY_OUTPUT_SPARQL,
  QUERY_OUTPUT_LAST = QUERY_OUTPUT_SPARQL
} query_output_format;

const char* query_output_format_labels[QUERY_OUTPUT_LAST + 1][2] = {
  { NULL, NULL },
  { "debug", "Debug query dump (output format may change)" },
  { "structure", "Query structure walk (output format may change)" },
  { "sparql", "SPARQL" }
};



static void
print_bindings_result_simple(rasqal_query_results *results,
                             FILE* output, int quiet, int count)
{
  if(!quiet)
    fprintf(stderr, "%s: Query has a variable bindings result\n", program);
  
  while(!rasqal_query_results_finished(results)) {
    if(!count) {
      int i;
      
      fputs("result: [", output);
      for(i = 0; i < rasqal_query_results_get_bindings_count(results); i++) {
        const unsigned char *name;
        rasqal_literal *value;

        name = rasqal_query_results_get_binding_name(results, i);
        value = rasqal_query_results_get_binding_value(results, i);
        
        if(i > 0)
          fputs(", ", output);

        fprintf(output, "%s=", name);

        if(value)
          rasqal_literal_print(value, output);
        else
          fputs("NULL", output);
      }
      fputs("]\n", output);
    }
    
    rasqal_query_results_next(results);
  }

  if(!quiet)
    fprintf(stderr, "%s: Query returned %d results\n", program, 
            rasqal_query_results_get_count(results));
}


static void
print_boolean_result_simple(rasqal_query_results *results,
                            FILE* output, int quiet)
{
  fprintf(stderr, "%s: Query has a boolean result: %s\n", program,
          rasqal_query_results_get_boolean(results) ? "true" : "false");
}


static int
print_graph_result(rasqal_query* rq,
                   rasqal_query_results *results,
                   raptor_world* raptor_world_ptr,
                   FILE* output,
                   const char* serializer_syntax_name, raptor_uri* base_uri,
                   int quiet)
{
  int triple_count = 0;
  rasqal_prefix* prefix;
  int i;
  raptor_serializer* serializer = NULL;
  
  if(!quiet)
    fprintf(stderr, "%s: Query has a graph result:\n", program);
  
  if(!raptor_world_is_serializer_name(raptor_world_ptr, serializer_syntax_name)) {
    fprintf(stderr, 
            "%s: invalid query result serializer name `%s' for `" HELP_ARG(r, results) "'\n",
            program, serializer_syntax_name);
    return 1;
  }

  serializer = raptor_new_serializer(raptor_world_ptr,
                                     serializer_syntax_name);
  if(!serializer) {
    fprintf(stderr, "%s: Failed to create raptor serializer type %s\n",
            program, serializer_syntax_name);
    return(1);
  }
  
  /* Declare any query namespaces in the output serializer */
#ifdef HAVE_RAPTOR2_API
  for(i = 0; (prefix = rasqal_query_get_prefix(rq, i)); i++)
    raptor_serializer_set_namespace(serializer, prefix->uri, prefix->prefix);
  raptor_serializer_start_to_file_handle(serializer, base_uri, output);
#else
  for(i = 0; (prefix = rasqal_query_get_prefix(rq, i)); i++)
    raptor_serialize_set_namespace(serializer, prefix->uri, prefix->prefix);
  raptor_serialize_start_to_file_handle(serializer, base_uri, output);
#endif
  
  while(1) {
    raptor_statement *rs = rasqal_query_results_get_triple(results);
    if(!rs)
      break;

#ifdef HAVE_RAPTOR2_API
    raptor_serializer_serialize_statement(serializer, rs);
#else
    raptor_serialize_statement(serializer, rs);
#endif
    triple_count++;
    
    if(rasqal_query_results_next_triple(results))
      break;
  }
  
#ifdef HAVE_RAPTOR2_API
  raptor_serializer_serialize_end(serializer);
#else
  raptor_serialize_end(serializer);
#endif
  raptor_free_serializer(serializer);
  
  if(!quiet)
    fprintf(stderr, "%s: Total %d triples\n", program, triple_count);

  return 0;
}


static int
print_formatted_query_results(rasqal_world* world,
                              rasqal_query_results* results,
                              raptor_world* raptor_world_ptr,
                              FILE* output,
                              const char* result_format,
                              raptor_uri* base_uri,
                              int quiet)
{
  raptor_iostream *iostr;
  rasqal_query_results_formatter* results_formatter;
  int rc = 0;
  
  results_formatter = rasqal_new_query_results_formatter2(world,
                                                          result_format,
                                                          NULL, NULL);
  if(!results_formatter) {
    fprintf(stderr, "%s: Invalid bindings result format `%s'\n",
            program, result_format);
    rc = 1;
    goto tidy;
  }
  

  iostr = raptor_new_iostream_to_file_handle(raptor_world_ptr, output);
  if(!iostr) {
    rasqal_free_query_results_formatter(results_formatter);
    rc = 1;
    goto tidy;
  }
  
  rc = rasqal_query_results_formatter_write(iostr, results_formatter,
                                            results, base_uri);
  raptor_free_iostream(iostr);
  rasqal_free_query_results_formatter(results_formatter);

  tidy:
  if(rc)
    fprintf(stderr, "%s: Formatting query results failed\n", program);

  return rc;
}



static rasqal_query_results*
roqet_call_sparql_service(rasqal_world* world, raptor_uri* service_uri,
                          const char* query_string,
                          raptor_sequence* data_graphs,
                          const char* format)
{
  rasqal_service* svc;
  rasqal_query_results* results;

  svc = rasqal_new_service(world, service_uri, query_string,
                           data_graphs);
  if(!svc) {
    fprintf(stderr, "%s: Failed to create service object\n", program);
    return NULL;
  }

  rasqal_service_set_format(svc, format);

  results = rasqal_service_execute(svc);

  rasqal_free_service(svc);

  return results;
}



static rasqal_query*
roqet_init_query(rasqal_world *world, 
                 const char* ql_name,
                 const char* ql_uri, const char* query_string,
                 raptor_uri* base_uri,
                 rasqal_feature query_feature, int query_feature_value,
                 const unsigned char* query_feature_string_value,
                 int store_results,
                 raptor_sequence* data_graphs)
{
  rasqal_query* rq;
  rasqal_query_results* results = NULL;

  rq = rasqal_new_query(world, (const char*)ql_name,
                        (const unsigned char*)ql_uri);
  if(!rq) {
    fprintf(stderr, "%s: Failed to create query name %s\n",
            program, ql_name);
    goto tidy_query;
  }
  
#ifdef HAVE_RAPTOR2_API
#else
  rasqal_query_set_error_handler(rq, world, roqet_error_handler);
  rasqal_query_set_fatal_error_handler(rq, world, roqet_error_handler);
#endif

  if(query_feature_value >= 0)
    rasqal_query_set_feature(rq, query_feature, query_feature_value);
  if(query_feature_string_value)
    rasqal_query_set_feature_string(rq, query_feature,
                                    query_feature_string_value);

#ifdef STORE_RESULTS_FLAG
  if(store_results >= 0)
    rasqal_query_set_store_results(rq, store_results);
#endif
  
  if(data_graphs) {
    rasqal_data_graph* dg;
    
    while((dg = (rasqal_data_graph*)raptor_sequence_pop(data_graphs))) {
      if(rasqal_query_add_data_graph2(rq, dg)) {
        fprintf(stderr, "%s: Failed to add data graph to query\n",
                program);
        rasqal_free_query_results(results); results = NULL;
        goto tidy_query;
      }
    }
  }

  if(rasqal_query_prepare(rq, (const unsigned char*)query_string, base_uri)) {
    size_t len = strlen((const char*)query_string);
    
    fprintf(stderr, "%s: Parsing query '", program);
    if(len > MAX_QUERY_ERROR_REPORT_LEN) {
      (void)fwrite(query_string, MAX_QUERY_ERROR_REPORT_LEN, sizeof(char),
                   stderr);
      fprintf(stderr, "...' (%d bytes) failed\n", (int)len);
    } else {
      (void)fwrite(query_string, len, sizeof(char), stderr);
      fputs("' failed\n", stderr);
    }

    rasqal_free_query(rq); rq = NULL;
    goto tidy_query;
  }

  tidy_query:
  return rq;
}


static
void roqet_print_query(rasqal_query* rq, 
                       raptor_world* raptor_world_ptr,
                       query_output_format output_format,
                       raptor_uri* base_uri)
{
  fprintf(stderr, "Query:\n");
  
  switch(output_format) {
    case QUERY_OUTPUT_DEBUG:
      rasqal_query_print(rq, stdout);
      break;
      
    case QUERY_OUTPUT_STRUCTURE:
      roqet_query_walk(rq, stdout, 0);
      break;
      
    case QUERY_OUTPUT_SPARQL:
      if(1) {
        raptor_iostream* output_iostr;
        output_iostr = raptor_new_iostream_to_file_handle(raptor_world_ptr,
                                                          stdout);
        rasqal_query_write(output_iostr, rq, NULL, base_uri);
        raptor_free_iostream(output_iostr);
      }
      break;
      
    case QUERY_OUTPUT_UNKNOWN:
    default:
      fprintf(stderr, "%s: Unknown query output format %d\n", program,
              output_format);
      abort();
  }
}



/* Default parser for input graphs */
#define DEFAULT_DATA_GRAPH_FORMAT "guess"
/* Default serializer for output graphs */
#define DEFAULT_GRAPH_FORMAT "ntriples"

int
main(int argc, char *argv[]) 
{ 
  int query_from_string = 0;
  void *query_string = NULL;
  unsigned char *uri_string = NULL;
  int free_uri_string = 0;
  unsigned char *base_uri_string = NULL;
  rasqal_query *rq = NULL;
  rasqal_query_results *results;
  const char *ql_name = "sparql";
  char *ql_uri = NULL;
  int rc = 0;
  raptor_uri *uri = NULL;
  raptor_uri *base_uri = NULL;
  char *filename = NULL;
  char *p;
  int usage = 0;
  int help = 0;
  int quiet = 0;
  int count = 0;
  int dryrun = 0;
  raptor_sequence* data_graphs = NULL;
  const char *result_format = NULL;
  query_output_format output_format = QUERY_OUTPUT_UNKNOWN;
  rasqal_feature query_feature = (rasqal_feature)-1;
  int query_feature_value= -1;
  unsigned char* query_feature_string_value = NULL;
  rasqal_world *world;
  raptor_world* raptor_world_ptr = NULL;
#ifdef RASQAL_INTERNAL
  int store_results = -1;
  const rasqal_query_execution_factory* engine = NULL;
#endif
  char* data_graph_parser_name = NULL;
  raptor_iostream* iostr = NULL;
  const unsigned char* service_uri_string = 0;
  raptor_uri* service_uri = NULL;
  
  program = argv[0];
  if((p = strrchr(program, '/')))
    program = p + 1;
  else if((p = strrchr(program, '\\')))
    program = p + 1;
  argv[0] = program;

  world = rasqal_new_world();
  if(!world || rasqal_world_open(world)) {
    fprintf(stderr, "%s: rasqal_world init failed\n", program);
    return(1);
  }
  
  raptor_world_ptr = rasqal_world_get_raptor(world);
#ifdef HAVE_RAPTOR2_API
  rasqal_world_set_log_handler(world, world, roqet_log_handler);
#endif
  
#ifdef STORE_RESULTS_FLAG
  /* This is for debugging only */
  if(1) {
    char* sr = getenv("RASQAL_DEBUG_STORE_RESULTS");
    if(sr)
      store_results = atoi(sr);
  }
#endif

  while (!usage && !help)
  {
    int c;
    
#ifdef HAVE_GETOPT_LONG
    int option_index = 0;

    c = getopt_long (argc, argv, GETOPT_STRING, long_options, &option_index);
#else
    c = getopt (argc, argv, GETOPT_STRING);
#endif
    if (c == -1)
      break;

    switch (c) {
      case 0:
      case '?': /* getopt() - unknown option */
        usage = 1;
        break;
        
      case 'c':
        count = 1;
        break;

      case 'd':
        output_format = QUERY_OUTPUT_UNKNOWN;
        if(optarg) {
          int i;

          for(i = 1; i <= QUERY_OUTPUT_LAST; i++)
            if(!strcmp(optarg, query_output_format_labels[i][0])) {
              output_format = (query_output_format)i;
              break;
            }
            
        }
        if(output_format == QUERY_OUTPUT_UNKNOWN) {
          int i;
          fprintf(stderr,
                  "%s: invalid argument `%s' for `" HELP_ARG(d, dump-query) "'\n",
                  program, optarg);
          for(i = 1; i <= QUERY_OUTPUT_LAST; i++)
            fprintf(stderr, 
                    "  %-12s for %s\n", query_output_format_labels[i][0],
                   query_output_format_labels[i][1]);
          usage = 1;
        }
        break;
        

      case 'e':
        if(optarg) {
          query_string = optarg;
          query_from_string = 1;
        }
        break;

      case 'f':
        if(optarg) {
          if(!strcmp(optarg, "help")) {
            int i;
            
            fprintf(stderr, "%s: Valid query features are:\n", program);
            for(i = 0; i < (int)rasqal_get_feature_count(); i++) {
              const char *feature_name;
              const char *feature_label;
              if(!rasqal_features_enumerate(world, (rasqal_feature)i,
                                            &feature_name, NULL,
                                            &feature_label)) {
                const char *feature_type;
                feature_type = (rasqal_feature_value_type((rasqal_feature)i) == 0) ? "" : " (string)";
                fprintf(stderr, "  %-20s  %s%s\n", feature_name, feature_label, 
                       feature_type);
              }
            }
            fputs("Features are set with `" HELP_ARG(f, feature) " FEATURE=VALUE or `-f FEATURE'\nand take a decimal integer VALUE except where noted, defaulting to 1 if omitted.\n", stderr);

            rasqal_free_world(world);
            exit(0);
          } else {
            int i;
            size_t arg_len = strlen(optarg);
            
            for(i = 0; i < (int)rasqal_get_feature_count(); i++) {
              const char *feature_name;
              size_t len;
              
              if(rasqal_features_enumerate(world, (rasqal_feature)i,
                                           &feature_name, NULL, NULL))
                continue;

              len = strlen(feature_name);

              if(!strncmp(optarg, feature_name, len)) {
                query_feature = (rasqal_feature)i;
                if(rasqal_feature_value_type(query_feature) == 0) {
                  if(len < arg_len && optarg[len] == '=')
                    query_feature_value=atoi(&optarg[len + 1]);
                  else if(len == arg_len)
                    query_feature_value = 1;
                } else {
                  if(len < arg_len && optarg[len] == '=')
                    query_feature_string_value = (unsigned char*)&optarg[len + 1];
                  else if(len == arg_len)
                    query_feature_string_value = (unsigned char*)"";
                }
                break;
              }
            }
            
            if(query_feature_value < 0 && !query_feature_string_value) {
              fprintf(stderr, "%s: invalid argument `%s' for `" HELP_ARG(f, feature) "'\nTry '%s " HELP_ARG(f, feature) " help' for a list of valid features\n",
                      program, optarg, program);
              usage = 1;
            }
          }
        }
        break;

      case 'F':
        if(optarg) {
          if(!raptor_world_is_parser_name(raptor_world_ptr, optarg)) {
              fprintf(stderr,
                      "%s: invalid parser name `%s' for `" HELP_ARG(F, format) "'\n\n",
                      program, optarg);
              usage = 1;
          } else {
            data_graph_parser_name = optarg;
          }
        }
        break;
        
      case 'h':
        help = 1;
        break;

      case 'n':
        dryrun = 1;
        break;

      case 'p':
        if(optarg)
          service_uri_string = (const unsigned char*)optarg;
        break;

      case 'r':
        if(optarg)
          result_format = optarg;
        break;

      case 'i':
        if(rasqal_language_name_check(world, optarg))
          ql_name = optarg;
        else {
          int i;

          fprintf(stderr,
                  "%s: invalid argument `%s' for `" HELP_ARG(i, input) "'\n",
                  program, optarg);
          fprintf(stderr, "Valid arguments are:\n");
          for(i = 0; 1; i++) {
            const char *help_name;
            const char *help_label;
            if(rasqal_languages_enumerate(world, i, 
                                          &help_name, &help_label, NULL))
              break;

            fprintf(stderr, "  %-12s for %s\n", help_name, help_label);
          }
          usage = 1;
        }
        break;

      case 'q':
        quiet = 1;
        break;

      case 's':
      case 'D':
      case 'G':
        if(optarg) {
          rasqal_data_graph *dg = NULL;
          rasqal_data_graph_flags type;

          type = (c == 's' || c == 'G') ? RASQAL_DATA_GRAPH_NAMED : 
                                          RASQAL_DATA_GRAPH_BACKGROUND;

          if(!strcmp((const char*)optarg, "-")) {
            /* stdin: use an iostream not a URI data graph */
            unsigned char* source_uri_string;
            raptor_uri* iostr_base_uri = NULL;
            raptor_uri* graph_name = NULL;
            
            /* FIXME - get base URI from somewhere else */
            source_uri_string = (unsigned char*)"file:///dev/stdin";

            iostr_base_uri = raptor_new_uri(raptor_world_ptr, source_uri_string);
            if(iostr_base_uri) {
              iostr = raptor_new_iostream_from_file_handle(raptor_world_ptr,
                                                           stdin);
              if(iostr)
                dg = rasqal_new_data_graph_from_iostream(world,
                                                         iostr, iostr_base_uri,
                                                         graph_name,
                                                         type,
                                                         NULL,
                                                         data_graph_parser_name,
                                                         NULL);
            }

            if(base_uri)
              raptor_free_uri(base_uri);
          } else if(!access((const char*)optarg, R_OK)) {
            /* file: use URI */
            unsigned char* source_uri_string;
            raptor_uri* source_uri;
            raptor_uri* graph_name = NULL;

            source_uri_string = raptor_uri_filename_to_uri_string((const char*)optarg);
            source_uri = raptor_new_uri(raptor_world_ptr, source_uri_string);
            raptor_free_memory(source_uri_string);

            if(type == RASQAL_DATA_GRAPH_NAMED) 
              graph_name = source_uri;
            
            if(source_uri)
              dg = rasqal_new_data_graph_from_uri(world,
                                                  source_uri,
                                                  graph_name,
                                                  type,
                                                  NULL, data_graph_parser_name,
                                                  NULL);

            if(source_uri)
              raptor_free_uri(source_uri);
          } else {
            raptor_uri* source_uri;
            raptor_uri* graph_name = NULL;

            /* URI: use URI */
            source_uri = raptor_new_uri(raptor_world_ptr,
                                        (const unsigned char*)optarg);
            if(type == RASQAL_DATA_GRAPH_NAMED) 
              graph_name = source_uri;
            
            if(source_uri)
              dg = rasqal_new_data_graph_from_uri(world,
                                                  source_uri,
                                                  graph_name,
                                                  type,
                                                  NULL, data_graph_parser_name,
                                                  NULL);

            if(source_uri)
              raptor_free_uri(source_uri);
          }
          
          if(!dg) {
            fprintf(stderr, "%s: Failed to create data graph for `%s'\n",
                    program, optarg);
            return(1);
          }
          
          if(!data_graphs) {
#ifdef HAVE_RAPTOR2_API
            data_graphs = raptor_new_sequence((raptor_data_free_handler)rasqal_free_data_graph,
                                              NULL);

#else
            data_graphs = raptor_new_sequence((raptor_sequence_free_handler*)rasqal_free_data_graph,
                                              NULL);
#endif
            if(!data_graphs) {
              fprintf(stderr, "%s: Failed to create data graphs sequence\n",
                      program);
              return(1);
            }
          }

          raptor_sequence_push(data_graphs, dg);
        }
        break;

      case 'v':
        fputs(rasqal_version_string, stdout);
        fputc('\n', stdout);
        rasqal_free_world(world);
        exit(0);

      case 'w':
        fprintf(stderr,
                "%s: WARNING: `-w' is deprecated.  Please use `" HELP_ARG(d, dump-query) " structure' instead.\n",
                program);
        output_format = QUERY_OUTPUT_STRUCTURE;
        break;

#ifdef STORE_RESULTS_FLAG
      case STORE_RESULTS_FLAG:
        store_results = (!strcmp(optarg, "yes") || !strcmp(optarg, "YES"));
        break;
#endif

#ifdef RASQAL_INTERNAL
      case 'g':
        engine = rasqal_query_get_engine_by_name(optarg);
        break;
#endif

    }
    
  }

  if(!help && !usage) {
    if(service_uri_string) {
      if(optind != argc && optind != argc-1)
        usage = 2; /* Title and usage */
    } else if(query_string) {
      if(optind != argc && optind != argc-1)
        usage = 2; /* Title and usage */
    } else {
      if(optind != argc-1 && optind != argc-2)
        usage = 2; /* Title and usage */
    }
  }

  
  if(usage) {
    if(usage > 1) {
      fprintf(stderr, title_format_string, rasqal_version_string);
      fputs("Rasqal home page: ", stderr);
      fputs(rasqal_home_url_string, stderr);
      fputc('\n', stderr);
      fputs(rasqal_copyright_string, stderr);
      fputs("\nLicense: ", stderr);
      fputs(rasqal_license_string, stderr);
      fputs("\n\n", stderr);
    }
    fprintf(stderr, "Try `%s " HELP_ARG(h, help) "' for more information.\n",
                    program);
    rasqal_free_world(world);

    exit(1);
  }

  if(help) {
    int i, j;
    
    printf(title_format_string, rasqal_version_string);
    puts("Run an RDF query giving variable bindings or RDF triples.");
    printf("Usage: %s [OPTIONS] <query URI> [base URI]\n", program);
    printf("       %s [OPTIONS] -e <query string> [base URI]\n", program);
    printf("       %s [OPTIONS] -p <SPARQL protocol service URI> -e <query string> [base URI]\n\n", program);

    fputs(rasqal_copyright_string, stdout);
    fputs("\nLicense: ", stdout);
    puts(rasqal_license_string);
    fputs("Rasqal home page: ", stdout);
    puts(rasqal_home_url_string);

    puts("\nNormal operation is to execute the query retrieved from URI <query URI>");
    puts("and print the results in a simple text format.");
    puts("\nMain options:");
    puts(HELP_TEXT("e", "exec QUERY      ", "Execute QUERY string instead of <query URI>"));
    puts(HELP_TEXT("p", "protocol URI    ", "Execute QUERY against a SPARQL protocol service URI"));
    puts(HELP_TEXT("i", "input LANGUAGE  ", "Set query language name to one of:"));
    for(i = 0; 1; i++) {
      const char *help_name;
      const char *help_label;

      if(rasqal_languages_enumerate(world, i, &help_name, &help_label, NULL))
        break;

      printf("    %-15s         %s", help_name, help_label);
      if(!i)
        puts(" (default)");
      else
        putchar('\n');
    }

    puts(HELP_TEXT("r", "results FORMAT  ", "Set query results output format to one of:"));
    puts("    For variable bindings and boolean results:");
    puts("      simple                A simple text format (default)");

#ifdef HAVE_RAPTOR2_API
    j = raptor_option_get_count();
#else
    j = raptor_get_feature_count();
#endif

    for(i = 0; i < j; i++) {
      const char *name;
      const char *label;
      int qr_flags = 0;

      if(!rasqal_query_results_formats_enumerate(world, i, &name, &label, 
                                                 NULL, NULL, &qr_flags) &&
         (qr_flags & RASQAL_QUERY_RESULTS_FORMAT_FLAG_WRITER))
        printf("      %-10s            %s\n", name, label);
    }

    puts("    For RDF graph results:");

    for(i = 0; 1; i++) {
#ifdef HAVE_RAPTOR2_API
      const raptor_syntax_description *desc;
      desc = raptor_world_get_parser_description(raptor_world_ptr, i);
      if(!desc)
        break;

      printf("      %-15s       %s", desc->names[0], desc->label);
#else
      const char *help_name;
      const char *help_label;
      if(raptor_serializers_enumerate(i, &help_name, &help_label, NULL, NULL))
        break;

      printf("      %-15s       %s", help_name, help_label);
#endif
      if(!i)
        puts(" (default)");
      else
        putchar('\n');
    }
    puts("\nAdditional options:");
    puts(HELP_TEXT("c", "count             ", "Count triples - no output"));
    puts(HELP_TEXT("d", "dump-query FORMAT ", "Print the parsed query out in FORMAT:"));
    puts(HELP_TEXT("D", "data URI          ", "RDF data source URI"));
    for(i = 1; i <= QUERY_OUTPUT_LAST; i++)
      printf("      %-15s         %s\n", query_output_format_labels[i][0],
             query_output_format_labels[i][1]);
    puts(HELP_TEXT("f FEATURE(=VALUE)", "feature FEATURE(=VALUE)", HELP_PAD "Set query features" HELP_PAD "Use `-f help' for a list of valid features"));
    puts(HELP_TEXT("F", "format NAME       ", "Set data source format name (default: guess)"));
#ifdef RASQAL_INTERNAL
    puts(HELP_TEXT("g", "engine NAME       ", "INTERNAL: Pick execution engine NAME"));
#endif
    puts(HELP_TEXT("G", "named URI         ", "RDF named graph data source URI"));
    puts(HELP_TEXT("h", "help              ", "Print this help, then exit"));
    puts(HELP_TEXT("n", "dryrun            ", "Prepare but do not run the query"));
    puts(HELP_TEXT("q", "quiet             ", "No extra information messages"));
    puts(HELP_TEXT("s", "source URI        ", "Same as `-G URI'"));
    puts(HELP_TEXT("v", "version           ", "Print the Rasqal version"));
    puts(HELP_TEXT("w", "walk-query        ", "Print query.  Same as '-d structure'"));
#ifdef STORE_RESULTS_FLAG
    puts(HELP_TEXT_LONG("store-results BOOL", "DEBUG: Set store results yes/no BOOL"));
#endif
    puts("\nReport bugs to http://bugs.librdf.org/");

    rasqal_free_world(world);
    
    exit(0);
  }


  if(service_uri_string) {
    service_uri = raptor_new_uri(raptor_world_ptr, service_uri_string);
    if(optind == argc-1) {
      base_uri_string = (unsigned char*)argv[optind];
    }
  } else if(query_string) {
    if(optind == argc-1)
      base_uri_string = (unsigned char*)argv[optind];
  } else {
    if(optind == argc-1)
      uri_string = (unsigned char*)argv[optind];
    else {
      uri_string = (unsigned char*)argv[optind++];
      base_uri_string = (unsigned char*)argv[optind];
    }
    
    /* If uri_string is "path-to-file", turn it into a file: URI */
    if(!strcmp((const char*)uri_string, "-")) {
      if(!base_uri_string) {
        fprintf(stderr, "%s: A Base URI is required when reading from standard input.\n",
                program);
        return(1);
      }

      uri_string = NULL;
    } else if(!access((const char*)uri_string, R_OK)) {
      filename = (char*)uri_string;
      uri_string = raptor_uri_filename_to_uri_string(filename);
      free_uri_string = 1;
    }
    
    if(uri_string) {
      uri = raptor_new_uri(raptor_world_ptr, uri_string);
      if(!uri) {
        fprintf(stderr, "%s: Failed to create URI for %s\n",
                program, uri_string);
        return(1);
      }
    } else
      uri = NULL; /* stdin */
  }

  if(!base_uri_string) {
    if(uri)
      base_uri = raptor_uri_copy(uri);
  } else
    base_uri = raptor_new_uri(raptor_world_ptr, base_uri_string);

  if(base_uri_string && !base_uri) {
    fprintf(stderr, "%s: Failed to create URI for %s\n",
            program, base_uri_string);
    return(1);
  }


  if(service_uri_string) {
    /* NOP - nothing to do here */
  } else if(query_string) {
    /* NOP - already got it */
  } else if(!uri_string) {
    query_string = calloc(FILE_READ_BUF_SIZE, 1);
    rc = fread(query_string, FILE_READ_BUF_SIZE, 1, stdin);
    if(ferror(stdin)) {
      fprintf(stderr, "%s: query string stdin read failed - %s\n",
              program, strerror(errno));
      return(1);
    }

    query_from_string = 0;
  } else if(filename) {
    raptor_stringbuffer *sb = raptor_new_stringbuffer();
    size_t len;
    FILE *fh;

    fh = fopen(filename, "r");
    if(!fh) {
      fprintf(stderr, "%s: file '%s' open failed - %s", 
              program, filename, strerror(errno));
      rc = 1;
      goto tidy_setup;
    }
    
    while(!feof(fh)) {
      unsigned char buffer[FILE_READ_BUF_SIZE];
      size_t read_len;

      read_len = fread((char*)buffer, 1, FILE_READ_BUF_SIZE, fh);
      if(read_len > 0)
        raptor_stringbuffer_append_counted_string(sb, buffer, read_len, 1);

      if(read_len < FILE_READ_BUF_SIZE) {
        if(ferror(fh)) {
          fprintf(stderr, "%s: file '%s' read failed - %s\n",
                  program, filename, strerror(errno));
          fclose(fh);
          return(1);
        }

        break;
      }
    }
    fclose(fh);

    len = raptor_stringbuffer_length(sb);
    query_string = malloc(len + 1);
    raptor_stringbuffer_copy_to_string(sb, (unsigned char*)query_string, len);
    raptor_free_stringbuffer(sb);
    query_from_string = 0;
  } else {
    raptor_www *www;

    www = raptor_new_www(raptor_world_ptr);
    if(www) {
#ifndef HAVE_RAPTOR2_API
      raptor_www_set_error_handler(www, roqet_error_handler, world);
#endif
      raptor_www_fetch_to_string(www, uri, &query_string, NULL, malloc);
#ifdef HAVE_RAPTOR2_API
      raptor_free_www(www);
#else
      raptor_www_free(www);
#endif
    }

    if(!query_string || error_count) {
      fprintf(stderr, "%s: Retrieving query at URI '%s' failed\n", 
              program, uri_string);
      rc = 1;
      goto tidy_setup;
    }

    query_from_string = 0;
  }


  if(!quiet) {
    if(service_uri) {
      fprintf(stderr, "%s: Calling SPARQL service at URI %s", program,
              service_uri_string);
      if(query_string)
        fprintf(stderr, " with query '%s'", (char*)query_string);

      if(base_uri_string)
        fprintf(stderr, " with base URI %s\n", base_uri_string);

      fputc('\n', stderr);
    } else if(query_from_string) {
      if(base_uri_string)
        fprintf(stderr, "%s: Running query '%s' with base URI %s\n", program,
                (char*)query_string, base_uri_string);
      else
        fprintf(stderr, "%s: Running query '%s'\n", program,
                (char*)query_string);
    } else if(filename) {
      if(base_uri_string)
        fprintf(stderr, "%s: Querying from file %s with base URI %s\n", program,
                filename, base_uri_string);
      else
        fprintf(stderr, "%s: Querying from file %s\n", program, filename);
    } else if(uri_string) {
      if(base_uri_string)
        fprintf(stderr, "%s: Querying URI %s with base URI %s\n", program,
                uri_string, base_uri_string);
      else
        fprintf(stderr, "%s: Querying URI %s\n", program, uri_string);
    }
  }
  


  if(service_uri) {
    /* Execute query remotely */
    if(!dryrun)
      results = roqet_call_sparql_service(world, service_uri, query_string,
                                          data_graphs,
                                          /* service_format */ NULL);
  } else {
    /* Execute query in this query engine (from URI or from -e QUERY) */
    rq = roqet_init_query(world,
                          ql_name, ql_uri, query_string,
                          base_uri,
                          query_feature, query_feature_value,
                          query_feature_string_value,
                          store_results,
                          data_graphs);

    if(!rq) {
      rc = 1;
      goto tidy_query;
    }

    if(output_format != QUERY_OUTPUT_UNKNOWN && !quiet)
      roqet_print_query(rq, raptor_world_ptr, output_format, base_uri);
    
    if(!dryrun) {
#ifdef RASQAL_INTERNAL
      results = rasqal_query_execute_with_engine(rq, engine);
#else
      results = rasqal_query_execute(rq);
#endif
    }

  }


  /* No results from dryrun */
  if(dryrun)
    goto tidy_query;
  
  if(!results) {
    fprintf(stderr, "%s: Query execution failed\n", program);
    rc = 1;
    goto tidy_query;
  }

  if(rasqal_query_results_is_bindings(results)) {
    if(result_format)
      rc = print_formatted_query_results(world, results,
                                         raptor_world_ptr, stdout,
                                         result_format, base_uri, quiet);
    else
      print_bindings_result_simple(results, stdout, quiet, count);
  } else if(rasqal_query_results_is_boolean(results)) {
    if(result_format)
      rc = print_formatted_query_results(world, results,
                                         raptor_world_ptr, stdout,
                                         result_format, base_uri, quiet);
    else
      print_boolean_result_simple(results, stdout, quiet);
  } else if(rasqal_query_results_is_graph(results)) {
    if(!result_format)
      result_format = DEFAULT_GRAPH_FORMAT;
    
    rc = print_graph_result(rq, results, raptor_world_ptr,
                            stdout, result_format, base_uri, quiet);
  } else {
    fprintf(stderr, "%s: Query returned unknown result format\n", program);
    rc = 1;
  }

  rasqal_free_query_results(results);
  
 tidy_query:
  if(!query_from_string)
    free(query_string);

 tidy_setup:

  if(data_graphs)
    raptor_free_sequence(data_graphs);
  if(base_uri)
    raptor_free_uri(base_uri);
  if(uri)
    raptor_free_uri(uri);
  if(free_uri_string)
    raptor_free_memory(uri_string);
  if(iostr)
    raptor_free_iostream(iostr);
  if(service_uri)
    raptor_free_uri(service_uri);

  rasqal_free_world(world);
  
  return (rc);
}
