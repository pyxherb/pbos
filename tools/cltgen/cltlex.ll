%option noinput nounput noyywrap nounistd never-interactive

%top {
#include <cltparse.hh>

extern yy::parser::symbol_type yylval;

#define YY_USER_ACTION yylloc.columns(yyleng);
#define YY_DECL yy::parser::symbol_type yylex()
}

%x COMMENT LINE_COMMENT

%%

%{
	yylloc.step();
%}

","			return yy::parser::make_T_COMMA(yylloc);
"?"			return yy::parser::make_OP_TERNARY(yylloc);
":"			return yy::parser::make_T_COLON(yylloc);
";"			return yy::parser::make_T_SEMICOLON(yylloc);
"["			return yy::parser::make_T_LBRACKET(yylloc);
"]"			return yy::parser::make_T_RBRACKET(yylloc);
"{"			return yy::parser::make_T_LBRACE(yylloc);
"}"			return yy::parser::make_T_RBRACE(yylloc);
"("			return yy::parser::make_T_LPARENTHESE(yylloc);
")"			return yy::parser::make_T_RPARENTHESE(yylloc);
"."			return yy::parser::make_T_DOT(yylloc);
"..."		return yy::parser::make_T_VARARG(yylloc);

"="			return yy::parser::make_OP_ASSIGN(yylloc);
"$"			return yy::parser::make_OP_DOLLAR(yylloc);
"#"			return yy::parser::make_OP_DIRECTIVE(yylloc);

"const"		return yy::parser::make_KW_CONST(yylloc);
"subset"	return yy::parser::make_KW_SUBSET(yylloc);

"/*"				BEGIN(COMMENT); yylloc.step();
<COMMENT>"*/"		BEGIN(INITIAL); yylloc.step();
<COMMENT>\n			yylloc.lines(yyleng); yylloc.step();
<COMMENT>.*			yylloc.step();

"//"				BEGIN(LINE_COMMENT); yylloc.step();
<LINE_COMMENT>\n	BEGIN(INITIAL); yylloc.lines(yyleng); yylloc.step();
<LINE_COMMENT>.*	yylloc.step();

[a-zA-Z_][a-zA-Z0-9_]* return yy::parser::make_T_ID(yytext, yylloc);

[-]?[0-9]+ return yy::parser::make_L_INT(strtol(yytext, nullptr, 10), yylloc);
[-]?[0-9]+[uU] return yy::parser::make_L_UINT(strtoul(yytext, nullptr, 10), yylloc);
[-]?[0-9]+[lL] return yy::parser::make_L_LONG(strtoll(yytext, nullptr, 10), yylloc);
[-]?[0-9]+(([uU][lL])|([lL][uU])) return yy::parser::make_L_ULONG(strtoull(yytext, nullptr, 10), yylloc);

-0[xX][0-9a-fA-F]+ return yy::parser::make_L_INT(strtol(yytext, nullptr, 16), yylloc);
0[xX][0-9a-fA-F]+ return yy::parser::make_L_UINT(strtoul(yytext, nullptr, 16), yylloc);
-0[xX][0-9a-fA-F]+[lL] return yy::parser::make_L_LONG(strtoll(yytext, nullptr, 16), yylloc);
0[xX][0-9a-fA-F]+[lL] return yy::parser::make_L_ULONG(strtoull(yytext, nullptr, 16), yylloc);

[\t\r ]+ yylloc.step();
\n+ yylloc.lines(yyleng); yylloc.step();
<<EOF>> return yy::parser::make_YYEOF(yylloc);

%%

yy::parser::symbol_type yylval;
