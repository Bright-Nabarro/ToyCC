#include "decl_ast.hpp"

namespace toycc
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

auto ConstDef::get_const_init_val() const -> const ConstInitVal&
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
		 ConstDeclPtr const_decl)
	: BaseAST{ast_decl, std::move(location)},
	  m_value{std::move(const_decl)}
{
}

Decl::Decl(std::unique_ptr<Location> location,
		 VarDeclPtr var_decl)
	: BaseAST{ast_decl, std::move(location)},
	  m_value{std::move(var_decl)}
{
}

auto Decl::has_const_decl() const -> bool
{
	return std::holds_alternative<ConstDeclPtr>(m_value);
}

auto Decl::has_var_decl() const -> bool
{
	return std::holds_alternative<VarDeclPtr>(m_value);
}

auto Decl::get_const_decl() const -> const ConstDecl&
{
	return *std::get<ConstDeclPtr>(m_value);
}

auto Decl::get_var_decl() const -> const VarDecl&
{
	return *std::get<VarDeclPtr>(m_value);
}

// InitVal implementation
InitVal::InitVal(std::unique_ptr<Location> location, std::unique_ptr<Expr> expr)
    : BaseAST{ast_init_val, std::move(location)},
      m_expr{std::move(expr)}
{
}

auto InitVal::get_expr() const -> const Expr&
{
    return *m_expr;
}

// VarDef implementation
VarDef::VarDef(std::unique_ptr<Location> location, std::unique_ptr<Ident> ident)
    : BaseAST{ast_var_def, std::move(location)},
      m_initialized{false},
      m_ident{std::move(ident)},
      m_init_val{nullptr}
{
}

VarDef::VarDef(std::unique_ptr<Location> location, 
               std::unique_ptr<Ident> ident,
               std::unique_ptr<InitVal> init_val)
    : BaseAST{ast_var_def, std::move(location)},
      m_initialized{true},
      m_ident{std::move(ident)},
      m_init_val{std::move(init_val)}
{
}

auto VarDef::is_initialized() const -> bool
{
    return m_initialized;
}

auto VarDef::get_ident() const -> Ident&
{
    return *m_ident;
}

auto VarDef::get_init_val() const -> InitVal&
{
    return *m_init_val;
}

// VarDefList implementation
VarDefList::VarDefList(std::unique_ptr<Location> location)
    : BaseAST{ast_var_def_list, std::move(location)}
{
}

VarDefList::VarDefList(std::unique_ptr<Location> location,
                       std::unique_ptr<VarDefList> var_def_list,
                       std::unique_ptr<VarDef> var_def)
    : BaseAST{ast_var_def_list, std::move(location)}
{
    // Move existing definitions from var_def_list
    m_var_defs = std::move(var_def_list->m_var_defs);
    // Add new var_def
    m_var_defs.push_back(std::move(var_def));
}

auto VarDefList::get_var_defs() -> const Vector&
{
    return m_var_defs;
}

// VarDecl implementation
VarDecl::VarDecl(std::unique_ptr<Location> location,
                 std::unique_ptr<ScalarType> scalar_type,
                 std::unique_ptr<VarDef> var_def,
                 std::unique_ptr<VarDefList> var_def_list)
    : BaseAST{ast_var_decl, std::move(location)},
      m_scalar_type{std::move(scalar_type)},
      m_var_def{std::move(var_def)},
      m_var_def_list{std::move(var_def_list)}
{
}

auto VarDecl::get_scalar_type() const -> const ScalarType&
{
    return *m_scalar_type;
}

auto VarDecl::get_var_def() const -> const VarDef&
{
    return *m_var_def;
}

auto VarDecl::get_var_def_list() const -> const VarDefList&
{
    return *m_var_def_list;
}

}	//namespace toycc

