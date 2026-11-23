#include "ustm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>

#define ARENA_SZ_INIT 256

static FILE* src_fp = NULL;
static FILE* out_fp = NULL;
struct Arena arena = {.current_block = NULL};

void
free_prog_(bool do_exit)
{
  if (src_fp) fclose(src_fp);
  if (out_fp) fclose(out_fp);
  if (arena.current_block) arena_free(&arena);
  if (do_exit) exit(EXIT_FAILURE);
}

static inline bool
check_suffix(const char* str, const char* suffix)
{
  const char* dot = strrchr(str, '.');
  return (dot != NULL) && (dot != str) && (!strcmp(dot, suffix));
}

static void
open_src(const char* name)
{
  if (!check_suffix(name, ".ustm")) err("Source name requires suffix '.ustm' with preceding text");
  if (!(src_fp = fopen(name, "r"))) err("Unreadable/non-existent source file");
}

static void
open_out(const char* name)
{
  if (!check_suffix(name, ".ustmbyc")) err("Outfile name requires suffix '.ustmbyc' with preceding text");
  if (!(out_fp = fopen(name, "w"))) err("Failed to open outfile");
}

// USAGE: ./ustm src => bytecode outfile: '{src}.ustmbyc'
int
main(int argc, const char* argv[argc])
{
  if (argc != 3) err("Usage: ./ustm '{src}.ustm' '{out}.ustmbyc'");
  const char* src_name = argv[1];
  const char* out_name = argv[2];
  if (!strcmp(src_name, out_name)) err("Source name cannot be same as outfile name");
  open_src(src_name);
  open_out(out_name);
  if (!arena_init(&arena, ARENA_SZ_INIT)) err("Memory init fail");
  gen_code(src_fp, out_fp);
  free_prog_(false);
  return EXIT_SUCCESS;
}
