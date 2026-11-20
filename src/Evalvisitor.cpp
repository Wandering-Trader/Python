#include "Evalvisitor.h"
#include "int2048.h"
#define DEBUG
using LL = sjtu::int2048;
std::any EvalVisitor::visitAtom(Python3Parser::AtomContext *ctx)
{
#ifdef DEBUG
    // std::cout << "visitAtom" << std::endl;
#endif
    if (ctx->NAME() != nullptr)
    {
#ifdef DEBUG
        // std::cout << "name" << std::endl;
#endif
        std::string name = ctx->NAME()->getText();
        int varID = findVariable(name);
        varRef x = {varID, name};
        return x;
    }
    if (ctx->NUMBER() != nullptr)
    {
        std::string val = ctx->NUMBER()->getText();
#ifdef DEBUG
        // std::cout << "number" << std::endl;
        // std::cout << val << std::endl;
#endif
        if(val.find('.') != val.npos)
        {
            double x = stringToDouble(val);
            return x;
        }
        LL x(val);
        return x;
    }
    if (ctx->NONE() != nullptr)
    {
        return NoneType();
    }
    if (ctx->TRUE() != nullptr)
        return true;
    if (ctx->FALSE() != nullptr)
        return false;
    if (ctx->STRING().size())
    {
        auto vStr = ctx->STRING();
        std::string res = "";
        for (auto p : vStr)
        {
            std::string tmp = p->getText();
            res += peel(tmp);
        }
        return res;
    }
    if (ctx->OPEN_PAREN() != nullptr)
    {
        return visitTest(ctx->test());
    }
    if (ctx->format_string() != nullptr)
    {
        /*TODO*/
    }
    return std::any();
}

std::any EvalVisitor::visitAtom_expr(Python3Parser::Atom_exprContext *ctx)
{
    if (ctx->trailer() != nullptr)
    {
        auto para = std::any_cast<std::vector<std::any>>(visitTrailer(ctx->trailer()));
        if(ctx->atom()->getText() == "print")
        {
            for(auto& p : para)
            {
                p = getVal(p);
                Print(p);
                std::cout << " ";
            }
            std::cout << "\n";
            return std::any();
        }
        auto funcName = visitAtom(ctx->atom());
        /*TODO*/
    }
    else
        return visitAtom(ctx->atom());
    return std::any();
}

std::any EvalVisitor::visitTrailer(Python3Parser::TrailerContext *ctx)
{
    if (ctx->arglist() != nullptr)
        return visitArglist(ctx->arglist());
    return std::vector<std::any>();
}

std::any EvalVisitor::visitArglist(Python3Parser::ArglistContext *ctx)
{
    std::vector<std::any> args;
    for (auto arg : ctx->argument())
        args.push_back(visitArgument(arg));
    return args;
}

std::any EvalVisitor::visitArgument(Python3Parser::ArgumentContext *ctx)
{
    auto p = ctx->test();
    if(p.size() == 1)
        return visitTest(p[0]);
    /*TODO*/
    return std::any();
}

std::any EvalVisitor::visitTest(Python3Parser::TestContext *context)
{
#ifdef DEBUG
    auto val = visitOr_test(context->or_test());
    // Print(val);
    // std::cout << std::endl;
    return val;
#endif
    return visitOr_test(context->or_test());
}

std::any EvalVisitor::visitOr_test(Python3Parser::Or_testContext *ctx)
{
    auto vAndTest = ctx->and_test();
    int szAndTest = vAndTest.size();
    if (szAndTest == 1)
        return visitAnd_test(vAndTest[0]);
    for (int i = 0; i < szAndTest; ++i)
    {
        auto val = getVal(visitAnd_test(vAndTest[i]));
        if (toBool(val))
            return true;
    }
    return false;
}

std::any EvalVisitor::visitAnd_test(Python3Parser::And_testContext *ctx)
{
    auto vNotTest = ctx->not_test();
    int szNotTest = vNotTest.size();
    if (szNotTest == 1)
        return visitNot_test(vNotTest[0]);
    for (int i = 0; i < szNotTest; ++i)
    {
        auto Val = getVal(visitNot_test(vNotTest[i]));
        if (!toBool(Val))
            return false;
    }
    return true;
}

