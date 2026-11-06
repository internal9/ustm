/* NOTES:
 * fix 'ungetc' as 1st arg requires int
 * - read all of file into memory then byte by byte faster than byte per disk read?
 * - chunking necessary for embedded only?
 * - fixed-size 32-bit instrs => byc constant table for strings & etc w/ large bytes # => 
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <limits.h>

extern int ustm_err;  // FROM: main.c

static FILE* src_fp;	// SET: 'lex_init()'

// TBD: current c variable
#define nextc() (getc(src_fp))
#define lastc() (ungetc(src_fp))

// TBD
int tk;	// TBD: global tk over return value?
bool is_ident_duplicate = false;
// something* p_sym;	// TBD: global var over retrieving tk value & sym, etc lookups

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

static struct Id {
  char name[MAX_ID_LEN];
  int len;
  int tk;
} ids[ID_HASH_SIZE] = {0};
static double syms[];

// redundant for some cases?
static char tk_val_txt[MAX_STR_LEN];  // TBD: copy char (& process escape seqs) per fgetc?
// static char *tk_val_txt; NOTE: lol change it to this later (NEVER HAPPENING) so str lens can be handled, buffering may require >= 1 block, or just fread the entire thing
static ustm_int tk_val_int;  // NOTE: Holds character values too
static ustm_num tk_val_num;

static int txt_n = 0;


/* NOTES:
 * - single-char tks' values each == respective native char code
 * - unsure if even beyond 255, special things are reserved
 */

// single-char tks <= 255 (UCHAR_MAX)
enum {
#define TK_CMPLX_OP_FIRST (UCHAR_MAX + 1)

  TK_INC     = TK_CMPLX_OP_FIRST,
  TK_DEC,
  TK_ADD_AS,
  TK_SUB_AS,
  TK_MUL_AS,
  TK_DIV_AS,
  TK_POW_AS,
  TK_MOD_AS,

  TK_L_OR,
  TK_L_AND,

  TK_NOT,
  TK_NOT_AS,
  TK_AND,
  TK_AND_AS,
  TK_XOR,
  TK_XOR_AS,
  TK_SH_L,
  TK_SH_L_AS,
  TK_SH_R,
  TK_SH_R_AS,
  
#define TK_KW_FIRST (TK_SH_R_AS + 1)

  TK_NULL     = TK_KW_FIRST,
  TK_TRUE,
  TK_FALSE,
  TK_REF,
  TK_CONST,
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
  TK_WHILE,
  TK_FOR,
  TK_CASES,
  TK_JMP,
  TK_FN
  // NOTE: 'UD' => 'user-defined'
#define TK_UD_ID_FIRST    (TK_FN + 1)
};

// NOTE: In same order as enums
static const char* kws[] = {
  "null", "true", "false",
  "ref", "const",
  "bool", "char", "int", "num",
  "string", "array", "struct",
  "if", "elif", "else",
  "while", "for",
  "cases", "jmp",
  "fn"
};

static int tk_id = TK_UD_ID_FIRST;





