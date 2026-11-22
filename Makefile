.DELETE_ON_ERROR:

CC		:= clang
BUILD	:= build
BIN		:= $(BUILD)/fenxel
OBJ		:= $(BUILD)/obj
GAME	?= examples/demo

LJ_DIR	:= vendor/luajit
LJ_LIB	:= $(LJ_DIR)/src/libluajit.a

SDL_CFLAGS	:= $(shell pkg-config --cflags sdl3)
SDL_LIBS	:= $(shell pkg-config --libs sdl3)

SAN		?= 1
WARN	:= -Wall -Wextra -Wpedantic -Wshadow
CFLAGS	:= -std=c23 -g -O1 $(WARN) -MMD -MP \
		   -Ihost -I$(LJ_DIR)/src $(SDL_CLFAGS)
LDLIBS	:= $(LJ_LIB) $(SDL_LIBS) -lm -ldl

ifeq ($(SAN),1)
	CFLAGS	+= -fsanitize=address,undefined -fno-omit-frame-pointer
	LDFLAGS	+= -fsanitize=address,undefined
endif

SRCS := host/main.c
OBJS := $(SRCS:host/%.c=$(OBJ)/%.o)
DEPS := $(OBJS:.o=.d)

EMBEDDED := vendor/fennel.lua host/boot.lua host/api.h

.PHONY: all run clean distclean

all: $(BIN)

run: $(BIN)
	@$(BIN) $(GAME)

$(BIN): $(OBJS) $(LJ_LIB)
	@$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OBJ)/%.o: host/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/main.o: $(EMBEDDED)
 
$(LJ_LIB):
	$(MAKE) -C $(LJ_DIR) BUILDMODE=static
 
$(OBJ):
	mkdir -p $@
 
clean:
	rm -rf $(BUILD)
 
distclean: clean
	$(MAKE) -C $(LJ_DIR) clean
 
-include $(DEPS)
