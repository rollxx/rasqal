/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rasqal_internal.h - Rasqal RDF Query library internals
 *
 * Copyright (C) 2003-2010, David Beckett http://www.dajobe.org/
 * Copyright (C) 2003-2005, University of Bristol, UK http://www.bristol.ac.uk/
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
 */



#ifndef RASQAL_INTERNAL_H
#define RASQAL_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#define RASQAL_EXTERN_C extern "C"
#else
#define RASQAL_EXTERN_C
#endif

#ifdef RASQAL_INTERNAL

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define RASQAL_PRINTF_FORMAT(string_index, first_to_check_index) \
  __attribute__((__format__(__printf__, string_index, first_to_check_index)))
#else
#define RASQAL_PRINTF_FORMAT(string_index, first_to_check_index)
#endif

/* Can be over-ridden or undefined in a config.h file or -Ddefine */
#ifndef RASQAL_INLINE
#define RASQAL_INLINE inline
#endif

#ifdef LIBRDF_DEBUG
#define RASQAL_DEBUG 1
#endif

#if defined(RASQAL_MEMORY_SIGN)
#define RASQAL_SIGN_KEY 0x08A59A10
void* rasqal_sign_malloc(size_t size);
void* rasqal_sign_calloc(size_t nmemb, size_t size);
void* rasqal_sign_realloc(void *ptr, size_t size);
void rasqal_sign_free(void *ptr);
  
#define RASQAL_MALLOC(type, size)   rasqal_sign_malloc(size)
#define RASQAL_CALLOC(type, nmemb, size) rasqal_sign_calloc(nmemb, size)
#define RASQAL_REALLOC(type, ptr, size) rasqal_sign_realloc(ptr, size)
#define RASQAL_FREE(type, ptr)   rasqal_sign_free(ptr)

#else
#define RASQAL_MALLOC(type, size) malloc(size)
#define RASQAL_CALLOC(type, size, count) calloc(size, count)
#define RASQAL_FREE(type, ptr)   free((void*)ptr)

#endif

#ifdef RASQAL_DEBUG
/* Debugging messages */
#define RASQAL_DEBUG1(msg) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__); } while(0)
#define RASQAL_DEBUG2(msg, arg1) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1);} while(0)
#define RASQAL_DEBUG3(msg, arg1, arg2) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1, arg2);} while(0)
#define RASQAL_DEBUG4(msg, arg1, arg2, arg3) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1, arg2, arg3);} while(0)
#define RASQAL_DEBUG5(msg, arg1, arg2, arg3, arg4) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1, arg2, arg3, arg4);} while(0)
#define RASQAL_DEBUG6(msg, arg1, arg2, arg3, arg4, arg5) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, __func__, arg1, arg2, arg3, arg4, arg5);} while(0)

#if defined(HAVE_DMALLOC_H) && defined(RASQAL_MEMORY_DEBUG_DMALLOC)
void* rasqal_system_malloc(size_t size);
void rasqal_system_free(void *ptr);
#define SYSTEM_MALLOC(size)   rasqal_system_malloc(size)
#define SYSTEM_FREE(ptr)   rasqal_system_free(ptr)
#else
#define SYSTEM_MALLOC(size)   malloc(size)
#define SYSTEM_FREE(ptr)   free(ptr)
#endif

#ifndef RASQAL_ASSERT_DIE
#define RASQAL_ASSERT_DIE abort();
#endif

#else
/* DEBUGGING TURNED OFF */

/* No debugging messages */
#define RASQAL_DEBUG1(msg)
#define RASQAL_DEBUG2(msg, arg1)
#define RASQAL_DEBUG3(msg, arg1, arg2)
#define RASQAL_DEBUG4(msg, arg1, arg2, arg3)
#define RASQAL_DEBUG5(msg, arg1, arg2, arg3, arg4)
#define RASQAL_DEBUG6(msg, arg1, arg2, arg3, arg4, arg5)

#define SYSTEM_MALLOC(size)   malloc(size)
#define SYSTEM_FREE(ptr)   free(ptr)

#ifndef RASQAL_ASSERT_DIE
#define RASQAL_ASSERT_DIE
#endif

#endif


#ifdef RASQAL_DISABLE_ASSERT_MESSAGES
#define RASQAL_ASSERT_REPORT(line)
#else
#define RASQAL_ASSERT_REPORT(msg) fprintf(stderr, "%s:%d: (%s) assertion failed: " msg "\n", __FILE__, __LINE__, __func__);
#endif


#ifdef RASQAL_DISABLE_ASSERT

#define RASQAL_ASSERT(condition, msg) 
#define RASQAL_ASSERT_RETURN(condition, msg, ret) 
#define RASQAL_ASSERT_OBJECT_POINTER_RETURN(pointer, type) do { \
  if(!pointer) \
    return; \
} while(0)
#define RASQAL_ASSERT_OBJECT_POINTER_RETURN_VALUE(pointer, type, ret)

#else

#define RASQAL_ASSERT(condition, msg) do { \
  if(condition) { \
    RASQAL_ASSERT_REPORT(msg) \
    RASQAL_ASSERT_DIE \
  } \
} while(0)

#define RASQAL_ASSERT_RETURN(condition, msg, ret) do { \
  if(condition) { \
    RASQAL_ASSERT_REPORT(msg) \
    RASQAL_ASSERT_DIE \
    return ret; \
  } \
} while(0)

#define RASQAL_ASSERT_OBJECT_POINTER_RETURN(pointer, type) do { \
  if(!pointer) { \
    RASQAL_ASSERT_REPORT("object pointer of type " #type " is NULL.") \
    RASQAL_ASSERT_DIE \
    return; \
  } \
} while(0)

#define RASQAL_ASSERT_OBJECT_POINTER_RETURN_VALUE(pointer, type, ret) do { \
  if(!pointer) { \
    RASQAL_ASSERT_REPORT("object pointer of type " #type " is NULL.") \
    RASQAL_ASSERT_DIE \
    return ret; \
  } \
} while(0)

#endif


/* Fatal errors - always happen */
#define RASQAL_FATAL1(msg) do {fprintf(stderr, "%s:%d:%s: fatal error: " msg, __FILE__, __LINE__ , __func__); abort();} while(0)
#define RASQAL_FATAL2(msg,arg) do {fprintf(stderr, "%s:%d:%s: fatal error: " msg, __FILE__, __LINE__ , __func__, arg); abort();} while(0)
#define RASQAL_FATAL3(msg,arg1,arg2) do {fprintf(stderr, "%s:%d:%s: fatal error: " msg, __FILE__, __LINE__ , __func__, arg1, arg2); abort();} while(0)

#ifndef NO_STATIC_DATA
#define RASQAL_DEPRECATED_MESSAGE(msg) do {static int warning_given=0; if(!warning_given++) fprintf(stderr, "Function %s is deprecated - " msg,  __func__); } while(0)
#define RASQAL_DEPRECATED_WARNING(rq, msg) do {static int warning_given=0; if(!warning_given++) rasqal_query_warning(rq, msg); } while(0)
#else
#define RASQAL_DEPRECATED_MESSAGE(msg) do { fprintf(stderr, "Function %s is deprecated - " msg,  __func__); } while(0)
#define RASQAL_DEPRECATED_WARNING(rq, msg) do { rasqal_query_warning(rq, msg); } while(0)
#endif


typedef struct rasqal_query_execution_factory_s rasqal_query_execution_factory;
typedef struct rasqal_query_language_factory_s rasqal_query_language_factory;


/**
 * rasqal_projection:
 * @query: rasqal query
 * @wildcard: non-0 if @variables was '*'
 * @distinct: 1 if distinct, 2 if reduced, otherwise neither
 *
 * Query projection (SELECT vars, SELECT *, SELECT DISTINCT/REDUCED ...)
 *
 */
typedef struct {
  rasqal_query* query;
  
  raptor_sequence* variables;

  int wildcard:1;

  int distinct;
} rasqal_projection;  


/**
 * rasqal_solution_modifier:
 * @query: rasqal query
 * @order_conditions: sequence of order condition expressions (or NULL)
 * @group_conditions: sequence of group by condition expressions (or NULL)
 * @having_conditions: sequence of (group by ...) having condition expressions (or NULL)
 * @limit: result limit LIMIT (>=0) or <0 if not given
 * @offset: result offset OFFSET (>=0) or <0 if not given
 *
 * Query solution modifiers
 *
 */
typedef struct {
  rasqal_query* query;
  
  raptor_sequence* order_conditions;

  raptor_sequence* group_conditions;

  raptor_sequence* having_conditions;
  
  int limit;

  int offset;
} rasqal_solution_modifier;


/**
 * rasqal_bindings:
 * @query: rasqal query
 * @variables: sequence of variables
 * @rows: sequence of #rasqal_row (or NULL)
 *
 * Result bindings from SPARQL 1.1 BINDINGS block
 *
 */
