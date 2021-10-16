CC = gcc
SRCS = main.c DDP.c
TEST_SRCS = testDDP.c DDP.c
OBJS = $(SRCS:.c=.o)
OBJS_TEST = $(TEST_SRCS:.c=.o)
EXEC= processDictionay
TEST= ddpTest

all: test run

run:$(EXEC)
	./$(EXEC) 5

test:$(TEST)

$(TEST): $(OBJS_TEST)
	$(CC) $(OBJS_TEST) -o $(TEST)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

clean:
	@echo "suppression de fichier temporaires et des builds"
	rm *.o
	rm $(EXEC) $(TEST)