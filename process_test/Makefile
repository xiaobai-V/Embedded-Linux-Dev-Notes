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