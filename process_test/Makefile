CC := gcc

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