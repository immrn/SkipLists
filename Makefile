TARGET = skiplist

CC = gcc

SOURCE = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst %,$(BINDIR)/%,$(notdir $(SOURCE:.c=.o)))

INCDIR = inc
SRCDIR = src
BINDIR = bin

CFLAGS = -I$(INCDIR) -g

$(BINDIR)/$(TARGET): $(OBJ)
	$(CC) -o $@ $^ -lm

$(BINDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c 	$< -o $@

.PHONY: clean

clean:
		rm $(OBJ) $(BINDIR)/$(TARGET)