typedef struct {
  rasqal_query* query;
  
  raptor_sequence* variables;

  raptor_sequence* rows;
} rasqal_bindings;  


/*
 * Graph Pattern
 */
struct rasqal_graph_pattern_s {
  rasqal_query* query;

  /* operator for this graph pattern's contents */
  rasqal_graph_pattern_operator op;
  
  raptor_sequence* triples;          /* ... rasqal_triple*         */
  raptor_sequence* graph_patterns;   /* ... rasqal_graph_pattern*  */

  int start_column;
  int end_column;

  /* the FILTER / LET expression */
  rasqal_expression* filter_expression;

  /* index of the graph pattern in the query (0.. query->graph_pattern_count-1) */
  int gp_index;

  /* Graph literal / SERVICE literal */
  rasqal_literal *origin;

  /* Variable for LET graph pattern */
  rasqal_variable *var;

  /* SELECT projection */
  rasqal_projection* projection;

  /* SELECT modifiers */
  rasqal_solution_modifier* modifier;
};

rasqal_graph_pattern* rasqal_new_basic_graph_pattern(rasqal_query* query, raptor_sequence* triples, int start_column, int end_column);
rasqal_graph_pattern* rasqal_new_graph_pattern_from_sequence(rasqal_query* query, raptor_sequence* graph_patterns, rasqal_graph_pattern_operator op);
rasqal_graph_pattern* rasqal_new_filter_graph_pattern(rasqal_query* query, rasqal_expression* expr);
rasqal_graph_pattern* rasqal_new_let_graph_pattern(rasqal_query *query, rasqal_variable *var, rasqal_expression *expr);  
rasqal_graph_pattern* rasqal_new_select_graph_pattern(rasqal_query *query, rasqal_projection* projection, rasqal_graph_pattern* where, rasqal_solution_modifier* modifier);
rasqal_graph_pattern* rasqal_new_single_graph_pattern(rasqal_query* query, rasqal_graph_pattern_operator op, rasqal_graph_pattern* single);
void rasqal_free_graph_pattern(rasqal_graph_pattern* gp);
void rasqal_graph_pattern_adjust(rasqal_graph_pattern* gp, int offset);
void rasqal_graph_pattern_set_origin(rasqal_graph_pattern* graph_pattern, rasqal_literal* origin);


/**
 * rasqal_var_use_map_flags:
 * @RASQAL_VAR_USE_IN_SCOPE: variable is in-scope in this GP
 * @RASQAL_VAR_USE_MENTIONED_HERE: variable is mentioned/used in this GP
 * @RASQAL_VAR_USE_BOUND_HERE: variable is bound in this GP
 */
typedef enum {
  RASQAL_VAR_USE_IN_SCOPE       = 1 << 0,
  RASQAL_VAR_USE_MENTIONED_HERE = 1 << 1,
  RASQAL_VAR_USE_BOUND_HERE     = 1 << 2
} rasqal_var_use_map_flags;


/**
 * rasqal_var_use_map_offset:
 * @RASQAL_VAR_USE_MAP_OFFSET_VERBS: Variables in query verbs: ASK: never, SELECT: project-expressions (SPARQL 1.1 TBD), CONSTRUCT: in constructed triple patterns, DESCRIBE: in argument (SPARQL 1.0)
 * @RASQAL_VAR_USE_MAP_OFFSET_GROUP_BY: Variables in GROUP BY expr/var (SPARQL 1.1 TBD)
 * @RASQAL_VAR_USE_MAP_OFFSET_HAVING: Variables in HAVING expr (SPARQL 1.1 TBD)
 * @RASQAL_VAR_USE_MAP_OFFSET_ORDER_BY: Variables in ORDER BY list-of-expr (SPARQL 1.0)
 * @RASQAL_VAR_USE_MAP_OFFSET_LAST: internal
 *
 * Offsets into variables use-map for non-graph pattern parts of #rasqal_query structure
 */
typedef enum {
  RASQAL_VAR_USE_MAP_OFFSET_VERBS    = 0,
  RASQAL_VAR_USE_MAP_OFFSET_GROUP_BY = 1,
  RASQAL_VAR_USE_MAP_OFFSET_HAVING   = 2,
  RASQAL_VAR_USE_MAP_OFFSET_ORDER_BY = 3,
  RASQAL_VAR_USE_MAP_OFFSET_LAST     = RASQAL_VAR_USE_MAP_OFFSET_ORDER_BY,
} rasqal_var_use_map_offset;


/* Magic (non-column number) values for query->variables_bound_in */
#define BOUND_IN_UNBOUND   -1
#define BOUND_IN_ELSEWHERE -2

/*
 * A query in some query language
 */
struct rasqal_query_s {
  rasqal_world* world; /* world object */

  int usage; /* reference count - 1 for itself, plus for query_results */
  
  unsigned char* query_string;
  int query_string_length; /* length including NULs */

  raptor_namespace_stack* namespaces;

  /* query graph pattern, containing the sequence of graph_patterns below */
  rasqal_graph_pattern* query_graph_pattern;
  
  /* the query verb - in SPARQL terms: SELECT, CONSTRUCT, DESCRIBE or ASK */
  rasqal_query_verb verb;
  
  /* sequences of ... */
  raptor_sequence* selects;     /* ... rasqal_variable* names only */
  raptor_sequence* data_graphs; /* ... rasqal_data_graph*          */
  /* NOTE: Cannot assume that triples are in any of 
   * graph pattern use / query execution / document order 
   */
  raptor_sequence* triples;     /* ... rasqal_triple*              */
  raptor_sequence* prefixes;    /* ... rasqal_prefix*              */
  raptor_sequence* constructs;  /* ... rasqal_triple*       SPARQL */
  raptor_sequence* optional_triples; /* ... rasqal_triple*  SPARQL */
  raptor_sequence* describes;   /* ... rasqal_literal* (var or URIs) SPARQL */

  /* DISTINCT mode:
   * 0 if not given
   * 1 if DISTINCT: ensure solutions are unique
   * 2 if SPARQL REDUCED: permit elimination of some non-unique solutions 
   * otherwise undefined
   */
  int distinct;

  /* WAS: result limit LIMIT (>=0) or <0 if not given */
  int unused4;

  /* WAS: result offset OFFSET (>=0) or <0 if not given */
  int unused5;

  /* non-0 if '*' was seen after a verb (the appropriate list such as selects or constructs will be NULL) */
  int wildcard;

  int prepared;

  rasqal_variables_table* vars_table;

  /* The number of selected variables: these are always the first
   * in the variables table and are the ones returend to the user.
   */
  int select_variables_count;

  /* array of size (number of total variables)
   * pointing to either:
   *   a) triple column where a variable[offset] is bound (0 or larger)
   *   b) -1 if not bound anywhere (BOUND_IN_UNBOUND)
   *   c) -2 if bound outside a triple (e.g. GRAPH, LET ) (BOUND_IN_ELSEWHERE)
   */
  int* variables_bound_in;

  /* can be filled with error location information */
  raptor_locator locator;

  /* base URI of this query for resolving relative URIs in queries */
  raptor_uri* base_uri;

  /* non 0 if query had fatal error in parsing and cannot be executed */
  int failed;

  /* stuff for our user */
  void* user_data;

  /* former state for generating blank node IDs; now in world object */
  int unused1;
  char *unused2;
  size_t unused3;

  void *generate_bnodeid_handler_user_data;
  rasqal_generate_bnodeid_handler generate_bnodeid_handler;


  /* query engine specific stuff */
  void* context;

  struct rasqal_query_language_factory_s* factory;

  rasqal_triples_source_factory* triples_source_factory;

  /* sequence of query results made from this query */
  raptor_sequence* results;

  /* incrementing counter for declaring prefixes in order of appearance */
  int prefix_depth;

  /* WAS: sequence of order condition expressions */
  void* unused6;

  /* WAS: sequence of group by condition expressions */
  void* unused7;

  /* INTERNAL rasqal_literal_compare / rasqal_expression_evaluate flags */
  int compare_flags;

  /* Number of graph patterns in this query */
  int graph_pattern_count;
  
  /* Graph pattern shared pointers by gp index (after prepare) */
  raptor_sequence* graph_patterns_sequence;

  /* Features */
  int features[RASQAL_FEATURE_LAST+1];

  /* Name of requested query results syntax.  If present, this
   * is the name used for constructing a rasqal_query_formatter
   * from the results.
   */
  const char* query_results_formatter_name;

  /* EXPLAIN was given */
  int explain;

  /* generated counter - increments at every generation */
  int genid_counter;

  /* INTERNAL lexer internal data */
  void* lexer_user_data;

  /* INTERNAL for now: non-0 to store results otherwise lazy eval results */
  int store_results;

