# Nazev vysledne aplikace
TARGET = prog
SOURCES = openmp.cpp

# Nazev kompileru
CC = g++

# Dodatecne parametry pro kompilator
CFLAGS = -fopenmp -g3 -lrt
CFLAGS_PROFILE = -pg

# Pravidlo, ktere se vola defaultne pokud se zavola samotny prikaz make
all: $(TARGET)

# Oznaceni cilu, ktere nejsou vystupnim souborem
.PHONY: clean run debug profile

# Pravidlo pro linkovani vysledne aplikace
$(TARGET): $(SOURCES)
	$(CC) -o $(TARGET) $(SOURCES) $(CFLAGS)

# Pravidlo pro zkompilovani a nasledne spusteni
# make run
run: $(TARGET)
	./$(TARGET) 

# Pravidlo pro zkompilovani a nasledene spusteni v gdb
debug: $(TARGET)
	gdb $(TARGET)

# Pravidlo pro spusteni profilovani
# make profile
profile: clean $(SOURCES)
	$(CC) -o $(TARGET) $(CFLAGS) $(CFLAGS_PROFILE) $(DEFINE) $(SOURCES)
	./$(TARGET) 
	gprof $(TARGET) | less

# Pravidlo, ktere vrati stav adresare do stavu pred kompilaci. 
# make clean
clean:
	if [ -e $(TARGET) ]; then rm $(TARGET); fi
	if [ -e gmon.out ]; then rm gmon.out; fi
