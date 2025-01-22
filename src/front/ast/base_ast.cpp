#include "base_ast.hpp"

namespace tinyc
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
	}
}

void BaseAST::report(Location::DiagKind kind, std::string_view msg) const
{
	m_location->report(kind, msg);
}

}	//namespace tinyc
	

