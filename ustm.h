/* ustm.h */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_STR_LEN 512
/* 'exit()' should close all open streams anyways, could be replaced be a global cleanup func if needed */
#define err(msg)          (fprintf(stderr, msg "\n"), free_prog_(true))
#define err_fmt(fmt, ...) (fprintf(stderr, fmt "\n", __VA_ARGS__), free_prog_(true))
/* lol what a mess */
extern void free_prog_(bool do_thing);

/* NOTES:
 * - single-char tks' values each == respective native char code
 * - unsure if even beyond 255, special things are reserved
 * - single-char tks <= 255 (UCHAR_MAX)
 */
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

  TK_CONST = TK_KW_FIRST,
  TK_REF,
  TK_NULL,
  TK_TRUE,
  TK_FALSE,
  TK_BOOL,
  TK_CHAR,
  TK_INT,
  TK_NUM,
  /* TBD: */
  /* TK_STRING, */
  TK_ARRAY,
  TK_STRUCT,
  TK_IF,
  TK_ELIF,
  TK_ELSE,
  TK_WHILE,
  TK_FOR,
  TK_CASES,
  TK_JMP,
  TK_FN,
  
  /* NOTE: 'UD' => 'user-defined' */
  /* NOTE: Will be used for string-interning (unles string len fitsinstr) */
#define TK_UD_ID_FIRST (TK_FN + 1)
};

enum {
  TY_BOOL   = 0x0,  // 0b000
  TY_CHAR   = 0x1,  // 0b001
  TY_INT    = 0x2,  // 0b010
  TY_NUM    = 0x3,  // 0b100
  TY_STRUCT = 0x4,  // 0b101
  TY_ARR    = 0x8,  // 0b1000
  MOD_REF   = 0x10, // 0b10000
  MOD_CONST = 0x20, // 0b100000
};

/* ustmlex.c */
void lex_init(FILE* src_fp_arg);
int  lex_next(void);
/* ustmgen.c */
void gen_code(FILE* src_fp_arg, FILE* out_fp_arg);
/* ustmvm.c  */
void vm_run(FILE* byc_fp);

/* arena.c */
struct Arena {
    void* top_ptr;
    struct Block* current_block;
    size_t current_block_capacity;
    size_t current_block_used;
    size_t default_block_capacity;
};

bool arena_init(struct Arena *restrict p_arena, size_t default_block_capacity);
void* arena_alloc(struct Arena *restrict p_arena, size_t amount);
void* arena_realloc(struct Arena *restrict p_arena, void *ptr, size_t old_amount, size_t new_amount);
void* arena_realloc_top(struct Arena *restrict p_arena, size_t new_amount); /* grow in place */
void arena_reset(struct Arena *restrict p_arena); /* clear until first block */
void arena_free(const struct Arena *restrict p_arena); /* clear all blocks, effectively making arena unusable */
