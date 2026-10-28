#include <stdio.h>

extern int ustm_status;  // FROM: main.c

static FILE* src_fp;

// NOTE: char type just uses native C type, no seperate typedef
typedef int64_t ustm_int;
typedef double ustm_num;

typedef int file_pos;

static file_pos line = 1;
static file_pos column = 0;

// NOTE: Assumption, despite ambiguity
#define TAB_WIDTH_GUESS 8

static char tk_val_txt[512];
static ustm_int tk_val_int;	// NOTE: Holds character values too
static ustm_num tk_val_num;


static const char* reserved[] = {
        "null", "true", "false", // keywords that are *literals*
        "bool", "char", "int", "num",
        "string", "array", "struct",
        "if", "elif", "else",
        "while", "for",
        "switch", "jmp",
        "fn", "ref"
};


static void
lex_init(FILE* src_fp)
{
  src_fp = src_fp;
  // keywords, etc
}

static void
lex_next(void)
{
  char c = getc(src_fp);
  
}

static void
lex_peek(void)
{

}
