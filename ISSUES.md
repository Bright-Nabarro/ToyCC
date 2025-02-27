# 需要添加的功能
- [ ] 添加llvm-ir直接向控制台输出的方法
- [ ] (较困难) `ConversionHelper`根据整型常量判断细致化转换是否合法
- [ ] `ConversionHelper`添加`config`的配置功能
- [ ] 动态变量检测未初始化功能(需要修改`SymbolTable`)

# 目前待改正问题
- [x] 语义分析(生成代码阶段)报错没有关联到源代码位置，需要在语法树中添加位置的记录信息
- [ ] handle的错误输出需要与main统一
- [ ] handle 的返回值使用std::expected包装
- [ ] 较多实现没有标记为[[nodiscard]]
- [ ] ast.hpp 模板添加概念约束
- [ ] 添加优化的参数选项
- [ ] ctype class改名，添加更多帮助型方法
- [ ] 尝试修改flex和bison的报错输出，用spdlog注入
- [ ] 拆解, 重构CodeGenVisitor
- [ ] 模块改名 main->backend front->frontend semantix->langspec
- [ ] ast没有logger, 需要重新设计包装
- [ ] handle错误处理的调用栈的错误报告内容重复
- [ ] `ConversionHelper`关于整型无法区分是否有符号

# 重构需要注意的问题
- [x] 在前端和管理工具统一日志输出逻辑，使用外部库spdlog
- [ ] parser.yy 中的静态断言可以去除
- [ ] ast添加对自己的描述，便于输出开始，结束信息
- [ ] llvm::Block添加对应功能的描述性名字

