%require "3.2"

%{
#include <cltparse.hh>
#include "global.h"

yy::parser::symbol_type yylex();

%}

%code requires {
#include <cltparse.hh>
#include <cstdio>
#include <cstdarg>
#include <memory>
}

%code provides {
extern yy::parser::location_type yylloc;
extern std::shared_ptr<yy::parser> yyparser;
}

%locations
%language "c++"
%define api.token.constructor
%define api.value.type variant

%define parse.assert
%define parse.trace
%define parse.error detailed
%define parse.lac full

%token <std::string> T_ID "identifier"
%token T_VARARG "..."

%token <int> L_INT "integer literal"
%token <unsigned int> L_UINT "unsigned integer literal"
%token <long long> L_LONG "long integer literal"
%token <unsigned long long> L_ULONG "unsigned long integer literal"

%token OP_TERNARY "?"
%token OP_ASSIGN "="
%token OP_WRAP "->"
%token OP_DOLLAR "$"
%token OP_DIRECTIVE "#"

%token T_LPARENTHESE "("
%token T_RPARENTHESE ")"
%token T_LBRACKET "["
%token T_RBRACKET "]"
%token T_LBRACE "{"
%token T_RBRACE "}"
%token T_COMMA ","
%token T_COLON ":"
%token T_SEMICOLON ";"
%token T_DOT "."

%token KW_SUBSET "subset"
%token KW_CONST "const"

%expect 0

%%

Stmts: Stmt Stmts | Stmt;
Stmt:
SubsetDef "{" EntryDefs "}" {
	currentSubset.reset();
}
;
SubsetDef:
"subset" T_ID {
	if(globalSubsets.count($2))
		throw syntax_error(@2, "Redefinition of subset `" + $2 + "'");
	currentSubset = std::make_shared<Subset>(0);
	globalSubsets[$2] = currentSubset;
}
| "const" "subset" T_ID {
	if(globalSubsets.count($3))
		throw syntax_error(@3, "Redefinition of subset `" + $3 + "'");
	currentSubset = std::make_shared<Subset>(SUBSET_CONST);
	globalSubsets[$3] = currentSubset;
}
;
EntryDefs:
EntryDef "," EntryDefs
| EntryDef
;
EntryDef:
"[" L_INT "]" "=" T_ID {
	if(currentSubset->entries.count($2))
		throw syntax_error(@2, "Redefinition of entry #" + std::to_string($2));
	currentSubset->entries[$2] = $5;
}
;

%%

yy::parser::location_type yylloc;
std::shared_ptr<yy::parser> yyparser;
