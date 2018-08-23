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



CCSTransition::CCSTransition()
{}

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
