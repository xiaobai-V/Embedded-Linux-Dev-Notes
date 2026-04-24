# 定义变量
objects := main.o hello.o

# 定义目标
main: $(objects)
	gcc -o main $(objects)

# 定义编译规则
%.o: %.c
	gcc -c $< -o $@
# 定义清理规则
clean:
	rm -f $(objects) main	
