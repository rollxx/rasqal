/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rasqal_algebra.c - Rasqal algebra class
 *
 * Copyright (C) 2008, David Beckett http://www.dajobe.org/
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
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdarg.h>

#include "rasqal.h"
#include "rasqal_internal.h"


#ifndef STANDALONE

static rasqal_algebra_node* rasqal_algebra_graph_pattern_to_algebra(rasqal_query* query, rasqal_graph_pattern* gp);

/*
 * rasqal_new_algebra_node:
 * @query: #rasqal_algebra_node query object
 * @op: enum #rasqal_algebra_operator operator
 *
 * INTERNAL - Create a new algebra object.
 * 
 * Return value: a new #rasqal_algebra object or NULL on failure
 **/
static rasqal_algebra_node*
rasqal_new_algebra_node(rasqal_query* query, rasqal_algebra_node_operator op)
{
  rasqal_algebra_node* node;

  if(!query)
    return NULL;
  
  node=(rasqal_algebra_node*)RASQAL_CALLOC(rasqal_algebra, 1, 
                                           sizeof(rasqal_algebra_node));
  if(!node)
    return NULL;

  node->op = op;
  node->query=query;
  return node;
}


/*
 * rasqal_new_filter_algebra_node:
 * @query: #rasqal_query query object
 * @expr: FILTER expression
 * @node: algebra node being filtered
 *
 * INTERNAL - Create a new algebra node for an expression over a node
 * 
 * Return value: a new #rasqal_algebra_node_node object or NULL on failure
 **/
rasqal_algebra_node*
rasqal_new_filter_algebra_node(rasqal_query* query,
                               rasqal_expression* expr,
                               rasqal_algebra_node* node)
{
  rasqal_algebra_node* new_node;

  if(!query || !expr)
    return NULL;
  
  new_node=rasqal_new_algebra_node(query, RASQAL_ALGEBRA_OPERATOR_FILTER);
  if(!new_node)
    return NULL;

  new_node->expr=expr;
  new_node->node1=node;

  return new_node;
}


/*
 * rasqal_new_triples_algebra_node:
 * @query: #rasqal_query query object
 * @triples: triples sequence (SHARED) (or NULL for empty BGP)
 * @start_column: first triple
 * @end_column: last triple
 *
 * INTERNAL - Create a new algebra node for Basic Graph Pattern
 * 
 * Return value: a new #rasqal_algebra_node_node object or NULL on failure
 **/
rasqal_algebra_node*
rasqal_new_triples_algebra_node(rasqal_query* query,
                                raptor_sequence* triples,
                                int start_column, int end_column)
{
  rasqal_algebra_node* node;

  if(!query)
    return NULL;
  
  node=rasqal_new_algebra_node(query, RASQAL_ALGEBRA_OPERATOR_BGP);
  if(!node)
    return NULL;

  node->triples=triples;
  if(!triples) {
    start_column= -1;
    end_column= -1;
  }
  node->start_column=start_column;
  node->end_column=end_column;

  return node;
}


/*
 * rasqal_new_empty_algebra_node:
 * @query: #rasqal_query query object
 *
 * INTERNAL - Create a new empty algebra node
 * 
 * Return value: a new #rasqal_algebra_node_node object or NULL on failure
 **/
rasqal_algebra_node*
rasqal_new_empty_algebra_node(rasqal_query* query)
{
  rasqal_algebra_node* node;

  if(!query)
    return NULL;
  
  node=rasqal_new_algebra_node(query, RASQAL_ALGEBRA_OPERATOR_BGP);
  if(!node)
    return NULL;

  node->triples=NULL;
  node->start_column= -1;
  node->end_column= -1;

  return node;
}


/*
 * rasqal_new_2op_algebra_node:
 * @query: #rasqal_query query object
 * @op: operator 
 * @node1: 1st algebra node
 * @node2: 2nd algebra node (pr NULL for #RASQAL_ALGEBRA_OPERATOR_TOLIST only)
 *
 * INTERNAL - Create a new algebra node for 1 or 2 graph patterns
 *
 * node1 and ndoe2 become owned by the new node
 *
 * Return value: a new #rasqal_algebra_node_node object or NULL on failure
 **/
