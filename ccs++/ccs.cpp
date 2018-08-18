#include "ccs.h"
#include "ccsvisitor.h"
#include <sstream>

using namespace std;
using namespace ccspp;

CCSAction::CCSAction(Type type, string name)
    :type(type), name(name)
{}

string CCSAction::getName() const
{ return name; }

CCSAction::Type CCSAction::getType() const
{ return type; }

void CCSAction::print(ostream& out) const
{
    switch(type)
    {
    case TAU:
        out << "i";
        break;
    case DELTA:
        out << "e";
        break;
    case SEND:
        out << name << "!";
        break;
    case RECV:
        out << name << "?";
        break;
    case NONE:
        out << name;
        break;
    }
}

ostream& ccspp::operator<< (ostream& out, const CCSAction& act)
{
    act.print(out);
    return out;
}

CCSAction::operator string() const
{
    stringstream ss;
    print(ss);
    return ss.str();
}

CCSAction CCSAction::operator~ () const
{
    Type t2 = type;
    if(t2 == SEND)
        t2 = RECV;
    else if(t2 == RECV)
        t2 = SEND;
    return CCSAction(t2, name);
}

int CCSAction::compare(const CCSAction& act) const
{
    if(type < act.type)
        return -1;
    else if(type > act.type)
        return 1;
    else if(name < act.name)
        return -1;
    else if(name > act.name)
        return 1;
    else
        return 0;
}

bool CCSAction::operator< (const CCSAction& act) const
{ return compare(act) < 0; }

bool CCSAction::operator== (const CCSAction& act) const
{
    return type == act.type && name == act.name;
}



CCSTransition::CCSTransition(CCSAction act, shared_ptr<CCSProcess> from, shared_ptr<CCSProcess> to)
    :act(act), from(from), to(to)
{}

void CCSTransition::print(std::ostream& out) const
{
    from->print(out);
    out << "   --( ";
    act.print(out);
    out << " )->   ";
    to->print(out);
}

ostream& ccspp::operator<< (ostream& out, const CCSTransition& t)
{
    t.print(out);
    return out;
}

CCSAction CCSTransition::getAction() const
{ return act; }

shared_ptr<CCSProcess> CCSTransition::getFrom() const
{ return from; }

shared_ptr<CCSProcess> CCSTransition::getTo() const
{ return to; }

int CCSTransition::compare(const CCSTransition& t) const
{
    int c = act.compare(t.act);
    if(c != 0)
        return c;
    c = from->compare(*t.from);
    if(c != 0)
        return c;
    return to->compare(*t.to);
}

bool CCSTransition::operator< (const CCSTransition& t) const
{ return compare(t) < 0; }



CCSBinding::CCSBinding()
{}

CCSBinding::CCSBinding(string name, shared_ptr<CCSProcess> process)
    :name(name), process(process)
{}

string CCSBinding::getName() const
{ return name; }

shared_ptr<CCSProcess> CCSBinding::getProcess() const
{ return process; }

void CCSBinding::print(ostream& out) const
{
    out << name << " := ";
    process->print(out);
    out << endl;
}

std::ostream& ccspp::operator<< (std::ostream& out, const CCSBinding& b)
{
    b.print(out);
    return out;
}



void CCSProgram::addBinding(string name, shared_ptr<CCSProcess> process)
{ bindings[name] = CCSBinding(name, process); }

void CCSProgram::setProcess(shared_ptr<CCSProcess> process)
{ this->process = process; }

shared_ptr<CCSProcess> CCSProgram::get(string name) const
{
    if(bindings.count(name))
        return bindings.at(name).getProcess();
    else
        return nullptr;
}

map<string, CCSBinding> CCSProgram::getBindings() const
{ return bindings; }

shared_ptr<CCSProcess> CCSProgram::getProcess() const
{ return process; }

void CCSProgram::print(ostream& out) const
{
    //does not work in gcc 6.3.0 :(
    /*for(auto [name, binding] : bindings)
    {
        binding.print(out);
        out << endl;
    }*/
    for(auto next : bindings)
        next.second.print(out);
    process->print(out);
    out << endl;
}

std::ostream& ccspp::operator<< (std::ostream& out, const CCSProgram& p)
{
    p.print(out);
    return out;
}



CCSProcess::CCSProcess(Type type)
    :type(type)
{}