  /* INTERNAL 2D array of:
   *   width (number of total variables)
   *   height (number of graph patterns)
   * marking how a variable is mentioned/used in a GP
   * Each graph pattern has a row and the short values are per-variable
   * with flags:
   *   1  RASQAL_USE_IN_SCOPE
   *   2  RASQAL_USE_MENTIONED_HERE
   *   4  RASQAL_USE_BOUND_HERE
   */
  short* variables_use_map;

  /* sequence of #rasqal_update_operation when @verb is
   * INSERT (deprecated), DELETE (deprecated) or UPDATE
   */
  raptor_sequence* updates;

  /* WAS: sequence of (group by ...) having condition expressions */
  raptor_sequence* unused8;
  
  /* INTERNAL solution modifier */
  rasqal_solution_modifier* modifier;

  /* INTERNAL SELECT bindings */
  rasqal_bindings* bindings;
};


/* Definitions from raptor V2 raptor.h - REMOVE when Raptor V2 is required */
#ifndef HAVE_RAPTOR2_API
typedef void (*raptor_data_free_handler)(void* data);
typedef void (*raptor_data_print_handler)(void *object, FILE *fh);

typedef struct {
  const char* mime_type;
  size_t mime_type_len;
  unsigned char q;
} raptor_type_q;


typedef struct {
  const char* const* names;

  const char* label;

  const raptor_type_q* mime_types;
  unsigned int mime_types_count;

  const char* uri_string;

  unsigned int flags;
} raptor_syntax_description;
#endif


/*
 * A query language factory for a query language.
 *
 * This structure is about turning a query syntax string into a
 * #rasqal_query structure.  It does not deal with execution of the
 * query in any manner.
 */
struct rasqal_query_language_factory_s {
  rasqal_world* world;
  
  struct rasqal_query_language_factory_s* next;

  /* static description that the query language registration initialises */
  raptor_syntax_description desc;
  
  /* the rest of this structure is populated by the
     query-language-specific register function */
  size_t context_length;
  
  /* create a new query */
  int (*init)(rasqal_query* rq, const char *name);
  
  /* destroy a query */
  void (*terminate)(rasqal_query* rq);
  
  /* prepare a query */
  int (*prepare)(rasqal_query* rq);
  
  /* finish the query language factory */
  void (*finish_factory)(rasqal_query_language_factory* factory);

  /* Write a string to an iostream in escaped form suitable for the query */
  int (*iostream_write_escaped_counted_string)(rasqal_query* rq, raptor_iostream* iostr, const unsigned char* string, size_t len);
};


typedef struct rasqal_rowsource_s rasqal_rowsource;

/*
 * A row of values from a query result, usually generated by a rowsource
 */
struct rasqal_row_s {
  /* reference count */
  int usage;

  /* Rowsource this row is associated with (or NULL if none) */
  rasqal_rowsource* rowsource;

  /* current row number in the sequence of rows*/
  int offset;

  /* values for each variable in the query sequence of values */
  int size;
  rasqal_literal** values;

  /* literal values for ORDER BY expressions evaluated for this row */
  /* number of expressions (can be 0) */
  int order_size;
  rasqal_literal** order_values;

  /* Group ID */
  int group_id;
};


typedef struct rasqal_map_s rasqal_map;

/**
 * rasqal_join_type:
 * @RASQAL_JOIN_TYPE_UNKNOWN: unknown join type
 * @RASQAL_JOIN_TYPE_NATURAL: natural join.  returns compatible rows and no NULLs
 * @RASQAL_JOIN_TYPE_LEFT: left join.  returns compatible rows plus rows from left rowsource that are not compatible or fail filter condition
 *
 * Rowsource join type.
 */
typedef enum {
  RASQAL_JOIN_TYPE_UNKNOWN,
  RASQAL_JOIN_TYPE_NATURAL,
  RASQAL_JOIN_TYPE_LEFT
} rasqal_join_type;



/* rasqal_rowsource_aggregation.c */
rasqal_rowsource* rasqal_new_aggregation_rowsource(rasqal_world *world, rasqal_query* query, rasqal_rowsource* rowsource, raptor_sequence* expr_seq, raptor_sequence* vars_seq);

/* rasqal_rowsource_empty.c */
rasqal_rowsource* rasqal_new_empty_rowsource(rasqal_world *world, rasqal_query* query);

/* rasqal_rowsource_engine.c */
rasqal_rowsource* rasqal_new_execution_rowsource(rasqal_query_results* query_results);

/* rasqal_rowsource_assignment.c */
rasqal_rowsource* rasqal_new_assignment_rowsource(rasqal_world *world, rasqal_query *query, rasqal_variable* var, rasqal_expression* expr);

/* rasqal_rowsource_distinct.c */
rasqal_rowsource* rasqal_new_distinct_rowsource(rasqal_world *world, rasqal_query *query, rasqal_rowsource* rs);

/* rasqal_rowsource_filter.c */
rasqal_rowsource* rasqal_new_filter_rowsource(rasqal_world *world, rasqal_query *query, rasqal_rowsource* rs, rasqal_expression* expr);

/* rasqal_rowsource_graph.c */
rasqal_rowsource* rasqal_new_graph_rowsource(rasqal_world *world, rasqal_query *query, rasqal_rowsource* rowsource, rasqal_variable *var);

/* rasqal_rowsource_groupby.c */
rasqal_rowsource* rasqal_new_groupby_rowsource(rasqal_world *world, rasqal_query* query, rasqal_rowsource* rowsource, raptor_sequence* expr_seq);

/* rasqal_rowsource_join.c */
rasqal_rowsource* rasqal_new_join_rowsource(rasqal_world *world, rasqal_query* query, rasqal_rowsource* left, rasqal_rowsource* right, rasqal_join_type join_type, rasqal_expression *expr);

/* rasqal_rowsource_project.c */
rasqal_rowsource* rasqal_new_project_rowsource(rasqal_world *world, rasqal_query *query, rasqal_rowsource* rowsource, raptor_sequence* projection_variables);

/* rasqal_rowsource_rowsequence.c */
rasqal_rowsource* rasqal_new_rowsequence_rowsource(rasqal_world *world, rasqal_query* query, rasqal_variables_table* vt, raptor_sequence* row, raptor_sequence* vars_seq);

/* rasqal_rowsource_sort.c */
rasqal_rowsource* rasqal_new_sort_rowsource(rasqal_world *world, rasqal_query *query, rasqal_rowsource *rowsource);

/* rasqal_rowsource_triples.c */
rasqal_rowsource* rasqal_new_triples_rowsource(rasqal_world *world, rasqal_query* query, rasqal_triples_source* triples_source, raptor_sequence* triples, int start_column, int end_column, int *bound_in, int bound_in_size);

/* rasqal_rowsource_union.c */
rasqal_rowsource* rasqal_new_union_rowsource(rasqal_world *world, rasqal_query* query, rasqal_rowsource* left, rasqal_rowsource* right);


/**
 * rasqal_rowsource_init_func:
 * @context: stream context data
 *
 * Handler function for #rasqal_rowsource initialising.
 *
 * Return value: non-0 on failure.
 */
typedef int (*rasqal_rowsource_init_func) (rasqal_rowsource* rowsource, void *user_data);

/**
 * rasqal_rowsource_finish_func:
 * @user_data: user data
 *
 * Handler function for #rasqal_rowsource terminating.
 *
 * Return value: non-0 on failure
 */
typedef int (*rasqal_rowsource_finish_func) (rasqal_rowsource* rowsource, void *user_data);

/**
 * rasqal_rowsource_ensure_variables_func
 * @rowsource: #rasqal_rowsource
 * @user_data: user data
 *
 * Handler function for ensuring rowsource variables fields are initialised
 *
 * Return value: non-0 on failure
 */
typedef int (*rasqal_rowsource_ensure_variables_func) (rasqal_rowsource* rowsource, void *user_data);

/**
 * rasqal_rowsource_read_row_func
 * @user_data: user data
 *
 * Handler function for returning the next result row
 *
 * Return value: a query result row or NULL if exhausted
 */
typedef rasqal_row* (*rasqal_rowsource_read_row_func) (rasqal_rowsource* rowsource, void *user_data);


/**
 * rasqal_rowsource_read_all_rows_func
 * @user_data: user data
 *
 * Handler function for returning all rows as a sequence
 *
 * Return value: a sequence of result rows (which may be size 0) or NULL if exhausted
 */
typedef raptor_sequence* (*rasqal_rowsource_read_all_rows_func) (rasqal_rowsource* rowsource, void *user_data);


/**
 * rasqal_rowsource_reset_func
 * @user_data: user data
 *
 * Handler function for resetting a rowsource to generate the same set of rows
 *
 * Return value: non-0 on failure
 */
typedef int (*rasqal_rowsource_reset_func) (rasqal_rowsource* rowsource, void *user_data);


/**
 * rasqal_rowsource_set_preserve_func
 * @user_data: user data
 * @preserve: flag
 *
 * Handler function for setting preserve binding states on a rowsource
 *
 * Return value: non-0 on failure
 */
