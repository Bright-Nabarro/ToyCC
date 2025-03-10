# 需要添加的功能
- [ ] (优先) 增加短路求值
- [ ] (困难) `ConversionHelper`根据整型常量判断细致化转换是否合法
- [ ] `ConversionHelper`添加`config`的配置功能
- [ ] 动态变量检测未初始化功能(需要修改`SymbolTable`)
- [ ] 完整运算符

# 目前待改正问题
- [x] 语义分析(生成代码阶段)报错没有关联到源代码位置，需要在语法树中添加位置的记录信息
- [x] handle的错误输出需要与main统一
- [x] ast.hpp 模板添加概念约束
- [x] 模块改名 main->backend front->frontend semantix->langspec
- [x] ast没有logger, 需要重新设计包装
- [x] handle错误处理的调用栈的错误报告内容重复
- [ ] 较多实现没有标记为[[nodiscard]]
- [ ] 添加优化的参数选项
- [ ] ctype class改名，添加更多帮助型方法
- [ ] (较困难) 拆解, 重构CodeGenVisitor
- [ ] `ConversionHelper`关于整型无法区分是否有符号
- [ ] 文件名添加到`CGContext`
- [ ] `ast`位置移动: Block相关->block.hpp, FuncDef及以后->ast.hpp

# 重构需要注意的问题
- [x] 在前端和管理工具统一日志输出逻辑，使用外部库spdlog
- [x] ast添加对自己的描述，便于输出开始，结束信息
- [ ] parser.yy 中的静态断言可以去除

