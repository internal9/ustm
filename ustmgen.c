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

/* TBD: prob remove 'MAX_SYMS' & 'MAX_IDS' if using heap */
#define MAX_SYMS 256
#define skip_cases_err_(tk_, expected_tk_) ( (expected_tk_ == TK_UD_ID_FIRST) ? (tk_ < TK_UD_ID_FIRST) : (tk_ != expected_tk_) )
// iso c warning lol
#define skip_cur(expected_tk)  ( skip_cases_err_(tk, expected_tk) ? err("Expected 'tk' got 'tk'") : lex_next() )
#define skip_next(expected_tk) ( (lex_next() != expected_tk) ? err("Expected 'tk' got 'tk'") : lex_next() )

#define gen_err_fmt(fmt, ...) err_fmt("[%d,%d]: " fmt, tk_line, tk_column, __VA_ARGS__)
#define gen_err(msg)          err_fmt("[%d,%d]: " msg, tk_line, tk_column)

/* FROM: 'ustm.c' */
extern struct Arena arena;
/* FROM: 'ustmlex.c' */
extern int tk;
extern int tk_line, tk_column;
extern char tk_val_txt[MAX_STR_LEN];
extern int txt_n;
extern ustm_int tk_val_int;  // NOTE: Holds character values too
extern ustm_num tk_val_num;

static FILE* out_fp;


/* int bits atleast 16: 0b0000000000000_000 */
// arr_szs: 0b 00000000_00000000_00000000_00000000
// arr_flags: 0b 00_00_00_00
// struct_fields: 0b 0000
// ty_data: 0b 00_00_000

#define SZ_STATIC  
#define SZ_DYNAMIC 0x8000 // 0b1000000000000000

#define SET_TY(ty) ty 

struct TySpec {
  union {
    union {
      struct TySpec* fields;
      int ref;
    } struct_data;
    struct {
      struct TySpec* ptr;
      int len;
    } arr_data;
  } v;
  int flags;
};

static struct Sym {
  struct TySpec ty_spec;
  
} syms[MAX_SYMS];

#define TY 0x7 // 0b111

#define IS_UD_ID(tk) ((tk) >= TK_UD_ID_FIRST)

/* static immense_bool
sym_find(int tk)
{
  if (tk < TK_UD_ID_FIRST) return NULL;
} */

#define UNARY_PREC 50

static int
tk_to_ty(int tk)
{
  switch (tk) {
  case TK_NULL: return TY_NULL;
  case TK_TRUE:
  case TK_FALSE: return TY_BOOL;
  case TK_LIT_CHAR: return TY_CHAR;
  case TK_LIT_INT: return TY_LIT;
  case TK_LIT_NUM: return TY_NUM;
  default: return -1;
  }
}

static int
gen_expr(int prec_limit, int dest, bool is_stack_dest)
{
  /* regs used in order of incremental count, de-allocated regs decremented in order too */
  int regs_used = 0;
  int prec;
  int ty1, ty2, ty_final;
  do {
    // TBD: post & pre-fix unary operators '++', '--'
    switch (tk) {
    case '-':
      lex_next();
      ty1 = gen_expr(UNARY_PREC, -1, 0);
      if (ty1 != TY_INT && ty1 != TY_NUM) gen_err("Expected integer or number for unary '-'");
      break;
    case '!':
      lex_next();
      ty2 = gen_expr(UNARY_PREC, -1, 0);
      if (!IS_TY_TRUTHY(ty2)) gen_err("Cannot negate 'tk'");
      ty1 = TY_BOOL;
      break;
    case '(':
      lex_next();
      ty1 = gen_expr();
      skip_cur(')');
      break;
    case TK_NULL:
    case TK_TRUE:
    case TK_FALSE:
    case TK_LIT_CHAR:
    case TK_LIT_LIT:
      ty1 = tk_to_ty(tk);
      // emit(MOV REGS[regs_used], tk_val_int);
      regs_used++;
      break;
    case TK_LIT_NUM:
      ty1 = TY_NUM;
      // emit(MOV FP_REGS[regs_used], tk_val_num);
      regs_used++;
      break;
    default:
      // UD_ID
    }
    
    switch (lex_next()) {
    case '?': prec = 0; break;
    case '-':
    case '+': prec = 1; break;
    case '/':
    case '*':
    case '%': prec = 2; break;
    case '^': prec = 3; break;
    // TBD: handling postfix unary ops
    //case TK_INC:
    //case TK_DEC:
    
    // NOTE: logical ops
    case TK_EQ: 
    case TK_NEQ:
    case TK_LEQ:
    case TK_GEQ: prec = 1; break;
    // default:
    }
    
    // TODO: handle '^' prec
  } while (prec > prec_limit); //tk != EOF);
  
  return ty_final;
}

// TODO: 'ref' param handling
static struct TySpec*
read_decl_spec(bool allow_modifiers)
{
  struct TySpec* ty_spec = arena_alloc(&arena, sizeof(struct TySpec));
  switch (tk) {
  case TK_CONST: {ty_spec->flags |= MOD_CONST; lex_next(); break;}
  /* ... */
  default:
    ;
  }
  
  switch (tk) {
  case TK_BOOL: ty_spec->flags|= TY_BOOL; break;
  case TK_CHAR: ty_spec->flags |= TY_CHAR; break;
  case TK_INT: ty_spec->flags |= TY_INT; break;
  case TK_NUM: ty_spec->flags |= TY_NUM; break;
  default: err("uhhh");
  }
  
  // TODO: reusable arr & struct indexing function
  if (lex_next() == '[') {
    ty_spec->flags |= TY_ARR;
    for (int i; lex_next() == '['; i++) {
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
  }
  return ty_spec;
}


static void
gen_decl(void)
{
  puts("FOOO");
  struct TySpec* ty_spec = read_decl_spec(true);
  //if (!IS_UD_ID(tk)) err("Expected identifier");
  skip_cur(TK_UD_ID_FIRST);
  lex_next();
  if (tk == '='); //gen_expr();
  
}

static void
gen_stmt(void)
{
  
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
    gen_stmt();
    break;
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