std::any EvalVisitor::visitNot_test(Python3Parser::Not_testContext *ctx)
{
    if (ctx->NOT() != nullptr)
        return !toBool(getVal(visitNot_test(ctx->not_test())));
    return visitComparison(ctx->comparison());
}

std::any EvalVisitor::visitComparison(Python3Parser::ComparisonContext *ctx)
{
    auto vCompOp = ctx->comp_op();
    int szOp = vCompOp.size();
    auto vArith = ctx->arith_expr();
    int szArith = vArith.size();
    if (!szOp)
        return visitArith_expr(vArith[0]);
    auto nodeLHS = visitArith_expr(vArith[0]);
    for (int i = 0; i < szOp; ++i)
    {
        auto nodeOp = visitComp_op(vCompOp[i]);
        std::string op = std::any_cast<const std::string &>(nodeOp);
        auto nodeRHS = visitArith_expr(vArith[i + 1]);
        if (!comp(nodeLHS, nodeRHS, op))
            return false;
        nodeLHS = nodeRHS;
    }
    return true;
}

std::any EvalVisitor::visitComp_op(Python3Parser::Comp_opContext *ctx)
{
    return ctx->getText();
}

std::any EvalVisitor::visitArith_expr(Python3Parser::Arith_exprContext *ctx)
{
#ifdef DEBUG
    // std::cout << "visitArith_expr" << std::endl;
#endif
    int szArith = ctx->addorsub_op().size();
    auto vTerm = ctx->term();
    auto vArith = ctx->addorsub_op();
    std::any res = visitTerm(vTerm[0]);
    if (szArith == 0)
        return res;
    bool isDouble = false;
    std::vector<std::any> vArithRes;
    // preprocess the first vertex
    res = getVal(res);
    vArithRes.push_back(res);
    if(res.type() == typeid(double))
        isDouble = true;
    // now process the following ones
    for (int i = 1; i < szArith + 1; ++i)
    {
        auto val = getVal(visitTerm(vTerm[i]));
        vArithRes.push_back(val);
        if(val.type() == typeid(double))
            isDouble = true;
    }
#ifdef DEBUG
    // if(res.type() == typeid(LL))
    //     std::cout << "LL" << std::endl;
    // if(res.type() == typeid(std::any))
    //     std::cout << "any" << std::endl;
    // if(res.type() == typeid(std::string))
    //     std::cout << "string" << std::endl; 
    // if(res.type() == typeid(double))
    //     std::cout << "double" << std::endl; 
#endif
    // string plus
    if (res.type() == typeid(std::string))
    {
        std::string ans = std::any_cast<const std::string&>(res);
        for (int i = 0; i < szArith; ++i)
        {
            std::string op = std::any_cast<const std::string&>(visitAddorsub_op(vArith[i]));
            if (op == "+")
            {
                auto rhs = getVal(visitTerm(vTerm[i+1]));
                if(rhs.type() != typeid(std::string))
                    throw std::runtime_error("string add non-string\n");
                ans += std::any_cast<const std::string&>(rhs);
            }
            else
                throw std::runtime_error("string minus\n");
        }
        return ans;
    }
    // number plus or minus
    if(isDouble)
    {
        double ans = toNumeric<double>(vArithRes[0]);
        for(int i = 0;i < szArith;++i)
        {
            std::string op = std::any_cast<const std::string&>(visitAddorsub_op(vArith[i]));
            if(op == "+")
                ans += toNumeric<double>(vArithRes[i+1]);
            else
                ans -= toNumeric<double>(vArithRes[i+1]);
        }
        return ans;
    }
    LL ans = toNumeric<LL>(vArithRes[0]);
    for(int i = 0;i < szArith;++i)
    {
        std::string op = std::any_cast<const std::string&>(visitAddorsub_op(vArith[i]));
        if(op == "+")
            ans += toNumeric<LL>(vArithRes[i+1]);
        else
            ans -= toNumeric<LL>(vArithRes[i+1]);
    }
    return ans;
}

std::any EvalVisitor::visitAddorsub_op(Python3Parser::Addorsub_opContext *ctx)
{
    return ctx->getText();
}

