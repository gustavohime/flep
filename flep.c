/*
 * FLEP - Fast Lite Expression Parser
 * Copyright (C) 2019 Gustavo Hime
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>. 
 */

/* Parser and evaluator for math expressions
 * user interface is through flep_parse and flep_eval functions, ONLY.
 * parser converts parethesized expressions into RNP.
 */
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flep.h"

/* Codes used internally for tokenizing and runtime opcodes
 * Also used as return values/error codes
 */
#define FLEP_UNARY_MINUS  1
#define FLEP_OPEN         2
#define FLEP_CLOSE        3
#define FLEP_PLUS         4
#define FLEP_MINUS        5
#define FLEP_MULT         6
#define FLEP_DIV          7
#define FLEP_POWER        8
#define FLEP_VAR          9
#define FLEP_CONST       10
#define FLEP_SIN         11
#define FLEP_COS         12
#define FLEP_TAN         13
#define FLEP_EXP         14
#define FLEP_LOG         15
#define FLEP_ABS         16
#define FLEP_SQRT        17
#define FLEP_START       18
#define FLEP_END         19
/* public (return) values in include file: 
define FLEP_OK           
define FLEP_BADSYNTAX    
define FLEP_BADTOKEN     
define FLEP_EXPECTED_OPEN
define FLEP_UNBALANCED
*/


/* joins/retrieves an integer parameter with the FLEP_VAR or FLEP_CONST in
 * FLEPCode's "text" array.
 */
#define FLEP_BITFUSE(a,b) (a | ((b)<<8))
#define FLEP_OPCODE(a) (a & 0xff)
#define FLEP_OPPARM(a) ((a)>>8)

/* matching 'enum' FLEP_* defines */
const char* dbg_strings[] = {
  "FLEP_OK",
  "FLEP_UNARY_MINUS",
  "FLEP_OPEN",
  "FLEP_CLOSE",
  "FLEP_PLUS",
  "FLEP_MINUS",
  "FLEP_MULT",
  "FLEP_DIV",
  "FLEP_POWER",
  "FLEP_VAR",
  "FLEP_CONST",
  "FLEP_SIN",
  "FLEP_COS",
  "FLEP_TAN",
  "FLEP_EXP",
  "FLEP_LOG",
  "FLEP_ABS",
  "FLEP_SQRT",
  "FLEP_START",
  "FLEP_END",
  "FLEP_BADSYNTAX",
  "FLEP_BADTOKEN",
  "FLEP_EXPECTED_OPEN",
  "FLEP_UNBALANCED"};

const char* flep_translate(int c) {
  return dbg_strings[c];
}

/* Stores RPN representation of parenthesized expression */
struct FLEP {
  double *data; /* numerical constants */
  int *text; /* opcodes */
  int sd, st; /* allocated size of the above */
  int nd, nt; /* number of used elements in the above */
};

/* token stream "object" */
struct FLEPTokens {
  const char *src, *p, *q;
  int ival;
  double fval;
  int curr, last; /* 'enum' FLEP_* */
};

