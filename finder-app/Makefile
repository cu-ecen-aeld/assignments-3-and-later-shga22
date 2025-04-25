

CC = gcc

CFLAGS = -g -Wall

ifdef CROSS_COMPILE
	CC = $(CROSS_COMPILE)gcc
endif

TARGET = writer

all:$(TARGET)

$(TARGET) : $(TARGET).c 
	$(CC) $(CFLAGS) -c -o $(TARGET).o $(TARGET).c
	$(CC) $(CFLAGS) -I/ $(TARGET).o -o $(TARGET)
	
	
clean: 
	rm -rf $(TARGET) *.o


