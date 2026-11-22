#include "Evalvisitor.h"
#include "int2048.h"
#define DEBUG
using LL = sjtu::int2048;
std::any EvalVisitor::visitAtom(Python3Parser::AtomContext *ctx)
{
    if (ctx->NAME() != nullptr)
    {
        std::string name = ctx->NAME()->getText();
        return findName(name);
    }
    if (ctx->NUMBER() != nullptr)
    {
        std::string val = ctx->NUMBER()->getText();
        if(val.find('.') != val.npos)
        {
            double x = std::stod(val);
            return x;
        }
        LL x(val);
        return x;
    }
    if (ctx->NONE() != nullptr)
        return NoneType();
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
            res += unescape(peel(tmp),false);
        }
        return res;
    }
    if (ctx->OPEN_PAREN() != nullptr)
        return visitTest(ctx->test());
    if (ctx->format_string() != nullptr)
        return visit(ctx->format_string());
    return std::any();
}

std::any EvalVisitor::visitAtom_expr(Python3Parser::Atom_exprContext *ctx)
{
    if (ctx->trailer() != nullptr)
    {
        auto para = std::any_cast<std::vector<std::any>>(visit(ctx->trailer()));
        std::string name = ctx->atom()->getText();
        if(name == "print")
        {
            int sz = para.size();
            for(int i=0;i<sz;++i)
            {
                Print(para[i]);
                if(i != sz - 1)
                    std::cout << " ";
            }
            std::cout << "\n";
            return (Flow){0};
        }
        if(name == "int")
        {
            if(para.empty())
                return (LL)0;
            std::any res = getVal(para[0]);
            if(res.type() == typeid(LL))
                return res;
            if(res.type() == typeid(double))
                return static_cast<LL>(std::any_cast<double>(res));
            if(res.type() == typeid(bool))
                return static_cast<LL>(std::any_cast<bool>(res));
            if(res.type() == typeid(std::string))
                return LL(std::any_cast<const std::string&>(res));
            std::cout << "pass";
        }
        if(name == "float")
        {
            if(para.empty())
                return 0.0;
            std::any res = getVal(para[0]);
            if(res.type() == typeid(LL) || res.type() == typeid(double) || res.type() == typeid(bool))
                return toNumeric<double>(res);
            if(res.type() == typeid(std::string))
                return std::stod(std::any_cast<const std::string&>(res));
            std::cout << "pass";
        }
        if(name == "str")
            return para.empty() ? "" : toString(getVal(para[0]));
        if(name == "bool")
            return para.empty() ? false : toBool(getVal(para[0]));
            
        auto it = mapFunc.find(name);
        if(it == mapFunc.end())
            throw std::runtime_error("function not founded\n");
        Func f = func[it->second];
        // change scope
        if(scopeDepth == 0)
            globalVar = localVar;
        else
            scopeStack.push_back(localVar);
        localVar.clear();
        ++scopeDepth;
        // create new scope
        bool keyword = false;
        int sz = para.size();
        if(sz > f.vArg.size())
            std::cout << "pass";
        std::vector<bool> hasValue(f.vArg.size());
        for(int i=0;i<sz;++i)
        {
            // keyword
            auto& p = para[i];
            if(p.type() == typeid(varRef))
            {
                keyword = true;
                varRef tmpVar = std::any_cast<const varRef&>(p);
                auto it = f.mapArg.find(tmpVar.name);
                if(it == f.mapArg.end())
                    std::cout << "pass";
                int id = it->second;
                if(hasValue[id])
                    std::cout << "pass";
                hasValue[id] = true;
                tmpVar.ID = ++numVar;
                var[numVar] = tmpVar;
                localVar.insert(make_pair(tmpVar.name,numVar));
            }
            // positional
            else
            {
                if(keyword)
                    std::cout << "pass";
                std::string name = f.vArg[i].name;
                ++numVar;
                var[numVar] = {numVar,name,p};
                localVar.insert(make_pair(name,numVar));
                hasValue[i] = true;
            }
        }
        int szArgs = f.vArg.size();
        for(int i=0;i<szArgs;++i)
        {
            if(!hasValue[i])
            {
                if(!f.vArg[i].hasValue)
                    throw std::runtime_error("argument have no value");
                ++numVar;
                var[numVar] = {numVar,f.vArg[i].name,f.vArg[i].value,true};
                localVar.insert(make_pair(f.vArg[i].name,numVar));
            }
        }
        std::any ret = visit(f.suite);
        numVar -= localVar.size();
        if(scopeDepth == 1)
            localVar = globalVar;
        else
        {
            localVar = scopeStack.back();
            scopeStack.pop_back();
        }
        --scopeDepth;
        // process return value
        if(ret.type() == typeid(Flow))
        {
            Flow flow = std::any_cast<const Flow&>(ret);
            if(flow.state == 4)
                return getVal(flow.value);
            return NoneType();
        }
        return NoneType();
    }
    std::any res = visitAtom(ctx->atom());
    return res;
}

