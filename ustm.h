/* ustm.h */
#define err(msg)          (fprintf(stderr, msg "\n"), (ustm_err = 1))
#define err_fmt(fmt, ...) (fprintf(stderr, fmt "\n", __VA_ARGS__), (ustm_err = 1))
