#include "ccs.h"
#include "ccsvisitor.h"
#include <sstream>
#include <algorithm>

using namespace std;
using namespace ccspp;

CCSProcess::CCSProcess(Type type)
    :type(type)
{}

CCSProcess::Type CCSProcess::getType() const
{ return type; }

set<CCSTransition> CCSProcess::getTransitions(CCSProgram& program, bool fold)
{
    set<CCSTransition> res = getTransitions(program, fold, {});
    for(const CCSTransition& t : res)
        if(t.getAction().getInput() != "")
            throw CCSProcessException(t.getTo(), "unrestricted input variable `" + t.getAction().getInput() + "`");
    return move(res);
}

int CCSProcess::compare(CCSProcess& p) const
{
    if(type < p.type)
        return -1;
    else if(type > p.type)
        return 1;
    else
        return compare(&p);
}

bool CCSProcess::operator< (CCSProcess& p) const
{ return compare(p) < 0; }

CCSProcess::operator string() const
{
    stringstream ss;
    print(ss);
    return ss.str();
}

ostream& ccspp::operator<< (ostream& out, const CCSProcess& p)
{
    p.print(out);
    return out;
}



bool PtrCmp::operator() (const shared_ptr<CCSProcess>& p1, const shared_ptr<CCSProcess>& p2) const
{
    return *p1 < *p2;
}



CCSNull::CCSNull()
    :CCSProcess(CCSNULL)
{}

int CCSNull::compare(CCSProcess* p2) const
{
    return 0;
}

set<CCSTransition> CCSNull::getTransitions(CCSProgram& program, bool fold, set<string> seen)
{
    return {};
}

shared_ptr<CCSProcess> CCSNull::subst(string id, int val, bool fold)
{
    return shared_from_this();
}

void CCSNull::print(ostream& out) const
{
    out << "0";
}

void CCSNull::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSTerm::CCSTerm()
    :CCSProcess(TERM)
{}

int CCSTerm::compare(CCSProcess* p2) const
{
    return 0;
}

set<CCSTransition> CCSTerm::getTransitions(CCSProgram& program, bool fold, set<string> seen)
{
    return { CCSTransition(CCSAction(CCSAction::DELTA), shared_from_this(), make_shared<CCSNull>()) };
}

shared_ptr<CCSProcess> CCSTerm::subst(string id, int val, bool fold)
{
    return shared_from_this();
}

void CCSTerm::print(ostream& out) const
{
    out << "1";
}

void CCSTerm::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSProcessName::CCSProcessName(string name, vector<shared_ptr<CCSExp>> args)
    :CCSProcess(PROCESSNAME), name(name), args(args)
{}

string CCSProcessName::getName() const
{ return name; }

vector<shared_ptr<CCSExp>> CCSProcessName::getArgs()
{ return args; }

int CCSProcessName::compare(CCSProcess* p2) const
{
    CCSProcessName* _p2 = (CCSProcessName*)p2;
    if(name < _p2->name)
        return -1;
    else if(name > _p2->name)
        return 1;

    const vector<shared_ptr<CCSExp>>& args2 = _p2->args;;
    for(int i = 0; i < min(args.size(), args2.size()); i++)
    {
        int c = args.at(i)->compare(*args2.at(i));
        if(c != 0)
            return c;
    }

    if(args.size() < args2.size())
        return -1;
    else if(args.size() > args2.size())
        return 1;
    else
        return 0;
}

set<CCSTransition> CCSProcessName::getTransitions(CCSProgram& program, bool fold, set<string> seen)
{
    vector<int> args;
    for(const shared_ptr<CCSExp>& next : this->args)
        args.push_back(next->eval());
    shared_ptr<CCSProcess> p = program.get(name, args, fold);

    if(seen.count(name))
        throw CCSRecursionException(shared_from_this(), "unguarded recursion in process \"" + name + " := " + (string)*p + "\"");
    seen.insert(name);
    if(p)
    {
        set<CCSTransition> res;
        for(CCSTransition t : p->getTransitions(program, fold, seen))
            res.emplace(t.getAction(), shared_from_this(), t.getTo());
        return res;
    }
    else
        return {};
}

shared_ptr<CCSProcess> CCSProcessName::subst(string id, int val, bool fold)
{
    bool changed = false;
    vector<shared_ptr<CCSExp>> args2;
    for(const shared_ptr<CCSExp>& next : args)
        args2.push_back(next->subst(id, val, fold));
    if(args2 == args)
        return shared_from_this();
    else
        return make_shared<CCSProcessName>(name, args2);
}