rasqal_algebra_node*
rasqal_new_2op_algebra_node(rasqal_query* query,
                            rasqal_algebra_node_operator op,
                            rasqal_algebra_node* node1,
                            rasqal_algebra_node* node2)
{
  rasqal_algebra_node* node;

  if(!query || !node1)
    goto fail;
  if(op != RASQAL_ALGEBRA_OPERATOR_TOLIST && !node2)
    goto fail;
  
  node=rasqal_new_algebra_node(query, op);
  if(node) {
    node->node1=node1;
    node->node2=node2;
    
    return node;
  }

  fail:
  if(node1)
    rasqal_free_algebra_node(node1);
  if(node2)
    rasqal_free_algebra_node(node2);
  return NULL;
}


/*
 * rasqal_new_leftjoin_algebra_node:
 * @query: #rasqal_query query object
 * @node1: 1st algebra node
 * @node2: 2nd algebra node
 * @expr: expression
 *
 * INTERNAL - Create a new LEFTJOIN algebra node for 2 graph patterns
 * 
 * node1 and ndoe2 become owned by the new node
 *
 * Return value: a new #rasqal_algebra_node_node object or NULL on failure
 **/
rasqal_algebra_node*
rasqal_new_leftjoin_algebra_node(rasqal_query* query,
                                 rasqal_algebra_node* node1,
                                 rasqal_algebra_node* node2,
                                 rasqal_expression* expr)
{
  rasqal_algebra_node* node;

  if(!query || !node1 || !node2 || !expr)
    goto fail;

  node=rasqal_new_algebra_node(query, RASQAL_ALGEBRA_OPERATOR_LEFTJOIN);
  if(node) {
    node->node1=node1;
    node->node2=node2;
    node->expr=expr;
    
    return node;
  }

  fail:
  if(node1)
    rasqal_free_algebra_node(node1);
  if(node2)
    rasqal_free_algebra_node(node2);
  if(expr)
    rasqal_free_expression(expr);
  return NULL;
}


/*
 * rasqal_free_algebra_node:
 * @gp: #rasqal_algebra_node object
 *
 * INTERNAL - Free an algebra node object.
 * 
 **/
void
rasqal_free_algebra_node(rasqal_algebra_node* node)
{
  RASQAL_ASSERT_OBJECT_POINTER_RETURN(node, rasqal_algebra_node);

  /* node->triples is SHARED with the query - not freed here */

  if(node->node1)
    rasqal_free_algebra_node(node->node1);

  if(node->node2)
    rasqal_free_algebra_node(node->node2);

  if(node->expr)
    rasqal_free_expression(node->expr);

  RASQAL_FREE(rasqal_algebra, node);
}


/**
 * rasqal_algebra_node_get_operator:
 * @algebra_node: #rasqal_algebra_node algebra node object
 *
 * Get the algebra node operator .
 * 
 * The operator for the given algebra node. See also
 * rasqal_algebra_node_operator_as_string().
 *
 * Return value: algebra node operator
 **/
rasqal_algebra_node_operator
rasqal_algebra_node_get_operator(rasqal_algebra_node* node)
{
  return node->op;
}


static const char* const rasqal_algebra_node_operator_labels[RASQAL_ALGEBRA_OPERATOR_LAST+1]={
  "UNKNOWN",
  "BGP",
  "Filter",
  "Join",
  "Diff",
  "Leftjoin",
  "Union",
  "ToList",
  "OrderBy",
  "Project",
  "Distinct",
  "Reduced",
  "Slice"
};


/**
 * rasqal_algebra_node_operator_as_string:
 * @op: the #rasqal_algebra_node_operator verb of the query
 *
 * Get a string for the query verb.
 * 
 * Return value: pointer to a shared string label for the query verb
 **/
const char*
rasqal_algebra_node_operator_as_string(rasqal_algebra_node_operator op)
{
  if(op <= RASQAL_ALGEBRA_OPERATOR_UNKNOWN || 
     op > RASQAL_ALGEBRA_OPERATOR_LAST)
    op=RASQAL_ALGEBRA_OPERATOR_UNKNOWN;

  return rasqal_algebra_node_operator_labels[(int)op];
}
  


