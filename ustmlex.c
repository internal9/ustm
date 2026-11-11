/* NOTES:
 * - line & column? handling
 * - fix 'ungetc' as 1st arg requires int
 * - read all of file into memory then byte by byte faster than byte per disk read?
 * - chunking necessary for embedded only?
 * - fixed-size 32-bit instrs => byc constant table for strings & etc w/ large bytes # => 
 */

#include "ustm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>

extern int ustm_err;  // FROM: main.c

static FILE* src_fp;  // SET: 'lex_init()'

// TBD: current c variable
static int lazy_cur_c;

#define nextc() (lazy_cur_c = getc(src_fp))
#define peekc() (ungetc(getc(src_fp), src_fp))
#define lastc() (ungetc(lazy_cur_c, src_fp))

// TBD: global tk over return value?
// int tk;
bool is_ident_duplicate = false;
// something* p_sym;  // TBD: global var over retrieving tk value & sym, etc lookups

// NOTE: char type just uses native C type, no seperate typedef
typedef int64_t ustm_int;
typedef double  ustm_num;

//typedef int file_pos;
// static file_pos line = 1;
// static file_pos column = 0;

// NOTE: Assumption, despite ambiguity
#define TAB_WIDTH_GUESS 8

// NOTE: applies to kws & user-defined idents (var, func name, etc)
#define MAX_ID_LEN  64
#define MAX_STR_LEN 512
// NOTE: Static hashing
// TBD: Might need const table (including string) in byc => string interning too? ORRR 'pushstr' 'movestr'
#define ID_HASH_SIZE 512

// TBD: change 'id' -> 'sym'
static struct Id {
  char name[MAX_ID_LEN];
  int len;
  int tk;
} ids[ID_HASH_SIZE] = {0};
// static double syms[2];

// redundant for some cases?
static char tk_val_txt[MAX_STR_LEN];  // TBD: copy char (& process escape seqs) per fgetc?
// static char *tk_val_txt; NOTE: lol change it to this later (NEVER HAPPENING) so str lens can be handled, buffering may require >= 1 block, or just fread the entire thing
static ustm_int tk_val_int;  // NOTE: Holds character values too
static ustm_num tk_val_num;

static int txt_n = 0;

// NOTE: In same order as enums
static const char* kws[] = {
  "null", "true", "false",
  "bool", "char", "int", "num",
  "string", "array", "struct",
  "if", "elif", "else",
  "while", "for",
  "cases", "jmp",
  "fn", "ref"
};

// NOTE: 'lex_init()' will make it equal 'TK_UD_ID_FIRST' anyways
static int tk_id = TK_KW_FIRST;


// TODO: handling signed ints & nums when unary '-' expr
static void
lex_int(bool is_hex)
{
  tk_val_int = 0;
  int c;
  if (is_hex) {
    for (int i = 0; i < 8; i++) {
      switch (c = nextc()) {
      case 'a': case 'A': tk_val_int <<= 4 | 10; break;
      case 'b': case 'B': tk_val_int <<= 4 | 11; break;
      case 'c': case 'C': tk_val_int <<= 4 | 12; break;
      case 'd': case 'D': tk_val_int <<= 4 | 13; break;
      case 'e': case 'E': tk_val_int <<= 4 | 14; break;
      case 'f': case 'F': tk_val_int <<= 4 | 15; break;
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        tk_val_int <<= 4 | (c - '0');
      default:
        return;
      }
    }
    if (isxdigit(peekc())) err("Hexadecimal int exceeds max size");
  } else {
    // binary
    for (int i = 0; i < 64; i++) {
      switch (c = nextc()) {
      case '0': case '1':
        tk_val_int <<= 1 | (c - '0');
        break;
      case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9':
        err("Invalid binary digit");
        return;
      default:
        lastc();
        return;
      }
    }
    if (isdigit(peekc())) err("Binary int exceeds max size");
  }
}

/*
  IF 'non-fractional_int' is true (1000% sure there is no *decimal point*,
  resulting from attempting to lex an int that would've *overflowed*)

  CHECK for *suffix* 'f', otherwise error, assuming an attempt at lexing a massive 'int'
*/