std::any EvalVisitor::visitTrailer(Python3Parser::TrailerContext *ctx)
{
    if (ctx->arglist() != nullptr)
        return visit(ctx->arglist());
    return std::vector<std::any>();
}

std::any EvalVisitor::visitArglist(Python3Parser::ArglistContext *ctx)
{
    std::vector<std::any> args;
    for (auto arg : ctx->argument())
        args.push_back(visit(arg));
    return args;
}

std::any EvalVisitor::visitArgument(Python3Parser::ArgumentContext *ctx)
{
    auto p = ctx->test();
    // pure value
    if(p.size() == 1)
        return getVal(visit(p[0]));
    // keyword and value
    varRef var;
    var.name = p[0]->getText();
    var.value = getVal(visit(p[1]));
    var.hasValue = true;
    return var;
}

std::any EvalVisitor::visitTest(Python3Parser::TestContext *context)
{
#ifdef DEBUG
    auto val = visit(context->or_test());
    return val;
#endif
    return visit(context->or_test());
}

std::any EvalVisitor::visitTestlist(Python3Parser::TestlistContext *ctx)
{
    auto vTest = ctx->test();
    std::vector<std::any> resTest;
    for(auto& p:vTest)
        resTest.push_back(visit(p));
    int sz = vTest.size();
    if(sz == 1)
        return resTest[0];
    Tuple t;
    t.size = sz;
    t.vTuple.clear();
    for(auto& p:resTest)
        t.vTuple.push_back(p);
    return t;
}

std::any EvalVisitor::visitAugassign(Python3Parser::AugassignContext *ctx)
{
    return ctx->getText();
}