#define SPACES_LENGTH 80
static const char spaces[SPACES_LENGTH+1]="                                                                                ";

static void
rasqal_algebra_write_indent(raptor_iostream *iostr, int indent) 
{
  while(indent > 0) {
    int sp=(indent > SPACES_LENGTH) ? SPACES_LENGTH : indent;
    raptor_iostream_write_bytes(iostr, spaces, sizeof(char), sp);
    indent -= sp;
  }
}

static int
rasqal_algebra_algebra_node_write_internal(rasqal_algebra_node *node, 
                                           raptor_iostream* iostr, int indent)
{
  const char* op_string=rasqal_algebra_node_operator_as_string(node->op);
  int arg_count=0;
  int indent_delta;
  
  if(node->op == RASQAL_ALGEBRA_OPERATOR_BGP && !node->triples) {
    raptor_iostream_write_byte(iostr, 'Z');
    return 0;
  }
  
  indent_delta=strlen(op_string);

  raptor_iostream_write_counted_string(iostr, op_string, indent_delta);
  raptor_iostream_write_counted_string(iostr, "(\n", 2);
  indent_delta++;
  
  indent+=indent_delta;
  rasqal_algebra_write_indent(iostr, indent);

  if(node->op == RASQAL_ALGEBRA_OPERATOR_BGP) {
    int i;
    
    for(i=node->start_column; i <= node->end_column; i++) {
      rasqal_triple *t=(rasqal_triple*)raptor_sequence_get_at(node->triples, i);
      if(arg_count) {
        raptor_iostream_write_counted_string(iostr, " ,\n", 3);
        rasqal_algebra_write_indent(iostr, indent);
      }
      rasqal_triple_write(t, iostr);
      arg_count++;
    }
  }
  if(node->node1) {
    if(arg_count) {
      raptor_iostream_write_counted_string(iostr, " ,\n", 3);
      rasqal_algebra_write_indent(iostr, indent);
    }
    rasqal_algebra_algebra_node_write_internal(node->node1, iostr, indent);
    arg_count++;
    if(node->node2) {
      if(arg_count) {
        raptor_iostream_write_counted_string(iostr, " ,\n", 3);
        rasqal_algebra_write_indent(iostr, indent);
      }
      rasqal_algebra_algebra_node_write_internal(node->node2, iostr, indent);
      arg_count++;
    }
  }

  /* look for FILTER expression */
  if(node->expr) {
    if(arg_count) {
      raptor_iostream_write_counted_string(iostr, " ,\n", 3);
      rasqal_algebra_write_indent(iostr, indent);
    }
    rasqal_expression_write(node->expr, iostr);
    arg_count++;
  }

  if(node->op == RASQAL_ALGEBRA_OPERATOR_SLICE) {
    if(arg_count) {
      raptor_iostream_write_counted_string(iostr, " ,\n", 3);
      rasqal_algebra_write_indent(iostr, indent);
    }
    raptor_iostream_write_string(iostr, "slice start ");
    raptor_iostream_write_decimal(iostr, node->start);
    raptor_iostream_write_string(iostr, " length ");
    raptor_iostream_write_decimal(iostr, node->length);
    raptor_iostream_write_byte(iostr, '\n');
    arg_count++;
  }

  raptor_iostream_write_byte(iostr, '\n');
  indent-= indent_delta;

  rasqal_algebra_write_indent(iostr, indent);
  raptor_iostream_write_byte(iostr, ')');

  return 0;
}


int
rasqal_algebra_algebra_node_write(rasqal_algebra_node *node, 
                                  raptor_iostream* iostr)
{
  return rasqal_algebra_algebra_node_write_internal(node, iostr, 0);
}
  

/**
 * rasqal_algebra_node_print:
 * @gp: the #rasqal_algebra_node object
 * @fh: the #FILE* handle to print to
 *
 * Print a #rasqal_algebra_node in a debug format.
 * 
 * The print debug format may change in any release.
 * 
 **/
