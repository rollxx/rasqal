/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * srxread.c - SPARQL Results XML Format reading test program
 *
 * Copyright (C) 2007-2008, David Beckett http://www.dajobe.org/
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
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif


#include <raptor.h>

/* Rasqal includes */
#include <rasqal.h>

#ifdef RAPTOR_V2_AVAILABLE
#else
#define raptor_new_uri(world, string) raptor_new_uri(string)
#define raptor_new_iostream_from_filename(world, filename) raptor_new_iostream_from_filename(filename)
#define raptor_new_iostream_to_file_handle(world, fh) raptor_new_iostream_to_file_handle(fh)
#endif


static char *program = NULL;

int main(int argc, char *argv[]);

int
main(int argc, char *argv[]) 
{ 
  int rc = 0;
  const char* srx_filename = NULL;
  raptor_iostream* iostr = NULL;
  char* p;
  unsigned char* uri_string = NULL;
  raptor_uri* base_uri = NULL;
  rasqal_query_results* results = NULL;
  const char* read_formatter_name = NULL;
  const char* write_formatter_name = NULL;
  rasqal_query_results_formatter* read_formatter = NULL;
  rasqal_query_results_formatter* write_formatter = NULL;
  raptor_iostream *write_iostr = NULL;
  rasqal_world *world;
  rasqal_variables_table* vars_table;
#ifdef RAPTOR_V2_AVAILABLE
  raptor_world *raptor_world_ptr;
#endif
  
  program = argv[0];
  if((p=strrchr(program, '/')))
    program = p+1;
  else if((p=strrchr(program, '\\')))
    program = p+1;
  argv[0] = program;
  
  world = rasqal_new_world();
  if(!world || rasqal_world_open(world)) {
    fprintf(stderr, "%s: rasqal_world init failed\n", program);
    return(1);
  }

  if(argc < 2 || argc > 4) {
    fprintf(stderr, "USAGE: %s SRX file [read formatter] [write formatter]\n",
            program);

    rc = 1;
    goto tidy;
  }

  srx_filename = argv[1];
  if(argc > 2) {
    if(strcmp(argv[2], "-"))
      read_formatter_name = argv[2];
    if(argc > 3) {
      if(strcmp(argv[3], "-"))
        write_formatter_name = argv[3];
    }
  }

#ifdef RAPTOR_V2_AVAILABLE
  raptor_world_ptr = rasqal_world_get_raptor(world);
#endif
  
  uri_string = raptor_uri_filename_to_uri_string((const char*)srx_filename);
  if(!uri_string)
    goto tidy;
  
  base_uri = raptor_new_uri(raptor_world_ptr, uri_string);
  raptor_free_memory(uri_string);

  vars_table = rasqal_new_variables_table(world);
  results = rasqal_new_query_results(world, NULL,
                                     RASQAL_QUERY_RESULTS_BINDINGS, vars_table);
  rasqal_free_variables_table(vars_table);
  if(!results) {
    fprintf(stderr, "%s: Failed to create query results\n", program);
    rc = 1;
    goto tidy;
  }
  
  iostr = raptor_new_iostream_from_filename(raptor_world_ptr, srx_filename);
  if(!iostr) {
    fprintf(stderr, "%s: Failed to open iostream to file %s\n", program,
            srx_filename);
    rc = 1;
    goto tidy;
  }

  read_formatter = rasqal_new_query_results_formatter2(world,
                                                       read_formatter_name,
                                                       NULL, NULL);
  if(!read_formatter) {
    fprintf(stderr, "%s: Failed to create query results read formatter '%s'\n",
            program, read_formatter_name);
    rc = 1;
    goto tidy;
  }
  
  rc = rasqal_query_results_formatter_read(world, iostr, read_formatter,
                                           results, base_uri);
  if(rc)
    goto tidy;

  write_formatter = rasqal_new_query_results_formatter2(world, 
                                                        write_formatter_name,
                                                        NULL, NULL);
  if(!write_formatter) {
    fprintf(stderr, "%s: Failed to create query results write formatter '%s'\n",
            program, write_formatter_name);
    rc = 1;
    goto tidy;
  }
  
  write_iostr = raptor_new_iostream_to_file_handle(raptor_world_ptr, stdout);
  if(!write_iostr) {
    fprintf(stderr, "%s: Creating output iostream failed\n", program);
  } else {
    rasqal_query_results_formatter_write(write_iostr, write_formatter,
                                         results, base_uri);
    raptor_free_iostream(write_iostr);
  }


  tidy:
  if(write_formatter)
    rasqal_free_query_results_formatter(write_formatter);

  if(read_formatter)
    rasqal_free_query_results_formatter(read_formatter);

  if(iostr)
    raptor_free_iostream(iostr);
  
  if(results)
    rasqal_free_query_results(results);

  if(base_uri)
    raptor_free_uri(base_uri);

  rasqal_free_world(world);

  return (rc);
}
