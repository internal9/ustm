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
#include <stdio.h>

/* FROM: 'ustmlex.c' */
extern char tk_val_txt[MAX_STR_LEN];

static FILE* out_fp;



/* static immense_bool
sym_find(int tk)
{
  if (tk < TK_UD_ID_FIRST) return NULL;
} */

static void
gen_block(void)
{
  printf("%d\n", lex_next());
  /* switch (lex_next()) {
  case EOF:
  case TK_IF:
  case TK_DO:
  case TK_WHILE:
  case TK_FOR:
  case TK_CASES:
  case TK_JMP:
  case TK_FN: */
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