void CCSProcessName::print(ostream& out) const
{
    out << name;
    if(args.size())
    {
        out << "[";
        bool first = true;
        for(const shared_ptr<CCSExp>& next : args)
        {
            if(!first)
                out << ", ";
            first = false;
            next->print(out);
        }
        out << "]";
    }
}

void CCSProcessName::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSPrefix::CCSPrefix(CCSAction act, shared_ptr<CCSProcess> p)
    :CCSProcess(PREFIX), act(act), p(p)
{}

CCSAction CCSPrefix::getAction() const
{ return act; }

shared_ptr<CCSProcess> CCSPrefix::getProcess() const
{ return p; }

int CCSPrefix::compare(CCSProcess* p2) const
{
    CCSPrefix* _p2 = (CCSPrefix*)p2;
    int c = act.compare(_p2->act);
    if(c != 0)
        return c;
    return p->compare(*_p2->p);
}

set<CCSTransition> CCSPrefix::getTransitions(CCSProgram& program, bool fold, set<string> seen)
{
    shared_ptr<CCSExp> exp = act.getExp();
    if(exp != nullptr && exp->getType() != CCSExp::CONST)
    {
        CCSAction act2(act.getType(), act.getName(), make_shared<CCSConstExp>(exp->eval()));
        return { CCSTransition(act2, shared_from_this(), p) };
    }
    else
        return { CCSTransition(act, shared_from_this(), p) };
}

shared_ptr<CCSProcess> CCSPrefix::subst(string id, int val, bool fold)
{
    if(act.getInput() == id)
        return shared_from_this();
    if(act.getExp() != nullptr)
    {
        CCSAction act2(act.getType(), act.getName(), act.getExp()->subst(id, val, fold));
        shared_ptr<CCSProcess> p2 = p->subst(id, val, fold);
        if(act2 == act && p2 == p)
            return shared_from_this();
        else
            return make_shared<CCSPrefix>(act2, p2);
    }
    else
    {
        shared_ptr<CCSProcess> p2 = p->subst(id, val, fold);
        if(p2 == p)
            return shared_from_this();
        else
            return make_shared<CCSPrefix>(act, p2);
    }
}

void CCSPrefix::print(ostream& out) const
{
    act.print(out);
    out << ".";
    p->print(out);
}

void CCSPrefix::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSChoice::CCSChoice(shared_ptr<CCSProcess> left, shared_ptr<CCSProcess> right)
    :CCSProcess(CHOICE), left(left), right(right)
{}

shared_ptr<CCSProcess> CCSChoice::getLeft() const
{ return left; }

shared_ptr<CCSProcess> CCSChoice::getRight() const
{ return right; }

int CCSChoice::compare(CCSProcess* p2) const
{
    CCSChoice* _p2 = (CCSChoice*)p2;
    int c = left->compare(*_p2->left);
    if(c != 0)
        return c;
    return right->compare(*_p2->right);
}

set<CCSTransition> CCSChoice::getTransitions(CCSProgram& program, bool fold, set<string> seen)
{
    set<CCSTransition> res;
    for(CCSTransition t : left->getTransitions(program, fold, seen))
        res.emplace(t.getAction(), shared_from_this(), t.getTo());
    for(CCSTransition t : right->getTransitions(program, fold, seen))
        res.emplace(t.getAction(), shared_from_this(), t.getTo());
    return res;
}

shared_ptr<CCSProcess> CCSChoice::subst(string id, int val, bool fold)
{
    shared_ptr<CCSProcess> left2 = left->subst(id, val, fold);
    shared_ptr<CCSProcess> right2 = right->subst(id, val, fold);
    if(left2 == left && right2 == right)
        return shared_from_this();
    else
        return make_shared<CCSChoice>(left2, right2);
}

void CCSChoice::print(ostream& out) const
{
    out << "(";
    left->print(out);
    out << " + ";
    right->print(out);
    out << ")";
}

void CCSChoice::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSParallel::CCSParallel(shared_ptr<CCSProcess> left, shared_ptr<CCSProcess> right)
    :CCSProcess(PARALLEL), left(left), right(right)
{}

shared_ptr<CCSProcess> CCSParallel::getLeft() const
{ return left; }

shared_ptr<CCSProcess> CCSParallel::getRight() const
{ return right; }

