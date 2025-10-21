CC = clang
CFLAGS = -Wall -Wextra
LIBS = $(shell pkg-config --libs --cflags raylib)
TARGET = bounc
SOURCES = boun.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) $(LIBS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)
