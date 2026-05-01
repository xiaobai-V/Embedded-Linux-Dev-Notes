[根目录](../CLAUDE.md) > **makefile**

# makefile

## 模块职责

学习和练习 GNU Makefile 的编写方法，包括变量定义、多文件编译、模式规则和清理目标等核心概念。

## 入口与启动

```bash
# 编译生成 main 可执行文件
make

# 清理所有生成物(.o 文件和可执行文件)
make clean
```

## 对外接口

本模块为学习示例，不对外提供接口。程序功能简单:

- `main.c` 调用 `hello.c` 中定义的 `hello()` 函数，输出 "Hello, World!"

## 关键依赖与配置

- **编译器**: GCC
- **Makefile 变量**:
  - `objects := main.o hello.o` -- 定义中间目标文件列表
- **构建规则**:
  - 链接: `gcc -o main $(objects)`
  - 编译: `gcc -c $< -o $@` (模式规则 `%.o: %.c`)
  - 清理: `rm -f $(objects) main`

## 数据模型

本模块不涉及数据模型，仅为简单的多文件 C 程序:

```
main.c  -- 包含 main() 函数，调用 hello()
  |
  +-- #include "hello.h"  -- 声明 void hello()
hello.c -- 定义 hello() 函数实现
```

## 测试与质量

- 无自动化测试
- 通过运行 `./main` 验证输出是否为 "Hello, World!"
- 本模块重点在 Makefile 的编写方式，而非程序功能本身

## 常见问题 (FAQ)

**Q: 为什么这个模块不像其他模块那样自动运行和清理?**
A: 本模块的练习重点是 Makefile 本身的结构(变量、模式规则、clean 目标)，而非快速测试，因此保留了标准的编译-清理分离模式。

**Q: 模式规则 `%.o: %.c` 的含义是什么?**
A: `%` 是 Make 的模式匹配通配符。该规则表示"任何 `.o` 文件都从对应的 `.c` 文件编译生成"，避免为每个源文件重复编写编译规则。

## 相关文件清单

```
makefile/
  Makefile    -- 构建脚本(变量、模式规则、clean 目标)
  main.c      -- 主程序入口，调用 hello()
  hello.h     -- hello() 函数声明
  hello.c     -- hello() 函数实现
```

## 变更记录 (Changelog)

| 日期 | 操作 | 说明 |
|------|------|------|
| 2026-05-02 | 初始化 | 首次生成模块 CLAUDE.md |
