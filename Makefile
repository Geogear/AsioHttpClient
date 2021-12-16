TARGET = ./sticker_client

CC = g++
CFLAGS = -Wall -g -std=c++11
SRCS = ./src/main.cpp
OBJS = $(SRCS:.c=.o)
INC = -I ./asio-1.20.0/include
LIB = -lws2_32 

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INC) -o $(TARGET) $(OBJS) $(LIB)

clean:
	$(RM) $(OBJS)