// support octal integers?
/*
static void lex_bin_int(struct Tk *p_tk)
{
        p_tk->type_group = G_LITERAL;
        p_tk->type = TK_LIT_INT;
        p_tk->value.int_v = 0;
        char c = GET_C();
        // never write this again please
        for (int i = 0; i < 64; i++, (INCPOS(), c = GET_C())) {
                if (c == '0' || c == '1') {
                        p_tk->value.int_v <<= 1 | (c - '0');
                        continue;
                }

                if (isdigit(c))
                        err("Expected binary digits after binary"
                                    " integer literal prefix '0b',"
                                    " instead got decimal digit '%c'", c);
                else if (isxdigit(c))
                        err("Expected binary digits after binary"
                                    " integer literal prefix '0b',"
                                    " instead got hexadecimal digit '%c'", c);
                // non-digit character case
                break;
        }

        printf("%ld %c\n", src_column, c);
        if (c == '0' || c == '1')
                LEX_ERR("Binary integer literal exceeds 64 digits, above 64-bit range");
        if (isdigit(c))
                LEX_ERR_FMT("Binary integer literal exceeds 64 digits,"
                        " above 64-bit range AND got decimal digit '%c'", c);
        else if (isxdigit(c))
                LEX_ERR_FMT("Binary integer literal exceeds 64 digits,"
                            " above 64-bit range AND got hexadecimal"
                            " digit '%c'", c);
}

static void lex_hex_int(struct Tk *p_tk)
{
        p_tk->type_group = G_LITERAL;
        p_tk->type = TK_LIT_INT;
        p_tk->value.int_v = 0;
        char c;
        for (int i = 0; i < 8; i++) {
                switch (c = GET_C()) {
                case 'a': case 'A': p_tk->value.int_v <<= 4 | 10; break;
                case 'b': case 'B': p_tk->value.int_v <<= 4 | 11; break;
                case 'c': case 'C': p_tk->value.int_v <<= 4 | 12; break;
                case 'd': case 'D': p_tk->value.int_v <<= 4 | 13; break;
                case 'e': case 'E': p_tk->value.int_v <<= 4 | 14; break;
                case 'f': case 'F': p_tk->value.int_v <<= 4 | 15; break;
                default:
                        if (!isdigit(c))
                                return;
                        p_tk->value.int_v <<= 4 | (c - '0');
                }
                INCPOS();
        }
        if (isxdigit(GET_C()))
                LEX_ERR("Hexadecimal integer literal exceeds 8 digits, above 64-bit range");
}


  IF 'non-fractional_int' is true (1000% sure there is no *decimal point*,
  resulting from attempting to lex an int that would've *overflowed*)

  CHECK for *suffix* 'f', otherwise error, assuming an attempt at lexing a massive 'int'
*/

/*
static void
lex_num(void)
{
        p_tk->type_group = G_LITERAL;
        p_tk->type = TK_LIT_NUM;
        // IF success 'src_txt' is advanced beyond the lexed num
        errno = 0;
        char *num_src_txt_end;
        p_tk->value.fp_v = strtod(src_txt + src_i, &num_src_txt_end);
        bool no_decimal = strchr(src_txt + src_i, '.') == NULL;
        // point to *last digit* instead incase of overflow debugging error messages
        SET_POS((num_src_txt_end - src_txt) - 1);

        if (*num_src_txt_end != 'f') {
                // assume a very large 'int' was attempted to be lexed
                if (no_decimal)
                        LEX_ERR("64-bit integer literal overflow");
        } else
                INCPOS();
        if (errno == ERANGE)
                LEX_ERR("floating-point number overflow");
        INCPOS();
} */

// probably need a more robust integer lexer
/*
static int
lex_dec_int_or_num(void)
{
        p_tk->type_group = G_LITERAL;
        p_tk->type = TK_LIT_INT;
        long int_src_i_start = src_i;
        char c;
        while (true) {
                c = GET_C();
                if (c == '.' || c == 'f')
                        goto try_lex_num;
                if (!isdigit(c))
                        return;
                if (p_tk->value.int_v > INT64_MAX / 10) { printf("asd\n");
                        goto try_lex_num;}
                // C standard guarantees decimal digits are in order
                p_tk->value.int_v *= 10;
                if (p_tk->value.int_v > INT64_MAX - (c - '0'))
                        goto try_lex_num;
                p_tk->value.int_v += (c - '0');
                INCPOS();
        }

try_lex_num:
        p_tk->value.fp_v = 0;
        // to allow rereading previous digit(s)
        SET_POS(int_src_i_start);
        lex_num(p_tk);
}
*/

// FNV Constants for 32-Bit
#define HASH_BASIS (uint32_t) 0x811c9dc5
#define HASH_PRIME (uint32_t) 0x01000193
#define FNV1A(hash, c) ((hash ^ c) * HASH_PRIME)

#define IS_ID_C(c) (isalnum((c)) || (c) ==  '_')

// TBD
static void
id_put()
{

}

