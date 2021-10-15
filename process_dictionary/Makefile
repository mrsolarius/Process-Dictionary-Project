CC = gcc
CPPFLAGS = -Iinclude
SRCS = main.c DDP.c
OBJS = $(SRCS:.c=.o)
EXEC= processDictionay

all: $(EXEC)

run:$(EXEC)
	./$(EXEC) 5

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

clean:
	@echo "suppression de fichier temporaires et des builds"
	rm *.o
	rm $(EXEC)