std::any EvalVisitor::visitTerm(Python3Parser::TermContext *ctx)
{
    auto vTerm = ctx->muldivmod_op();
    int szTerm = vTerm.size();
    auto vFactor = ctx->factor();
    std::any res0 = visitFactor(vFactor[0]);
    if (szTerm == 0)
        return res0;
    bool isDouble = false;
    std::vector<std::any> vFactorRes;
    std::vector<std::string> Op;
    // preprocess res0
    res0 = getVal(res0);
    vFactorRes.push_back(res0);
    if(typeid(res0) == typeid(double))
        isDouble = true;
    // now process the following ones
    for (int i = 1; i < szTerm + 1; ++i)
    {
        auto val = getVal(visitFactor(vFactor[i]));
        vFactorRes.push_back(val);
        if(val.type() == typeid(double))
            isDouble = true;
    }
    std::any res1 = vFactorRes[1];
    if (res0.type() == typeid(std::string) || res1.type() == typeid(std::string))
    {
        if(szTerm > 1)
            throw std::runtime_error("too much terms\n");
        if(res0.type() != typeid(std::string))
            swap(res0,res1);
        std::string op = std::any_cast<std::string>(visitMuldivmod_op(ctx->muldivmod_op()[0]));
        return stringMul(res0,res1,op);
    }
#ifdef DEBUG
    // std::cout << szTerm << " " << isDouble << std::endl;
    for(int i=0;i<szTerm+1;++i)
    {
        Print(vFactorRes[i]);
        std::cout << std::endl;
    }
#endif
    for(int i=0;i<szTerm;++i)
    {
        std::string op = std::any_cast<const std::string&>(visitMuldivmod_op(vTerm[i]));
        Op.push_back(op);
        if(op == "/")
            isDouble = true;
    }
    if(isDouble)
    {
        double ans = toNumeric<double>(vFactorRes[0]);
        for(int i = 0;i < szTerm;++i)
        {
            std::string op = std::any_cast<const std::string&>(visitMuldivmod_op(vTerm[i]));
            if(op == "*")
                ans *= toNumeric<double>(vFactorRes[i+1]);
            else if(op == "/")
                ans /= toNumeric<double>(vFactorRes[i+1]);
            else if(op == "//")
            {
                ans /= toNumeric<double>(vFactorRes[i+1]);
                ans = floor(ans);
            }
            else
            {
                double div = toNumeric<double>(vFactorRes[i+1]);
                double quot = ans / div;
                quot = floor(quot);
                ans = ans - quot * div;
            }
        }
        return ans;
    }
    LL ans = toNumeric<LL>(vFactorRes[0]);
    for(int i = 0;i < szTerm;++i)
    {
        std::string op = Op[i];
        if(op == "*")
            ans *= toNumeric<LL>(vFactorRes[i+1]);
        else if(op == "//")
            ans /= toNumeric<LL>(vFactorRes[i+1]);
        else
            ans %= toNumeric<LL>(vFactorRes[i+1]);
    }
    return ans;
}

std::any EvalVisitor::visitMuldivmod_op(Python3Parser::Muldivmod_opContext *ctx)
{
    return ctx->getText();
}

std::any EvalVisitor::visitFactor(Python3Parser::FactorContext *ctx)
{
    if (ctx->factor() != nullptr)
    {
        std::any res = visitFactor(ctx->factor());
        if (res.type() == typeid(LL))
            return -std::any_cast<const LL &>(res);
        if (res.type() == typeid(double))
            return -std::any_cast<const double &>(res);
        throw std::runtime_error("'-' before non-number\n");
    }
    return visitAtom_expr(ctx->atom_expr());
}

int findVariable(std::string name)
{
    /*TODO*/
    return 0;
}

std::string peel(const std::string str)
{
    if (str.substr(0, 3) == "\"\"\"" || str.substr(0, 3) == "'''")
        return str.substr(3, str.size() - 6);
    return str.substr(1, str.size() - 2);
}

bool toBool(const std::any &val)
{
    if (val.type() == typeid(LL))
        return (std::any_cast<const LL &>(val) != (LL)0);
    if (val.type() == typeid(std::string))
        return (std::any_cast<const std::string &>(val) != "");
    if (val.type() == typeid(bool))
        return std::any_cast<const bool &>(val);
    if (val.type() == typeid(double))
        return fabs(std::any_cast<const double &>(val)) > 1e-6;
    if (val.type() == typeid(EvalVisitor::varRef))
        return toBool(std::any_cast<const EvalVisitor::varRef &>(val).value);
    throw std::runtime_error(std::string("Cannot convert type ") + val.type().name() + " to bool\n");
}

