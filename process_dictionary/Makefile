CC = gcc

SRCS = main.c DDP.c controller.c node.c table.c
OBJS = $(SRCS:.c=.o)
EXEC= processDictionay
run:$(EXEC)
$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

TEST_DDP_SRCS = testDDP.c DDP.c
OBJS_DDP_TEST = $(TEST_DDP_SRCS:.c=.o)
TEST_DDP= ddpTest
test_DDP:$(TEST_DDP)
$(TEST_DDP): $(OBJS_DDP_TEST)
	$(CC) $(OBJS_DDP_TEST) -o $(TEST_DDP)

TEST_CONTROLLER_SRCS = testController.c controller.c node.c table.c DDP.c
OBJS_CONTROLLER_TEST = $(TEST_CONTROLLER_SRCS:.c=.o)
TEST_CONTROLLER = controllerTest
test_CONTROLLER:$(TEST_CONTROLLER)
$(TEST_CONTROLLER): $(OBJS_CONTROLLER_TEST)
	$(CC) $(OBJS_CONTROLLER_TEST) -o $(TEST_CONTROLLER)

all: test_DDP test_CONTROLLER run

clean:
	@echo "suppression de fichier temporaires et des builds"
	rm *.o
	rm $(EXEC) $(TEST_DDP) $(TEST_CONTROLLER)