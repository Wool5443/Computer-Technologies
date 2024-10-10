PREF_SRC=src/
PREF_OBJ=obj/

EXE=useless

SRC=$(wildcard $(PREF_SRC)*.c)
OBJ=$(patsubst $(PREF_SRC)%.c, $(PREF_OBJ)%.o, $(SRC))

debug : dirs
debug : CFLAGS = -Wall -Wextra -O0 -g -ggdb3 -fsanitize=address
debug : main

release : dirs
release : CFLAGS = -Wall -O3
release : main

main : $(OBJ)
	@gcc $(CFLAGS) $^ -o $@ -no-pie

$(PREF_OBJ)%.o : $(PREF_SRC)%.c
	@gcc $(CFLAGS) $^ -c -o $@

.PHONY : dirs
dirs:
	@mkdir -p $(PREF_OBJ)

.PHONY : clean
clean:
	@rm $(PREF_OBJ)* $(EXE)

