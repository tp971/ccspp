#include "ccs.h"
#include "ccsvisitor.h"
#include <sstream>

using namespace std;
using namespace ccspp;

CCSExp::CCSExp(Type type)
    :type(type)
{}

CCSExp::Type CCSExp::getType() const
{ return type; }

int CCSExp::compare(CCSExp& p) const
{
    if(type < p.type)
        return -1;
    else if(type > p.type)
        return 1;
    else
        return compare(&p);
}

bool CCSExp::operator< (CCSExp& p) const
{ return compare(p) < 0; }

CCSExp::operator string() const
{
    stringstream ss;
    print(ss);
    return ss.str();
}

ostream& ccspp::operator<< (ostream& out, const CCSExp& p)
{
    p.print(out);
    return out;
}



CCSConstExp::CCSConstExp(int val)
    :CCSExp(CONST), val(val)
{}

int CCSConstExp::getVal() const
{ return val; }

int CCSConstExp::compare(CCSExp* e) const
{
    CCSConstExp* _e = (CCSConstExp*)e;
    if(val < _e->val)
        return -1;
    else if(val == _e->val)
        return 0;
    else
        return 1;
}

shared_ptr<CCSExp> CCSConstExp::subst(string id, int val, bool fold)
{
    return shared_from_this();
}

int CCSConstExp::eval()
{
    return val;
}

void CCSConstExp::print(ostream& out) const
{
    out << val;
}

void CCSConstExp::accept(CCSExpVisitor<void>* v)
{ v->visit(this); }



CCSIdExp::CCSIdExp(string id)
    :CCSExp(ID), id(id)
{}

string CCSIdExp::getId() const
{ return id; }

int CCSIdExp::compare(CCSExp* e) const
{
    CCSIdExp* _e = (CCSIdExp*)e;
    if(id < _e->id)
        return -1;
    else if(id == _e->id)
        return 0;
    else
        return 1;
}

shared_ptr<CCSExp> CCSIdExp::subst(string id, int val, bool fold)
{
    if(this->id == id)
        return make_shared<CCSConstExp>(val);
    else
        return shared_from_this();
}

int CCSIdExp::eval()
{
    if(id == "true")
        return 1;
    else if(id == "false")
        return 0;
    else
        throw CCSUnboundException(shared_from_this(), id, "unbound identifier: " + id);
}

void CCSIdExp::print(ostream& out) const
{
    out << id;
}

void CCSIdExp::accept(CCSExpVisitor<void>* v)
{ v->visit(this); }



CCSUnaryExp::CCSUnaryExp(Op op, shared_ptr<CCSExp> exp)
    :CCSExp(UNARY), op(op), exp(exp)
{}

CCSUnaryExp::Op CCSUnaryExp::getOp() const
{ return op; }

shared_ptr<CCSExp> CCSUnaryExp::getExp() const
{ return exp; }

int CCSUnaryExp::compare(CCSExp* e) const
{
    CCSUnaryExp* _e = (CCSUnaryExp*)e;
    if(op < _e->op)
        return -1;
    else if(op > _e->op)
        return 1;
    return exp->compare(*_e->exp);
}

shared_ptr<CCSExp> CCSUnaryExp::subst(string id, int val, bool fold)
{
    shared_ptr<CCSExp> exp2 = exp->subst(id, val, fold);
    if(fold && exp2->getType() == CCSExp::CONST)
        return make_shared<CCSConstExp>(eval(exp2->eval()));
    if(exp2 == exp)
        return shared_from_this();
    else
        return make_shared<CCSUnaryExp>(op, exp2);
}

int CCSUnaryExp::eval(int val)
{
    switch(op)
    {
    case PLUS: return val;
    case MINUS: return -val;
    case NOT: return !val;
    }
    throw CCSExpException(shared_from_this(), "this should not happen");
}

int CCSUnaryExp::eval()
{
    int val;
    try
    {
        val = exp->eval();
    }
    catch(CCSExpException& e)
    {
        e.setExp(shared_from_this());
        throw;
    }
    return eval(val);
}

