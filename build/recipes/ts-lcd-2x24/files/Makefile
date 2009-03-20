TOOLBIN  = $(HOME)/dev/OE/tmp/cross/armv5te/bin
TOOLNAME = $(TOOLBIN)/arm-angstrom-linux-uclibcgnueabi-

CC      = $(TOOLNAME)gcc
AR      = $(TOOLNAME)ar
AS      = $(TOOLNAME)as
LD      = $(TOOLNAME)ld
RANLIB  = $(TOOLNAME)ranlib
OBJCOPY = $(TOOLNAME)objcopy
OBJDUMP = $(TOOLNAME)objdump

PRG = ts-lcd-2x24
SRC = \
	$(PRG).c
OBJ = $(SRC:%.c=%.o)

CFLAGS  = -g -Wall
LDFLAGS =

all: $(PRG)

$(PRG): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)

clean:
	rm -rf *.o $(PRG)
