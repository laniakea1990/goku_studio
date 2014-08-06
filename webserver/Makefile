SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

CFLAGS = -g -c -I./
LDFLAGS = -lpthread

TARGET = simple_http_server

CROSS_COMPILE = 
CC = gcc

.PHONY : all
all : $(TARGET)
$(TARGET) : $(OBJS)
	$(CROSS_COMPILE)$(CC) -o $@ $^ $(LDFLAGS)
$(OBJS) : $(SRCS)
	$(CROSS_COMPIEL)$(CC) -o $@ $(CFLAGS) $(patsubst %.o,%.c,$@)

.PHONY : clean
clean :
	rm -rf $(OBJS) $(TARGET)

