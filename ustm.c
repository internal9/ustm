#include "lex.h"
#include "util.h"
#include "hashmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>

									// NOTE: 'abort()' bypasses and goes straight to 'exit(1)'
int ustm_err = 0;	// USED BY: ustm.c, lex.c, codegen.c

#define err(msg)   (fprintf(stderr, msg "\n"), (ustm_err = 1))
#define abort(msg) (fprintf(stderr, msg "\n"), exit(1))

static inline void
check_suffix(const char* str, const char* suffix)
{
	const char* dot = strrchr(name, '.');
	return (dot != str) && (!strcmp(dot, suffix));
}

static FILE*
open_src(const char *name)
{
	if (!check_suffix(name)) abort("Source name requires suffix '.ustm' with preceding text");
	FILE *src_fp;
	if (!(src_fp = fopen(name, "r"))) abort("Unreadable/non-existent source file");
	return src_fp;
}

static FILE*
open_out(const char *name)
{
	if (!check_suffix(name)) abort("Outfile name requires suffix '.ustmbyc' with preceding text");
	FILE *out_fp;
	if (!(out_fp = fopen(name, "w"))) abort("Failed to open outfile");
	return out_fp;
}

// USAGE: ./ustm src => bytecode outfile: '{src}.ustmbyc'
int
main(int argc, const char* argv[argc])
{
	if (argc != 3) abort("Usage: ./ustm src.ustm out.ustmbyc");
	const char* src_name = argv[1];
	const char* out_name = argv[2];
  FILE *src_fp = open_src(src_name);
	FILE *out_fp = open_out(out_name);
	vm_run(src_fp, out_fp);
	fclose(src_fp);
	fclose(out_fp);
	return ustm_err;
}
