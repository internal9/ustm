/* ustm.h */
#include <stdio.h>
#include <ctype.h>
#include <limits.h>

#define err(msg)          (fprintf(stderr, msg "\n"), (ustm_err = 1))
#define err_fmt(fmt, ...) (fprintf(stderr, fmt "\n", __VA_ARGS__), (ustm_err = 1))

#define MAX_STR_LEN 512

/* NOTES:
 * - single-char tks' values each == respective native char code
 * - unsure if even beyond 255, special things are reserved
 */

// single-char tks <= 255 (UCHAR_MAX)
enum {
#define TK_CMPLX_FIRST (UCHAR_MAX + 1)

  TK_INC = TK_CMPLX_FIRST,
  TK_DEC,
  TK_ADD_AS,
  TK_SUB_AS,
  TK_MUL_AS,
  TK_DIV_AS,
  TK_POW_AS,
  TK_MOD_AS,

  TK_EQ,
  TK_LEQ,
  TK_GEQ,
  TK_L_OR,
  TK_L_AND,

  TK_NOT,
  TK_NOT_AS,
  TK_OR,
  TK_OR_AS,
  TK_AND,
  TK_AND_AS,
  TK_XOR,
  TK_XOR_AS,
  TK_SH_L,
  TK_SH_L_AS,
  TK_SH_R,
  TK_SH_R_AS,
  
  TK_LIT_CHAR,
  TK_LIT_STR,
  TK_LIT_INT,
  TK_LIT_NUM,
  
#define TK_KW_FIRST (TK_LIT_NUM + 1)

  TK_NULL = TK_KW_FIRST,
  TK_TRUE,
  TK_FALSE,
  TK_BOOL,
  TK_CHAR,
  TK_INT,
  TK_NUM,
  TK_STRING,
  TK_ARRAY,
  TK_STRUCT,
  TK_IF,
  TK_ELIF,
  TK_ELSE,
  TK_DO,
  TK_WHILE,
  TK_FOR,
  TK_CASES,
  TK_JMP,
  TK_FN,
  TK_REF
  
  // NOTE: 'UD' => 'user-defined'
  // NOTE: Will be used for string-interning (unles string len fitsinstr)
#define TK_UD_ID_FIRST (TK_REF + 1)
};

/* ustmlex.c */
void lex_init(FILE* src_fp_arg);
int  lex_next(void);
/* ustmgen.c */
void gen_code(FILE* src_fp_arg, FILE* out_fp_arg);
/* ustmvm.c  */
void vm_run(FILE* byc_fp);
