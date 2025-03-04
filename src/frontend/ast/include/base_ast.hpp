#pragma once
#include <llvm/Support/SourceMgr.h>
#include <memory>
#include <expected>

namespace toycc
{

class BaseAST;

class ASTVisitor
{
public:
	virtual
	~ASTVisitor() = default;

	virtual
	auto visit(BaseAST*) -> std::expected<void, std::string> = 0;
};


/// @brief BaseAST依赖的位置抽象基类
class Location
{
public:
	enum DiagKind
	{
		dk_error,
		dk_warning,
		dk_remark,
		dk_note,
	};

	virtual
	~Location() = default;
	
	virtual
	void report(Location::DiagKind kind, std::string_view msg) const = 0;

	virtual
	void set_src_mgr(const llvm::SourceMgr* src_mgr) const = 0;

private:
};


/// 使用llvm-rtti进行动态转换
class BaseAST
{
public:
	enum AstKind
	{
#define AST_KIND(ast_expr, msg) ast_expr,

#include "ast.def"

#undef AST_KIND
	};

	BaseAST(AstKind kind, std::unique_ptr<Location> location);

	virtual
	~BaseAST() = default;

	virtual
	auto accept(ASTVisitor& visitor) -> std::expected<void, std::string>;

	[[nodiscard]]
	auto get_kind() const -> AstKind;

	[[nodiscard]]
	auto get_kind_str() const -> const char*;

	void report(Location::DiagKind kind, std::string_view msg,
				llvm::SourceMgr* src_mgr) const;

	//void report_location() const;

private:
	AstKind m_kind;
	std::unique_ptr<Location> m_location;
};

#define TOYCC_AST_FILL_CLASSOF(ast_enum)                                       \
	[[nodiscard]]                                                              \
	static auto classof(const BaseAST* ast) -> bool                            \
	{                                                                          \
		return ast->get_kind() == ast_enum;                                    \
	}

}	//namespace toycc
