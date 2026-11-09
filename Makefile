# temp
CC = gcc
CCFLAGS = -std=c99 -pedantic -Wall -Wformat=2 -Wconversion -Wimplicit-fallthrough \
-Werror=format-security \
-Wfloat-equal -Wimplicit-int \
-Wundef -Wshadow -Wpointer-arith \
-Wcast-align -Wstrict-prototypes \
-Wwrite-strings -Waggregate-return \
-Wstrict-overflow=4 -Wstrict-aliasing=3 \
-Wcast-qual -Wswitch-default -Wswitch-enum \
-Wconversion -Wunreachable-code -Wsequence-point \
--strict-flex-arrays=3 -Wtrampolines \
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 \
-fstack-clash-protection -fstack-protector-strong \
-Wl,-z,nodlopen -Wl,-z,noexecstack \
-Wl,-z,relro -Wl,-z,now \
-Wl,--as-needed -Wl,--no-copy-dt-needed-entries \
-fno-diagnostics-color -Wno-switch -Wno-switch-enum

ustm: ustm.c ustmlex.c ustmgen.c
	$(CC) $(CCFLAGS) -o ustm ustm.c ustmlex.c ustmgen.c
