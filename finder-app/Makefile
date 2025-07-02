INCLUDE_DIR = .
SRC_DIR = .

CC ?= $(CROSS_COMPILE)gcc
CFLAGS += -I$(INCLUDE_DIR)
OBJ = writer.o


all : $(OBJ)
	$(CC) $(OBJ) -o writer

writer.o : $(SRC_DIR)/writer.c
	$(CC) -c $(SRC_DIR)/writer.c $(CFLAGS)

clean:
	rm -f *.o ./writer	 