typedef int (*rasqal_rowsource_set_preserve_func) (rasqal_rowsource* rowsource, void *user_data, int preserve);


/**
 * rasqal_rowsource_get_inner_rowsource_func
 * @user_data: user data
 * @offset: offset
 *
 * Handler function for getting an inner rowsource at an offset
 *
 * Return value: rowsource object or NULL if offset out of range
 */
typedef rasqal_rowsource* (*rasqal_rowsource_get_inner_rowsource_func) (rasqal_rowsource* rowsource, void *user_data, int offset);


/**
 * rasqal_rowsource_set_origin_func
 * @user_data: user data
 * @origin: Graph URI literal
 *
 * Handler function for setting the rowsource origin (GRAPH)
 *
 * Return value: non-0 on failure
 */
typedef int (*rasqal_rowsource_set_origin_func) (rasqal_rowsource* rowsource, void *user_data, rasqal_literal *origin);


/**
 * rasqal_rowsource_handler:
 * @version: API version - 1
 * @name: rowsource name for debugging
 * @init:  initialisation handler - optional, called at most once (V1)
 * @finish: finishing handler - optional, called at most once (V1)
 * @ensure_variables: update variables handler- optional, called at most once (V1)
 * @read_row: read row handler - this or @read_all_rows required (V1)
 * @read_all_rows: read all rows handler - this or @read_row required (V1)
 * @reset: reset rowsource to starting state handler - optional (V1)
 * @set_preserve: set preserve flag handler - optional (V1)
 * @get_inner_rowsource: get inner rowsource handler - optional if has no inner rowsources (V1)
 * @set_origin: set origin (GRAPH) handler - optional (V1)
 *
 * Row Source implementation factory handler structure.
 * 
 */
typedef struct {
  int version;
  const char* name;
  /* API V1 methods */
  rasqal_rowsource_init_func                 init;
  rasqal_rowsource_finish_func               finish;
  rasqal_rowsource_ensure_variables_func     ensure_variables;
  rasqal_rowsource_read_row_func             read_row;
  rasqal_rowsource_read_all_rows_func        read_all_rows;
  rasqal_rowsource_reset_func                reset;
  rasqal_rowsource_set_preserve_func         set_preserve;
  rasqal_rowsource_get_inner_rowsource_func  get_inner_rowsource;
  rasqal_rowsource_set_origin_func           set_origin;
} rasqal_rowsource_handler;


/**
 * rasqal_row_compatible:
 * @variables_table: variables table
 * @rowsource1: first rowsource
 * @rowsource2: second rowsource
 * @variables_count: number of variables in the map
 * @variables_in_both_rows_count: number of shared variables
 * @defined_in_map: of size @variables_count
 *
 * Lookup data constructed for two rowsources to enable quick
 * checking if rows from the two rowsource are compatible with
 * rasqal_row_compatible_check()
 *
 */
typedef struct {
  rasqal_variables_table* variables_table;
  rasqal_rowsource *first_rowsource;
  rasqal_rowsource *second_rowsource;
  int variables_count;
  int variables_in_both_rows_count;
  int* defined_in_map;
} rasqal_row_compatible;


/**
 * rasqal_rowsource:
 * @world: rasqal world
 * @query: query that this may be associated with (or NULL)
 * @flags: flags - none currently defined.
 * @user_data: rowsource handler data
 * @handler: rowsource handler pointer
 * @finished: non-0 if rowsource has been exhausted
 * @count:  number of rows returned
 * @updated_variables: non-0 if ensure_variables factory method has been called to get the variables_sequence updated
 * @vars_table: variables table where variables used in this row are declared/owned
 * @variables_sequence: variables declared in this row from @vars_table
 * @size: number of variables in @variables_sequence
 * @rows_sequence: stored sequence of rows for use by rasqal_rowsource_read_row() (or NULL)
 * @offset: size of @rows_sequence
 * @generate_group: non-0 to generate a group (ID 0) around all the returned rows, if there is no grouping returned.
 *
 * Rasqal Row Source class providing a sequence of rows of values similar to a SQL table.
 *
 * The table has @size columns which are #rasqal_variable names that
 * are declared in the associated variables table.
 * @variables_sequence contains the ordered projection of the
 * variables for the columns for this row sequence, from the full set
 * of variables in @vars_table.
 * 
 * Each row has @size #rasqal_literal values for the variables or
 * NULL if unset.
 *
 * Row sources are constructed indirectly via an @handler passed
 * to the rasqal_new_rowsource_from_handler() constructor.
 *
 * The main methods are rasqal_rowsource_read_row() to read one row
 * and rasqal_rowsource_read_all_rows() to get all rows as a
 * sequence, draining the row source.
 * rasqal_rowsource_get_rows_count() returns the current number of
 * rows that have been read which is only useful in the read one row
 * case.
 * 
 * The variables associated with a rowsource can be read by
 * rasqal_rowsource_get_variable_by_offset() and
 * rasqal_rowsource_get_variable_offset_by_name() which all are
 * offsets into @variables_sequence but refer to variables owned by
 * the full internal variables table @vars_table
 *
 * The @rows_sequence and @offset variables are used by the
 * rasqal_rowsource_read_row() function when operating over a handler
 * that will only return a full sequence: handler->read_all_rows is NULL.
 */
struct rasqal_rowsource_s
{
  rasqal_world* world;

  rasqal_query* query;
  
  int flags;
  
  void *user_data;

  const rasqal_rowsource_handler* handler;

  int finished : 1;

  int count;

  int updated_variables;

  rasqal_variables_table* vars_table;

  raptor_sequence* variables_sequence;
  
  int size;

  raptor_sequence* rows_sequence;

  int offset;

  int generate_group : 1;
};


/* rasqal_rowsource.c */
rasqal_rowsource* rasqal_new_rowsource_from_handler(rasqal_world *world, rasqal_query* query, void* user_data, const rasqal_rowsource_handler *handler, rasqal_variables_table* vars_table, int flags);
void rasqal_free_rowsource(rasqal_rowsource *rowsource);

rasqal_row* rasqal_rowsource_read_row(rasqal_rowsource *rowsource);
int rasqal_rowsource_get_rows_count(rasqal_rowsource *rowsource);
raptor_sequence* rasqal_rowsource_read_all_rows(rasqal_rowsource *rowsource);
int rasqal_rowsource_get_size(rasqal_rowsource *rowsource);
int rasqal_rowsource_add_variable(rasqal_rowsource *rowsource, rasqal_variable* v);
rasqal_variable* rasqal_rowsource_get_variable_by_offset(rasqal_rowsource *rowsource, int offset);
int rasqal_rowsource_get_variable_offset_by_name(rasqal_rowsource *rowsource, const unsigned char* name);
int rasqal_rowsource_copy_variables(rasqal_rowsource *dest_rowsource, rasqal_rowsource *src_rowsource);
void rasqal_rowsource_print_row_sequence(rasqal_rowsource* rowsource,raptor_sequence* seq, FILE* fh);
int rasqal_rowsource_reset(rasqal_rowsource* rowsource);
int rasqal_rowsource_set_preserve(rasqal_rowsource* rowsource, int preserve);
rasqal_rowsource* rasqal_rowsource_get_inner_rowsource(rasqal_rowsource* rowsource, int offset);
int rasqal_rowsource_write(rasqal_rowsource *rowsource,  raptor_iostream *iostr);
void rasqal_rowsource_print(rasqal_rowsource* rs, FILE* fh);
int rasqal_rowsource_ensure_variables(rasqal_rowsource *rowsource);
int rasqal_rowsource_set_origin(rasqal_rowsource* rowsource, rasqal_literal *literal);
int rasqal_rowsource_request_grouping(rasqal_rowsource* rowsource);

typedef int (*rasqal_query_results_formatter_func)(raptor_iostream *iostr, rasqal_query_results* results, raptor_uri *base_uri);

typedef rasqal_rowsource* (*rasqal_query_results_get_rowsource_func)(rasqal_world*, rasqal_variables_table* vars_table, raptor_iostream *iostr, raptor_uri *base_uri);

typedef int (*rasqal_rowsource_visit_fn)(rasqal_rowsource* rowsource, void *user_data);

int rasqal_rowsource_visit(rasqal_rowsource* rowsource, rasqal_rowsource_visit_fn fn, void *user_data);



typedef struct {
  /* query results format name */
  const char* name;

  /* query results format name */
  const char* label;

  /* query results format URI (or NULL) */
  const unsigned char* uri_string;

  /* format writer: READ from results, WRITE syntax (using base URI) to iostr */
  rasqal_query_results_formatter_func writer;

  /* format reader: READ syntax from iostr using base URI, WRITE to results */
  rasqal_query_results_formatter_func reader;

  /* format get rowsource: get a rowsource that will return a sequence of rows from an iostram */
  rasqal_query_results_get_rowsource_func get_rowsource;

  /* MIME Type of the format */
  const char* mime_type;
} rasqal_query_results_format_factory;