void
rasqal_algebra_node_print(rasqal_algebra_node* node, FILE* fh)
{
  raptor_iostream* iostr;

  iostr=raptor_new_iostream_to_file_handle(fh);
  rasqal_algebra_algebra_node_write(node, iostr);
  raptor_free_iostream(iostr);
}

/**
 * rasqal_algebra_node_visit:
 * @query: #rasqal_query to operate on
 * @node: #rasqal_algebra_node graph pattern
 * @fn: pointer to function to apply that takes user data and graph pattern parameters
 * @user_data: user data for applied function 
 * 
 * Visit a user function over a #rasqal_algebra_node
 *
 * If the user function @fn returns 0, the visit is truncated.
 *
 * Return value: 0 if the visit was truncated.
 **/
int
rasqal_algebra_node_visit(rasqal_query *query,
                          rasqal_algebra_node* node,
                          rasqal_algebra_node_visit_fn fn,
                          void *user_data)
{
  int result;
  
  result=fn(query, node, user_data);
  if(result)
    return result;
  
  if(node->node1) {
    result=rasqal_algebra_node_visit(query, node->node1, fn, user_data);
    if(result)
      return result;
  }
  if(node->node2) {
    result=rasqal_algebra_node_visit(query, node->node2, fn, user_data);
    if(result)
      return result;
  }

  return 0;
}


static rasqal_algebra_node*
rasqal_algebra_basic_graph_pattern_to_algebra(rasqal_query* query,
                                              rasqal_graph_pattern* gp)
{
  raptor_sequence* triples;
  triples=rasqal_query_get_triple_sequence(query);
  return rasqal_new_triples_algebra_node(query, triples, 
                                         gp->start_column, gp->end_column);

}

static rasqal_algebra_node*
rasqal_algebra_union_graph_pattern_to_algebra(rasqal_query* query,
                                              rasqal_graph_pattern* gp)
{
  int idx=0;
  rasqal_algebra_node* node=NULL;

  while(1) {
    rasqal_graph_pattern* sgp;
    rasqal_algebra_node* gnode;
    
    sgp=rasqal_graph_pattern_get_sub_graph_pattern(gp, idx);
    if(!sgp)
      break;
    
    gnode=rasqal_algebra_graph_pattern_to_algebra(query, sgp);
    if(!gnode) {
      RASQAL_DEBUG1("rasqal_algebra_graph_pattern_to_algebra() failed");
      goto fail;
    }
    
    if(!node)
      node=gnode;
    else {
      node=rasqal_new_2op_algebra_node(query, RASQAL_ALGEBRA_OPERATOR_UNION,
                                       node, gnode);
      if(!node) {
        RASQAL_DEBUG1("rasqal_new_2op_algebra_node() failed");
        goto fail;
      }
    }
    
    idx++;
  }

  return node;

  fail:
  if(node)
    rasqal_free_algebra_node(node);

  return NULL;
}