/* advance token stream */
static int flep_next(struct FLEPTokens *tok) {
  tok->last = tok->curr;
  tok->p = tok->q;
  while (isspace((int)*tok->p)) tok->p++;
  tok->curr = FLEP_BADTOKEN;
  if (!*tok->p) {
    tok->curr = FLEP_END;
  } else if (isalpha((int)*tok->p)) {
    tok->q = tok->p + 1;
    while(isalpha((int)*tok->q)) tok->q++;
    if (tok->q - tok->p == 1) {
      if (*tok->p == 'e') {
	tok->curr = FLEP_CONST;
	tok->fval = exp(1);
      } else {
	static const char vars[] = "abcxyzw";
	int i = strchr(vars, *tok->p) - vars;
	if (i >= 0) {
	  tok->curr = FLEP_VAR;
	  tok->ival = i;
	}
      }
    } else if (tok->q - tok->p == 2) {
      if (!strncmp(tok->p, "pi", 2)) {
	tok->curr = FLEP_CONST;
	tok->fval = M_PI;
      }
    } else if (tok->q - tok->p == 3) {
      /* Order is critical - search for "FLEPCodeDep" to see related data */
      static const char* built_in[] = {
	"sin", "cos", "tan", "exp", "log", "abs"}; 
      static int n_built_in = sizeof(built_in) / sizeof(char*);
      int i;
      for (i = 0; i < n_built_in; i++) {
	if (!strncmp(tok->p, built_in[i], 3)) {
	  tok->curr = FLEP_SIN + i;
	  break;
	}
      }
    } else if (tok->q - tok->p == 4) {
      if (!strncmp(tok->p, "sqrt", 4)) {
	tok->curr = FLEP_SQRT;
      }
    }
  } else if (isdigit((int)*tok->p)) {
    char *ftail = 0;
    tok->fval = (double)strtod(tok->p, &ftail);
    if (ftail) {
      tok->curr = FLEP_CONST;
      tok->q = ftail;
    }
  } else {
    /* Order is critical - search for "FLEPCodeDep" to see related data */
    static const char sym[] = "()+-*/^";
    int i = strchr(sym, *tok->p) - sym;
    if (i >= 0) {
      tok->curr = (int)(FLEP_OPEN) + i;
      tok->q = tok->p + 1;
    }
  }
  return tok->curr;
}
  
/* initialize token stream */
static int flep_tokenize(struct FLEPTokens* t, const char* s) {
  t->src = t->p = t->q = s;
  t->curr = FLEP_START;
  return flep_next(t);
}

/* recursive parser gets sums of products of powers of operands, these last
 * consisting of literals, variables or parenthesized expressions, these
 * last consisting of sums of...
 */
static int flep_get_sum(struct FLEPTokens* tok, struct FLEP* out);
static int flep_get_prod(struct FLEPTokens* tok, struct FLEP* out);
static int flep_get_power(struct FLEPTokens* tok, struct FLEP* out);

void flep_accomodate_text(struct FLEP *out, int n) {
  while(out->st < n) out->st *= 2;
  out->text = (int*)realloc(out->text, out->st * sizeof(double));
}

void flep_accomodate_data(struct FLEP *out, int n) {
  while(out->sd < n) out->sd *= 2;
  out->data = (double*)realloc(out->data, out->sd * sizeof(double));
}

void flep_add_opcode(struct FLEP *out, int op) {
    flep_accomodate_text(out, out->nt+1);
    out->text[out->nt++] = op;
}

void flep_add_data(struct FLEP *out, double val) {
    flep_accomodate_data(out, out->nd+1);
    out->data[out->nd++] = val;
}

static int flep_get_operand(struct FLEPTokens* tok, struct FLEP* out) {
  int ret = FLEP_BADSYNTAX;
  if (tok->curr == FLEP_OPEN) {
    flep_next(tok);
    ret = flep_get_sum(tok, out);
    if (ret != FLEP_CLOSE) return FLEP_UNBALANCED;
    ret = flep_next(tok);
  } else if (tok->curr == FLEP_PLUS) {
    if (tok->last == FLEP_START || tok->last == FLEP_OPEN ||
        tok->last == FLEP_PLUS || tok->last == FLEP_MULT ||
        tok->last == FLEP_DIV || tok->last == FLEP_POWER) {
      flep_next(tok);
      ret = flep_get_operand(tok, out);
    }
  } else if (tok->curr == FLEP_MINUS) {
    if (tok->last == FLEP_START || tok->last == FLEP_OPEN ||
        tok->last == FLEP_MULT || tok->last == FLEP_DIV) {
      flep_next(tok);
      ret = flep_get_prod(tok, out);
      flep_add_opcode(out, FLEP_UNARY_MINUS);
    } else if (tok->last == FLEP_POWER) {
      flep_next(tok);
      ret = flep_get_power(tok, out);
      flep_add_opcode(out, FLEP_UNARY_MINUS);
    } else {
      flep_next(tok);
      ret = flep_get_operand(tok, out);
      flep_add_opcode(out, FLEP_MINUS);
    }
  } else if (tok->curr >= FLEP_SIN && tok->curr <= FLEP_SQRT) {
    /* Order is critical - search for "FLEPCodeDep" to see related data */
    int op = tok->curr;
    flep_next(tok);
    if (tok->curr != FLEP_OPEN) return FLEP_EXPECTED_OPEN;
    ret = flep_get_operand(tok, out);
    flep_add_opcode(out, op);
  } else if (tok->curr == FLEP_CONST) {
    flep_add_data(out, tok->fval);
    flep_add_opcode(out, FLEP_BITFUSE(FLEP_CONST,out->nd-1));
    ret = flep_next(tok);
  } else if (tok->curr == FLEP_VAR) {
    flep_add_opcode(out, FLEP_BITFUSE(FLEP_VAR,tok->ival));
    ret = flep_next(tok);
  }
  return ret;
}

