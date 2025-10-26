CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
TARGET = grostd
SRC = td.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) -lm

clean:
	rm -f $(TARGET) *.o *.bin

run: $(TARGET)
	./$(TARGET) data.txt

test: $(TARGET)
	./$(TARGET) data.txt --save class.bin
	./$(TARGET) data.txt --load class.bin

.PHONY: all clean run test
