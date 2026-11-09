/* NOTES:
 * - bytecode file format especially with constants
 *
 */

/* TBD:
 * - error handling & cleanup
 * - write to output file and pass that file in or a buf
 */

#include <stdio.h>

static void
gen_block(void)
{

}

static void
gen_code(FILE* src_fp, FILE* out_fp)
{
  lex_init(src_fp);
  gen_headers();
  gen_const_table();
  gen_block();
}
