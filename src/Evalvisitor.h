#pragma once
#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H


#include "Python3ParserBaseVisitor.h"


class EvalVisitor : public Python3ParserBaseVisitor {
	static const int MAXVAR = 2e4+5;
	struct varRef
	{
		int ID;
		std::string name;
		std::any value;
		bool hasValue;
	} var[MAXVAR];
	struct NoneType{ NoneType(){} };
	struct Tuple 
	{
		int size;
		std::vector<std::any> vTuple;
	};
	struct Func
	{
		int ID;
		std::string name;
		std::unordered_map<std::string,int> mapArg;
		std::vector<varRef> vArg;
		Python3Parser::SuiteContext *suite;
	} func[MAXVAR];
	struct Flow
	{
		/*
		0: nothing happens
		1: break
		2: continue
		3: return (without return value)
		4: return (with value)
		*/
		int state;
		std::any value;
	};

	std::unordered_map<std::string,int> globalVar,localVar,mapFunc;
	std::vector<std::unordered_map<std::string,int>> scopeStack;
	int scopeDepth = 0;
	int numVar = 0;
	int numFunc = 0;

	std::any visitAtom(Python3Parser::AtomContext *ctx) override;
	std::any visitAtom_expr(Python3Parser::Atom_exprContext *ctx) override;
	std::any visitTrailer(Python3Parser::TrailerContext *ctx) override;
	std::any visitArglist(Python3Parser::ArglistContext *ctx) override;
	std::any visitArgument(Python3Parser::ArgumentContext *ctx) override;
	std::any visitTest(Python3Parser::TestContext *context) override;
	std::any visitTestlist(Python3Parser::TestlistContext *ctx) override;
	std::any visitAugassign(Python3Parser::AugassignContext *ctx) override;
	std::any visitOr_test(Python3Parser::Or_testContext *ctx) override;
	std::any visitAnd_test(Python3Parser::And_testContext *ctx) override;
	std::any visitNot_test(Python3Parser::Not_testContext *ctx) override;
	std::any visitComparison(Python3Parser::ComparisonContext *ctx) override;
	std::any visitComp_op(Python3Parser::Comp_opContext *ctx) override;
	std::any visitArith_expr(Python3Parser::Arith_exprContext *ctx) override;
	std::any visitAddorsub_op(Python3Parser::Addorsub_opContext *ctx) override;
	std::any visitTerm(Python3Parser::TermContext *ctx) override;
	std::any visitMuldivmod_op(Python3Parser::Muldivmod_opContext *ctx) override;
	std::any visitFactor(Python3Parser::FactorContext *ctx) override;
	std::any visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) override;
	std::any visitIf_stmt(Python3Parser::If_stmtContext *ctx) override;
	std::any visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) override;
	std::any visitSuite(Python3Parser::SuiteContext *ctx) override;
	std::any visitWhile_stmt(Python3Parser::While_stmtContext *ctx) override;
	std::any visitFormat_string(Python3Parser::Format_stringContext *ctx) override;
	std::any visitTerminal(antlr4::tree::TerminalNode*) override;
	std::any visitFuncdef(Python3Parser::FuncdefContext *ctx) override;
	std::any visitParameters(Python3Parser::ParametersContext *ctx) override;
	std::any visitTypedargslist(Python3Parser::TypedargslistContext *ctx) override;
	std::any visitTfpdef(Python3Parser::TfpdefContext *ctx) override;
	std::any visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) override;
	std::any visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) override;
	std::any visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) override;
	std::any visitStmt(Python3Parser::StmtContext *ctx) override;
	std::any visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) override;
	std::any visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) override;
	std::any visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) override;
	std::any findName(const std::string&);
	void Bind(std::any&,const std::any&);
	friend bool toBool(const std::any&);
	friend std::any getVal(const std::any&);
	friend void Print(const std::any&);
	friend std::string toString(const std::any&);
	friend bool comp(const std::any&,const std::any&,const std::string&);
	friend std::any calc(const std::any&,const std::any&,const std::string&);
};
std::string peel(std::string);
bool toBool(const std::any&);
std::any getVal(const std::any &);
bool comp(const std::any&,const std::any&,const std::string&);
template <typename T>
bool compareSameType(const T&,const std::string&,const T&);
template <typename T>
T toNumeric(const std::any& val);
std::string stringMul(const std::any&,const std::any&);
void Print(const std::any&);
std::any calc(const std::any&,const std::any&,const std::string&);
std::string unescape(const std::string&,bool);
std::string toString(const std::any&);
#endif//PYTHON_INTERPRETER_EVALVISITOR_H