CCSProcess::Type CCSProcess::getType() const
{ return type; }

set<CCSTransition> CCSProcess::getTransitions(CCSProgram& program)
{ return getTransitions(program, {}); }

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

void CCSNull::print(ostream& out) const
{
    out << "0";
}

set<CCSTransition> CCSNull::getTransitions(CCSProgram& program, set<string> seen)
{
    return {};
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

void CCSTerm::print(ostream& out) const
{
    out << "1";
}

set<CCSTransition> CCSTerm::getTransitions(CCSProgram& program, set<string> seen)
{
    return { CCSTransition(CCSAction(CCSAction::DELTA), shared_from_this(), make_shared<CCSNull>()) };
}

void CCSTerm::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSProcessName::CCSProcessName(string name)
    :CCSProcess(PROCESSNAME), name(name)
{}

int CCSProcessName::compare(CCSProcess* p2) const
{
    CCSProcessName* _p2 = (CCSProcessName*)p2;
    if(name < _p2->name)
        return -1;
    else if(name > _p2->name)
        return 1;
    else
        return 0;
}

string CCSProcessName::getName() const
{ return name; }

void CCSProcessName::print(ostream& out) const
{
    out << name;
}

set<CCSTransition> CCSProcessName::getTransitions(CCSProgram& program, set<string> seen)
{
    shared_ptr<CCSProcess> p = program.get(name);

    if(seen.count(name))
        throw CCSRecursionException(shared_from_this(), "unguarded recursion in process \"" + name + " := " + (string)*p + "\"");
    seen.insert(name);
    if(p)
    {
        set<CCSTransition> res;
        for(CCSTransition t : p->getTransitions(program, seen))
            res.emplace(t.getAction(), shared_from_this(), t.getTo());
        return res;
    }
    else
        return {};
}

void CCSProcessName::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSPrefix::CCSPrefix(CCSAction act, shared_ptr<CCSProcess> p)
    :CCSProcess(PREFIX), act(act), p(p)
{}

int CCSPrefix::compare(CCSProcess* p2) const
{
    CCSPrefix* _p2 = (CCSPrefix*)p2;
    int c = act.compare(_p2->act);
    if(c != 0)
        return c;
    return p->compare(*_p2->p);
}

CCSAction CCSPrefix::getAction() const
{ return act; }

shared_ptr<CCSProcess> CCSPrefix::getProcess() const
{ return p; }

void CCSPrefix::print(ostream& out) const
{
    act.print(out);
    out << ".";
    p->print(out);
}

set<CCSTransition> CCSPrefix::getTransitions(CCSProgram& program, set<string> seen)
{
    return { CCSTransition(act, shared_from_this(), p) };
}

void CCSPrefix::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSChoice::CCSChoice(shared_ptr<CCSProcess> left, shared_ptr<CCSProcess> right)
    :CCSProcess(CHOICE), left(left), right(right)
{}

int CCSChoice::compare(CCSProcess* p2) const
{
    CCSChoice* _p2 = (CCSChoice*)p2;
    int c = left->compare(*_p2->left);
    if(c != 0)
        return c;
    return right->compare(*_p2->right);
}

shared_ptr<CCSProcess> CCSChoice::getLeft() const
{ return left; }

shared_ptr<CCSProcess> CCSChoice::getRight() const
{ return right; }

void CCSChoice::print(ostream& out) const
{
    out << "(";
    left->print(out);
    out << " + ";
    right->print(out);
    out << ")";
}

set<CCSTransition> CCSChoice::getTransitions(CCSProgram& program, set<string> seen)
{
    set<CCSTransition> res;
    for(CCSTransition t : left->getTransitions(program, seen))
        res.emplace(t.getAction(), shared_from_this(), t.getTo());
    for(CCSTransition t : right->getTransitions(program, seen))
        res.emplace(t.getAction(), shared_from_this(), t.getTo());
    return res;
}

void CCSChoice::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSParallel::CCSParallel(shared_ptr<CCSProcess> left, shared_ptr<CCSProcess> right)
    :CCSProcess(PARALLEL), left(left), right(right)
{}

int CCSParallel::compare(CCSProcess* p2) const
{
    CCSParallel* _p2 = (CCSParallel*)p2;
    int c = left->compare(*_p2->left);
    if(c != 0)
        return c;
    return right->compare(*_p2->right);
}

shared_ptr<CCSProcess> CCSParallel::getLeft() const
{ return left; }

shared_ptr<CCSProcess> CCSParallel::getRight() const
{ return right; }

void CCSParallel::print(ostream& out) const
{
    out << "(";
    left->print(out);
    out << " | ";
    right->print(out);
    out << ")";
}

set<CCSTransition> CCSParallel::getTransitions(CCSProgram& program, set<string> seen)
{
    set<CCSTransition> res;
    set<CCSTransition> resl = left->getTransitions(program, seen);
    set<CCSTransition> resr = right->getTransitions(program, seen);

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
            if(act == ~t2.getAction())
            {
                res.emplace(CCSAction(CCSAction::TAU), shared_from_this(), make_shared<CCSParallel>(t.getTo(), t2.getTo()));
            }
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

void CCSParallel::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSRestrict::CCSRestrict(shared_ptr<CCSProcess> p, set<CCSAction> r, bool complement)
    :CCSProcess(RESTRICT), p(p), r(r), complement(complement)
{}

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

shared_ptr<CCSProcess> CCSRestrict::getProcess() const
{ return p; }

set<CCSAction> CCSRestrict::getR() const
{ return r; }

bool CCSRestrict::isComplement() const
{ return complement; }

void CCSRestrict::print(ostream& out) const
{
    out << "(";
    p->print(out);
    out << ")\\{";
    if(complement)
        out << "*,";
    bool first = true;
    for(CCSAction act : r)
    {
        if(!first)
            out << ",";
        first = false;
        act.print(out);
    }
    out << "}";
}

set<CCSTransition> CCSRestrict::getTransitions(CCSProgram& program, set<string> seen)
{
    set<CCSTransition> res;
    for(const CCSTransition& t : p->getTransitions(program, seen))
    {
        CCSAction act = t.getAction();
        if(act.getType() != CCSAction::TAU && act.getType() != CCSAction::DELTA)
        {
            bool inr = r.count(act) || r.count(CCSAction(CCSAction::NONE, act.getName()));
            if(inr != complement)
                continue;
        }
        res.emplace(act, shared_from_this(), make_shared<CCSRestrict>(t.getTo(), r, complement));
    }
    return res;
}

void CCSRestrict::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSSequential::CCSSequential(shared_ptr<CCSProcess> left, shared_ptr<CCSProcess> right)
    :CCSProcess(SEQUENTIAL), left(left), right(right)
{}

int CCSSequential::compare(CCSProcess* p2) const
{
    CCSSequential* _p2 = (CCSSequential*)p2;
    int c = left->compare(*_p2->left);
    if(c != 0)
        return c;
    return right->compare(*_p2->right);
}

shared_ptr<CCSProcess> CCSSequential::getLeft() const
{ return left; }

shared_ptr<CCSProcess> CCSSequential::getRight() const
{ return right; }

void CCSSequential::print(ostream& out) const
{
    out << "(";
    left->print(out);
    out << "; ";
    right->print(out);
    out << ")";
}

set<CCSTransition> CCSSequential::getTransitions(CCSProgram& program, set<string> seen)
{
    set<CCSTransition> res;
    for(CCSTransition t : left->getTransitions(program, seen))
    {
        if(t.getAction().getType() == CCSAction::DELTA)
            res.emplace(CCSAction(CCSAction::TAU), shared_from_this(), right);
        else
            res.emplace(t.getAction(), shared_from_this(), make_shared<CCSSequential>(t.getTo(), right));
    }
    return res;
}

void CCSSequential::accept(CCSVisitor<void>* v)
{ v->visit(this); }



CCSException::CCSException(shared_ptr<CCSProcess> process, string message)
    :runtime_error(message), process(process)
{}

shared_ptr<CCSProcess> CCSException::getProcess() const
{ return process; }

CCSRecursionException::CCSRecursionException(shared_ptr<CCSProcessName> process, string message)
    :CCSException(process, message), name(process->getName())
{}

string CCSRecursionException::getName() const
{ return name; }



void CCSVisitor<void>::visit(CCSProcess& p)
{ p.accept(this); }

void CCSVisitor<void>::visit(CCSProcess* p)
{ p->accept(this); }

void CCSVisitor<void>::visit(shared_ptr<CCSProcess> p)
{ p->accept(this); }
