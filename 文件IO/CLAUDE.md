[根目录](../CLAUDE.md) > **文件IO**

# 文件IO

## 模块职责

学习和实践 C 标准库(stdio.h)提供的文件 IO 操作函数，涵盖文件的打开、关闭、字符级读写、字符串读写和格式化读写。

## 入口与启动

每个 `.c` 文件均为独立可执行程序，通过 Makefile 中的对应目标编译运行:

```bash
make fopen_test    # 测试 fopen() 文件打开
make fclose_test   # 测试 fclose() 文件关闭
make fputc_test    # 测试 fputc() 单字符写入
make fputs_test    # 测试 fputs() 字符串写入
make fprintf_test  # 测试 fprintf() 格式化写入
make fgetc_test    # 测试 fgetc() 字符读取
```

Makefile 使用"编译-运行-清理"一体化模式，执行 `make <target>` 后会自动运行程序并删除可执行文件。

## 对外接口

本模块为学习示例，不对外提供接口。各程序练习的标准库函数:

| 函数 | 头文件 | 说明 |
|------|--------|------|
| `fopen()` | `<stdio.h>` | 打开文件，支持 r/w/a/r+/w+/a+ 六种模式 |
| `fclose()` | `<stdio.h>` | 关闭文件，成功返回 0，失败返回 EOF |
| `fputc()` | `<stdio.h>` | 向文件写入单个字符 |
| `fputs()` | `<stdio.h>` | 向文件写入字符串 |
| `fprintf()` | `<stdio.h>` | 向文件写入格式化字符串 |
| `fgetc()` | `<stdio.h>` | 从文件读取单个字符，到达末尾返回 EOF |

## 关键依赖与配置

- **编译器**: GCC(通过 Makefile 中 `CC:=gcc` 定义)
- **系统头文件**: 仅依赖 `<stdio.h>`
- **测试文件**: `example.txt`(fopen/fclose 使用)、`io.txt`(其他写入函数使用)

## 数据模型

本模块不涉及数据结构，操作对象为磁盘文本文件:

- `example.txt` -- 预置的测试文件，内容为 "This is a file."
- `io.txt` -- 由多个写入程序共同操作的文件，内容随运行累积

## 测试与质量

- 无自动化测试框架
- 每个程序通过观察控制台输出和文件内容变化来验证正确性
- 代码中包含详尽的 Doxygen 风格注释，记录每个函数的参数、返回值和注意事项

## 常见问题 (FAQ)

**Q: Makefile 中 `-` 前缀的含义是什么?**
A: `-` 前缀让 make 忽略该命令的错误返回值，即使编译或运行失败也不会中断构建流程。

**Q: `io.txt` 文件内容越来越多怎么办?**
A: 这是正常现象。`a+` 模式是追加写入，多次运行会持续添加内容。可以手动删除或清空该文件。

**Q: 为什么有的程序用 `example.txt`，有的用 `io.txt`?**
A: `fopen_test.c` 和 `fclose_test.c` 只做打开/关闭操作，使用只读模式读取预置的 `example.txt`；其余程序涉及写入，统一使用 `io.txt`。

## 相关文件清单

```
文件IO/
  Makefile          -- 构建脚本(编译-运行-清理一体化)
  fopen_test.c      -- fopen() 打开文件示例
  fclose_test.c     -- fclose() 关闭文件示例
  fputc_test.c      -- fputc() 单字符写入示例
  fputs_test.c      -- fputs() 字符串写入示例
  fprintf_test.c    -- fprintf() 格式化写入示例
  fgetc_test.c      -- fgetc() 字符读取示例
  example.txt       -- 预置测试文件
  io.txt            -- 写入操作的输出文件
```

## 变更记录 (Changelog)

| 日期 | 操作 | 说明 |
|------|------|------|
| 2026-05-02 | 初始化 | 首次生成模块 CLAUDE.md |
