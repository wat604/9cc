CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(filter-out test_func.c, $(SRCS:.c=.o))

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc test_func.o
	./test.sh

asb_test_func:
	$(CC) -fno-asynchronous-unwind-tables -masm=intel -S test_func.c $(LDFLAGS)

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean asb_test_func