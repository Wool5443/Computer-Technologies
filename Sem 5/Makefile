TARGET=main5

PREF_OBJ=build
PREF_SRC=src

SRC=src/main5.c
OBJ=$(patsubst $(PREF_SRC)/%.c, $(PREF_OBJ)/%.o, $(SRC))

LFLAGS=-lpthread

.PHONY : all
all : $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^ -o $(PREF_OBJ)/$@ $(LFLAGS)

$(PREF_OBJ)/%.o : $(PREF_SRC)/%.c
	$(CC) $^ -c -o $@

.PHONY : clean
clean:
	rm -f $(PREF_OBJ)/*

