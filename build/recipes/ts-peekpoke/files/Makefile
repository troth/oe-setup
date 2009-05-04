PRG = ts-peekpoke
SRC = ts-peekpoke.c
OBJ = $(SRC:%.c=%.o)

CFLAGS = -g -Wall

all : $(PRG)

$(PRG): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)

clean :
	rm -f *.o $(PRG)
