TARGET=main3

PREF_OBJ=build
PREF_SRC=src

SRC=src/main3.c
OBJ=$(patsubst $(PREF_SRC)/%.c, $(PREF_OBJ)/%.o, $(SRC))

LFLAGS=-lpthread

.PHONY : all
all : $(TARGET)
	echo $(SRC) $(OBJ)

$(TARGET) : $(OBJ)
	$(CC) $^ -o $@ $(LFLAGS)

$(PREF_OBJ)/%.o : $(PREF_SRC)/%.c
	$(CC) $^ -c -o $@

