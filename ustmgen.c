/* NOTES:
 * - bytecode file format especially with constants
 *
 */

/* TBD:
 * - static err var & return code func for gen, lex, & vm
 * - error handling & cleanup
 * - write to output file and pass that file in or a buf
 */

#include "ustm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* FROM: 'ustm.c' */
extern int ustm_err;

/* FROM: 'ustmlex.c' */
extern int tk;
extern char tk_val_txt[MAX_STR_LEN];

// static fields;

static FILE* out_fp;

/* int bits atleast 16: 0b0000000000000_000 */
// arr_szs: 0b 00000000_00000000_00000000_00000000
// arr_flags: 0b 00_00_00_00
// struct_fields: 0b 0000
// ty_data: 0b 00_00_000

#define SZ_STATIC  
#define SZ_DYNAMIC 0x8000 // 0b1000000000000000

#define SET_TY(ty) ty 

/* TBD: prob remove this & 'MAX_IDS' if using heap */
#define MAX_SYMS 128


struct TySpec {
  union {
    unsigned int* fields;
    int struct_ref;
  } aggr;
  //struct TySpec* ptr;
  int flags;
  /* arr or struct */
  //int fields_n;
};

static struct Sym {
  struct TySpec ty_spec;
  
} syms[MAX_SYMS];

#define TY 2
#define MOD_CONST 0x1234567890

#define IS_UD_ID(tk) ((tk) >= TK_UD_ID_FIRST)

/* static immense_bool
sym_find(int tk)
{
  if (tk < TK_UD_ID_FIRST) return NULL;
} */

// TODO: 'ref' param handling
static struct TySpec
read_decl_spec(bool allow_modifiers)
{
  struct TySpec ty_spec;
  switch (tk) {
  case TK_CONST: {ty_spec.flags |= MOD_CONST; lex_next(); break;}
  /* ... */
  default:
    ;
  }
  
  switch (tk) {
  case TK_BOOL: ty_spec.flags|= TY_BOOL;
  case TK_CHAR: ty_spec.flags |= TY_CHAR;
  case TK_INT: ty_spec.flags |= TY_INT;
  case TK_NUM: ty_spec.flags |= TY_NUM;
  default: err("uhhh");
  }
  
  for (lex_next() == '[' && (ty_spec.flags |= TY_ARR); lex_next() == '[';) {
    lex_next();
    if (tk == ']') {
      
      continue;
    }
      
    if (tk == '*') {
      // optional expr
    } else {
      // if (!const expr)
    }
    if (tk != ']') {err("Expected closing ']'"); break;}
  }
  return ty_spec;
}


static void
gen_decl(void)
{
  struct TySpec ty_spec = read_decl_spec(true);
  if (!IS_UD_ID(tk)) {err("Expected identifier"); return;}
  lex_next();
  if (tk == ';'); // TBD: null or garbage
  else if (tk == '='); //gen_expr();
  
}

static void
gen_block(void)
{
  // NOTE: first tk set by 'lex_init'
  printf("TK: %d\n", tk);
  switch (tk) {
  case EOF: break;
  case TK_REF:
  case TK_BOOL: case TK_CHAR: case TK_INT: case TK_NUM:
    gen_decl();
    break;
  case TK_IF:
  case TK_WHILE:
  case TK_FOR:
  case TK_CASES:
  case TK_JMP:
  case TK_FN:
  default:
    
  }
}

void
gen_code(FILE* src_fp_arg, FILE* out_fp_arg)
{
  out_fp = out_fp_arg;
  lex_init(src_fp_arg);
  // gen_headers();
  // gen_const_table();
  gen_block();
}
