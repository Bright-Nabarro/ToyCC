#include "decl_ast.hpp"

namespace tinyc
{

/// ConstInitVal Implementation
ConstInitVal::ConstInitVal(std::unique_ptr<Location> location,
						   std::unique_ptr<ConstExpr> const_expr)
	: BaseAST{ast_const_init_val, std::move(location)},
	  m_const_expr{std::move(const_expr)}
{
}

auto ConstInitVal::get_const_expr() const -> const ConstExpr&
{
	return *m_const_expr;
}


/// ConstDef Implementation
ConstDef::ConstDef(std::unique_ptr<Location> location,
				   std::unique_ptr<Ident> ident,
				   std::unique_ptr<ConstInitVal> const_init_val)
	: BaseAST{ast_const_def, std::move(location)}, m_ident{std::move(ident)},
	  m_const_init_val{std::move(const_init_val)}
{
}

auto ConstDef::get_ident() const -> const Ident&
{
	return *m_ident;
}

auto ConstDef::get_const_int_val() const -> const ConstInitVal&
{
	return *m_const_init_val;
}


/// ConstDefList
ConstDefList::ConstDefList(std::unique_ptr<Location> location):
	BaseAST { ast_const_def_list,  std::move(location) },
	m_const_defs {}
{
}

ConstDefList::ConstDefList(std::unique_ptr<Location> location,
				 std::unique_ptr<ConstDefList> rhs,
				 std::unique_ptr<ConstDef> ptr):
	BaseAST { ast_const_def_list, std::move(location) },
	m_const_defs { std::move(rhs->get_vector()) }
{
	m_const_defs.push_back(std::move(ptr));
}

auto ConstDefList::get_const_defs() const -> const Vector&
{
	return m_const_defs;
}

auto ConstDefList::begin() const -> Vector::const_iterator
{
	return m_const_defs.cbegin();
}

auto ConstDefList::end() const -> Vector::const_iterator
{
	return m_const_defs.cend();
}

auto ConstDefList::get_vector() -> Vector&
{
	return m_const_defs;
}


/// ConstDecl Implementation
ConstDecl::ConstDecl(std::unique_ptr<Location> location,
					 std::unique_ptr<ScalarType> scalar_type,
			  		 std::unique_ptr<ConstDef> const_def,
					 std::unique_ptr<ConstDefList> const_def_list)
	: BaseAST{ast_const_decl, std::move(location)},
	  m_scalar_type{std::move(scalar_type)},
	  m_const_def { std::move(const_def) },
	  m_const_def_list{std::move(const_def_list)}
{
}

auto ConstDecl::get_scalar_type() const -> const ScalarType&
{
	return *m_scalar_type;

}

auto ConstDecl::get_first_const_def() const -> const ConstDef&
{
	return *m_const_def;
}

auto ConstDecl::get_const_def_list() const -> const ConstDefList&
{
	return *m_const_def_list;
}

/// Decl Implementation
Decl::Decl(std::unique_ptr<Location> location,
		   std::unique_ptr<ConstDecl> const_decl)
	: BaseAST{ast_decl, std::move(location)},
	  m_const_decl{std::move(const_decl)}
{
}

auto Decl::get_const_decl() const -> const ConstDecl&
{
	return *m_const_decl;
}

}	//namespace tinyc