static rasqal_algebra_node*
rasqal_algebra_group_graph_pattern_to_algebra(rasqal_query* query,
                                              rasqal_graph_pattern* gp)
{
  int idx=0;
  /* Let FS := the empty set */
  rasqal_expression* fs=NULL;
  /* Let G := the empty pattern, Z, a basic graph pattern which
   * is the empty set. */
  rasqal_algebra_node* gnode=NULL;

  gnode=rasqal_new_empty_algebra_node(query);
  if(!gnode) {
    RASQAL_DEBUG1("rasqal_new_empty_algebra_node() failed");
    goto fail;
  }

  while(1) {
    rasqal_graph_pattern* egp;
    egp=rasqal_graph_pattern_get_sub_graph_pattern(gp, idx);
    if(!egp)
      break;

    if(egp->constraints) {
      /* If E is of the form FILTER(expr)
         FS := FS set-union {expr} 
      */
      int i;
      
      /* add all gp->conditions_sequence to FS */
      for(i=0; i< raptor_sequence_size(egp->constraints); i++) {
        rasqal_expression* e;
        e=(rasqal_expression*)raptor_sequence_get_at(egp->constraints, i);
        if(e) {
          e=rasqal_new_expression_from_expression(e);
          if(!e) {
            RASQAL_DEBUG1("rasqal_new_expression_from_expression() failed");
            goto fail;
          }
          fs=fs ? rasqal_new_2op_expression(RASQAL_EXPR_AND, fs, e) : e;
        }
      }
    }

    if(egp->op == RASQAL_GRAPH_PATTERN_OPERATOR_OPTIONAL) {
      /*  If E is of the form OPTIONAL{P} */
      int sgp_idx=0;
      int sgp_size=raptor_sequence_size(egp->graph_patterns);

      /* walk through all optionals */
      for(sgp_idx=0; sgp_idx < sgp_size; sgp_idx++) {
        rasqal_graph_pattern* sgp;
        rasqal_algebra_node* anode;

        sgp=rasqal_graph_pattern_get_sub_graph_pattern(egp, sgp_idx);

        /* Let A := Transform(P) */
        anode=rasqal_algebra_graph_pattern_to_algebra(query, sgp);
        if(!anode) {
          RASQAL_DEBUG1("rasqal_algebra_graph_pattern_to_algebra() failed");
          goto fail;
        }
        
        if(anode->op == RASQAL_ALGEBRA_OPERATOR_FILTER) {
          rasqal_expression* f_expr=anode->expr;
          rasqal_algebra_node *a2node=anode->node1;
          /* If A is of the form Filter(F, A2)
             G := LeftJoin(G, A2, F)
          */
          gnode=rasqal_new_leftjoin_algebra_node(query, gnode, a2node,
                                                 f_expr);
          if(!gnode) {
            RASQAL_DEBUG1("rasqal_new_leftjoin_algebra_node() failed");
            goto fail;
          }

          anode->expr=NULL;
          anode->node1=NULL;
          rasqal_free_algebra_node(anode);
        } else  {
          rasqal_literal *true_lit=NULL;
          rasqal_expression *true_expr=NULL;

          true_lit=rasqal_new_boolean_literal(query->world, 1);
          if(!true_lit) {
            RASQAL_DEBUG1("rasqal_new_boolean_literal() failed");
            goto fail;
          }
          
          true_expr=rasqal_new_literal_expression(true_lit);
          if(!true_expr) {
            RASQAL_DEBUG1("rasqal_new_literal_expression() failed");
            goto fail;
          }
          true_lit=NULL; /* now owned by true_expr */

          /* G := LeftJoin(G, A, true) */
          gnode=rasqal_new_leftjoin_algebra_node(query, gnode, anode,
                                                 true_expr);
          if(!gnode) {
            RASQAL_DEBUG1("rasqal_new_leftjoin_algebra_node() failed");
            rasqal_free_expression(true_expr);
            goto fail;
          }

          true_expr=NULL; /* now owned by gnode */
        }
      } /* end for all optional */
    } else {
      /* If E is any other form:*/
      rasqal_algebra_node* anode;

      /* Let A := Transform(E) */
      anode=rasqal_algebra_graph_pattern_to_algebra(query, egp);
      if(!anode) {
        RASQAL_DEBUG1("rasqal_algebra_graph_pattern_to_algebra() failed");
        goto fail;
      }

      /* G := Join(G, A) */
      gnode=rasqal_new_2op_algebra_node(query, RASQAL_ALGEBRA_OPERATOR_JOIN,
                                       gnode, anode);
      if(!gnode) {
        RASQAL_DEBUG1("rasqal_new_2op_algebra_node() failed");
        goto fail;
      }
    }
    idx++;
  }

  /*
    If FS is not empty:
    Let X := Conjunction of expressions in FS
    G := Filter(X, G)
    
    The result is G.
  */
  if(fs) {
    gnode=rasqal_new_filter_algebra_node(query, fs, gnode);
    if(!gnode) {
      RASQAL_DEBUG1("rasqal_new_filter_algebra_node() failed");
      goto fail;
    }
    fs=NULL; /* now owned by gnode */
  }

  if(gnode)
    return gnode;

  fail:

  if(gnode)
    rasqal_free_algebra_node(gnode);
  if(fs)
    rasqal_free_expression(fs);
  return NULL;
}


