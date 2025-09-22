# compiler
CC = gcc
CFLAGS = -Wall -Wextra -g

# executable name
TARGET = mygit

# source files
SRCS = main.c git_functions/check_repo.c 
OBJS = $(SRCS:.c=.o)

# default rule
all: $(TARGET)

# link all objects into final binary
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# compile .c into .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# remove build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# force rebuild
rebuild: clean all
