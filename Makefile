TARGET = ./bin/sticker_client

CC = g++
CFLAGS = -Wall -g -std=c++11
SRCS = ./src/main.cpp ./src/functions.cpp
OBJS = $(SRCS:.cpp=.o)
INC = -I ./asio-1.20.0/include
LIB = -lws2_32 

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INC) -o $(TARGET) $(OBJS) $(LIB)

./src/%.o: ./src/%.cpp
	$(CC) $(CFLAGS) $(INC) -c $^ -o $@

clean:
	$(RM) $(OBJS)