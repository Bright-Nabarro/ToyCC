# 目前待改正问题
- [x] 语义分析(生成代码阶段)报错没有关联到源代码位置，需要在语法树中添加位置的记录信息
- [ ] handle的错误输出需要与main统一
- [ ] handle 的返回值使用std::expected包装
- [ ] 较多实现没有标记为[[nodiscard]]
- [ ] ast.hpp 模板添加概念约束
- [ ] 添加优化的参数选项
- [ ] ctype class改名，添加更多帮助型方法

# 重构需要注意的问题
- [ ] 在前端和管理工具统一日志输出逻辑，使用外部库spdlog
- [ ] parser.yy 中的静态断言可以去除
- [ ] ast添加对自己的描述，便于输出开始，结束信息
- [ ] llvm::Block添加对应功能的描述性名字

