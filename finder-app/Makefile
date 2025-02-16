INCLUDE_DIR = .
SRC_DIR = .

CROSS_COMPILE ?= 
COMPILER = gcc
CFLAGS = -I$(INCLUDE_DIR)
OBJ = writer.o


all : $(OBJ)
	$(CROSS_COMPILE)$(COMPILER) $(OBJ) -o writer

writer.o : $(SRC_DIR)/writer.c
	$(CROSS_COMPILE)$(COMPILER) -c $(SRC_DIR)/writer.c $(CFLAGS)

clean:
	rm -f *.o ./writer	 
