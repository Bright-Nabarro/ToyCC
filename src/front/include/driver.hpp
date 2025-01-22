#pragma once

#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/SMLoc.h>
#include <llvm/Support/MemoryBuffer.h>
#include <string_view>
#include <expected>
#include <memory>
#include "ast.hpp"
#include "bison_parser.hpp"
#include "llvm_location.hpp"

#define YY_DECL \
	auto yylex(tinyc::Driver& driver) -> yy::parser::symbol_type

YY_DECL;

namespace tinyc
{

/**
 * @brief 对于词法分析，语法分析的驱动类, parser开始分析
 * @note 词法分析过程内嵌在parser方法中
 * @note 需要通过DriverFactory构造，无法单独构造
 */
class Driver
{
	friend class DriverFactory;
private:
	Driver(llvm::SourceMgr& src_mgr);

public:
	/*
	 * @note 延迟构造，用于在非异常环境下处理构造函数错误
	 * @return 出错时返回std::unexpected, 描述错误内容
	 */
	auto construct(std::string_view file_name)
		-> std::expected<void, std::string>;

	/**
	 * @note 解析函数，只能调用一次
	 * @return true 成功, false 失败
	 * @note 失败自动通过parser.error输出消息
	 */
	auto parse() -> bool;
	
	/// @param ast 语法树根节点
	void set_ast(std::unique_ptr<CompUnit> ast)
	{ m_ast = std::move(ast); }
	auto get_ast() const -> const CompUnit&
	{ return *m_ast; }
	auto get_ast_ptr() -> CompUnit*
	{ return m_ast.get(); }

	/// @brief 获取parser实例，用于在flex中调用parser的方法
	auto get_parser() -> yy::parser&
	{ return *m_parser; }

	/**
	 * @brief 设置flex的读取buffer, 和debug_trace模式
	 * @note 在lexer.ll中定义
	 */
	void set_flex(const char* buffer, int buffer_size);

	/// @brief 设置是否输出debug调用栈
	void set_trace(bool debug_trace)
	{ m_debug_trace = debug_trace; }
	auto get_trace() -> bool
	{ return m_debug_trace; }

	/// @brief 提供给location用于定位
	auto get_src_mgr() const -> const llvm::SourceMgr&
	{ return m_src_mgr; }

	/// @brief 解析时获取位置记录，在yylex中调用
	auto get_location() -> LLVMLocation&;

private:
	/// @brief 获取文件的内存映射
	auto get_buffer() const -> const char*;

private:
	std::unique_ptr<CompUnit> m_ast;
	llvm::SourceMgr& m_src_mgr;
	unsigned m_bufferid;
	bool m_debug_trace;
	std::unique_ptr<yy::parser> m_parser;
	LLVMLocation m_location;
};


/** 
 * @warning driver持有parser的unique_ptr, \
 * parser持有由unique_ptr管理的内部driver的引用 \
 * 需要注意unique_ptr的生存周期
 */
class DriverFactory
{
public:
	DriverFactory(llvm::SourceMgr& src_mgr):
		m_src_mgr { src_mgr }
	{}

	auto produce_driver(std::string_view file_name)
		-> std::expected<std::unique_ptr<Driver>, std::string>;

private:
	llvm::SourceMgr& m_src_mgr;
};


}	//namespace tinyc