void CCSUnaryExp::print(ostream& out) const
{
    out << "(";
    switch(op)
    {
    case PLUS: out << "+"; break;
    case MINUS: out << "-"; break;
    case NOT: out << "!"; break;
    }
    exp->print(out);
    out << ")";
}

void CCSUnaryExp::accept(CCSExpVisitor<void>* v)
{ v->visit(this); }



CCSBinaryExp::CCSBinaryExp(Op op, shared_ptr<CCSExp> lhs, shared_ptr<CCSExp> rhs)
    :CCSExp(BINARY), op(op), lhs(lhs), rhs(rhs)
{}

CCSBinaryExp::Op CCSBinaryExp::getOp() const
{ return op; }

shared_ptr<CCSExp> CCSBinaryExp::getLhs() const
{ return lhs; }

shared_ptr<CCSExp> CCSBinaryExp::getRhs() const
{ return rhs; }

int CCSBinaryExp::compare(CCSExp* e) const
{
    CCSBinaryExp* _e = (CCSBinaryExp*)e;
    if(op < _e->op)
        return -1;
    else if(op > _e->op)
        return 1;
    int c = lhs->compare(*_e->lhs);
    if(c != 0)
        return c;
    return rhs->compare(*_e->rhs);
}


shared_ptr<CCSExp> CCSBinaryExp::subst(string id, int val, bool fold)
{
    shared_ptr<CCSExp> lhs2 = lhs->subst(id, val, fold);
    shared_ptr<CCSExp> rhs2 = rhs->subst(id, val, fold);
    if(fold && lhs2->getType() == CCSExp::CONST && rhs2->getType() == CCSExp::CONST)
        return make_shared<CCSConstExp>(eval(lhs2->eval(), rhs2->eval()));
    if(lhs2 == lhs && rhs2 == rhs)
        return shared_from_this();
    else
        return make_shared<CCSBinaryExp>(op, lhs2, rhs2);
}

int CCSBinaryExp::eval(int lval, int rval)
{
    switch(op)
    {
    case PLUS: return lval + rval;
    case MINUS: return lval - rval;
    case MUL: return lval * rval;
    case DIV:
        if(rval == 0)
            throw CCSUndefinedException(shared_from_this(), "division by zero");
        return lval / rval;
    case MOD:
        if(rval == 0)
            throw CCSUndefinedException(shared_from_this(), "division by zero");
        return lval % rval;
    case OR: return lval || rval;
    case AND: return lval && rval;
    case EQ: return lval == rval;
    case NEQ: return lval != rval;
    case LT: return lval < rval;
    case LEQ: return lval <= rval;
    case GT: return lval > rval;
    case GEQ: return lval >= rval;
    }
    throw CCSExpException(shared_from_this(), "this should not happen");
}

int CCSBinaryExp::eval()
{
    int lval;
    int rval;
    try
    {
        lval = lhs->eval();
        rval = rhs->eval();
    }
    catch(CCSExpException& e)
    {
        e.setExp(shared_from_this());
        throw;
    }
    return eval(lval, rval);
}

void CCSBinaryExp::print(ostream& out) const
{
    out << "(";
    lhs->print(out);
    switch(op)
    {
    case PLUS: out << " + "; break;
    case MINUS: out << " - "; break;
    case MUL: out << " * "; break;
    case DIV: out << " / "; break;
    case MOD: out << " % "; break;
    case OR: out << " | "; break;
    case AND: out << " && "; break;
    case EQ: out << " == "; break;
    case NEQ: out << " != "; break;
    case LT: out << " < "; break;
    case LEQ: out << " <= "; break;
    case GT: out << " > "; break;
    case GEQ: out << " >= "; break;
    }
    rhs->print(out);
    out << ")";
}

void CCSBinaryExp::accept(CCSExpVisitor<void>* v)
{ v->visit(this); }