/*
 * A query results formatter for some query_results
 */
struct rasqal_query_results_formatter_s {
  rasqal_query_results_format_factory* factory;

  const char *mime_type;
};

typedef struct {
  rasqal_world *world;
  raptor_sequence *triples;
  rasqal_literal *value;
} rasqal_formula;


/* rasqal_datetime.c */
int rasqal_xsd_datetime_check(const unsigned char* string);
int rasqal_xsd_date_check(const unsigned char* string);


/* rasqal_general.c */
char* rasqal_vsnprintf(const char* message, va_list arguments);
unsigned char* rasqal_world_generate_bnodeid(rasqal_world* world, unsigned char *user_bnodeid);

rasqal_query_language_factory* rasqal_query_language_register_factory(rasqal_world *world, int (*factory) (rasqal_query_language_factory*));
rasqal_query_language_factory* rasqal_get_query_language_factory (rasqal_world*, const char* name, const unsigned char* uri);
void rasqal_log_error_simple(rasqal_world* world, raptor_log_level level, raptor_locator* locator, const char* message, ...) RASQAL_PRINTF_FORMAT(4, 5);
void rasqal_log_error_varargs(rasqal_world* world, raptor_log_level level, raptor_locator* locator, const char* message, va_list arguments) RASQAL_PRINTF_FORMAT(4, 0);
void rasqal_query_simple_error(void* user_data /* query */, const char *message, ...) RASQAL_PRINTF_FORMAT(2, 3);
void rasqal_world_simple_error(void* user_data /* world */, const char *message, ...) RASQAL_PRINTF_FORMAT(2, 3);

const char* rasqal_basename(const char* name);
unsigned char* rasqal_world_default_generate_bnodeid_handler(void *user_data, unsigned char *user_bnodeid);

unsigned char* rasqal_escaped_name_to_utf8_string(const unsigned char* src, size_t len, size_t* dest_lenp, int (*error_handler)(rasqal_query *error_data, const char *message, ...) RASQAL_PRINTF_FORMAT(2, 3), rasqal_query* error_data);

/* rasqal_graph_pattern.c */
unsigned char* rasqal_query_generate_bnodeid(rasqal_query* rdf_query, unsigned char *user_bnodeid);

rasqal_graph_pattern* rasqal_new_basic_graph_pattern_from_formula(rasqal_query* query, rasqal_formula* formula);
rasqal_graph_pattern* rasqal_new_basic_graph_pattern_from_triples(rasqal_query* query, raptor_sequence* triples);

rasqal_graph_pattern* rasqal_new_2_group_graph_pattern(rasqal_query* query, rasqal_graph_pattern* first_gp, rasqal_graph_pattern* second_gp);

/* rdql_parser.y */
int rasqal_init_query_language_rdql(rasqal_world*);

/* sparql_parser.y */
int rasqal_init_query_language_sparql(rasqal_world*);
int rasqal_init_query_language_sparql11(rasqal_world*);
int rasqal_init_query_language_laqrs(rasqal_world*);

/* rasqal_query_transform.c */
int rasqal_query_expand_triple_qnames(rasqal_query* rq);
int rasqal_sequence_has_qname(raptor_sequence* seq);
int rasqal_query_constraints_has_qname(rasqal_query* gp);
int rasqal_query_expand_graph_pattern_constraints_qnames(rasqal_query* rq, rasqal_graph_pattern* gp);
int rasqal_query_expand_query_constraints_qnames(rasqal_query* rq);
int rasqal_query_build_anonymous_variables(rasqal_query* rq);
int rasqal_query_expand_wildcards(rasqal_query* rq);
int rasqal_query_remove_duplicate_select_vars(rasqal_query* rq);
int rasqal_query_prepare_common(rasqal_query *query);
int rasqal_query_merge_graph_patterns(rasqal_query* query, rasqal_graph_pattern* gp, void* data);
int rasqal_graph_patterns_join(rasqal_graph_pattern *dest_gp, rasqal_graph_pattern *src_gp);
int rasqal_graph_pattern_move_constraints(rasqal_graph_pattern* dest_gp, rasqal_graph_pattern* src_gp);
int* rasqal_query_triples_build_bound_in(rasqal_query* query, int size, int start_column, int end_column);

/* rasqal_expr.c */
rasqal_literal* rasqal_new_string_literal_node(rasqal_world*, const unsigned char *string, const char *language, raptor_uri *datatype);
int rasqal_literal_as_boolean(rasqal_literal* literal, int* error);
int rasqal_literal_as_integer(rasqal_literal* l, int* error);
double rasqal_literal_as_floating(rasqal_literal* l, int* error);
raptor_uri* rasqal_literal_as_uri(rasqal_literal* l);
int rasqal_literal_string_to_native(rasqal_literal *l, int flags);
int rasqal_literal_has_qname(rasqal_literal* l);
int rasqal_literal_expand_qname(void* user_data, rasqal_literal* l);
int rasqal_literal_is_constant(rasqal_literal* l);
int rasqal_expression_has_qname(void* user_data, rasqal_expression* e);
int rasqal_expression_expand_qname(void* user_data, rasqal_expression* e);
int rasqal_literal_ebv(rasqal_literal* l);
int rasqal_expression_is_constant(rasqal_expression* e);
void rasqal_expression_clear(rasqal_expression* e);
void rasqal_expression_convert_to_literal(rasqal_expression* e, rasqal_literal* l);
int rasqal_expression_mentions_variable(rasqal_expression* e, rasqal_variable* v);
void rasqal_triple_write(rasqal_triple* t, raptor_iostream* iostr);
void rasqal_variable_write(rasqal_variable* v, raptor_iostream* iostr);
int rasqal_expression_is_aggregate(rasqal_expression* e);
int rasqal_expression_convert_aggregate_to_variable(rasqal_expression* e_in, rasqal_variable* v, rasqal_expression** e_out);
int rasqal_expression_mentions_aggregate(rasqal_expression* e);

raptor_sequence* rasqal_expression_copy_expression_sequence(raptor_sequence* expr_seq);
int rasqal_literal_sequence_compare(int compare_flags, raptor_sequence* values_a, raptor_sequence* values_b);
raptor_sequence* rasqal_expression_sequence_evaluate(rasqal_query* query, raptor_sequence* expr_seq, int ignore_errors, raptor_sequence* literal_seq, int* error_p);

/* strcasecmp.c */
#ifdef HAVE_STRCASECMP
#  define rasqal_strcasecmp strcasecmp
#  define rasqal_strncasecmp strncasecmp
#else
#  ifdef HAVE_STRICMP
#    define rasqal_strcasecmp stricmp
#    define rasqal_strncasecmp strnicmp
#   else
int rasqal_strcasecmp(const char* s1, const char* s2);
int rasqal_strncasecmp(const char* s1, const char* s2, size_t n);
#  endif
#endif

/* rasqal_raptor.c */
int rasqal_raptor_init(rasqal_world*);

#ifdef RAPTOR_TRIPLES_SOURCE_REDLAND
/* rasqal_redland.c */
int rasqal_redland_init(rasqal_world*);
void rasqal_redland_finish(void);
#endif  


/* rasqal_general.c */
int rasqal_uri_init(rasqal_world*);
void rasqal_uri_finish(rasqal_world*);

/* rasqal_literal.c */
rasqal_formula* rasqal_new_formula(rasqal_world* world);
void rasqal_free_formula(rasqal_formula* formula);
int rasqal_formula_print(rasqal_formula* formula, FILE *stream);
rasqal_formula* rasqal_formula_join(rasqal_formula* first_formula, rasqal_formula* second_formula);

/* The following should be public eventually in rasqal.h or raptor.h or ...? */

typedef int (rasqal_compare_fn)(void* user_data, const void *a, const void *b);
typedef void (rasqal_compare_free_user_data_fn)(const void *data);
typedef void (rasqal_kv_free_fn)(const void *key, const void *value);


#define RASQAL_XSD_BOOLEAN_TRUE (const unsigned char*)"true"
#define RASQAL_XSD_BOOLEAN_FALSE (const unsigned char*)"false"

rasqal_literal* rasqal_literal_cast(rasqal_literal* l, raptor_uri* datatype, int flags,  int* error_p);
rasqal_literal* rasqal_new_numeric_literal(rasqal_world*, rasqal_literal_type type, double d);
int rasqal_literal_is_numeric(rasqal_literal* literal);
rasqal_literal* rasqal_literal_add(rasqal_literal* l1, rasqal_literal* l2, int *error);
rasqal_literal* rasqal_literal_subtract(rasqal_literal* l1, rasqal_literal* l2, int *error);
rasqal_literal* rasqal_literal_multiply(rasqal_literal* l1, rasqal_literal* l2, int *error);
rasqal_literal* rasqal_literal_divide(rasqal_literal* l1, rasqal_literal* l2, int *error);
rasqal_literal* rasqal_literal_negate(rasqal_literal* l, int *error_p);
int rasqal_literal_equals_flags(rasqal_literal* l1, rasqal_literal* l2, int flags, int* error);
int rasqal_literal_not_equals_flags(rasqal_literal* l1, rasqal_literal* l2, int flags, int* error);
void rasqal_literal_write_type(rasqal_literal* l, raptor_iostream* iostr);
void rasqal_literal_write(rasqal_literal* l, raptor_iostream* iostr);
void rasqal_expression_write_op(rasqal_expression* e, raptor_iostream* iostr);
void rasqal_expression_write(rasqal_expression* e, raptor_iostream* iostr);

