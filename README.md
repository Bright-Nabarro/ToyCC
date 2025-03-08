# ToyCC - 基于Flex/Bison/LLVM的实验性C编译器

一个用于学习编译器设计的C语言编译器前端，计划支持C99子集，生成LLVM IR并编译为本地可执行文件。

### 选项
- `<input file>` 目前只能指定一个, 代表待编译源代码
- `-filetype=`  指定输出文件的类型: `asm` 输出汇编, `obj` 输出二进制文件。
如果未指定，默认输出汇编文件
- `-emit-llvm` 指定输出llvm-ir,
    如果`-filetype=asm`, 则生成`.ll` llvm汇编文件,
    如果`-filetype=obj`, 则生成`.bc`llvm二进制文件, 默认为`false`
- `-o` 指定文件名，指定的文件名后缀不会自动更改。
- `-trace` 开启`flex`, `bison`的`debug trace`和`spdlog`的`debug`输出
- `-O` 指定优化级别，目前没有实现

### 依赖项
- [LLVM 19](https://github.com/llvm/llvm-project)
- [Flex 2.6+](https://github.com/westes/flex)
- [bison 3.7+](https://www.gnu.org/software/bison)
- [spdlog 1.15.0](https://github.com/gabime/spdlog)

### 依赖工具
- gcc 14.0+
- CMake 3.20+


