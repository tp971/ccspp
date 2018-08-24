#include "ccs.h"
#include "ccsvisitor.h"
#include <sstream>

using namespace std;
using namespace ccspp;

CCSAction::CCSAction(Type type, string name, shared_ptr<CCSExp> param, string input, shared_ptr<CCSExp> exp)
    :type(type), name(name), param(param), input(input), exp(exp)
{}

CCSAction::CCSAction(Type type)
    :type(type)
{
    if(type != NONE && type != TAU && type != DELTA)
        throw CCSException("invalid action type without name");
}

CCSAction::CCSAction(Type type, string name, shared_ptr<CCSExp> param)
    :type(type), name(name), param(param)
{
    if(type != NONE && type != SEND && type != RECV && (name != "" || param != nullptr))
        throw CCSException("invalid action type with name");
}

CCSAction::CCSAction(Type type, string name, shared_ptr<CCSExp> param, string input)
    :type(type), name(name), param(param), input(input)
{
    if(type != RECV)
        throw CCSException("invalid action type with input");
    if(name == "")
        throw CCSException("invalid action type without name");
}

CCSAction::CCSAction(Type type, string name, shared_ptr<CCSExp> param, shared_ptr<CCSExp> exp)
    :type(type), name(name), param(param), exp(exp)
{
    if(type != SEND && type != RECV)
        throw CCSException("invalid action type with expression");
    if(name == "")
        throw CCSException("invalid action type without name");
}

string CCSAction::getName() const
{ return name; }

CCSAction::Type CCSAction::getType() const
{ return type; }

shared_ptr<CCSExp> CCSAction::getParam() const
{ return param; }

string CCSAction::getInput() const
{ return input; }

shared_ptr<CCSExp> CCSAction::getExp() const
{ return exp; }

CCSAction CCSAction::getBase() const
{ return CCSAction(type, name, param); }

CCSAction CCSAction::getPlain() const
{ return CCSAction(type, name); }

CCSAction CCSAction::getNone() const
{ return CCSAction(NONE, name); }

CCSAction CCSAction::subst(string id, int v, bool fold) const
{
    shared_ptr<CCSExp> param2;
    shared_ptr<CCSExp> exp2;
    if(param != nullptr)
        param2 = param->subst(id, v, fold);
    if(exp != nullptr)
        exp2 = exp->subst(id, v, fold);
    return CCSAction(type, name, param2, input, exp2);
}

CCSAction CCSAction::eval() const
{
    shared_ptr<CCSExp> param2;
    shared_ptr<CCSExp> exp2;
    if(param != nullptr)
        if(param->getType() == CCSExp::CONST)
            param2 = param;
        else
            param2 = make_shared<CCSConstExp>(param->eval());
    if(exp != nullptr)
        if(exp->getType() == CCSExp::CONST)
            exp2 = exp;
        else
            exp2 = make_shared<CCSConstExp>(exp->eval());
    return CCSAction(type, name, param2, input, exp2);
}

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
        out << name;
        if(param != nullptr)
        {
            out << "(";
            param->print(out);
            out << ")";
        }
        out << "!";
        if(exp != nullptr)
            exp->print(out);
        break;
    case RECV:
        out << name;
        if(param != nullptr)
        {
            out << "(";
            param->print(out);
            out << ")";
        }
        out << "?";
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
    return CCSAction(t2, name, param, input, exp);
}

int CCSAction::compare(const CCSAction& act) const
{
    if(type < act.type)
        return -1;
    else if(type > act.type)
        return 1;
    if(name < act.name)
        return -1;
    else if(name > act.name)
        return 1;
    if(param != act.param)
    {
        if(param == nullptr)
            return -1;
        else if(act.param == nullptr)
            return 1;
        int c = param->compare(*act.param);
        if(c != 0)
            return c;
    }
    if(input < act.input)
        return -1;
    else if(input > act.input)
        return 1;
    if(exp != act.exp)
    {
        if(exp == nullptr)
            return -1;
        else if(act.exp == nullptr)
            return 1;
        int c = exp->compare(*act.exp);
        if(c != 0)
            return c;
    }
    return 0;
}

bool CCSAction::operator< (const CCSAction& act) const
{ return compare(act) < 0; }

bool CCSAction::operator== (const CCSAction& act) const
{
    return type == act.type && name == act.name && input == act.input &&
        (param == act.param || (param != nullptr && act.param != nullptr && param->compare(*act.param) == 0)) &&
        (exp == act.exp || (exp != nullptr && act.exp != nullptr && exp->compare(*act.exp) == 0));
}



CCSTransition::CCSTransition()
{}

CCSTransition::CCSTransition(CCSAction act, shared_ptr<CCSProcess> from, shared_ptr<CCSProcess> to)
    :act(act), from(from), to(to)
{}

void CCSTransition::print(ostream& out) const
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

ostream& ccspp::operator<< (ostream& out, const CCSBinding& b)
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

ostream& ccspp::operator<< (ostream& out, const CCSProgram& p)
{
    p.print(out);
    return out;
}



CCSException::CCSException(string message)
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
