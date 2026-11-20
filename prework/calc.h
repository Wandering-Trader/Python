// 你需要提交这份代码
#pragma once

#include "visitor.h"

struct calculator : visitor {
    std::any visit_num(num_node *p) override 
    {
        return p->number;
    }
    std::any visit_add(add_node *p) override 
    {
        std::any A = visit(p->lnode), B = visit(p->rnode);
        if(A.type() == B.type())
        {
            if(A.type() == typeid(double))
            {
                double x = std::any_cast<double>(A),y = std::any_cast<double>(B);
                std::any sum = x+y;
                return sum;
            }
            if(A.type() == typeid(long long))
            {
                long long x = std::any_cast<long long>(A),y = std::any_cast<long long>(B);
                std::any sum = x+y;
                return sum;
            }
        }
        return -114514;
    }
    std::any visit_sub(sub_node *p) override 
    {
        std::any A = visit(p->lnode), B = visit(p->rnode);
        if(A.type() == B.type())
        {
            if(A.type() == typeid(double))
            {
                double x = std::any_cast<double>(A),y = std::any_cast<double>(B);
                std::any sum = x-y;
                return sum;
            }
            if(A.type() == typeid(long long))
            {
                long long x = std::any_cast<long long>(A),y = std::any_cast<long long>(B);
                std::any sum = x-y;
                return sum;
            }
        }
        return -114514;
    }
    std::any visit_mul(mul_node *p) override 
    {
        std::any A = visit(p->lnode), B = visit(p->rnode);
        if(A.type() == B.type())
        {
            if(A.type() == typeid(double))
            {
                double x = std::any_cast<double>(A),y = std::any_cast<double>(B);
                std::any sum = x*y;
                return sum;
            }
            if(A.type() == typeid(long long))
            {
                long long x = std::any_cast<long long>(A),y = std::any_cast<long long>(B);
                std::any sum = x*y;
                return sum;
            }
        }
        return -114514;
    }
    std::any visit_div(div_node *p) override 
    {
        std::any A = visit(p->lnode), B = visit(p->rnode);
        if(A.type() == B.type())
        {
            if(A.type() == typeid(double))
            {
                double x = std::any_cast<double>(A),y = std::any_cast<double>(B);
                std::any sum = x/y;
                return sum;
            }
            if(A.type() == typeid(long long))
            {
                long long x = std::any_cast<long long>(A),y = std::any_cast<long long>(B);
                std::any sum = x/y;
                return sum;
            }
        }
        return -114514;
    }
    ~calculator() override = default;
};