// TODO: check this
static void
lex_num(void)
{
  // IF success 'src_txt' is advanced beyond the lexed num
  errno = 0;

  bool has_decimal = false, has_suffix = false;
  char buf[64];
  for (int i = 0, c; c = nextc(), i < 64; i++) {
    if (c == '.') {
      if (has_decimal) {err("Duplicate decimal '.'"); return;}
      has_decimal = true;
    }
    else if (c == 'f' || c == 'F') {
      has_suffix = true;
      break;
    }
    if (!isdigit(c)) {lastc(); break;}
    buf[i] = c;
  }

  tk_val_num = strtod(buf, NULL);
  // assume a very large 'int' was attempted to be lexed
  if (!has_suffix && !has_decimal) {err("64-bit integer literal overflow"); return;}
  if (errno == ERANGE) {err("floating-point number overflow"); return;}
}

// NOTE: too lazy to error handle fget & set pos
static int
lex_dec_int_or_num(void)
{
  tk_val_int = 0;
  fpos_t start;
  fgetpos(src_fp, &start);
  for (int c; c = nextc();) {
    if (c == '.' || c == 'f') break;
    if (!isdigit(c)) {lastc(); return TK_INT;}
    if (tk_val_int > INT64_MAX / 10) break;
    // C standard guarantees decimal digits are in order
    tk_val_int *= 10;
    if (tk_val_int > INT64_MAX - (c - '0')) break;
    tk_val_int += (c - '0');
  }

  // to allow rereading previous digit(s)
  fsetpos(src_fp, &start);
  lex_num();
  return TK_NUM;
}

// FNV Constants for 32-Bit
#define HASH_BASIS (uint32_t) 0x811c9dc5
#define HASH_PRIME (uint32_t) 0x01000193
//NOTE: 'c' -> 'unsigned char' to remove possible zero-extension if direct conversion to 'uint32_t'
#define FNV1A(hash, c) ((hash ^ (unsigned char) (c)) * HASH_PRIME)

#define IS_ID_C(c) (isalnum((c)) || (c) ==  '_')

// TBD
static int
id_put_or_find(uint32_t hash)
{
  hash &= (ID_HASH_SIZE - 1);
  for (int i = 0; i < ID_HASH_SIZE; i++) {
    struct Id id = ids[((int) hash + i) & (ID_HASH_SIZE - 1)];
    if (id.len == txt_n && !strncmp(tk_val_txt, id.name, txt_n)) return id.tk;
    if (id.len == 0) {
      memcpy(&id.name, tk_val_txt, txt_n);
      id.len = txt_n;
      id.tk = tk_id++;
      return id.tk;
    }
  }
}

// TBD: 'tk_val_txt' replaced by txt buffers containing all txt & js use ptrs
// TODO: reset tk_val_txt buf & txt_n
// TODO: 'isalnum' => handle _
static inline int
lex_ud_id(void)
{
  // TODO: max_sym & str checks
  txt_n = 0;
  uint32_t hash = HASH_BASIS;
  for (int c; c = nextc(), IS_ID_C(c); FNV1A(hash, c), tk_val_txt[txt_n++] = c);
  return id_put_or_find(hash);
}

static inline char
esc_seq(int c2)
{
  switch(c2) {
  case '0':  return '\0';
  case 'n':  return '\n';
  case 't':  return '\t';
  case '\\': return '\\';
  case '\'': return '\'';
  case '"':  return '"';
  default:   err("Invalid escape sequence"); return -1;
  }
}

// used to lex a lit char & used by lit str cuz lazy
static bool
lex_char(bool is_tk_lit_str)
{
  int c = nextc();
  switch (c) {
  case '\'':
    err("Char cannot be empty"); return 0;
  case '\\': c = esc_seq(nextc());
  case '"': if (is_tk_lit_str) return true;
  case '_': case '\t':              break;
  default:
    if (!isalnum(c) && !isspace(c) && !ispunct(c) && c != '_') err("Char must be ASCII idk");
  }
  if (is_tk_lit_str) tk_val_txt[txt_n++] = c;
  else {
    if (nextc() != '\'') err("Expected closing '\\'");
    tk_val_int = c;
  }
  return false;
}

static void
lex_str(void)
{
  for (;;) {
    if (txt_n > MAX_STR_LEN) {err("String size exceeded limit"); return;}
    if (lex_char(true)) return;
    if (ustm_err) {err("Unterminated string"); break;}
  }
}