static rasqal_algebra_node*
rasqal_algebra_graph_pattern_to_algebra(rasqal_query* query,
                                        rasqal_graph_pattern* gp)
{
  rasqal_algebra_node* node=NULL;
  
  switch(gp->op) {
    case RASQAL_GRAPH_PATTERN_OPERATOR_BASIC:
      node=rasqal_algebra_basic_graph_pattern_to_algebra(query, gp);
      break;

    case RASQAL_GRAPH_PATTERN_OPERATOR_UNION:
      node=rasqal_algebra_union_graph_pattern_to_algebra(query, gp);
      break;
      
    case RASQAL_GRAPH_PATTERN_OPERATOR_OPTIONAL:
    case RASQAL_GRAPH_PATTERN_OPERATOR_GROUP:
      node=rasqal_algebra_group_graph_pattern_to_algebra(query, gp);
      break;
      
    case RASQAL_GRAPH_PATTERN_OPERATOR_GRAPH:

    case RASQAL_GRAPH_PATTERN_OPERATOR_UNKNOWN:
    default:
      RASQAL_DEBUG3("Unsupported graph pattern operator %s (%d)\n",
                    rasqal_graph_pattern_operator_as_string(gp->op),
                    gp->op);
      break;
  }

#if RASQAL_DEBUG
  if(!node)
    abort();
#endif
  
#if RASQAL_DEBUG > 1
  RASQAL_DEBUG1("Resulting node:\n");
  rasqal_algebra_node_print(node, stderr);
  fputc('\n', stderr);
#endif

  return node;
}


/*
 * rasqal_algebra_node_is_empty:
 * @node: #rasqal_algebra_node node
 *
 * INTERNAL - Check if a an algebra node is empty
 * 
 * Return value: non-0 if empty
 **/
int
rasqal_algebra_node_is_empty(rasqal_algebra_node* node)
{
  return (node->op == RASQAL_ALGEBRA_OPERATOR_BGP && !node->triples);
}


static int
rasqal_algebra_remove_znodes(rasqal_query* query, rasqal_algebra_node* node,
                             void* data)
{
  int* modified=(int*)data;
  int is_z1;
  int is_z2;
  
  /* Look at 2-node operations and see if they can be merged */
  if(!node->node1 || !node->node2)
    return 0;

  is_z1=rasqal_algebra_node_is_empty(node->node1);
  is_z2=rasqal_algebra_node_is_empty(node->node2);
  
#if RASQAL_DEBUG > 1
  RASQAL_DEBUG1("Checking node\n");
  rasqal_algebra_node_print(node, stderr);
  fprintf(stderr, "\nNode 1 (%s): %s\n", 
          is_z1 ? "empty" : "not empty",
          rasqal_algebra_node_operator_as_string(node->node1->op));
  fprintf(stderr, "Node 2 (%s): %s\n", 
          is_z2 ? "empty" : "not empty",
          rasqal_algebra_node_operator_as_string(node->node2->op));
#endif

  if(is_z1 && !is_z2) {
    /* Replace join(Z, A) by A */
    memcpy(node, node->node2, sizeof(rasqal_algebra_node));
    /* an empty node has no extra things to free */
    RASQAL_FREE(rasqal_algebra_node, node->node1);
    *modified=1;
  } else if(!is_z1 && is_z2) {
    /* Replace join(A, Z) by A */
    memcpy(node, node->node1, sizeof(rasqal_algebra_node));
    /* ditto */
    RASQAL_FREE(rasqal_algebra_node, node->node2);
    *modified=1;
  }

  return 0;
}


/**
 * rasqal_algebra_query_to_algebra:
 * @query: #rasqal_query to operate on
 *
 * Turn a graph pattern into query algebra structure
 *
 * Return value: algebra expression or NULL on failure
 */
