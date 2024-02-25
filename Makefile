CC = gcc
CFLAGS = -c -g -m32 -fno-builtin -nostdlib -fno-stack-protector
TARGET = minicrt.a
CFILES := $(wildcard *.c)
ALL_OBJS := $(patsubst %.c,%.o,$(CFILES))
# 过滤掉test.o
OBJS := $(filter-out test.o,$(ALL_OBJS))
$(TARGET):$(OBJS)
	ar -rs $(TARGET) $(OBJS)

test:$(TARGET)
	$(CC) -g $(CFLAGS) test.c; \
	ld -m elf_i386 -static -e mini_crt_entry entry.o test.o minicrt.a

clean:
	-$(RM) $(TARGET) $(ALL_OBJS) a.out
r: clean $(TARGET)