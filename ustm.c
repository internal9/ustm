#include "ustm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>

                  // NOTE: 'abort()' bypasses and goes straight to 'exit(1)'
int ustm_err = 0; // USED BY: ustm.c, lex.c, codegen.c

#define abort(msg) (fprintf(stderr, msg "\n"), exit(1))

static inline bool
check_suffix(const char* str, const char* suffix)
{
  const char* dot = strrchr(str, '.');
  return (dot != NULL) && (dot != str) && (!strcmp(dot, suffix));
}

static FILE*
open_src(const char* name)
{
  if (!check_suffix(name, ".ustm")) {err("Source name requires suffix '.ustm' with preceding text"); return NULL;}
  FILE *src_fp;
  if (!(src_fp = fopen(name, "r"))) {err("Unreadable/non-existent source file"); return NULL;}
  return src_fp;
}

static FILE*
open_out(const char* name)
{
  if (!check_suffix(name, ".ustmbyc")) {err("Outfile name requires suffix '.ustmbyc' with preceding text"); return NULL;}
  FILE *out_fp;
  if (!(out_fp = fopen(name, "w"))) {err("Failed to open outfile"); return NULL;}
  return out_fp;
}

// USAGE: ./ustm src => bytecode outfile: '{src}.ustmbyc'
int
main(int argc, const char* argv[argc])
{
  if (argc != 3) abort("Usage: ./ustm '{src}.ustm' '{out}.ustmbyc'");
  const char* src_name = argv[1];
  const char* out_name = argv[2];
  if (!strcmp(src_name, out_name)) abort("Source name cannot be same as outfile name");
  FILE* src_fp = open_src(src_name);
  if (!src_fp) return 1;
  FILE* out_fp = open_out(out_name);
  if (!out_fp) {fclose(src_fp); return 1;}
  gen_code(src_fp, out_fp);
  fclose(src_fp);
  fclose(out_fp);
  return ustm_err;
}
