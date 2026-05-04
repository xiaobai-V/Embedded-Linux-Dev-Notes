CC := gcc
CFLAGS := -Wall -Wextra
LDFLAGS := -lpthread

# 所有目标
TARGETS := create_test thread_ringbuf_test terminate_test \
           pthread_detach_test pthread_cancel_deferred_test \
           pthread_cancel_async_test pthread_cancel_disabled_test \
           sem_ringbuf_test

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

race_condition_test: race_condition_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

mutex_test: mutex_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

rwlock_test_writter_unlock: rwlock_test_writter_unlock.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

rwlock_test: rwlock_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

rwlock_random_order: rwlock_random_order.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

rwlock_write_hungry: rwlock_write_hungry.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

condition_var: condition_var.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

ringbuf_demo: ringbuf_demo.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

sem_ringbuf_test: sem_ringbuf_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

unnamed_sem_bin_thread_condition: unnamed_sem_bin_thread_condition.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

unnamed_sem_bin_thread: unnamed_sem_bin_thread.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

unnamed_sem_bin_process_condition: unnamed_sem_bin_process_condition.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

unnamed_sem_bin_process: unnamed_sem_bin_process.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

unnamed_sem_bin_process_illegal: unnamed_sem_bin_process_illegal.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@


unnamed_sem_bin_process_isolate: unnamed_sem_bin_process_isolate.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

unnamed_sem_count_thread: unnamed_sem_count_thread.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

unnamed_sem_count_process: unnamed_sem_count_process.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

named_sem_bin: named_sem_bin.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

named_sem_count: named_sem_count.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

thread_pool_test: thread_pool_test.c
	$(CC) $(CFLAGS) -o $@ $^ `pkg-config --cflags --libs glib-2.0`
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@


clean:
	rm -f $(TARGETS) /tmp/cancel_test.txt


