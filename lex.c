/* NOTES:
 * - read all of file into memory then byte by byte faster than byte per disk read?
 * - chunking necessary for embedded only?
 * - 
 */

#include <stdio.h>

extern int ustm_status;  // FROM: main.c

static FILE* src_fp;

// NOTE: char type just uses native C type, no seperate typedef
typedef int64_t ustm_int;
typedef double ustm_num;

//typedef int file_pos;
// static file_pos line = 1;
// static file_pos column = 0;

// NOTE: Assumption, despite ambiguity
#define TAB_WIDTH_GUESS 8

#define MAX_TXT_N 512
#define TK_IDENT_START = index of last reserved

static char syms[MAX_SYMS];        // store reserved &
static something ident_data[MAX_SYMS - TK_IDENT_START];

static char tk_val_txt[MAX_TXT_N];  // TBD: copy char (& process escape seqs) per fgetc?
// static char *tk_val_txt; NOTE: lol change it to this later (NEVER HAPPENING) so str lens can be handled, buffering may require >= 1 block, or just fread the entire thing
static ustm_int tk_val_int;  // NOTE: Holds character values too
static ustm_num tk_val_num;

static int txt_n = 0;

/* NOTES:
 * - single-char tks' values each == respective native char code
 * - unsure if even beyond 255, special things are reserved
 */
enum {
	TK_ADD = (UCHAR_MAX + 1),
	TK_SUB,
};



static const char* reserved[] = {
        "null", "true", "false", // keywords that are *literals*
        "bool", "char", "int", "num",
        "string", "array", "struct",
        "if", "elif", "else",
        "while", "for",
        "switch", "jmp",
        "fn", "ref"
};

static inline void handle_tab_char(void)
{
        WARN_FMT("Tab character '\\t' width assumed to be %d spaces despite ambigious "
                 "width and interpration, may lead to inaccurate lexing column numbers",
                 TAB_WIDTH_GUESS);
        src_i++;
        src_column += TAB_WIDTH_GUESS;
}

static inline void handle_newline_char(void)
{
        src_i++;
        src_line++;
        src_column = 0;
}

// support octal integers?
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
                        LEX_ERR_FMT("Expected binary digits after binary"
                                    " integer literal prefix '0b',"
                                    " instead got decimal digit '%c'", c);
                else if (isxdigit(c))
                        LEX_ERR_FMT("Expected binary digits after binary"
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

/*
  IF 'non-fractional_int' is true (1000% sure there is no *decimal point*,
  resulting from attempting to lex an int that would've *overflowed*)

  CHECK for *suffix* 'f', otherwise error, assuming an attempt at lexing a massive 'int'
*/
static void lex_num(struct Tk *p_tk)
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
}

// probably need a more robust integer lexer
static void lex_dec_int_or_num(struct Tk *p_tk)
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

static void lex_op_other_or_assign(struct Tk *p_tk, enum TkTypeGroup TK_OP_type_group,
                        enum TkType TK_OP_type, enum TkType assigner_type)
{
        if (GET_C() == '=') {
                p_tk->type_group = G_OP_ASSIGN;
                p_tk->type = assigner_type;
                INCPOS();
        } else {
                p_tk->type_group = TK_OP_type_group;
                p_tk->type = TK_OP_type;
        }
}

// temp to remove warnings
// will finish
// test this later
static void lex_keyword_or_identifier(struct Tk *p_tk)
{
        char *txt_start = src_txt + src_i;
        char c = GET_C();
        while (isalpha(c) || isdigit(c) || c == '_') {
                INCPOS();
                c = GET_C();
        }
				// TODO: check this, might fail since src_column += 8 when tab
        size_t len = (size_t) (src_column - p_tk->column);
        int keyword_type_enum = hashmap_get_int(&keywords_hashmap,
                                                txt_start, len);
        // -1 means key *not found*
        if (keyword_type_enum == -1) {
                p_tk->type_group = G_MISC;
                p_tk->type = TK_IDENT;
                p_tk->value.txt = malloc(len + 1);
                if (p_tk->value.txt == NULL)
                        LEX_ERR("Failed memory alloc for identifier");
                p_tk->value.txt[len] = '\0';
                memcpy(p_tk->value.txt, txt_start, len);
        }
        else {
                // p_tk->type_group = G_KEYWORD;
                p_tk->type = (enum TkType) keyword_type_enum;
                // make this better
        }
}


// Only *ascii* chars will be supported, may be changed to *utf-8*, and allow wide chars
static int lex_lit_struct Tk *p_tk)
{
	switch (getc(src_fp)) {
	case '\'': err("Character literal cannot be empty"); return;
	case '\\': 
	}
	
	if (c == '\\') {
					c = GET_C();
					int esc_char = esc_seq_from_c);
					if (esc_char == -1)
									LEX_ERR("Invalid escape sequence");
					p_tk->value.c = (char) esc_char;
									
	}
	else if (c == '\t')
					handle_tab_);
	else if (!isalnum(c) && !isspace(c) && !ispunct(c) && c != '_')
					LEX_ERR("Invalid character literal.");
	INCPOS();
	c = GET_C();
	if (c != '\'')
					LEX_ERR("Expected ''' to end character literal.");
	INCPOS();
}

