#include <cassert>
#include "base_ast.hpp"

namespace toycc
{

BaseAST::BaseAST(AstKind kind, std::unique_ptr<Location> location)
	: m_kind{kind}, m_location{std::move(location)}
{}

auto BaseAST::accept(ASTVisitor& visitor) -> std::expected<void, std::string>
{
	return visitor.visit(this);
}

auto BaseAST::get_kind() const -> AstKind
{ return m_kind; }

[[nodiscard]]
auto BaseAST::get_kind_str() const -> const char*
{
	switch (get_kind())
	{
#define AST_KIND(ast_kind, ast_category)                                       \
	case ast_kind:                                                             \
		return ast_category;

#include "ast.def"

#undef AST_KIND
	default:
		assert(false && "Unkown AST kind");
	}
}

void BaseAST::report(Location::DiagKind kind, std::string_view msg,
				llvm::SourceMgr* src_mgr) const
{
	m_location->report_in_srcmgr(kind, msg, src_mgr);
}

}	//namespace toycc
	

