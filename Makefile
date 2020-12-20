RMRF = rm -rf
MKDIRP = mkdir -p

SRC_DIR = src

PERLFLAGS = $(shell perl -MExtUtils::Embed -e ccopts -e ldopts)
CFLAGS = -O2 -Wall -Wextra

LIBS = -lncurses

inb: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(PERLFLAGS)
