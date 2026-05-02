CC := gcc

.PHONY: named_fifo named_fifo_clean

system_test: system_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm $@

fork_test: fork_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm $@

	
fork_fd_test: fork_fd_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm $@

erlou: erlou.c
	-$(CC) -o $@ $^

execve_test: execve_test.c erlou
	-$(CC) -o $@ $<
	-./$@
	-rm ./$@ ./$(word 2, $^)

fork_execve_test: fork_execve_test.c erlou
	-$(CC) -o $@ $(firstword $^)
	-./$@
	-rm ./$@ ./$(lastword $^)

waitpid_test: waitpid_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

pstree_test: pstree_test.c erlou
	-$(CC) -o $@ $(firstword $^)
	-./$@
	-rm ./$@ ./$(lastword $^)

erlou_block: erlou_block.c
	-$(CC) -o $@ $^

orphan_process_test: orphan_process_test.c erlou_block
	-$(CC) -o $@ $(firstword $^)
	-./$@
	-rm ./$@ ./$(lastword $^)

perror_test: perror_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

errno_test: errno_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

process_test: process_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

unnamed_pipe_test: unnamed_pipe_test.c
	-$(CC) -o $@ $^
	-./$@ hello_pipe
	-rm ./$@

my_unnamed_pipe_test: my_unnamed_pipe_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

fifo_write: fifo_write.c
	-$(CC) -o $@ $^

fifo_read: fifo_read.c
	-$(CC) -o $@ $^

named_fifo: fifo_write fifo_read

named_fifo_clean:
	-rm ./fifo_write ./fifo_read
	-rm -f /tmp/myfifo