std::any EvalVisitor::visitOr_test(Python3Parser::Or_testContext *ctx)
{
    auto vAndTest = ctx->and_test();
    int szAndTest = vAndTest.size();
    if (szAndTest == 1)
        return visit(vAndTest[0]);
    for (int i = 0; i < szAndTest; ++i)
    {
        auto val = getVal(visit(vAndTest[i]));
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
        return visit(vNotTest[0]);
    for (int i = 0; i < szNotTest; ++i)
    {
        auto Val = getVal(visit(vNotTest[i]));
        if (!toBool(Val))
            return false;
    }
    return true;
}

std::any EvalVisitor::visitNot_test(Python3Parser::Not_testContext *ctx)
{
    if (ctx->NOT() != nullptr)
        return !toBool(getVal(visit(ctx->not_test())));
    return visit(ctx->comparison());
}

std::any EvalVisitor::visitComparison(Python3Parser::ComparisonContext *ctx)
{
    auto vCompOp = ctx->comp_op();
    int szOp = vCompOp.size();
    auto vArith = ctx->arith_expr();
    int szArith = vArith.size();
    if (!szOp)
        return visit(vArith[0]);
    auto nodeLHS = visit(vArith[0]);
    for (int i = 0; i < szOp; ++i)
    {
        auto nodeOp = visit(vCompOp[i]);
        std::string op = std::any_cast<const std::string &>(nodeOp);
        auto nodeRHS = visit(vArith[i + 1]);
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
    int szArith = ctx->addorsub_op().size();
    auto vTerm = ctx->term();
    auto vArith = ctx->addorsub_op();
    std::any res = visit(vTerm[0]);
    if (szArith == 0)
        return res;
    // preprocess the first vertex
    std::any ans = getVal(res);
    // now process the following ones
    for(int i = 0;i < szArith;++i)
    {
        std::string op = std::any_cast<const std::string&>(visit(vArith[i]));
        ans = calc(ans,getVal(visit(vTerm[i+1])),op);
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
    std::any res0 = visit(vFactor[0]);
    if (szTerm == 0)
        return res0;
    // now process the following ones
    std::any ans = getVal(res0);
    for(int i = 0;i < szTerm;++i)
        ans = calc(ans,getVal(visit(vFactor[i+1])),std::any_cast<const std::string&>(visit(vTerm[i])));
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
        std::any res = visit(ctx->factor());
        res = getVal(res);
        if(ctx->ADD() != nullptr)
        {
            if(res.type() != typeid(bool))
                return res;
            return toNumeric<LL>(res);
        }
        res = getVal(res);
        if (res.type() == typeid(LL) || res.type() == typeid(bool))
            return -toNumeric<LL>(res);
        if (res.type() == typeid(double))
            return -std::any_cast<const double &>(res);
    }
    return visit(ctx->atom_expr());
}

std::any EvalVisitor::visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx)
{
    // "=" 
    if(ctx->augassign() == nullptr)
    {
        auto vTestlist = ctx->testlist();
        int sz = vTestlist.size();
        std::vector<std::any> vTerm;
        if(sz == 1)
            return visit(vTestlist[0]);
        // calculate from right to left
        std::any val = visit(vTestlist.back());
        std::any res;
        val = getVal(val);
        for(int i = sz - 2; ~i; --i)
        {
            res = visit(vTestlist[i]);
            Bind(res,val);
        }
        return res;
    }
    // augassign
    else
    {
        auto vTestlist = ctx->testlist();
        std::any res0 = visit(vTestlist[0]), res1 = visit(vTestlist[1]);
        std::string op = std::any_cast<std::string>(visit(ctx->augassign()));
        op.pop_back();
        std::any res = calc(getVal(res0),getVal(res1),op);
        Bind(res0,res);
    }
    return std::any();
}

std::any EvalVisitor::visitIf_stmt(Python3Parser::If_stmtContext *ctx)
{
    if(ctx->IF() == nullptr)
        return std::any();
    auto vConditions = ctx->test();
    auto vSuites = ctx->suite();
    int sz = vConditions.size();
    for(int i=0;i<sz;++i)
    {
        bool res = toBool(visit(vConditions[i]));
        if(res)
        {
            std::any val = visit(vSuites[i]);
            return val;

        }
    }
    if(ctx->ELSE() != nullptr)
        return visit(vSuites.back());
    return (Flow){0};
}

std::any EvalVisitor::visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx)
{
    if(ctx->if_stmt() != nullptr)
        return visit(ctx->if_stmt());
    if(ctx->while_stmt() != nullptr)
        return visit(ctx->while_stmt());
    if(ctx->funcdef() != nullptr)
        return visit(ctx->funcdef());
    return (Flow){0};
}

std::any EvalVisitor::visitSuite(Python3Parser::SuiteContext *ctx)
{
    if(ctx->simple_stmt() != nullptr)
    {
        std::any val = visit(ctx->simple_stmt());
        return val;
    }
    auto vStmt = ctx->stmt();
    for(auto& p : vStmt)
    {
        std::any ret = visit(p);
        if(ret.type() == typeid(Flow))
        {
            Flow flow = std::any_cast<const Flow&>(ret);
            // if not zero then should quit
            if(flow.state)
                return flow;
        }
    }
    return (Flow){0};
}

std::any EvalVisitor::visitWhile_stmt(Python3Parser::While_stmtContext *ctx)
{
    auto pSuite = ctx->suite();
    auto pTest = ctx->test();
    while(toBool(visit(pTest)))
    {
        std::any ret = visit(pSuite);
        if(ret.type() == typeid(Flow))
        {
            Flow flow = std::any_cast<const Flow&>(ret);
            // break
            if(flow.state == 1)
                break;
            // continue
            if(flow.state == 2)
                continue;
            if(flow.state == 3 || flow.state == 4)
                return flow;
        }
    }
    return (Flow){0};
}

std::any EvalVisitor::visitFormat_string(Python3Parser::Format_stringContext *ctx)
{
    auto vChildren = ctx->children;
    int sz = vChildren.size();
    std::string ans;
    for(int i = 1;i < sz - 1;++i)
    {
        auto p = vChildren[i];
        std::any res = visit(p);
        if(res.type() == typeid(std::string))
        {
            std::string str = std::any_cast<std::string>(res);
            if(str == "{")
            {
                ++i;
                p = vChildren[i];
                std::any resP = visit(p);
                resP = getVal(resP);
                ans += toString(resP);
                ++i;
                continue;
            }
            ans += unescape(str,true);
        }
    }
    return ans;
}

std::any EvalVisitor::visitTerminal(antlr4::tree::TerminalNode* node)
{
    return node->getText();
}

