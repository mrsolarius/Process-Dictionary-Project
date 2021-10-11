CC = gcc
CPPFLAGS = -Iinclude
SRCS = table.c testTable.c
OBJS = $(SRCS:.c=.o)
EXEC=testTable

all: $(EXEC)

run:$(EXEC)
	./$(EXEC) 5

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

clean:
	@echo "suppression de fichier temporaires et des builds"
	rm *.o
	rm $(EXEC)