#pragma once

#include <llvm/Support/SMLoc.h>
#include <llvm/Support/SourceMgr.h>
#include <ostream>
#include "base_ast.hpp"

#define YYLLOC_DEFAULT(Cur, Rhs, N)                                            \
	do                                                                         \
	{                                                                          \
		if (N)                                                                 \
		{                                                                      \
			Cur.begin = YYRHSLOC(Rhs, 1).begin;                                \
			Cur.end = YYRHSLOC(Rhs, N).end;                                    \
		}                                                                      \
		else                                                                   \
		{                                                                      \
			Cur.begin = Cur.end = YYRHSLOC(Rhs, 0).end;                        \
		}                                                                      \
	} while (0)


namespace tinyc
{

class LLVMLocation: public Location
{
	friend auto operator<< (std::ostream& o, const LLVMLocation& loc) -> std::ostream&;
public:
	LLVMLocation() = default;

	llvm::SMLoc begin;
	llvm::SMLoc end;

	/**
	 * @note 需要在初始化时调用
	 * @param buf llvm::SourceMgr的字符指针
	 */
	void set_begin(const char* buf);

	/**
	 * @note 需要在初始化时调用
	 * @param buf llvm::SourceMgr的字符指针
	 */
	void set_end(const char* buf);
	
	auto get_range() const -> llvm::SMRange;
	
	/// @brief 调用report的前置函数
	//void set_src_mgr(const llvm::SourceMgr* src_mgr);
	
	/**
	 * @param src_mgr begin和end对应的SourceMgr
	 * @param dk 错误级别
	 */
	void report(Location::DiagKind kind, std::string_view msg) const override ;

	/// @brief set begin to end
	void step();

	/// @brief end后移len字节
	void update(std::size_t len);

	/// @note 需要在report前调用
	void set_src_mgr(const llvm::SourceMgr* src_mgr) const;

	/// @brief 查询某个级别输出信息的次数
	static
	auto search_counter(Location::DiagKind kind) -> std::size_t;

private:
	static inline
	std::unordered_map<Location::DiagKind, std::size_t> trace_counter;
	
	static
	void count(Location::DiagKind kind);

	static constexpr
	auto cvt_kind_to_llvm(Location::DiagKind kind) -> llvm::SourceMgr::DiagKind;

	mutable
	const llvm::SourceMgr* m_src_mgr;

};

auto operator<< (std::ostream& o, const LLVMLocation& loc) -> std::ostream&;

}	//namespace tinyc
