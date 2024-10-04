TARGET=main

PREF_OBJ=build
PREF_SRC=src

SRC=src/main.c
OBJ=$(patsubst $(PREF_SRC)/%.c, $(PREF_OBJ)/%.o, $(SRC))

.PHONY : all
all : $(TARGET)
	echo $(SRC) $(OBJ)

$(TARGET) : $(OBJ)
	$(CC) $^ -o $@

$(PREF_OBJ)/%.o : $(PREF_SRC)/%.c
	$(CC) $^ -c -o $@

