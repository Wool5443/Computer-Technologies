INCLUDE=-I./include

PREF_SRC=src/
PREF_OBJ=obj/

EXE=useless

SRC=$(wildcard $(PREF_SRC)*.c)
OBJ=$(patsubst $(PREF_SRC)%.c, $(PREF_OBJ)%.o, $(SRC))

debug : dirs
debug : CFLAGS = -Wall -Wextra -O0 -g -ggdb3 -fsanitize=address -std=gnu17
debug : $(EXE)

release : dirs
release : CFLAGS = -Wall -O3 -std=gnu17
release : $(EXE)

$(EXE) : $(OBJ)
	@gcc $(CFLAGS) $^ -o $@ -no-pie

$(PREF_OBJ)%.o : $(PREF_SRC)%.c
	@gcc $(INCLUDE) $(CFLAGS) $^ -c -o $@

.PHONY : dirs
dirs:
	@mkdir -p $(PREF_OBJ)

.PHONY : clean
clean:
	@rm $(PREF_OBJ)* $(EXE)