/* rasqal_map.c */
typedef void (*rasqal_map_visit_fn)(void *key, void *value, void *user_data);

#ifdef HAVE_RAPTOR2_API
rasqal_map* rasqal_new_map(rasqal_compare_fn* compare_fn, void* compare_user_data, rasqal_compare_free_user_data_fn* free_compare_user_data, rasqal_kv_free_fn* free_fn, raptor_data_print_handler print_key_fn, raptor_data_print_handler print_value_fn, int flags);
#else
rasqal_map* rasqal_new_map(rasqal_compare_fn* compare_fn, void* compare_user_data, rasqal_compare_free_user_data_fn* free_compare_user_data, rasqal_kv_free_fn* free_fn, raptor_sequence_print_handler* print_key_fn, raptor_sequence_print_handler* print_value_fn, int flags);
#endif
void rasqal_free_map(rasqal_map *map);
int rasqal_map_add_kv(rasqal_map* map, void* key, void *value);
void rasqal_map_visit(rasqal_map* map, rasqal_map_visit_fn fn, void *user_data);
int rasqal_map_print(rasqal_map* map, FILE* fh);
void* rasqal_map_search(rasqal_map* map, const void* key);


/* rasqal_query.c */
rasqal_query_results* rasqal_query_execute_with_engine(rasqal_query* query, const rasqal_query_execution_factory* engine);
int rasqal_query_remove_query_result(rasqal_query* query, rasqal_query_results* query_results);
int rasqal_query_declare_prefix(rasqal_query* rq, rasqal_prefix* prefix);
int rasqal_query_declare_prefixes(rasqal_query* rq);
unsigned char* rasqal_query_get_genid(rasqal_query* query, const unsigned char* base, int counter);
void rasqal_query_set_base_uri(rasqal_query* rq, raptor_uri* base_uri);
void rasqal_query_set_store_results(rasqal_query* query, int store_results);
rasqal_variable* rasqal_query_get_variable_by_offset(rasqal_query* query, int idx);
const rasqal_query_execution_factory* rasqal_query_get_engine_by_name(const char* name);
int rasqal_query_variable_bound_in_triple(rasqal_query *query, rasqal_variable *v, int column);

/* rasqal_query_results.c */
int rasqal_init_query_results(void);
void rasqal_finish_query_results(void);
rasqal_query_results* rasqal_query_results_execute_with_engine(rasqal_query* query, const rasqal_query_execution_factory* factory);
int rasqal_query_results_check_limit_offset(rasqal_query_results* query_results);
void rasqal_query_results_remove_query_reference(rasqal_query_results* query_results);
rasqal_variables_table* rasqal_query_results_get_variables_table(rasqal_query_results* query_results);
rasqal_row* rasqal_query_results_get_current_row(rasqal_query_results* query_results);
rasqal_world* rasqal_query_results_get_world(rasqal_query_results* query_results);


/* rasqal_query_write.c */
int rasqal_query_write_sparql_20060406(raptor_iostream *iostr, rasqal_query* query, raptor_uri *base_uri);

/* rasqal_result_formats.c */
int rasqal_query_results_format_register_factory(rasqal_world*, const char *name, const char *label, const unsigned char* uri_string, rasqal_query_results_formatter_func writer, rasqal_query_results_formatter_func reader, rasqal_query_results_get_rowsource_func get_rowsource, const char *mime_type);
int rasqal_init_result_formats(rasqal_world*);
void rasqal_finish_result_formats(rasqal_world*);

/* rasqal_format_sv.c */
int rasqal_init_result_format_sv(rasqal_world* world);

/* rasqal_format_json.c */
int rasqal_init_result_format_json(rasqal_world*);

/* rasqal_format_sparql_xml.c */
int rasqal_init_result_format_sparql_xml(rasqal_world*);

/* rasqal_format_table.c */
int rasqal_init_result_format_table(rasqal_world*);

/* rasqal_format_html.c */
int rasqal_init_result_format_html(rasqal_world*);

/* rasqal_row.c */
rasqal_row* rasqal_new_row(rasqal_rowsource* rowsource);
rasqal_row* rasqal_new_row_from_row(rasqal_row* row);
int rasqal_row_print(rasqal_row* row, FILE* fh);
raptor_sequence* rasqal_new_row_sequence(rasqal_world* world, rasqal_variables_table* vt, const char* const row_data[], int vars_count, raptor_sequence** vars_seq_p);
int rasqal_row_to_nodes(rasqal_row* row);
void rasqal_row_set_values_from_variables_table(rasqal_row* row, rasqal_variables_table* vars_table);
int rasqal_row_set_order_size(rasqal_row *row, int order_size);
int rasqal_row_expand_size(rasqal_row *row, int size);
int rasqal_row_bind_variables(rasqal_row* row, rasqal_variables_table* vars_table);


/* rasqal_row_compatible.c */
rasqal_row_compatible* rasqal_new_row_compatible(rasqal_variables_table* vt, rasqal_rowsource *first_rowsource, rasqal_rowsource *second_rowsource);
void rasqal_free_row_compatible(rasqal_row_compatible* map);
int rasqal_row_compatible_check(rasqal_row_compatible* map, rasqal_row *first_row, rasqal_row *second_row);
void rasqal_print_row_compatible(FILE *handle, rasqal_row_compatible* map);

/* rasqal_triples_source.c */
rasqal_triples_source* rasqal_new_triples_source(rasqal_query* query);
int rasqal_reset_triple_meta(rasqal_triple_meta* m);
void rasqal_free_triples_source(rasqal_triples_source *rts);
int rasqal_triples_source_triple_present(rasqal_triples_source *rts, rasqal_triple *t);
int rasqal_triples_source_support_feature(rasqal_triples_source *rts, rasqal_triple_source_feature feature);

rasqal_triples_match* rasqal_new_triples_match(rasqal_query* query, rasqal_triples_source* triples_source, rasqal_triple_meta *m, rasqal_triple *t);
rasqal_triple_parts rasqal_triples_match_bind_match(struct rasqal_triples_match_s* rtm, rasqal_variable *bindings[4],rasqal_triple_parts parts);
void rasqal_triples_match_next_match(struct rasqal_triples_match_s* rtm);
int rasqal_triples_match_is_end(struct rasqal_triples_match_s* rtm);


/* rasqal_xsd_datatypes.c */
int rasqal_xsd_init(rasqal_world*);
void rasqal_xsd_finish(rasqal_world*);
rasqal_literal_type rasqal_xsd_datatype_uri_to_type(rasqal_world*, raptor_uri* uri);
raptor_uri* rasqal_xsd_datatype_type_to_uri(rasqal_world*, rasqal_literal_type type);
int rasqal_xsd_datatype_check(rasqal_literal_type native_type, const unsigned char* string, int flags);
const char* rasqal_xsd_datatype_label(rasqal_literal_type native_type);
int rasqal_xsd_is_datatype_uri(rasqal_world*, raptor_uri* uri);
const unsigned char* rasqal_xsd_datetime_string_to_canonical(const unsigned char* datetime_string);
const unsigned char* rasqal_xsd_date_string_to_canonical(const unsigned char* date_string);

int rasqal_xsd_datatype_is_numeric(rasqal_literal_type type);
unsigned char* rasqal_xsd_format_integer(int i, size_t *len_p);
unsigned char* rasqal_xsd_format_float(float f, size_t *len_p);
unsigned char* rasqal_xsd_format_double(double d, size_t *len_p);
rasqal_literal_type rasqal_xsd_datatype_parent_type(rasqal_literal_type type);

typedef struct rasqal_graph_factory_s rasqal_graph_factory;

/* rasqal_world structure */
struct rasqal_world_s {
  /* opened flag */
  int opened;
  
  /* raptor_world object */
  raptor_world *raptor_world_ptr;

  /* should rasqal free the raptor_world */
  int raptor_world_allocated_here;

#ifdef HAVE_RAPTOR2_API
  /* log handler */
  raptor_log_handler log_handler;
  void *log_handler_user_data;
#else
  /* error handlers structure */
  raptor_error_handlers error_handlers;
#endif

  /* sequence of query language factories */
  raptor_sequence *query_languages;

  /* registered query results formats */
  raptor_sequence *query_results_formats;

