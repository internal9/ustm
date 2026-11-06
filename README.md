# ustm
## tbd
- kws: 'array', 'string, 'iter', 'dynamic'
- stuff for binary data idk
- jagged array how handle 'char [FIXED_N][*] strs  strs[2] = "r2substr"' where '*' in '[*]' is dynamic
  no need for kw 'dynamic' then? also '[]' == length determined at initializion
  

## notes


## keywords
literals:   'null', 'true', 'false'

types:      'bool', 'char', 'int', 'num', 'string', 'array', 'struct'

statements: 'if', 'elif', 'else', 'while', 'for', 'cases', 'jmp', 'ret'

modifiers:  'ref', 'const'

misc:       'func'

## operators
arithmetic: '++', '--', '+', '-', '*', '/', '^', '%'

arithmetic (compound): '+=' , '-=', '*=', '/=', '^=', '%='


logical: '==', '!=', '!', '||', '&&', '?:', '<', '>', '<=, '>='



bitwise: '~', '|', '&', '^^', '<<', '>>'

bitwise (compound): '~=', '|=', '&=', '^^=', '<<=', '>>='

## syntax
### declarations
scalar: [modifier(s)] [type] name
        [modifier(s)] [type] name = expr
        
aggregate:  [modifier(s)] [type][len|*|] name
 [modifier(s)] [type][] name


## examples
```
// TBD: 'string' == '[]', len determined by initialization
// TBD: no 'string' type cuz non-explicit static & dynamic storage stuff
const int BUF_LEN = 2^8
// TBD: remove 'array' kw (char array[5] v => char[5] v)
char[BUF_LEN] test_buf

/* great */
const bool is_upcase[] = {
  ['A'] = true, ['B'] = true, ['C'] = true, ['D'] = true,
  ['E'] = true, ['F'] = true, ['G'] = true, ['H'] = true,
  ['I'] = true, ['J'] = true, ['K'] = true, ['L'] = true,
  ['M'] = true, ['N'] = true, ['O'] = true, ['P'] = true,
  ['Q'] = true, ['R'] = true, ['S'] = true, ['T'] = true,
  ['U'] = true, ['V'] = true, ['W'] = true, ['X'] = true,
  ['Y'] = true, ['Z'] = true,

  ['a'] = false, ['b'] = false, ['c'] = false, ['d'] = false,
  ['e'] = false, ['f'] = false, ['g'] = false, ['h'] = false,
  ['i'] = false, ['j'] = false, ['k'] = false, ['l'] = false,
  ['m'] = false, ['n'] = false, ['o'] = false, ['p'] = false,
  ['q'] = false, ['r'] = false, ['s'] = false, ['t'] = false,
  ['u'] = false, ['v'] = false, ['w'] = false, ['x'] = false,
  ['y'] = false, ['z'] = false,
}

fn int non_redundant_optimized_low_case_check_function_Example(char c)
{
  if (is_upcase[c] == false && !is_upcase[c])
    return (is_upcase[c] == false && !is_upcase[c])
}

// NOTE: hope that 'str' contains alphabetic chars only & hope for best
fn int lowercase_split_No_work(char[] str)
{
  const int hope_str_n_dont_Exceed_this_amount = 5
  const int p_ = 256
  char [hope_str_n_dont_Exceed_this_amount][p_] strs = {"Hi"}
  __BUILT_IN_INLINE_SIMD_FORCE_ASM_X64_BUZZWORDS_PRINT(strs[1])

  
  int ii = 0, i_start = 0
  for (int i=0, char c=str[0]; c != '\0'; c=str[i++]) // => 'iter (str) {...}'
  {
    if (non_redundant_optimized_low_case_check_function_Example(c))
    {
      for (int ii = 0; ii <= (i - i_start); ii++)
      {
        strs[ii] = c
        ii++
      }
      i_start = i
    }
  }

  ret 5000;
}

lowercase_split_No_work(test_buf)
```

```
int array[25][12] jar
```
