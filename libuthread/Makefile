# Target library
lib := libuthread.a
objs := queue.o context.o uthread.o sem.o preempt.o

CC := gcc
CFLAGS := -Wall -Werror -Wextra -MMD

ifneq ($(V),1)
Q = @
endif

ifneq ($(D),1)
CFLAGS    += -O2
else
CFLAGS    += -g
endif

all: $(lib)

deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

$(lib): $(objs)
	@echo "CC $@"
	$(Q)ar rcs $(lib) $(objs)

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "clean"
	$(Q)rm -f $(lib) $(objs) $(deps)
