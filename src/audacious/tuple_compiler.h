/*
 * Audacious - Tuplez compiler
 * Copyright (c) 2007 Matti 'ccr' H�m�l�inen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; under version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses>.
 *
 * The Audacious team does not consider modular code linking to
 * Audacious or using our public API to be a derived work.
 */
#ifndef __AUDACIOUS_TUPLE_COMPILER_H__
#define __AUDACIOUS_TUPLE_COMPILER_H__

#include <glib.h>
#include <mowgli.h>
#include "tuple.h"


#define MAX_VAR		(4)
#define MAX_STR		(256)
#define MIN_ALLOC_NODES (8)
#define MIN_ALLOC_BUF	(64)


enum {
    OP_RAW = 0,		/* plain text */
    OP_FIELD,		/* a field/variable */
    OP_EXISTS,
    OP_DEF_STRING,
    OP_DEF_INT,
    OP_EQUALS,
    OP_NOT_EQUALS,
    OP_GT,
    OP_GTEQ,
    OP_LT,
    OP_LTEQ,
    OP_IS_EMPTY,

    OP_FUNCTION,	/* function */
    OP_EXPRESSION	/* additional registered expressions */
};


enum {
    VAR_FIELD = 0,
    VAR_CONST,
    VAR_DEF
};
    

/* Caching structure for deterministic functions
 */
typedef struct {
    gchar *name;
    gboolean isdeterministic;
    gchar *(*func)(Tuple *tuple, gchar **argument);
} TupleEvalFunc;


typedef struct {
    gchar *name;
    gboolean islocal;		/* Local? true = will be cleaned with tuple_evalctx_reset() */
    gint type;			/* Type of this "variable", see VAR_* */
    gchar *defval;
    TupleValue *dictref;	/* Cached tuple value ref */
} TupleEvalVar;


typedef struct _TupleEvalNode {
    gint opcode;		/* operator, see OP_ enums */
    gint var[MAX_VAR];		/* tuple / global variable references (perhaps hashes, or just indexes to a list?) */
    gboolean global[MAX_VAR];
    gchar *text;		/* raw text, if any (OP_RAW) */
    gint function, expression;	/* for OP_FUNCTION and OP_EXPRESSION */
    struct _TupleEvalNode *children, *next, *prev; /* children of this struct, and pointer to next node. */
} TupleEvalNode;


typedef struct {
    gint nvariables, nfunctions, nexpressions;
    TupleEvalVar **variables;
    TupleEvalFunc **functions;
} TupleEvalContext;


TupleEvalContext * tuple_evalctx_new(void);
void tuple_evalctx_reset(TupleEvalContext *ctx);
void tuple_evalctx_free(TupleEvalContext *ctx);
gint tuple_evalctx_add_var(TupleEvalContext *ctx, gchar *name, gboolean islocal, gint type);

void tuple_evalnode_free(TupleEvalNode *expr);

TupleEvalNode *tuple_formatter_compile(TupleEvalContext *ctx, gchar *expr);
gchar *tuple_formatter_eval(TupleEvalContext *ctx, TupleEvalNode *expr, Tuple *tuple);
/*
gchar *tuple_formatter_make_title_string(Tuple *tuple, const gchar *string);
*/

#endif