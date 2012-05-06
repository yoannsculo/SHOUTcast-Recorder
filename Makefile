CC		   = gcc
BUILD_PATH = build
EXEC 	   = $(BUILD_PATH)/shoutr
SRC        = $(wildcard src/*.c) 
OBJ        = $(SRC:src/%.c=$(BUILD_PATH)/%.o)

INCLUDES   = -Iinclude \
			 -I/usr/include

CFLAGS	   = -W -Wall -g
LDFLAGS	   = -L/usr/lib -lcurl

all: prepare $(EXEC)

prepare:
	@mkdir -p $(BUILD_PATH)

$(BUILD_PATH)/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
	-@(cp ./radios/radio.pls $(BUILD_PATH))
	-@(cp ./radios/frequence3.pls $(BUILD_PATH))

clean:
	-@(rm -f $(EXEC) $(OBJ))
