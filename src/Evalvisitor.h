#pragma once
#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H


#include "Python3ParserBaseVisitor.h"


class EvalVisitor : public Python3ParserBaseVisitor {
	struct varRef
	{
		int ID;
		std::string name;
		std::any value;
	};
	struct NoneType
	{
		NoneType(){}
	};
	struct Parameter
	{
		bool isKeyword;
		std::any value;
		std::string name;
	};
	std::unordered_map<std::string,int> ID_variables;
	std::vector<std::any> value_variables;
	std::any visitAtom(Python3Parser::AtomContext *ctx) override;
	std::any visitAtom_expr(Python3Parser::Atom_exprContext *ctx) override;
	std::any visitTrailer(Python3Parser::TrailerContext *ctx) override;
	std::any visitArglist(Python3Parser::ArglistContext *ctx) override;
	std::any visitArgument(Python3Parser::ArgumentContext *ctx) override;
	std::any visitTest(Python3Parser::TestContext *context) override;
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
	friend bool toBool(const std::any&);
	friend std::any getVal(const std::any&);
	friend void Print(const std::any&);
};

int findVariable(std::string name);
std::string peel(std::string str);
bool toBool(const std::any& val);
std::any getVal(const std::any &);
bool comp(const std::any&,const std::any&,const std::string&);
template <typename T>
bool compareSameType(const T&,const std::string&,const T&);
template <typename T>
T toNumeric(const std::any& val);
std::string stringMul(const std::any&,const std::any&,const std::string&);
void Print(const std::any&);
double stringToDouble(const std::string&);
#endif//PYTHON_INTERPRETER_EVALVISITOR_H