// whitespace characters are non-printable characters that define text layouts
static inline void
skip_whitespace(void)
{
  while (isspace(nextc()));
  lastc();
}

static void
skip_comment(bool is_multi_line)
{
  int c;
  if (is_multi_line)
    while (1) {
      if ((c = nextc()) == EOF) {err("Unterminated block comment"); return;}
    else if (c == '*') {
        if ((c = nextc()) == EOF) {err("Unterminated block comment"); return;}
        if (c == '/') break;
      }
    }
  else do if ((c = nextc()) == EOF) {lastc(); break;} while (c != '\n');
}

static inline void
err_invalid_char(int c)
{
  if (isprint(c)) err_fmt("Invalid symbol '%c', character code of %d", c, (int) c);
  else err_fmt("Invalid non-printable symbol, character code of %d", (int) c);
}

static const int c_cases2[] = {
  ['!'] = TK_NOT_AS,
  ['='] = TK_EQ,
  ['/'] = TK_DIV_AS,
  ['*'] = TK_MUL_AS,
  ['%'] = TK_MOD_AS
};

static const int c_cases3[] = {
  ['+'] = (TK_INC << 4) | TK_ADD_AS,
  ['-'] = (TK_DEC << 4) | TK_SUB_AS,
  ['&'] = (TK_L_AND << 4) | TK_AND_AS,
  ['|'] = (TK_L_OR << 4) | TK_OR_AS,
};

static const int c2_cases4[] = {
  ['^'] = (TK_XOR << 8) | (TK_XOR_AS << 4) | (TK_POW_AS),
  ['<'] = (TK_SH_L << 8) | (TK_SH_L_AS << 4) | (TK_LEQ),
  ['>'] = (TK_SH_R << 8) | (TK_SH_R_AS << 4) | (TK_GEQ),
};

// NOTES: line & column are not recorded only at error
int
lex_next(void)
{
  for (int c; c = nextc();) {
    switch (c) {
    case ' ': case '\n': case '\r': case '\t': case '\v': case '\f':
      skip_whitespace(); break;
    case EOF:
      if (ferror(src_fp)) err("Lexing source read error");
      return EOF;
    case '/':
      c = nextc();
      if (c == '/' || c == '*') skip_comment(c == '*');
      else c = lastc();
    case '!': case '=': case '*': case '%':
      return ( (nextc() == '=') ? (c_cases2[c]) : (c) );
    case '+': case '-': case '&': case '|':
      if (nextc() == c) return c_cases3[c] & 0xF0;
      return (nextc() == '=') ? (c_cases3[c] & 0x0F) : (c);
    case '<': case '>': case '^': {
      int c2 = nextc();
      if (c2 == '=') return c2_cases4[c] & 0x00F;
      else if (c2 == c) return (nextc() == '=') ? (c2_cases4[c] & 0x0F0) : (lastc(), c2_cases4[c] & 0xF00);
      return c;
    }
    case ',': case '[': case ']': case '{': case '}':
    case '(': case ')': case '?': case ':': case ';':
      return c;
    case '\'': lex_char(false); return TK_LIT_CHAR;
    case '"':  lex_str();       return TK_LIT_STR;
    case '0':
            switch (c = nextc()) {
            case 'b': case 'B':
            case 'x': case 'X':
             lex_int(c == 'x' || c == 'X');
             return TK_LIT_INT;
            default: lastc();
            }
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
            lastc();
            return lex_dec_int_or_num();
    case '.':
            if (isdigit(peekc())) {lastc(); lex_num(); return TK_LIT_NUM;}
            return '.';
    case '_': return lex_ud_id();
    default:
            if (isalpha(c)) return lex_ud_id();
            err_invalid_char(c);
            return EOF;
    }
  }
}

void
lex_init(FILE* src_fp_arg)
{
  uint32_t hash = HASH_BASIS;
  for (int i = 0; i < sizeof(kws) / sizeof(*kws); i++) {
    const char* kw = kws[i];
    for (int c; IS_ID_C(c = kw[txt_n]); FNV1A(hash, c), tk_val_txt[txt_n++] = c);
    id_put_or_find(hash);
    txt_n = 0;
  }
  
  src_fp = src_fp_arg;
  // keywords, etc
}

// NOTE: no peek func, each codegen func will end off with the current tk being next after last one needed