std::any EvalVisitor::visitFuncdef(Python3Parser::FuncdefContext *ctx)
{
    std::string funcName = std::any_cast<const std::string&>(visit(ctx->NAME()));
    if(funcName == "print" || funcName == "str" || funcName == "int" || funcName == "float" || funcName == "bool")
        std::cout << "pass";
    auto it = mapFunc.find(funcName);
    int funcID;
    if(it == mapFunc.end())
    {
        funcID = ++numFunc;
        mapFunc.insert(make_pair(funcName,funcID));
    }
    else
        funcID = it->second;
    auto vArg = std::any_cast<const std::vector<varRef>&>(visit(ctx->parameters()));
    std::unordered_map<std::string,int> mapArg;
    int sz = vArg.size();
    for(int i=0;i<sz;++i)
        mapArg.insert(make_pair(vArg[i].name,i));
    func[funcID] = {numFunc, funcName, mapArg, vArg, ctx->suite()};
    return std::any();
}

std::any EvalVisitor::visitParameters(Python3Parser::ParametersContext *ctx)
{
    if(ctx->typedargslist() == nullptr)
        return std::vector<varRef>();
    return visit(ctx->typedargslist());
}

std::any EvalVisitor::visitTypedargslist(Python3Parser::TypedargslistContext *ctx)
{
    std::vector<varRef> vArgs;
    auto vChildren = ctx->children;
    int sz = vChildren.size();
    varRef var;
    for(int i=0;i<sz;++i)
    {
        std::any res = visit(vChildren[i]);
        std::string name = std::any_cast<const std::string&>(res);
        ++i;
        var.name = name; 
        var.hasValue = false;
        if(i==sz)
            vArgs.push_back(var);
        else
        {
            std::string op = std::any_cast<const std::string&>(visit(vChildren[i]));
            if(op == "=")
            {
                ++i;
                std::any val = getVal(visit(vChildren[i]));
                var.value = val;
                var.hasValue = true;
                ++i;
            }
            vArgs.push_back(var);
        }
    }
    return vArgs;
}

std::any EvalVisitor::visitTfpdef(Python3Parser::TfpdefContext *ctx)
{
    return ctx->getText();
}

std::any EvalVisitor::visitBreak_stmt(Python3Parser::Break_stmtContext *ctx)
{
    Flow ret = {1};
    return ret;
}

std::any EvalVisitor::visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx)
{
    Flow ret = {2};
    return ret;
}

std::any EvalVisitor::visitReturn_stmt(Python3Parser::Return_stmtContext *ctx)
{
    if(ctx->testlist() == nullptr)
        return (Flow){3};
    std::any x = visit(ctx->testlist());
    return (Flow){4,x};
}

std::any EvalVisitor::visitStmt(Python3Parser::StmtContext *ctx)
{
    if(ctx->simple_stmt() != nullptr)
        return visit(ctx->simple_stmt());
    if(ctx->compound_stmt() != nullptr)
        return visit(ctx->compound_stmt());
    return std::any();
}

std::any EvalVisitor::visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx)
{
    if(ctx->small_stmt() != nullptr)
        return visit(ctx->small_stmt());
    return std::any();
}

std::any EvalVisitor::visitSmall_stmt(Python3Parser::Small_stmtContext *ctx)
{
    if(ctx->expr_stmt() != nullptr)
        return visit(ctx->expr_stmt());
    if(ctx->flow_stmt() != nullptr)
        return visit(ctx->flow_stmt());
    return std::any();
}

std::any EvalVisitor::visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx)
{
    if(ctx->return_stmt() != nullptr)
        return visit(ctx->return_stmt());
    if(ctx->break_stmt() != nullptr)
        return visit(ctx->break_stmt());
    if(ctx->continue_stmt() != nullptr)
        return visit(ctx->continue_stmt());
    return std::any();
}

std::any EvalVisitor::findName(const std::string &name)
{
    auto it = localVar.find(name);
    if(it != localVar.end())
        return var[it->second];
    it = globalVar.find(name);
    if(it != globalVar.end())
        return var[it->second];
    ++numVar;
    var[numVar] = {numVar, name, std::any(),true};
    localVar.insert(make_pair(name,numVar));
    return var[numVar];
}