// TBD: 
// TODO: reset tk_val_txt buf & txt_n
// TODO: 'isalnum' => handle _
static void
lex_id(void)
{
  
	uint32_t hash = HASH_BASIS;
	for (int c; c = nextc(), IS_ID_C(c); FNV1A(hash, c), tk_val_txt[txt_n++] = c);
	hash &= (ID_HASH_SIZE - 1);
	for (int i = 0; i < ID_HASH_SIZE; i++) {
    struct Id id = ids[((int) hash + i) & (ID_HASH_SIZE - 1)];
		if (id.len == 0) {
      memcpy(&id.name, tk_val_txt, txt_n);
      id.len = txt_n;
    }
	}
	// TBD: copy chars into txt bufs
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
	default: 	 err("Invalid escape sequence"); return -1;
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
	case '"': if (is_tk_lit_str) return false;	// NOTE: true: non-terminated
	case '_': case '\t': 							break;
	default:
		if (!isalnum(c) && !isspace(c) && !ispunct(c) && c != '_') err("Char must be ASCII idk");
	}
	if (is_tk_lit_str) tk_val_txt[txt_n++] = c;
	else {
		if (nextc() != '\'') err("Expected closing '\\'");
		tk_val_int = c;
	}
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
			if ((c = nextc()) == EOF) {err("Unterminated block comment"); return;
 		else if (c == '*') {
				if ((c = nextc()) == EOF)	{err("Unterminated block comment"); return;}
				if (c == '/') break;
			}
		}
	else do if ((c = nextc()) == EOF) {lastc(); break;}; while (c != '\n');
}

// TBD stuff
static inline void init_keywords_map(void)
{
        hashmap_init(&keywords_hashmap,
                     HASHMAP_INIT_SIZE + HASHMAP_INIT_SIZE / 4);
        for (int i = 0; i < (int) (sizeof keywords / sizeof *keywords); i++)
                // Keyword enumlex_chars are in order startiing from 'TK_KW_NULL'
                hashmap_put_int(&keywords_hashmap, keywords[i], strlen(keywords[i]),
                                (int) TK_KW_NULL + i);
        
}

static inline void
err_invalid_char(int c)
{
  if (isprint(c)) err("Invalid symbol '%c', character code of %d", c, (int) c);
	else err("Invalid non-printable symbol, character code of %d", (int) c);
}

static void
lex_init(FILE* src_fp)
{

  
  src_fp = src_fp;
  // keywords, etc
}

static const int c_cases2[] = {
	['!'] = TK_NOT_AS,
	['='] = TK_EQ,
	['/'] = TK_DEV_AS,
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
	['>'] = (TK_SH_R << 8) | (TK_SH_R_AS << 4) | (TK_REQ),
};

// NOTES: line & column are not recorded only at error
static int
lex_next(void)
{
	for (int c; ; c = nextc()) {
	  switch (c) {
		case ' ': case '\n': case '\r': case '\t': case '\v': case '\f':
			skip_whitespace(); break;
		case EOF:
			if (ferror(src_fp)) err("Lexing source read error");
			return EOF;
		case '/':
			c = nextc()
			if (c == '/' || c == '*') skip_comment(c == '*');
			else c = lastc();
		case '!': case '=': case '*': case '%':
			return ( (nextc() == '=') ? (c_cases2[c]) : (c) );
		case '+': case '-': case '&': case '|':
			if (nextc() == c) return c_cases3[c] & 0xF0;
			return (nextc() == '=') ? (c_cases4[c] & 0x0F) : (c);
		case '<': case '>': case '^':
			if (nextc() == '=') return c2_cases4[c] & 0x00F;
			if (currentc() == c) 	 return (nextc() == '=') ? (c2_cases4[c] & 0x0F0) : (c2_cases4[c] & 0xF00);
			return c;
			
		case ',': case '[': case ']': case '{': case '}':
		case '(': case ')': case '?': case ':': case ';':
			return c;
		case '\'': lex_char(false); return TK_LIT_CHAR;
		case '"':  lex_str();				return TK_LIT_STR;
		case '0':
						c = nextc();
						if (c == 'x') 		 lex_hex_int(p_tk);	return TK_LIT_INT;
						else if (c == 'b') lex_bin_int(p_tk);	return TK_LIT_INT;
		case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
						return lex_dec_int_or_num(p_tk);
		case '.':
						// no incpos, 'lex_num' needs to read '.' for fraction
						if (isdigit(nextc()))
										lex_num();
						else return '.';
		case '_': return lex_id(p_tk);
		default:
						if (isdigit(c)) 		 			return lex_dec_int_or_num(p_tk);
						else if (isalpha(c))		  return lex_keyword_or_identifier(p_tk);
						else err_invalid_char(c);	return EOF;
		}
	}
}

// NOTE: no peek func, each codegen func will end off with the current tk being next after last one needed
