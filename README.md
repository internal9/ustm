# ustm
## tbd
- kws: 'array', 'string, 'iter', 'dynamic'
- stuff for binary data idk

## notes


## keywords
literals:   "null", "true", "false"

types:      "bool", "char", "int", "num", "string", "array", "struct"

statements: "if", "elif", "else", "while", "for", "cases", "jmp"

modifiers:  "ref", "const"

misc:       "func"

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
fn int lowercase_split(char[] str)
{
  const int hope_str_n_dont_Exceed_this_amount = 5
  char [hope_str_n_dont_Exceed_this_amount] strs = {"Hi"}
  __BUILT_IN_INLINE_SIMD_FORCE_ASM_X64_PRINT(strs[1])

  for (int i=0, char c=str[0]; c != '\0'; c=str[i++]) // => 'iter (str) {...}'
  {
    int i_start = i
    if (non_redundant_optimized_low_case_check_function_Example(c))
    {
      
    }
  }
}
```

```
int array[25][12] jar
```
