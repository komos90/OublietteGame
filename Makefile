W_FLAGS := -Wall -Wshadow
CFLAGS := -std=c11 -O3
SRC_DIR := src/
LIBRARIES := -lSDL2 -lSDL2main -lSDL2_image -lSDL2_mixer -lm 
BIN_DIR := bin/
CC = clang
BIN_NAME := oubliette

all:
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(W_FLAGS) $(SRC_DIR)*.c $(LIBRARIES) -o $(BIN_DIR)$(BIN_NAME)

clean:
	rm -rf $(BIN_DIR)

