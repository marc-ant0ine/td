CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = student_manager
SOURCES = td.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean
