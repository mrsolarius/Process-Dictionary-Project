CC = gcc
CPPFLAGS = -Iinclude
SRCS = project_1.c
OBJS = $(SRCS:.c=.o)
EXEC=projet_1

all: run

run:$(EXEC)
	./$(EXEC) 5

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

clean:
	@echo "suppression de fichier temporaires et des builds"
	rm *.o
	rm $(EXEC)