static int flep_get_power(struct FLEPTokens* tok, struct FLEP* out) {
  int ret = flep_get_operand(tok, out);
  while (ret == FLEP_POWER) {
    flep_next(tok);
    ret = flep_get_power(tok, out);
    flep_add_opcode(out, FLEP_POWER);
  }
  return ret;
}

static int flep_get_prod(struct FLEPTokens* tok, struct FLEP* out) {
  int ret = flep_get_power(tok, out);
  while (ret == FLEP_MULT || ret == FLEP_DIV) {
    int op = ret;
    flep_next(tok);
    ret = flep_get_power(tok, out);
    flep_add_opcode(out, op);
  }
  return ret;
}

static int flep_get_sum(struct FLEPTokens* tok, struct FLEP* out) {
  int ret = flep_get_prod(tok, out);
  while (ret == FLEP_PLUS || ret == FLEP_MINUS) {
    int op = ret;
    ret = flep_next(tok);
    while (ret == FLEP_PLUS || ret == FLEP_MINUS) {
      /* deal with malformed expressions like "a+-b" or "x--y" */
      op = (op == ret) ? FLEP_PLUS : FLEP_MINUS;
      ret = flep_next(tok);
    }
    ret = flep_get_prod(tok, out);
    flep_add_opcode(out, op);
  }
  return ret;
}
/* helper for "flep_optimize" */
static void flep_delete_text(struct FLEP* out, int i, int n) {
  for (int j = i+n; j < out->nt; j++) {
    out->text[j-n] = out->text[j];
  }
  out->nt -= n;
}

/* Very basic compiled-expression optimization */
static void flep_optimize(struct FLEP* out) {
  for (int i = out->nt-2; i >= 0; i--) {
    if (out->text[i] == FLEP_UNARY_MINUS &&
	out->text[i-1] == FLEP_UNARY_MINUS) {
      flep_delete_text(out, i-1, 2); /* remove 2 successive sign changes */
      i--; continue;
    }
    if (FLEP_OPCODE(out->text[i]) == FLEP_CONST && i < out->nt-1) {
      /* (maybe) resolve unary operations on constants */
      int maybe_found_unary = 1;
      int dpx = FLEP_OPPARM(out->text[i]);
      double x = out->data[dpx];
      switch (out->text[i+1]) { 
	case FLEP_UNARY_MINUS: x = -x; break;
	case FLEP_SIN: x = sin(x); break;
	case FLEP_COS: x = cos(x); break;
	case FLEP_TAN: x = tan(x); break;
	case FLEP_EXP: x = exp(x); break;
	case FLEP_LOG: x = log(x); break;
	case FLEP_ABS: x = fabs(x); break;
	case FLEP_SQRT: x = sqrt(x); break;
	default: maybe_found_unary = 0;
      }
      if (maybe_found_unary) {
	out->data[dpx] = x;
	flep_delete_text(out, i+1, 1);
	i++; continue;
      } else {
	if (i && FLEP_OPCODE(out->text[i-1]) == FLEP_CONST) {
	  /* (maybe) resolve binary operations on constants on constants */
	  int maybe_found_binary = 1;
	  int dpx = FLEP_OPPARM(out->text[i-1]);
	  int dpy = FLEP_OPPARM(out->text[i]);
	  double x = out->data[dpx], y = out->data[dpy];
	  switch (out->text[i+1]) { 
	    case FLEP_PLUS: x += y; break;
	    case FLEP_MINUS: x -= y; break;
	    case FLEP_MULT: x *= y; break;
	    case FLEP_DIV: x /= y; break;
	    case FLEP_POWER: x = pow(x,y); break;
	    default:
	      maybe_found_binary = 0; break;
	  }
	  if (maybe_found_binary) {
	    out->data[dpx] = x;
	    flep_delete_text(out, i, 2);
	    if (FLEP_OPCODE(out->text[i]) == FLEP_CONST) {
	      i++; /* removed operation was between two FLEP_CONST, revisit */
	    }
	    continue;
	  }
	}
      }
    }
  }
}

