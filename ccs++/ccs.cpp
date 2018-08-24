#include "ccs.h"
#include "ccsvisitor.h"
#include <sstream>

using namespace std;
using namespace ccspp;

CCSAction::CCSAction(Type type, string name)
    :type(type), name(name)
{}

CCSAction::CCSAction(Type type, std::string name, std::string input)
    :type(type), name(name), input(input)
{
    if(type != RECV)
        throw CCSException("invalid action type with input");
}

CCSAction::CCSAction(Type type, std::string name, std::shared_ptr<CCSExp> exp)
    :type(type), name(name), exp(exp)
{
    if(type != SEND && type != RECV)
        throw CCSException("invalid action type with expression");
}

string CCSAction::getName() const
{ return name; }

CCSAction::Type CCSAction::getType() const
{ return type; }

string CCSAction::getInput() const
{ return input; }

shared_ptr<CCSExp> CCSAction::getExp() const
{ return exp; }

CCSAction CCSAction::getPlain() const
{ return CCSAction(type, name); }

CCSAction CCSAction::getNone() const
{ return CCSAction(NONE, name); }

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
        if(exp != nullptr)
            exp->print(out);
        break;
    case RECV:
        out << name << "?";
        if(input != "")
            out << input;
        if(exp != nullptr)
            exp->print(out);
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
    else if(input < act.input)
        return -1;
    else if(input > act.input)
        return 1;
    else if(exp == act.exp)
        return 0;
    else if(exp == nullptr)
        return -1;
    else if(act.exp == nullptr)
        return 1;
    else
        return exp->compare(*act.exp);
}

bool CCSAction::operator< (const CCSAction& act) const
{ return compare(act) < 0; }

bool CCSAction::operator== (const CCSAction& act) const
{
    return type == act.type && name == act.name && input == act.input &&
        (exp == act.exp || (exp != nullptr && act.exp != nullptr && exp->compare(*act.exp) == 0));
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

CCSBinding::CCSBinding(string name, vector<string> params, shared_ptr<CCSProcess> process)
    :name(name), params(params), process(process)
{}

string CCSBinding::getName() const
{ return name; }

vector<string> CCSBinding::getParams() const
{ return params; }

shared_ptr<CCSProcess> CCSBinding::getProcess() const
{ return process; }

void CCSBinding::print(ostream& out) const
{
    out << name;
    if(params.size())
    {
        out << "[";
        bool first = true;
        for(const string& next : params)
        {
            if(!first)
                out << ", ";
            first = false;
            out << next;
        }
        out << "]";
    }
    out << " := ";
    process->print(out);
    out << endl;
}

std::ostream& ccspp::operator<< (std::ostream& out, const CCSBinding& b)
{
    b.print(out);
    return out;
}



void CCSProgram::addBinding(string name, vector<string> params, shared_ptr<CCSProcess> process)
{ bindings[name] = CCSBinding(name, params, process); }

void CCSProgram::setProcess(shared_ptr<CCSProcess> process)
{ this->process = process; }

shared_ptr<CCSProcess> CCSProgram::get(string name, vector<int> args, bool fold) const
{
    if(bindings.count(name))
    {
        const CCSBinding& b = bindings.at(name);
        vector<string> params = b.getParams();
        if(args.size() != params.size())
            return nullptr;
        shared_ptr<CCSProcess> res = bindings.at(name).getProcess();
        for(int i = params.size() - 1; i >= 0; i--)
            res = res->subst(params[i], args[i], fold);
        return res;
    }
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



CCSException::CCSException(std::string message)
    :runtime_error(message)
{}



CCSProcessException::CCSProcessException(shared_ptr<CCSProcess> process, string message)
    :CCSException(message), process(process)
{}

shared_ptr<CCSProcess> CCSProcessException::getProcess() const
{ return process; }



CCSRecursionException::CCSRecursionException(shared_ptr<CCSProcessName> process, string message)
    :CCSProcessException(process, message), name(process->getName())
{}

string CCSRecursionException::getName() const
{ return name; }



CCSExpException::CCSExpException(shared_ptr<CCSExp> exp, string message)
    :CCSException(message), exp(exp)
{}

shared_ptr<CCSExp> CCSExpException::getExp() const
{ return exp; }

void CCSExpException::setExp(shared_ptr<CCSExp> exp)
{ this->exp = exp; }



CCSUnboundException::CCSUnboundException(shared_ptr<CCSExp> exp, string id, string message)
    :CCSExpException(exp, message), id(id)
{}

string CCSUnboundException::getId() const
{ return id; }



CCSUndefinedException::CCSUndefinedException(shared_ptr<CCSExp> exp, string message)
    :CCSExpException(exp, message)
{}