  /* rasqal_uri rdf uris */
  raptor_uri *rdf_namespace_uri;
  raptor_uri *rdf_first_uri;
  raptor_uri *rdf_rest_uri;
  raptor_uri *rdf_nil_uri;

  /* triples source factory */
  rasqal_triples_source_factory triples_source_factory;

  /* rasqal_xsd_datatypes */
  raptor_uri *xsd_namespace_uri;
  raptor_uri **xsd_datatype_uris;

  /* graph factory */
  rasqal_graph_factory *graph_factory;
  void *graph_factory_user_data;

  int default_generate_bnodeid_handler_base;
  char *default_generate_bnodeid_handler_prefix;
  size_t default_generate_bnodeid_handler_prefix_length;

  void *generate_bnodeid_handler_user_data;
  rasqal_generate_bnodeid_handler2 generate_bnodeid_handler;
};


/*
 * Rasqal Algebra
 *
 * Based on http://www.w3.org/TR/rdf-sparql-query/#sparqlAlgebra
 */

typedef enum {
  RASQAL_ALGEBRA_OPERATOR_UNKNOWN  = 0,
  RASQAL_ALGEBRA_OPERATOR_BGP      = 1,
  RASQAL_ALGEBRA_OPERATOR_FILTER   = 2,
  RASQAL_ALGEBRA_OPERATOR_JOIN     = 3,
  RASQAL_ALGEBRA_OPERATOR_DIFF     = 4,
  RASQAL_ALGEBRA_OPERATOR_LEFTJOIN = 5,
  RASQAL_ALGEBRA_OPERATOR_UNION    = 6,
  RASQAL_ALGEBRA_OPERATOR_TOLIST   = 7,
  RASQAL_ALGEBRA_OPERATOR_ORDERBY  = 8,
  RASQAL_ALGEBRA_OPERATOR_PROJECT  = 9,
  RASQAL_ALGEBRA_OPERATOR_DISTINCT = 10,
  RASQAL_ALGEBRA_OPERATOR_REDUCED  = 11,
  RASQAL_ALGEBRA_OPERATOR_SLICE    = 12,
  RASQAL_ALGEBRA_OPERATOR_GRAPH    = 13,
  RASQAL_ALGEBRA_OPERATOR_ASSIGN   = 14,
  RASQAL_ALGEBRA_OPERATOR_GROUP    = 15,
  RASQAL_ALGEBRA_OPERATOR_AGGREGATION = 16,

  RASQAL_ALGEBRA_OPERATOR_LAST = RASQAL_ALGEBRA_OPERATOR_AGGREGATION
} rasqal_algebra_node_operator;


/*
 * Algebra Node
 *
 * Rasqal graph pattern class.
 */
struct rasqal_algebra_node_s {
  rasqal_query* query;

  /* operator for this algebra_node's contents */
  rasqal_algebra_node_operator op;

  /* type BGP (otherwise NULL and start_column and end_column are -1) */
  raptor_sequence* triples;
  int start_column;
  int end_column;
  
  /* types JOIN, DIFF, LEFTJOIN, UNION, ORDERBY: node1 and node2 ALWAYS present
   * types FILTER, TOLIST: node1 ALWAYS present, node2 ALWAYS NULL
   * type PROJECT, GRAPH, GROUPBY, AGGREGATION: node1 always present
   * (otherwise NULL)
   */
  struct rasqal_algebra_node_s *node1;
  struct rasqal_algebra_node_s *node2;

  /* types FILTER, LEFTJOIN
   * (otherwise NULL) 
   */
  rasqal_expression* expr;

  /* types ORDERBY, GROUPBY, AGGREGATION always present: sequence of
   * #rasqal_expression
   * (otherwise NULL)
   */
  raptor_sequence* seq;

  /* types PROJECT, DISTINCT, REDUCED
   * FIXME: sequence of solution mappings */

  /* types PROJECT, SLICE, AGGREGATION: sequence of #rasqal_variable */
  raptor_sequence* vars_seq;

  /* type SLICE: start and length */
  unsigned int start;
  unsigned int length;

  /* type GRAPH */
  rasqal_literal *graph;

  /* type LET */
  rasqal_variable *var;
};
typedef struct rasqal_algebra_node_s rasqal_algebra_node;


/**
 * rasqal_algebra_node_visit_fn:
 * @query: #rasqal_query containing the graph pattern
 * @gp: current algebra_node
 * @user_data: user data passed in
 *
 * User function to visit an algebra_node and operate on it with
 * rasqal_algebra_node_visit() or rasqal_query_algebra_node_visit()
 *
 * Return value: 0 to truncate the visit
 */
typedef int (*rasqal_algebra_node_visit_fn)(rasqal_query* query, rasqal_algebra_node* node, void *user_data);

typedef struct
{
  rasqal_query* query;
  
  /* aggregate expression variables map
   * key: rasqal_expression* tree with an aggregate function at top
   * value: rasqal_variable*
   */
  rasqal_map* agg_vars;

  /* sequence of aggregate rasqal_expression pointing into @agg_vars */
  raptor_sequence* agg_exprs;

  /* number of aggregate expressions seen
   * ( = number of internal variables created )
   */
  int counter;

  /* compare flags */
  int flags;

  /* error indicator */
  int error;

  /* if set, finding a new expression is an error */
  int adding_new_vars_is_error : 1;

  /* query part for error messages */
  const char* error_part;
} rasqal_algebra_aggregate;


/* rasqal_algebra.c */

rasqal_algebra_node* rasqal_new_assignment_algebra_node(rasqal_query* query, rasqal_variable *var, rasqal_expression *expr);
rasqal_algebra_node* rasqal_new_distinct_algebra_node(rasqal_query* query, rasqal_algebra_node* node1);
rasqal_algebra_node* rasqal_new_filter_algebra_node(rasqal_query* query, rasqal_expression* expr, rasqal_algebra_node* node);
rasqal_algebra_node* rasqal_new_empty_algebra_node(rasqal_query* query);
rasqal_algebra_node* rasqal_new_triples_algebra_node(rasqal_query* query, raptor_sequence* triples, int start_column, int end_column);
rasqal_algebra_node* rasqal_new_2op_algebra_node(rasqal_query* query, rasqal_algebra_node_operator op, rasqal_algebra_node* node1, rasqal_algebra_node* node2);
rasqal_algebra_node* rasqal_new_leftjoin_algebra_node(rasqal_query* query, rasqal_algebra_node* node1, rasqal_algebra_node* node2, rasqal_expression* expr);
rasqal_algebra_node* rasqal_new_join_algebra_node(rasqal_query* query, rasqal_algebra_node* node1, rasqal_algebra_node* node2, rasqal_expression* expr);
rasqal_algebra_node* rasqal_new_orderby_algebra_node(rasqal_query* query, rasqal_algebra_node* node, raptor_sequence* seq);
rasqal_algebra_node* rasqal_new_project_algebra_node(rasqal_query* query, rasqal_algebra_node* node1, raptor_sequence* vars_seq);
rasqal_algebra_node* rasqal_new_graph_algebra_node(rasqal_query* query, rasqal_algebra_node* node1, rasqal_literal *graph);
rasqal_algebra_node* rasqal_new_let_algebra_node(rasqal_query* query, rasqal_variable *var, rasqal_expression *expr);
rasqal_algebra_node* rasqal_new_groupby_algebra_node(rasqal_query* query, rasqal_algebra_node* node1, raptor_sequence* seq);
rasqal_algebra_node* rasqal_new_aggregation_algebra_node(rasqal_query* query, rasqal_algebra_node* node1, raptor_sequence* exprs_seq, raptor_sequence* vars_seq);

void rasqal_free_algebra_node(rasqal_algebra_node* node);
rasqal_algebra_node_operator rasqal_algebra_node_get_operator(rasqal_algebra_node* node);
const char* rasqal_algebra_node_operator_as_string(rasqal_algebra_node_operator op);
int rasqal_algebra_algebra_node_write(rasqal_algebra_node *node, raptor_iostream* iostr);
int rasqal_algebra_node_print(rasqal_algebra_node* node, FILE* fh);
int rasqal_algebra_node_visit(rasqal_query *query, rasqal_algebra_node* node, rasqal_algebra_node_visit_fn fn, void *user_data);
rasqal_algebra_node* rasqal_algebra_query_to_algebra(rasqal_query* query);
rasqal_algebra_node* rasqal_algebra_query_add_modifiers(rasqal_query* query, rasqal_algebra_node* node);
rasqal_algebra_node* rasqal_algebra_query_add_projection(rasqal_query* query, rasqal_algebra_node* node);
rasqal_algebra_node* rasqal_algebra_query_add_distinct(rasqal_query* query, rasqal_algebra_node* node);
int rasqal_algebra_node_is_empty(rasqal_algebra_node* node);

rasqal_algebra_aggregate* rasqal_algebra_query_prepare_aggregates(rasqal_query* query, rasqal_algebra_node* node);
void rasqal_free_algebra_aggregate(rasqal_algebra_aggregate* ae);