/* callable functions: */

const struct FLEP* flep_parse(const char* s, int *error, 
  int* position) {

  struct FLEPTokens tok;
  int status;
  flep_tokenize(&tok, s);
  struct FLEP* out = (struct FLEP*)malloc(sizeof(struct FLEP));
  out->text = 0;
  out->data = 0;
  out->st = out->sd = 8;
  flep_accomodate_text(out, 16);
  flep_accomodate_data(out, 16);
  out->nt = out->nd = 0;
  status = flep_get_sum(&tok, out);
  if (status == FLEP_END) {
    flep_optimize(out);
    flep_add_opcode(out, FLEP_END);
    return out;
  } else {
    free(out);
    if (error) *error = status;
    if (position) *position = tok.p - tok.src + 1;
    return 0;
  }
}

/* no mysteries left - use stack to run compiled expression */
double flep_eval(const struct FLEP* f, double* val) {
  double stack[64];
  int ip = 0, sp = -1;
  for (;;ip++) {
    double x = stack[sp];
    int idx = FLEP_OPPARM(f->text[ip]);
    switch (FLEP_OPCODE(f->text[ip])) {
      case FLEP_UNARY_MINUS: stack[sp] = -x; continue;
      case FLEP_PLUS: stack[--sp] += x; continue;
      case FLEP_MINUS: stack[--sp] -= x; continue;
      case FLEP_MULT: stack[--sp] *= x; continue;
      case FLEP_DIV: stack[--sp] /= x; continue;
      case FLEP_POWER: --sp; stack[sp] = pow(stack[sp], x); continue;
      case FLEP_VAR: stack[++sp] = val[idx]; continue;
      case FLEP_CONST: stack[++sp] = f->data[idx]; continue;
      case FLEP_SIN: stack[sp] = sin(x); continue;
      case FLEP_COS: stack[sp] = cos(x); continue;
      case FLEP_TAN: stack[sp] = tan(x); continue;
      case FLEP_EXP: stack[sp] = exp(x); continue;
      case FLEP_LOG: stack[sp] = log(x); continue;
      case FLEP_ABS: stack[sp] = fabs(x); continue;
      case FLEP_SQRT: stack[sp] = sqrt(x); continue;
      case FLEP_END: return stack[0];
    }
  }
  return stack[0];
}

/* ... */
void flep_free(const struct FLEP* f) {
  if (f) {
    free(f->text);
    free(f->data);
    free((void*)f);
  }
}

/* published pretty printer for compiled expression */
void flep_dump(const struct FLEP* f) {
  printf("\n");
  for (int i = 0; i < f->nt; i++) {
    int op = f->text[i];
    switch(FLEP_OPCODE(op)) {
      case FLEP_CONST:
	printf("%d: %s (%12.6f)\n", i, dbg_strings[FLEP_CONST], 
	  f->data[FLEP_OPPARM(op)]);
	break;
      case FLEP_VAR:
	printf("%d: %s (%d)\n", i, dbg_strings[FLEP_VAR], FLEP_OPPARM(op));
	break;
      default:
	printf("%d: %s\n", i, dbg_strings[op]);
    }
  }
}