rasqal_algebra_node*
rasqal_algebra_query_to_algebra(rasqal_query* query)
{
  rasqal_graph_pattern* query_gp;
  rasqal_algebra_node* node;
  int modified=0;

  query_gp=rasqal_query_get_query_graph_pattern(query);
  if(!query_gp)
    return NULL;
  
  node=rasqal_algebra_graph_pattern_to_algebra(query, query_gp);

  if(!node)
    return node;


  rasqal_algebra_node_visit(query, node, 
                            rasqal_algebra_remove_znodes,
                            &modified);

#if RASQAL_DEBUG > 1
  RASQAL_DEBUG2("modified=%d after remove zones, algebra ndoe now:\n  ", modified);
  rasqal_algebra_node_print(node, stderr);
  fputs("\n", stderr);
#endif

  return node;
}


#endif

#ifdef STANDALONE
#include <stdio.h>

#define QUERY_LANGUAGE "sparql"
#define QUERY_FORMAT "\
         PREFIX ex: <http://example.org/ns#/> \
         SELECT $subject \
         FROM <http://librdf.org/rasqal/rasqal.rdf> \
         WHERE \
         { $subject ex:predicate $value . \
           FILTER (($value + 1) < 10) \
         }"


int main(int argc, char *argv[]);

int
main(int argc, char *argv[]) {
  char const *program=rasqal_basename(*argv);
  const char *query_language_name=QUERY_LANGUAGE;
  const unsigned char *query_format=(const unsigned char *)QUERY_FORMAT;
  int failures=0;
#define FAIL do { failures++; goto tidy; } while(0)
  rasqal_world *world;
  rasqal_query* query=NULL;
  rasqal_literal *lit1=NULL, *lit2=NULL;
  rasqal_expression *expr1=NULL, *expr2=NULL;
  rasqal_expression* expr=NULL;
  rasqal_algebra_node* node0=NULL;
  rasqal_algebra_node* node1=NULL;
  rasqal_algebra_node* node2=NULL;
  rasqal_algebra_node* node3=NULL;
  rasqal_algebra_node* node4=NULL;
  rasqal_algebra_node* node5=NULL;
  rasqal_algebra_node* node6=NULL;
  rasqal_algebra_node* node7=NULL;
  raptor_uri *base_uri=NULL;
  unsigned char *uri_string;
  rasqal_graph_pattern* query_gp;
  raptor_sequence* triples;

  world=rasqal_new_world();
  if(!world)
    FAIL;
  
  uri_string=raptor_uri_filename_to_uri_string("");
  if(!uri_string)
    FAIL;
  base_uri=raptor_new_uri(uri_string);  
  if(!base_uri)
    FAIL;
  raptor_free_memory(uri_string);
  
  query=rasqal_new_query(world, query_language_name, NULL);
  if(!query) {
    fprintf(stderr, "%s: creating query in language %s FAILED\n", program,
            query_language_name);
    FAIL;
  }

  if(rasqal_query_prepare(query, query_format, base_uri)) {
    fprintf(stderr, "%s: %s query prepare FAILED\n", program, 
            query_language_name);
    FAIL;
  }

  lit1=rasqal_new_integer_literal(world, RASQAL_LITERAL_INTEGER, 1);
  if(!lit1)
    FAIL;
  expr1=rasqal_new_literal_expression(lit1);
  if(!expr1)
    FAIL;
  lit1=NULL; /* now owned by expr1 */

  lit2=rasqal_new_integer_literal(world, RASQAL_LITERAL_INTEGER, 1);
  if(!lit2)
    FAIL;
  expr2=rasqal_new_literal_expression(lit2);
  if(!expr2)
    FAIL;
  lit2=NULL; /* now owned by expr2 */

  expr=rasqal_new_2op_expression(RASQAL_EXPR_PLUS, expr1, expr2);
  if(!expr)
    FAIL;
  expr1=NULL; expr2=NULL; /* now owned by expr */
  
  node0=rasqal_new_empty_algebra_node(query);
  if(!node0)
    FAIL;

  node1=rasqal_new_filter_algebra_node(query, expr, node0);
  if(!node1) {
    fprintf(stderr, "%s: rasqal_new_filter_algebra_node() failed\n", program);
    FAIL;
  }
  node0=NULL; expr=NULL; /* now owned by node1 */
  
  fprintf(stderr, "%s: node result: \n", program);
  rasqal_algebra_node_print(node1, stderr);
  fputc('\n', stderr);

  rasqal_free_algebra_node(node1);


  /* construct abstract nodes from query structures */
  query_gp=rasqal_query_get_query_graph_pattern(query);

  /* make a filter node around first (and only) expresion */
  expr=rasqal_graph_pattern_get_constraint(query_gp, 0);
  expr=rasqal_new_expression_from_expression(expr);
  node1=rasqal_new_filter_algebra_node(query, expr, NULL);

  if(!node1) {
    fprintf(stderr, "%s: rasqal_new_filter_algebra_node() failed\n", program);
    FAIL;
  }
  expr=NULL; /* now owned by node1 */

  fprintf(stderr, "%s: node1 result: \n", program);
  rasqal_algebra_node_print(node1, stderr);
  fputc('\n', stderr);


  /* make an triples node around first (and only) triple pattern */
  triples=rasqal_query_get_triple_sequence(query);
  node2=rasqal_new_triples_algebra_node(query, triples, 0, 0);
  if(!node2)
    FAIL;

  fprintf(stderr, "%s: node2 result: \n", program);
  rasqal_algebra_node_print(node2, stderr);
  fputc('\n', stderr);


  node3=rasqal_new_2op_algebra_node(query,
                                    RASQAL_ALGEBRA_OPERATOR_JOIN,
                                    node1, node2);

  if(!node3)
    FAIL;
  
  /* these become owned by node3 */
  node1=node2=NULL;
  
  fprintf(stderr, "%s: node3 result: \n", program);
  rasqal_algebra_node_print(node3, stderr);
  fputc('\n', stderr);

  node4=rasqal_new_empty_algebra_node(query);
  if(!node4)
    FAIL;

  fprintf(stderr, "%s: node4 result: \n", program);
  rasqal_algebra_node_print(node4, stderr);
  fputc('\n', stderr);

  node5=rasqal_new_2op_algebra_node(query,
                                    RASQAL_ALGEBRA_OPERATOR_UNION,
                                    node3, node4);

  if(!node5)
    FAIL;
  
  /* these become owned by node5 */
  node3=node4=NULL;
  
  fprintf(stderr, "%s: node5 result: \n", program);
  rasqal_algebra_node_print(node5, stderr);
  fputc('\n', stderr);



  lit1=rasqal_new_boolean_literal(world, 1);
  if(!lit1)
    FAIL;
  expr1=rasqal_new_literal_expression(lit1);
  if(!expr1)
    FAIL;
  lit1=NULL; /* now owned by expr1 */

  node6=rasqal_new_empty_algebra_node(query);
  if(!node6)
    FAIL;

  node7=rasqal_new_leftjoin_algebra_node(query, node5, node6, expr1);
  if(!node7)
    FAIL;
  /* these become owned by node7 */
  node5=node6=NULL;
  expr1=NULL;
  
  fprintf(stderr, "%s: node7 result: \n", program);
  rasqal_algebra_node_print(node7, stderr);
  fputc('\n', stderr);


  tidy:
  if(lit1)
    rasqal_free_literal(lit1);
  if(lit2)
    rasqal_free_literal(lit2);
  if(expr1)
    rasqal_free_expression(expr1);
  if(expr2)
    rasqal_free_expression(expr2);

  if(node7)
    rasqal_free_algebra_node(node7);
  if(node6)
    rasqal_free_algebra_node(node6);
  if(node5)
    rasqal_free_algebra_node(node5);
  if(node4)
    rasqal_free_algebra_node(node4);
  if(node3)
    rasqal_free_algebra_node(node3);
  if(node2)
    rasqal_free_algebra_node(node2);
  if(node1)
    rasqal_free_algebra_node(node1);
  if(node0)
    rasqal_free_algebra_node(node0);

  if(query)
    rasqal_free_query(query);
  if(base_uri)
    raptor_free_uri(base_uri);
  if(world)
    rasqal_free_world(world);
  
  return failures;
}
#endif