std::any getVal(const std::any &val)
{
    if (val.type() != typeid(EvalVisitor::varRef))
        return val;
    return std::any_cast<const EvalVisitor::varRef &>(val).value;
}

template <typename T>
bool compareSameType(const T &lhs, const std::string &op, const T &rhs)
{
    if (op == "<")
        return lhs < rhs;
    if (op == ">")
        return lhs > rhs;
    if (op == "<=")
        return lhs <= rhs;
    if (op == ">=")
        return lhs >= rhs;
    if (op == "==")
        return lhs == rhs;
    if (op == "!=")
        return lhs != rhs;
    throw std::runtime_error("Unknown comparison operator: " + op);
}

template <typename T>
T toNumeric(const std::any &val)
{
    if (val.type() == typeid(bool))
        return static_cast<T>(std::any_cast<bool>(val));
    if (val.type() == typeid(LL))
        return static_cast<T>(std::any_cast<const LL &>(val));
    if (val.type() == typeid(double))
        return static_cast<T>(std::any_cast<double>(val));
    throw std::runtime_error("Invalid numeric type");
}

bool comp(const std::any &A, const std::any &B, const std::string &op)
{
    std::any x = getVal(A), y = getVal(B);
    if (x.type() != typeid(LL) && x.type() != typeid(double) && x.type() != typeid(std::string) && x.type() != typeid(bool))
        throw std::runtime_error("type x is wrong");
    if (y.type() != typeid(LL) && y.type() != typeid(double) && y.type() != typeid(std::string) && y.type() != typeid(bool))
        throw std::runtime_error("type y is wrong");
    if (x.type() == typeid(std::string) || y.type() == typeid(std::string))
    {
        if (x.type() == typeid(std::string) && y.type() == typeid(std::string))
            return compareSameType(std::any_cast<const std::string &>(x), op, std::any_cast<const std::string &>(y));
        if (op == "==")
            return false;
        if (op == "!=")
            return true;
        throw std::runtime_error("exactly one type is string\n");
    }
    if (x.type() == typeid(double) || y.type() == typeid(double))
    {
        double valX = toNumeric<double>(x), valY = toNumeric<double>(y);
        return compareSameType(valX, op, valY);
    }
    if (x.type() == typeid(LL) || y.type() == typeid(LL))
    {
        LL valX = toNumeric<LL>(x), valY = toNumeric<LL>(y);
        return compareSameType(valX, op, valY);
    }
    return compareSameType(std::any_cast<bool>(x), op, std::any_cast<bool>(y));
}

// val1 is a string and val2 is an integer
std::string stringMul(const std::any &val1, const std::any &val2,const std::string& op)
{
    if(op != "*")
        throw std::runtime_error("string div or mod\n");
    if(val1.type() != typeid(std::string))
        throw std::runtime_error("val1 is not a string\n");
    if(val2.type() != typeid(LL) && val2.type() != typeid(bool))
        throw std::runtime_error("val2 is not an integer\n");
    std::string ans,text = std::any_cast<std::string>(val1);
    LL num = toNumeric<LL>(val2);
    while(num>0)
    {
        ans += text;
        num -= 1;
    }
    return ans;
}

void Print(const std::any& x)
{
    if(x.type() == typeid(LL))
        std::cout << std::any_cast<const LL&>(x);
    else if(x.type() == typeid(std::string))
        std::cout << std::any_cast<const std::string&>(x);
    else if(x.type() == typeid(double))
        std::cout << std::fixed << std::setprecision(6) << std::any_cast<double>(x);
    else if(x.type() == typeid(bool))
        std::cout << (std::any_cast<bool>(x) ? "True" : "False");
    else if(x.type() == typeid(EvalVisitor::NoneType))
        std::cout << "None";
    /*TODO*/
}

double stringToDouble(const std::string& str)
{
    double base = 1.0,ans = 0;
    bool dot = false;
    for(char c:str)
    {
        if(c == '.')
        {
            dot = true;
            continue;
        }
        if(!dot)
            ans = ans * 10 + c - '0';
        else
        {
            base /= 10;
            ans += base * (c - '0');
        }
    }
    return ans;
}