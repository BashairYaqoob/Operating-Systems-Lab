CC      = gcc
CFLAGS  = -Wall -Wextra -pthread
LIBS    = -lncurses -lrt
TARGET  = smart_parking
SRCS    = main.c parking.c dashboard.c

all: $(TARGET)

$(TARGET): $(SRCS) parking.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)

run: all
	./$(TARGET)