void EvalVisitor::Bind(std::any& lhs, const std::any& rhs)
{
    if(lhs.type() == typeid(Tuple))
    {
        if(rhs.type() != typeid(Tuple))
            return;
        Tuple L = std::any_cast<const Tuple&>(lhs), R = std::any_cast<const Tuple&>(rhs);
        int szL = L.vTuple.size(), szR = R.vTuple.size();
        if(szL != szR)
            std::cout << "pass";
        for(int i=0;i<szL;++i)
            Bind(L.vTuple[i],R.vTuple[i]);
    }
    else if(lhs.type() == typeid(varRef))
    {
        int ID = std::any_cast<const varRef&>(lhs).ID;
        var[ID].value = rhs;
        var[ID].hasValue = true;
    }
        
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
        return fabs(std::any_cast<const double &>(val)) != 0.0;
    if (val.type() == typeid(EvalVisitor::varRef))
        return toBool(std::any_cast<const EvalVisitor::varRef &>(val).value);
    if (val.type() == typeid(EvalVisitor::NoneType))
        return false;
    std::cout << "pass";
}

std::any getVal(const std::any &val)
{
    if (val.type() == typeid(EvalVisitor::varRef))
        return std::any_cast<const EvalVisitor::varRef &>(val).value;
    if(val.type() == typeid(EvalVisitor::Tuple))
    {
        EvalVisitor::Tuple res = std::any_cast<const EvalVisitor::Tuple&>(val);
        for(auto& p: res.vTuple)
            p = getVal(p);
        return res;
    }
    return val;
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
    std::cout << "pass";
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
    std::cout << "pass";
}

