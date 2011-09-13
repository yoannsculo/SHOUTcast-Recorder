CC		   = gcc
BUILD_PATH = build
EXEC 	   = $(BUILD_PATH)/shoutr
SRC        = $(wildcard src/*.c) 
OBJ        = $(SRC:src/%.c=$(BUILD_PATH)/%.o)

INCLUDES   = -Iinclude \
			 -I/usr/include

CFLAGS	   = -W -Wall -g
LDFLAGS	   = -L/usr/lib -lcurl

all: $(EXEC)

$(BUILD_PATH)/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
	-@(cp ./radios/radio.pls $(BUILD_PATH))

clean:
	-@(rm -f $(EXEC) $(OBJ))