// TBD: current c variable
#define nextc() (getc(src_fp)

static inline char
esc_seq(c2)
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
static void
lex_char(bool is_tk_lit_str)
{
	int c = nextc();
	switch (c) {
	case '\'':
		if (is_tk_lit_str) break;
		err("Char cannot be empty"); return;
	case '\\': c = esc_seq(nextc()); 	break;
	case '_': case '\t': 										break;
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
	for (int i = 0; i < MAX_TXT_N; i++) {
		lex_char(true);
		if (ustm_err) return;
	}
	if (nextc() != '\"') err("Expected closing '\"'");
}

// whitespace characters are non-printable characters that define text layouts
static bool handle_whitespace(void)
{
        char c = GET_C();
        if (!isspace(c)) return false;
        while (isspace(c = GET_C()))
                if (c == '\t')
                        handle_tab_);
                else if (c == '\n')
                        handle_newline_);
                else
                        INCPOS();
        return true;
}

static bool handle_line_comment(void)
{
        char c = GET_C();
        if (c != '/' || NEXT_C() != '/') return false;
        INCPOS();
        for (c = GET_C(); c != '\n' && c != '\0'; c = GET_C())
                if (c == '\t')
                        handle_tab_char();
                else
                        INCPOS();
        return true;
}

static bool handle_multiline_comment(void)
{
        char c = GET_C();
        if (c != '/' || NEXT_C() != '*') return false;
        long comment_src_line = src_line;
        long comment_src_column = src_column;
        INCPOS();
        for (c = GET_C(); c != '*' || NEXT_C() != '/'; c = GET_C())
                if (c == '\0')
                        LEX_ERR_FMT("Unclosed multiline comment "
                                    "starting at (L%ld C%ld)",
                                    comment_src_line,comment_src_column);
                else if (c == '\t')
                        handle_tab_char();
                else if (c == '\n')
                        handle_newline_char();
                else
                        INCPOS();

        // char after ending '*/'
        INCPOS();
        INCPOS();
        return true;
}

// probably the worst thing written on here
static inline void handle_non_lexable(void)
{
        while (handle_whitespace() || handle_line_comment() ||
               handle_multiline_comment());
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

static void
lex_init(FILE* src_fp)
{
  src_fp = src_fp;
  // keywords, etc
}

static const int c_cases2[] = {
	['!'] = TK_NOT_AS,
	['^'] = TK_XOR_AS,
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

static const int c2_cases3[] = {
	['<'] = (TK_SH_L << 8) | (TK_SH_L_AS << 4) | (TK_LEQ),
	['>'] = (TK_SH_R << 8) | (TK_SH_R_AS << 4) | (TK_REQ),
};

// NOTES: line & column are not recorded only at error
static int
lex_next(void)
{
  int c = getc(src_fp);
	switch (c) {
	case '!': case '^': case '=': case '/': case '*': case '%':
		return ( (nc() == '=') ? (c_cases2[c]) : (c) );
	case '+': case '-': case '&': case '|':
		if (nc() == c) return c_cases3[c] & 0xF0;
		return (cc() == '=') ? (c_cases3[c] & 0x0F) : c;
	case '<': case '>':
		if (nc() == '=') return c2_cases3[c] & 0x00F;
		if (cc() == c) 	 return (nc() == '=') ? (c2_cases3[c] & 0x0F0) : (c2_cases3[c] & 0xF00);
		return c;
	case ',': case '[': case ']': case '{': case '}':
	case '(': case ')': case '?': case ':': case ';':
		return c;
	case '\'': return lex_char(p_tk);
	case '"':  lex_str(p_tk); return TK_STR;
	case '0':
					c = GET_C();
					if (c == 'x') {
									INCPOS();
									lex_hex_int(p_tk);
					} else if (c == 'b') {
									INCPOS();
									lex_bin_int(p_tk);
					} else {
									DECPOS();
									lex_dec_int_or_num(p_tk);
					}
					break;
	case '.':
					// no incpos, 'lex_num' needs to read '.' for fraction
					if (isdigit(NEXT_C()))
									lex_num(p_tk);
					else {
									INCPOS();
									p_tk->type_group = G_MISC;
									p_tk->type = TK_PERIOD;
					}
	case '_':
					lex_keyword_or_identifier(p_tk);
	case '\0':
					if (src_i != src_len)
									LEX_ERR("Null terminator '\\0' should be at end of file");
					p_tk->type_group = G_MISC;
	default:
					if (isdigit(c))
									lex_dec_int_or_num(p_tk);
					else if (isalpha(c))
									lex_keyword_or_identifier(p_tk);
					else
									// Should have more debugging, to handle non-printable characters
									// Default argument promotions promote 'c' to 'int', but explicitness just to be fine
									if (isprint(c))
													LEX_ERR_FMT("Invalid symbol '%c', character code of %d", c, (int) c);
									else
													LEX_ERR_FMT("Invalid non-printable symbol, character code of %d", (int) c);
	}

}

static int
lex_expect(some_arg)
{

}

// no peek func, each codegen func will end off with the current tk being next after last one needed
