# ToyCC - 基于Flex/Bison/LLVM的实验性C编译器

一个用于学习编译器设计的C语言编译器前端，计划支持C99子集，生成LLVM IR并编译为本地可执行文件。

### 支持的语言特性
- **基本数据类型:** `int` `bool` `void` 等
- **运算符:** 一元`-`,`!`; 二元: 优先级表中 [`lv3`~`lv7`](https://zh.cppreference.com/w/c/language/operator_precedence)
- **控制流:** `if-else`(解决悬垂else问题), `while`, `return`
- **函数定义与调用:** 基本的参数传递和返回值处理
- **注释:** 支持`//`和`/* ... */`风格注释
- **错误报告:** 统一了前端与中端的错误报告，输出关联到源代码

### 编译与使用
#### 依赖项
- [LLVM 19](https://github.com/llvm/llvm-project)
- [Flex 2.6+](https://github.com/westes/flex)
- [bison 3.7+](https://www.gnu.org/software/bison)
- [spdlog 1.15.0](https://github.com/gabime/spdlog)
- gcc 14.0+
- CMake 3.20+

#### 编译步骤
```shell
git clone https://github.com/Bright-Nabarro/ToyCC.git
cd ToyCC
mkdir build
cmake -B build -DDEBUG_MODE=OFF
cmake --build build -j10
```

### 使用方法
``` shell
bin/toycc <input_file> [options]
```
#### 选项说明
- `<input file>` 目前只能指定一个, 代表待编译源代码
- `-filetype=`  指定输出文件的类型:
    - `asm` 输出汇编,
    - `obj` 输出二进制文件。
    - 如果未指定，默认输出汇编文件
- `-emit-llvm` 指定输出 llvm-ir,
    - 如果`-filetype=asm`, 则生成`.ll` llvm汇编文件,
    - 如果`-filetype=obj`, 则生成`.bc`llvm二进制文件, 默认为`false`
- `-o` 指定文件名，指定的文件名后缀不会自动更改。
- `-trace` 开启`flex`, `bison`的`debug trace`和`spdlog`的`debug`输出
- `-O` 指定优化级别，目前没有实现

### 示例
生成 llvm-ir
```shell
bin/toycc example.c -filetype=asm -emit-llvm
```
生成 二进制目标文件
```shell
bin/toycc example.c -filetype=obj
```