int CCSParallel::compare(CCSProcess* p2) const
{
    CCSParallel* _p2 = (CCSParallel*)p2;
    int c = left->compare(*_p2->left);
    if(c != 0)
        return c;
    return right->compare(*_p2->right);
}

set<CCSTransition> CCSParallel::getTransitions(CCSProgram& program, bool fold, set<string> seen)
{
    set<CCSTransition> res;
    set<CCSTransition> resl = left->getTransitions(program, fold, seen);
    set<CCSTransition> resr = right->getTransitions(program, fold, seen);

    for(const CCSTransition& t : resl)
    {
        CCSAction act = t.getAction();
        if(act.getType() == CCSAction::DELTA)
            continue;
        res.emplace(act, shared_from_this(), make_shared<CCSParallel>(t.getTo(), right));
    }

    for(const CCSTransition& t : resr)
    {
        CCSAction act = t.getAction();
        if(t.getAction().getType() == CCSAction::DELTA)
            continue;
        res.emplace(act, shared_from_this(), make_shared<CCSParallel>(left, t.getTo()));
    }

    for(const CCSTransition& t : resl)
    {
        CCSAction act = t.getAction();
        if(act.getType() != CCSAction::SEND && act.getType() != CCSAction::RECV)
            continue;
        for(const CCSTransition& t2 : resr)
        {
            CCSAction act2 = t2.getAction();
            if(!(act.getPlain() == ~act2.getPlain()))
                continue;
            CCSAction* send = act.getType() == CCSAction::SEND ? &act : &act2;
            CCSAction* recv = act.getType() == CCSAction::RECV ? &act : &act2;
            if(send->getExp() == nullptr && recv->getInput() == "" && recv->getExp() == nullptr)
                res.emplace(CCSAction(CCSAction::TAU), shared_from_this(),
                    make_shared<CCSParallel>(t.getTo(), t2.getTo()));
            else if(send->getExp() != nullptr && recv->getInput() != "")
                res.emplace(CCSAction(CCSAction::TAU), shared_from_this(),
                    make_shared<CCSParallel>(t.getTo(), t2.getTo()->subst(recv->getInput(), send->getExp()->eval(), fold)));
            else if(send->getExp() != nullptr && recv->getExp() != nullptr)
                if(send->getExp()->eval() == recv->getExp()->eval())
                    res.emplace(CCSAction(CCSAction::TAU), shared_from_this(),
                        make_shared<CCSParallel>(t.getTo(), t2.getTo()));
        }
    }

    for(const CCSTransition& t : resl)
        if(t.getAction().getType() == CCSAction::DELTA)
        {
            for(const CCSTransition& t2 : resr)
                if(t2.getAction().getType() == CCSAction::DELTA)
                {
                    res.emplace(t.getAction(), shared_from_this(), make_shared<CCSParallel>(t.getTo(), t2.getTo()));
                    break;
                }
            break;
        }

    return res;
}

shared_ptr<CCSProcess> CCSParallel::subst(string id, int val, bool fold)
{
    shared_ptr<CCSProcess> left2 = left->subst(id, val, fold);
    shared_ptr<CCSProcess> right2 = right->subst(id, val, fold);
    if(left2 == left && right2 == right)
        return shared_from_this();
    else
        return make_shared<CCSParallel>(left2, right2);
}

void CCSParallel::print(ostream& out) const
{
    out << "(";
    left->print(out);
    out << " | ";
    right->print(out);
    out << ")";
}

void CCSParallel::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSRestrict::CCSRestrict(shared_ptr<CCSProcess> p, set<CCSAction> r, bool complement)
    :CCSProcess(RESTRICT), p(p), r(r), complement(complement)
{}

shared_ptr<CCSProcess> CCSRestrict::getProcess() const
{ return p; }

set<CCSAction> CCSRestrict::getR() const
{ return r; }

bool CCSRestrict::isComplement() const
{ return complement; }

int CCSRestrict::compare(CCSProcess* p2) const
{
    CCSRestrict* _p2 = (CCSRestrict*)p2;
    int c = p->compare(*_p2->p);
    if(c != 0)
        return c;
    else if(complement < _p2->complement)
        return -1;
    else if(complement > _p2->complement)
        return 1;
    else if(r < _p2->r)
        return -1;
    else if(r > _p2->r)
        return 1;
    return 0;
}