bool comp(const std::any &A, const std::any &B, const std::string &op)
{
    std::any x = getVal(A), y = getVal(B);
    if (x.type() != typeid(LL) && x.type() != typeid(double) && x.type() != typeid(std::string) && x.type() != typeid(bool) && x.type() != typeid(EvalVisitor::NoneType))
        std::cout << "pass";
    if (y.type() != typeid(LL) && y.type() != typeid(double) && y.type() != typeid(std::string) && y.type() != typeid(bool) && y.type() != typeid(EvalVisitor::NoneType))
        std::cout << "pass";
    if (x.type() == typeid(std::string) || y.type() == typeid(std::string))
    {
        if (x.type() == typeid(std::string) && y.type() == typeid(std::string))
            return compareSameType(std::any_cast<const std::string &>(x), op, std::any_cast<const std::string &>(y));
        if (op == "==")
            return false;
        if (op == "!=")
            return true;
        std::cout << "pass";
    }
    bool xNone = x.type() == typeid(EvalVisitor::NoneType), yNone = y.type() == typeid(EvalVisitor::NoneType);
    if(xNone || yNone)
    {
        if(op == "==")
            return xNone && yNone;
        if(op == "!=")
            return xNone != yNone;
        std::cout << "pass";
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
std::string stringMul(const std::any &val1, const std::any &val2)
{
    if(val1.type() != typeid(std::string))
        std::cout << "pass";
    if(val2.type() != typeid(LL) && val2.type() != typeid(bool))
        std::cout << "pass";
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
    else if(x.type() == typeid(EvalVisitor::varRef))
        Print(getVal(x));
    /*TODO*/
}

std::any calc(const std::any& A,const std::any& B,const std::string& op)
{
    if(A.type() == typeid(EvalVisitor::Tuple) || B.type() == typeid(EvalVisitor::Tuple))
    {
        // if(A.type() != typeid(EvalVisitor::Tuple) || B.type() != typeid(EvalVisitor::Tuple))
        //     return std::any();
        EvalVisitor::Tuple x = std::any_cast<const EvalVisitor::Tuple&>(A),y = std::any_cast<const EvalVisitor::Tuple&>(B);
        if(x.size != y.size)
            std::cout << "pass";
        EvalVisitor::Tuple res;
        res.size = x.size;
        res.vTuple.clear();
        for(int i=0;i<res.size;++i)
            res.vTuple.push_back(calc(x.vTuple[i],y.vTuple[i],op));
        return res;
    }
    if(op == "+")
    {
        if(A.type() == typeid(std::string))
        {
            if(B.type() != typeid(std::string))
                std::cout << "pass";
            return std::any_cast<const std::string&>(A) + std::any_cast<const std::string&>(B);
        }
        if(A.type() == typeid(bool) || A.type() == typeid(LL) || A.type() == typeid(double))
        {
            if(B.type() != typeid(bool) && B.type() != typeid(LL) && B.type() != typeid(double))
                std::cout << "pass";
            if(A.type() == typeid(double) || B.type() == typeid(double))
                return toNumeric<double>(A) + toNumeric<double>(B);
            return toNumeric<LL>(A) + toNumeric<LL>(B);
        }
        std::cout << "pass";
    }
    if(op == "-")
    {
        if(A.type() == typeid(bool) || A.type() == typeid(LL) || A.type() == typeid(double))
        {
            if(B.type() != typeid(bool) && B.type() != typeid(LL) && B.type() != typeid(double))
                std::cout << "pass";
            if(A.type() == typeid(double) || B.type() == typeid(double))
                return toNumeric<double>(A) - toNumeric<double>(B);
            return toNumeric<LL>(A) - toNumeric<LL>(B);
        }
        std::cout << "pass";
    }
    if(op == "*")
    {
        if(A.type() == typeid(std::string))
            return stringMul(A,B);
        if(B.type() == typeid(std::string))
            return stringMul(B,A);
        if(A.type() == typeid(bool) || A.type() == typeid(LL) || A.type() == typeid(double))
        {
            if(B.type() != typeid(bool) && B.type() != typeid(LL) && B.type() != typeid(double))
                std::cout << "pass";
            if(A.type() == typeid(double) || B.type() == typeid(double))
                return toNumeric<double>(A) * toNumeric<double>(B);
            return toNumeric<LL>(A) * toNumeric<LL>(B);
        }
        std::cout << "pass";
    }
    if(op == "/")
    {
        if(A.type() == typeid(bool) || A.type() == typeid(LL) || A.type() == typeid(double))
        {
            if(B.type() != typeid(bool) && B.type() != typeid(LL) && B.type() != typeid(double))
                std::cout << "pass";
            return toNumeric<double>(A) / toNumeric<double>(B);
        }
        std::cout << "pass";
    }
    if(op == "//")
    {
        if(A.type() == typeid(bool) || A.type() == typeid(LL) || A.type() == typeid(double))
        {
            if(B.type() != typeid(bool) && B.type() != typeid(LL) && B.type() != typeid(double))
                std::cout << "pass";
            if(A.type() == typeid(double) || B.type() == typeid(double))
            {
                double ans = toNumeric<double>(A) / toNumeric<double>(B);
                return floor(ans);
            }
            return toNumeric<LL>(A) / toNumeric<LL>(B);
        }
        std::cout << "pass";
    }
    if(op == "%")
    {
        if(A.type() == typeid(bool) || A.type() == typeid(LL) || A.type() == typeid(double))
        {
            if(B.type() != typeid(bool) && B.type() != typeid(LL) && B.type() != typeid(double))
                std::cout << "pass";
            if(A.type() == typeid(double) || B.type() == typeid(double))
            {
                double doubleA = toNumeric<double>(A), doubleB = toNumeric<double>(B);
                double quot = floor(doubleA / doubleB);
                return doubleA - quot * doubleB;
            }
            return toNumeric<LL>(A) % toNumeric<LL>(B);
        }
        std::cout << "pass";
    }
    std::cout << "pass";
}

std::string unescape(const std::string& str,bool fString)
{
    std::string res;
    int len = str.length();
    for(int i=0;i<len;++i)
    {
        if(str[i] == '\\')
        {
            ++i;
            if(str[i] == '\\')
                res.push_back('\\');
            else if(str[i] == 'n')
                res.push_back('\n');
            else if(str[i] == 't')
                res.push_back('\t');
            else if(str[i] == 'r')
                res.push_back('\r');
            // including \", \'
            else
                res.push_back(str[i]);
        }
        else if(str[i] == '{' && fString)
        {
            res.push_back('{');
            ++i;
        }
        else if(str[i] == '}' && fString)
        {
            res.push_back('}');
            ++i;
        }
        else 
            res.push_back(str[i]);
    }
    return res;
}

std::string toString(const std::any& x)
{
    if(x.type() == typeid(bool))
    {
        if(std::any_cast<bool>(x))
            return "True";
        return "False";
    }
    if(x.type() == typeid(std::string))
        return std::any_cast<const std::string&>(x);
    if(x.type() == typeid(EvalVisitor::NoneType))
        return "None";
    if(x.type() == typeid(LL))
    {
        LL val = std::any_cast<const LL&>(x);
        std::stringstream ss;
        ss << val;
        return ss.str();
    }
    if(x.type() == typeid(double))
    {
        double val = std::any_cast<double>(x);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(6) << val;
        return ss.str();
    }
    return std::string();
}
