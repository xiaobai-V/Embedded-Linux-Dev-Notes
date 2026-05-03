CC := gcc
CFLAGS := -Wall -Wextra
LDFLAGS := -lpthread

# 所有目标
TARGETS := create_test thread_ringbuf_test terminate_test \
           pthread_detach_test pthread_cancel_deferred_test \
           pthread_cancel_async_test pthread_cancel_disabled_test

.PHONY: all clean

all: $(TARGETS)

# 通用编译-运行-清理规则
%: %.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

pthread_detach_test: pthread_detach_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

pthread_cancel_deferred_test: pthread_cancel_deferred_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

pthread_cancel_disabled_test: pthread_cancel_disabled_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

pthread_cancel_async_test: pthread_cancel_async_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@


clean:
	rm -f $(TARGETS) /tmp/cancel_test.txt