set<CCSTransition> CCSRestrict::getTransitions(CCSProgram& program, bool fold, set<string> seen)
{
    set<CCSTransition> res;
    for(const CCSTransition& t : p->getTransitions(program, fold, seen))
    {
        CCSAction act = t.getAction();
        if(act.getType() != CCSAction::TAU && act.getType() != CCSAction::DELTA)
        {
            bool inr = r.count(act.getPlain()) || r.count(act.getNone());
            if(inr != complement)
                continue;
        }
        res.emplace(act, shared_from_this(), make_shared<CCSRestrict>(t.getTo(), r, complement));
    }
    return res;
}

shared_ptr<CCSProcess> CCSRestrict::subst(string id, int val, bool fold)
{
    shared_ptr<CCSProcess> p2 = p->subst(id, val, fold);
    if(p2 == p)
        return shared_from_this();
    else
        return make_shared<CCSRestrict>(p2, r, complement);
}

void CCSRestrict::print(ostream& out) const
{
    out << "(";
    p->print(out);
    out << ")\\{";
    bool first = true;
    if(complement)
    {
        out << "*";
        first = false;
    }
    for(CCSAction act : r)
    {
        if(!first)
            out << ",";
        first = false;
        act.print(out);
    }
    out << "}";
}

void CCSRestrict::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSSequential::CCSSequential(shared_ptr<CCSProcess> left, shared_ptr<CCSProcess> right)
    :CCSProcess(SEQUENTIAL), left(left), right(right)
{}

shared_ptr<CCSProcess> CCSSequential::getLeft() const
{ return left; }

shared_ptr<CCSProcess> CCSSequential::getRight() const
{ return right; }

int CCSSequential::compare(CCSProcess* p2) const
{
    CCSSequential* _p2 = (CCSSequential*)p2;
    int c = left->compare(*_p2->left);
    if(c != 0)
        return c;
    return right->compare(*_p2->right);
}

set<CCSTransition> CCSSequential::getTransitions(CCSProgram& program, bool fold, set<string> seen)
{
    set<CCSTransition> res;
    for(CCSTransition t : left->getTransitions(program, fold, seen))
    {
        if(t.getAction().getType() == CCSAction::DELTA)
            res.emplace(CCSAction(CCSAction::TAU), shared_from_this(), right);
        else
            res.emplace(t.getAction(), shared_from_this(), make_shared<CCSSequential>(t.getTo(), right));
    }
    return res;
}

shared_ptr<CCSProcess> CCSSequential::subst(string id, int val, bool fold)
{
    shared_ptr<CCSProcess> left2 = left->subst(id, val, fold);
    shared_ptr<CCSProcess> right2 = right->subst(id, val, fold);
    if(left2 == left && right2 == right)
        return shared_from_this();
    else
        return make_shared<CCSParallel>(left2, right2);
}

void CCSSequential::print(ostream& out) const
{
    out << "(";
    left->print(out);
    out << "; ";
    right->print(out);
    out << ")";
}

void CCSSequential::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSWhen::CCSWhen(shared_ptr<CCSExp> cond, shared_ptr<CCSProcess> p)
    :CCSProcess(WHEN), cond(cond), p(p)
{}

shared_ptr<CCSExp> CCSWhen::getCond() const
{ return cond; }

shared_ptr<CCSProcess> CCSWhen::getProcess() const
{ return p; }

int CCSWhen::compare(CCSProcess* p2) const
{
    CCSWhen* _p2 = (CCSWhen*)p2;
    int c = cond->compare(*_p2->cond);
    if(c != 0)
        return c;
    return p->compare(*_p2->p);
}

set<CCSTransition> CCSWhen::getTransitions(CCSProgram& program, bool fold, set<string> seen)
{
    if(!cond->eval())
        return {};

    set<CCSTransition> res;
    for(CCSTransition t : p->getTransitions(program, fold, seen))
        res.emplace(t.getAction(), shared_from_this(), t.getTo());
    return res;
}

shared_ptr<CCSProcess> CCSWhen::subst(string id, int val, bool fold)
{
    shared_ptr<CCSExp> cond2 = cond->subst(id, val, fold);
    shared_ptr<CCSProcess> p2 = p->subst(id, val, fold);
    if(cond2 == cond && p2 == p)
        return shared_from_this();
    else
        return make_shared<CCSWhen>(cond2, p2);
}

void CCSWhen::print(ostream& out) const
{
    out << "when ";
    cond->print(out);
    out << " ";
    p->print(out);
}

void CCSWhen::accept(CCSVisitor<void>* v)
{ v->visit(this); }
