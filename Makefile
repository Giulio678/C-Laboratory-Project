.PHONY: clean all test run

CC = gcc
CFLAGS = -g -Wall -pedantic
LDLIBS = -lpthread
MAIN = bibserver
CLIENT = bibclient
INCLUDES = -Iinclude
MODE = mode
.DEFAULT_GOAL := run

# Array di argomenti per il server
SERVER_ARGS = \
	"test1 data/bib1.txt 5" \
	"test2 data/bib2.txt 3" \
	"test3 data/bib3.txt 1" \
	"test4 data/bib4.txt 4" \
	"test5 data/bib5.txt 2" 

TARGETS = common_utils.o \
		  mythreads.o \
		  unboundedqueue.o \
		  mysocket.o \
		  time_utils.o \
		  record_utils.o

SRCS = $(wildcard src/*.c)
SRCS := $(filter-out src/bibserver.c src/bibclient.c,$(SRCS))
OBJS = $(patsubst src/%.c, %.o, $(SRCS))

# Regola per generare gli oggetti .o dai file sorgenti .c
%.o: src/%.c
	$(CC) $(INCLUDES) -c $< -o $@

all: $(MODE) $(TARGETS) $(MAIN) $(CLIENT) 

$(MAIN): src/bibserver.c $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDLIBS)

$(CLIENT): src/bibclient.c common_utils.o mysocket.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $(CLIENT) $(LDLIBS)

$(MODE):
	chmod +rwx src/*.c
	chmod +rx bibaccess

clean:
	rm -f $(MAIN) $(CLIENT) config/bib.conf  *.o logs/*.log

test: $(MAIN)
	@echo "Esecuzione dei test"
	@for server_arg in $(SERVER_ARGS); do \
		echo $$server_arg; \
		./$(MAIN) $$server_arg & \
		sleep 1; \
	done


	@sleep 1

	@for i in 1 2 3 4 5 6 7 8; do \
		for j in 1 2 3 4 5; do \
			./$(CLIENT) --autore="Singh, Jaswinder Pal" --titolo="Parallel Computer Architecture: A Hardware/Software Approach" --p & \
		done; \
		sleep 1; \
	done

	@sleep 1

	@killall -INT $(MAIN)

	@echo "Chiamata SIGNAL(SIGINT o SIGTERM) attendo 10 secondi prima di lanciare bibaccess..."
	@sleep 10
	@./bibaccess --query logs/*.log
	@./bibaccess --loan logs/*.log


run: clean all test
