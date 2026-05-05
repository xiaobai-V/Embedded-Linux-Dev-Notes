CC := gcc
CFLAGS := -Wall -Wextra
LDFLAGS := -lpthread

.PHONY: clean

virtual_memory_test: virtual_memory_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

execve_invalid_path: execve_invalid_path.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

execve_full_path: execve_full_path.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

process_group_block_test: process_group_block_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

process_group_father_son_test: process_group_father_son_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@

process_group_two_group: process_group_two_group.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "===== 运行 $@ ====="
	-./$@
	-rm -f ./$@