/* rasqal_variable.c */
rasqal_variables_table* rasqal_new_variables_table_from_variables_table(rasqal_variables_table* vt);
rasqal_variable* rasqal_variables_table_get(rasqal_variables_table* vt, int idx);
rasqal_variable* rasqal_variables_table_get_by_name(rasqal_variables_table* vt, const unsigned char *name);
rasqal_literal* rasqal_variables_table_get_value(rasqal_variables_table* vt, int idx);
int rasqal_variables_table_has(rasqal_variables_table* vt, const unsigned char *name);
int rasqal_variables_table_set(rasqal_variables_table* vt, const unsigned char *name, rasqal_literal* value);
int rasqal_variables_table_get_named_variables_count(rasqal_variables_table* vt);
int rasqal_variables_table_get_anonymous_variables_count(rasqal_variables_table* vt);
int rasqal_variables_table_get_total_variables_count(rasqal_variables_table* vt);
raptor_sequence* rasqal_variables_table_get_named_variables_sequence(rasqal_variables_table* vt);
raptor_sequence* rasqal_variables_table_get_anonymous_variables_sequence(rasqal_variables_table* vt);
const unsigned char** rasqal_variables_table_get_names(rasqal_variables_table* vt);


/**
 * rasqal_engine_error:
 * @RASQAL_ENGINE_OK:
 * @RASQAL_ENGINE_FAILED:
 * @RASQAL_ENGINE_FINISHED:
 *
 * Execution engine errors.
 *
 */
typedef enum {
  RASQAL_ENGINE_OK,
  RASQAL_ENGINE_FAILED,
  RASQAL_ENGINE_FINISHED,
  RASQAL_ENGINE_ERROR_LAST = RASQAL_ENGINE_FINISHED
} rasqal_engine_error;


#ifdef RASQAL_DEBUG
const char* rasqal_engine_error_as_string(rasqal_engine_error error);
#endif

/*
 * A query execution engine factory
 *
 * This structure is about executing the query recorded in
 * #rasqal_query structure into results accessed via #rasqal_query_results
 */
struct rasqal_query_execution_factory_s {
  /* execution engine name */
  const char* name;

  /* size of execution engine private data */
  size_t execution_data_size;
  
  /*
   * @ex_data: execution data
   * @query: query to execute
   * @query_results: query results
   * @flags: execution flags.  1: execute and store results
   * @error_p: execution error (OUT variable)
   *
   * Initialise a new execution
   *
   * Return value: non-0 on failure
   */
  int (*execute_init)(void* ex_data, rasqal_query* query, rasqal_query_results* query_results, int flags, rasqal_engine_error *error_p);

  /**
   * @ex_data: execution data
   * @error_p: execution error (OUT variable)
   *
   * Get all bindings result rows (returning a new raptor_sequence object holding new objects.
   *
   * Will not be called if query results is NULL, finished or failed.
   */
  raptor_sequence* (*get_all_rows)(void* ex_data, rasqal_engine_error *error_p);

  /*
   * @ex_data: execution object
   * @error_p: execution error (OUT variable)
   *
   * Get current bindings result row (returning a new object) 
   *
   * Will not be called if query results is NULL, finished or failed.
   */
  rasqal_row* (*get_row)(void* ex_data, rasqal_engine_error *error_p);

  /* finish (free) execution */
  int (*execute_finish)(void* ex_data, rasqal_engine_error *error_p);
  
  /* finish the query execution factory */
  void (*finish_factory)(rasqal_query_execution_factory* factory);

};


/* rasqal_engine.c */

/* Original Rasqal 0.9.16 query engine executing over graph patterns */
extern const rasqal_query_execution_factory rasqal_query_engine_1;

/* rasqal_engine_sort.c */
rasqal_map* rasqal_engine_new_rowsort_map(int is_distinct, int compare_flags, raptor_sequence* order_conditions_sequence);
int rasqal_engine_rowsort_map_add_row(rasqal_map* map, rasqal_row* row);
raptor_sequence* rasqal_engine_rowsort_map_to_sequence(rasqal_map* map, raptor_sequence* seq);
int rasqal_engine_rowsort_calculate_order_values(rasqal_query* query, rasqal_row* row);

#ifdef RASQAL_DEBUG
const char* rasqal_engine_get_parts_string(rasqal_triple_parts parts);
#endif

/* rasqal_engine_algebra.c */

/* New query engine based on executing over query algebra */
extern const rasqal_query_execution_factory rasqal_query_engine_algebra;

/* rasqal_triples_source.c */
void rasqal_triples_source_error_handler(rasqal_query* rdf_query, raptor_locator* locator, const char* message);

/* rasqal_update.c */
const char* rasqal_update_type_label(rasqal_update_type type);
rasqal_update_operation* rasqal_new_update_operation(rasqal_update_type type, raptor_uri* graph_uri, raptor_uri* document_uri, raptor_sequence* insert_templates, raptor_sequence* delete_templates, rasqal_graph_pattern* graph_pattern, int flags);
void rasqal_free_update_operation(rasqal_update_operation *update);
int rasqal_update_operation_print(rasqal_update_operation *update, FILE* stream);
int rasqal_query_add_update_operation(rasqal_query* query, rasqal_update_operation *update);


/* rasqal_bindings.c */
rasqal_bindings* rasqal_new_bindings(rasqal_query* query, raptor_sequence* variables, raptor_sequence* rows);
void rasqal_free_bindings(rasqal_bindings* bindings);
int rasqal_bindings_print(rasqal_bindings* bindings, FILE* fh);


/* rasqal_projection.c */
rasqal_projection* rasqal_new_projection(rasqal_query* query, raptor_sequence* variables, int wildcard, int distinct);
void rasqal_free_projection(rasqal_projection* projection);
raptor_sequence* rasqal_projection_get_variables_sequence(rasqal_projection* projection);


/* rasqal_solution_modifier.c */
rasqal_solution_modifier* rasqal_new_solution_modifier(rasqal_query* query, raptor_sequence* order_conditions, raptor_sequence* group_conditions, raptor_sequence* having_conditions, int limit, int offset);
void rasqal_free_solution_modifier(rasqal_solution_modifier* sm);


/* raptor1 -> raptor2 migration */
/* use preprocessor macros to map raptor2 calls to raptor1 if raptor2 is not available */
#ifndef HAVE_RAPTOR2_API

#define raptor_new_iostream_to_file_handle(world, fh) raptor_new_iostream_to_file_handle(fh)
#define raptor_new_iostream_from_file_handle(world, fh) raptor_new_iostream_from_file_handle(fh)
#define raptor_iostream_write_bytes(ptr, size, nmemb, iostr) raptor_iostream_write_bytes(iostr, ptr, size, nmemb)
#define raptor_iostream_write_byte(byte, iostr) raptor_iostream_write_byte(iostr, byte)
#define raptor_iostream_counted_string_write(string, len, iostr) raptor_iostream_write_counted_string(iostr, string, len)
#define raptor_iostream_string_write(string, iostr) raptor_iostream_write_string(iostr, string)
#define raptor_iostream_decimal_write(decimal, iostr) raptor_iostream_write_decimal(iostr, decimal)
#define raptor_iostream_read_bytes(ptr, size, nmemb, iostr) raptor_iostream_read_bytes(iostr, ptr, size, nmemb)
#define raptor_xml_escape_string_write(string, len, quote, iostr) raptor_iostream_write_xml_escaped_string(iostr, string, len, quote, NULL, NULL)

#define raptor_string_ntriples_write(str, len, delim, iostr) raptor_iostream_write_string_ntriples(iostr, str, len, delim)

#define raptor_new_qname_from_namespace_uri(nstack, uri, xml_version) raptor_namespaces_qname_from_uri(nstack, uri, xml_version)

#define raptor_new_serializer(world, name) raptor_new_serializer(name)
#define raptor_world_is_serializer_name(world, name) raptor_serializer_syntax_name_check(name)
#define raptor_world_is_parser_name(world, name) raptor_syntax_name_check(name)

#define raptor_unicode_utf8_string_get_char(input, length, output) raptor_utf8_to_unicode_char(output, input, length)
#define raptor_unicode_utf8_string_put_char(char, output, length) raptor_unicode_char_to_utf8(char, output)

#define raptor_new_uri(world, uri_string) raptor_new_uri(uri_string)
#define raptor_new_uri_from_uri_local_name(world, base_uri, name) raptor_new_uri_from_uri_local_name(base_uri, name)
#define raptor_uri_write(uri, iostr) raptor_iostream_write_uri(iostr, uri)

#define raptor_new_www(world) raptor_www_new()

#define RAPTOR_LOG_LEVEL_WARN RAPTOR_LOG_LEVEL_WARNING

#endif /* !HAVE_RAPTOR2_API */

  
/* end of RASQAL_INTERNAL */
#endif


#ifdef __cplusplus
}
#endif

#endif
