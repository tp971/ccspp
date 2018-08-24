#include "ccsparser.h"
#include <cctype>
#include <sstream>
#include <stack>

using namespace std;
using namespace ccspp;

CCSParserException::CCSParserException(string source, int line, int col, string msg)
    :runtime_error(""), source(source), line(line), col(col), msg(msg)
{
    stringstream ss;
    ss << source << ":" << line << ":" << col << ": error: " << msg;
    (runtime_error&)(*this) = runtime_error(ss.str());
}

CCSParserException::CCSParserException(CCSToken t, string msg)
    :CCSParserException(t.source, t.line, t.col, msg)
{}

CCSToken::CCSToken(Type type, string str, string source, int line, int col)
    :type(type), str(str), source(source), line(line), col(col)
{}

CCSLexer::CCSLexer(istream& input, string name, int lookahead)
    :input(input), name(name), lookahead(lookahead), line(0), col(0)
{}

int CCSLexer::getch()
{
    char last = ch;

    if(input.eof())
        return -1;
    ch = input.get();
    if(input.eof())
        return -1;
    if(line == 0)
        line++;
    
    if(ch == '\r' || ch == '\n')
    {
        if(!(last == '\r' && ch == '\n'))
        {
            line++;
            col = 0;
        }
    }
    else
        col++;

    return ch;
}

void CCSLexer::read()
{
    if(line == 0)
        getch();

    while(!input.eof() && isspace(ch))
        getch();
    
    if(input.eof())
    {
        tokens.emplace_back(CCSToken::TEOF, "", name, line, col);
        return;
    }

    switch(ch)
    {
    case '(': tokens.emplace_back(CCSToken::TLPAR, "(", name, line, col); getch(); break;
    case ')': tokens.emplace_back(CCSToken::TRPAR, ")", name, line, col); getch(); break;
    case '[': tokens.emplace_back(CCSToken::TLSQBR, "[", name, line, col); getch(); break;
    case ']': tokens.emplace_back(CCSToken::TRSQBR, "]", name, line, col); getch(); break;
    case '+': tokens.emplace_back(CCSToken::TPLUS, "+", name, line, col); getch(); break;
    case '-': tokens.emplace_back(CCSToken::TMINUS, "-", name, line, col); getch(); break;
    case '*': tokens.emplace_back(CCSToken::TSTAR, "*", name, line, col); getch(); break;
    case '/': tokens.emplace_back(CCSToken::TSLASH, "/", name, line, col); getch(); break;
    case '%': tokens.emplace_back(CCSToken::TPERCENT, "%", name, line, col); getch(); break;
    case '&':
        getch();
        if(input.eof())
            throw CCSParserException(name, line, col, "unexpected end of file");
        if(ch == '&')
            { tokens.emplace_back(CCSToken::TANDAND, "&&", name, line, col); getch(); }
        else if(ch == '\r' || ch == '\n')
            throw CCSParserException(name, line, col, "unexpected end of line");
        else
            throw CCSParserException(name, line, col, string("unexpected character: `") + ch + "`");
        break;
    case '|':
        getch();
        if(input.eof())
            throw CCSParserException(name, line, col, "unexpected end of file");
        if(ch == '|')
            { tokens.emplace_back(CCSToken::TPIPEPIPE, "||", name, line, col); getch(); }
        else
            tokens.emplace_back(CCSToken::TPIPE, "|", name, line, col);
        break;
    case '=':
        getch();
        if(input.eof())
            throw CCSParserException(name, line, col, "unexpected end of file");
        if(ch == '=')
            { tokens.emplace_back(CCSToken::TEQEQ, "==", name, line, col); getch(); }
        else if(ch == '\r' || ch == '\n')
            throw CCSParserException(name, line, col, "unexpected end of line");
        else
            throw CCSParserException(name, line, col, string("unexpected character: `") + ch + "`");
        break;
    case '!':
        getch();
        if(input.eof())
            throw CCSParserException(name, line, col, "unexpected end of file");
        if(ch == '=')
            { tokens.emplace_back(CCSToken::TNEQ, "!=", name, line, col); getch(); }
        else
            tokens.emplace_back(CCSToken::TBANG, "!", name, line, col);
        break;
    case '<':
        getch();
        if(input.eof())
            throw CCSParserException(name, line, col, "unexpected end of file");
        if(ch == '=')
            { tokens.emplace_back(CCSToken::TLEQ, "<=", name, line, col); getch(); }
        else
            tokens.emplace_back(CCSToken::TLT, "<", name, line, col);
        break;
    case '>':
        getch();
        if(input.eof())
            throw CCSParserException(name, line, col, "unexpected end of file");
        if(ch == '=')
            { tokens.emplace_back(CCSToken::TGEQ, ">=", name, line, col); getch(); }
        else
            tokens.emplace_back(CCSToken::TGT, ">", name, line, col);
        break;
    case ':':
        getch();
        if(input.eof())
            throw CCSParserException(name, line, col, "unexpected end of file");
        if(ch == '=')
            { tokens.emplace_back(CCSToken::TCOLONEQ, ":=", name, line, col); getch(); }
        else if(ch == '\r' || ch == '\n')
            throw CCSParserException(name, line, col, "unexpected end of line");
        else
            throw CCSParserException(name, line, col, string("unexpected character: `") + ch + "`");
        break;
    case '.': tokens.emplace_back(CCSToken::TDOT, ".", name, line, col); getch(); break;
    case '?': tokens.emplace_back(CCSToken::TQUESTIONMARK, "?", name, line, col); getch(); break;
    case ';': tokens.emplace_back(CCSToken::TSEMICOLON, ";", name, line, col); getch(); break;
    case '\\': tokens.emplace_back(CCSToken::TBACKSLASH, "\\", name, line, col); getch(); break;
    case '{': tokens.emplace_back(CCSToken::TLBRACE, "{", name, line, col); getch(); break;
    case '}': tokens.emplace_back(CCSToken::TRBRACE, "}", name, line, col); getch(); break;
    case ',': tokens.emplace_back(CCSToken::TCOMMA, ",", name, line, col); getch(); break;
    default:
        if(isalpha(ch) || ch == '_')
        {
            int l = line;
            int c = col;
            string str = string();
            while(!input.eof() && (isalnum(ch) || ch == '_'))
            {
                str += ch;
                getch();
            }

            if(str == "when")
                tokens.emplace_back(CCSToken::TWHEN, str, name, l, c);
            else
                tokens.emplace_back(CCSToken::TID, str, name, l, c);
        }
        else if(isdigit(ch))
        {
            int l = line;
            int c = col;
            string str = string();
            while(!input.eof() && (isdigit(ch)))
            {
                str += ch;
                getch();
            }
            tokens.emplace_back(CCSToken::TNUM, str, name, l, c);
        }
        else
            throw CCSParserException(name, line, col, string("unexpected character: `") + ch + "`");
    }
}

void CCSLexer::update()
{
    while(tokens.size() < lookahead)
        read();
}

CCSToken& CCSLexer::peek(int i)
{
    update();
    return tokens[i];
}

CCSToken& CCSLexer::next()
{
    if(!tokens.empty())
        tokens.pop_front();
    update();
    return tokens.front();
}

CCSParser::CCSParser(istream& input, string name)
    :lex(input, name, 2)
{
    prec_i = 0;

    addPrec(1);
    addOp(CCSToken::TPIPEPIPE);

    addPrec(1);
    addOp(CCSToken::TANDAND);

    addPrec(1);
    addOp(CCSToken::TEQEQ);
    addOp(CCSToken::TNEQ);

    addPrec(1);
    addOp(CCSToken::TLT);
    addOp(CCSToken::TLEQ);
    addOp(CCSToken::TGT);
    addOp(CCSToken::TGEQ);

    addPrec(1);
    addOp(CCSToken::TPLUS);
    addOp(CCSToken::TMINUS);

    addPrec(1);
    addOp(CCSToken::TSTAR);
    addOp(CCSToken::TSLASH);
    addOp(CCSToken::TPERCENT);

    //unary
    addPrec(1);



    pprec_i = 0;

    addPPrec(1);
    addPOp(CCSToken::TSEMICOLON);

    addPPrec(1);
    addPOp(CCSToken::TPIPE);

    addPPrec(1);
    addPOp(CCSToken::TPLUS);

    //unary
    addPPrec(1);
}

void CCSParser::addPrec(int assoc)
{
    if(assoc < 0)
    {
        prec_i += 2;
        prec_left = true;
    }
    else
    {
        prec_i++;
        prec_left = false;
    }
}

void CCSParser::addOp(CCSToken::Type type)
{
    if(prec_left)
    {
        prec_l[type] = prec_i - 1;
        prec_r[type] = prec_i;
    }
    else
    {
        prec_l[type] = prec_i;
        prec_r[type] = prec_i;
    }
}

int CCSParser::getLPrec(CCSToken::Type type)
{
    if(prec_l.count(type))
        return prec_l[type];
    return -1;
}

int CCSParser::getRPrec(CCSToken::Type type)
{
    if(prec_r.count(type))
        return prec_r[type];
    return -1;
}

void CCSParser::addPPrec(int assoc)
{
    if(assoc < 0)
    {
        pprec_i += 2;
        pprec_left = true;
    }
    else
    {
        pprec_i++;
        pprec_left = false;
    }
}

void CCSParser::addPOp(CCSToken::Type type)
{
    if(pprec_left)
    {
        pprec_l[type] = pprec_i - 1;
        pprec_r[type] = pprec_i;
    }
    else
    {
        pprec_l[type] = pprec_i;
        pprec_r[type] = pprec_i;
    }
}

int CCSParser::getLPPrec(CCSToken::Type type)
{
    if(pprec_l.count(type))
        return pprec_l[type];
    return -1;
}

int CCSParser::getRPPrec(CCSToken::Type type)
{
    if(pprec_r.count(type))
        return pprec_r[type];
    return -1;
}

unique_ptr<CCSProgram> CCSParser::parse()
{
    unique_ptr<CCSProgram> res = make_unique<CCSProgram>();
    
    CCSToken t = lex.peek(0);
    CCSToken t2 = lex.peek(1);
    while(t.type == CCSToken::TID && (t2.type == CCSToken::TLSQBR || t2.type == CCSToken::TCOLONEQ))
    {
        shared_ptr<CCSProcessName> p = static_pointer_cast<CCSProcessName>(parsePrimaryProcess());

        bool allnames = true;
        vector<string> params;
        for(shared_ptr<CCSExp> next : p->getArgs())
            if(next->getType() == CCSExp::ID)
                params.push_back(((CCSIdExp&)*next).getId());
            else
            {
                allnames = false;
                break;
            }

        if(allnames && lex.peek(0).type == CCSToken::TCOLONEQ)
        {
            lex.next();
            res->addBinding(t.str, params, parseProcess());
        }
        else
        {
            res->setProcess(parseProcess(0, p));
            break;
        }
        t = lex.peek(0);
        t2 = lex.peek(1);
    }

    if(res->getProcess() == nullptr)
        res->setProcess(parseProcess());

    t = lex.peek(0);
    if(t.type != CCSToken::TEOF)
        throw CCSParserException(t, "unexpected `" + t.str + "`, expected end of file");

    return res;
}

shared_ptr<CCSExp> CCSParser::parseExp(int prec)
{
    shared_ptr<CCSExp> res;

    CCSToken t = lex.peek(0);
    switch(t.type)
    {
    case CCSToken::TPLUS: res = make_shared<CCSUnaryExp>(CCSUnaryExp::PLUS, parseExp(prec_i)); break;
    case CCSToken::TMINUS: res = make_shared<CCSUnaryExp>(CCSUnaryExp::MINUS, parseExp(prec_i)); break;
    case CCSToken::TBANG: res = make_shared<CCSUnaryExp>(CCSUnaryExp::NOT, parseExp(prec_i)); break;
    default: res = parsePrimaryExp(); break;
    }

    for(;;)
    {
        t = lex.peek(0);
        if(getLPrec(t.type) < prec)
            return res;
        else
        {
            lex.next();
            shared_ptr<CCSExp> rhs = parseExp(getRPrec(t.type));
            switch(t.type)
            {
            case CCSToken::TPLUS: res = make_shared<CCSBinaryExp>(CCSBinaryExp::PLUS, res, rhs); break;
            case CCSToken::TMINUS: res = make_shared<CCSBinaryExp>(CCSBinaryExp::MINUS, res, rhs); break;
            case CCSToken::TSTAR: res = make_shared<CCSBinaryExp>(CCSBinaryExp::MUL, res, rhs); break;
            case CCSToken::TSLASH: res = make_shared<CCSBinaryExp>(CCSBinaryExp::DIV, res, rhs); break;
            case CCSToken::TPERCENT: res = make_shared<CCSBinaryExp>(CCSBinaryExp::MOD, res, rhs); break;
            case CCSToken::TANDAND: res = make_shared<CCSBinaryExp>(CCSBinaryExp::AND, res, rhs); break;
            case CCSToken::TPIPEPIPE: res = make_shared<CCSBinaryExp>(CCSBinaryExp::OR, res, rhs); break;
            case CCSToken::TEQEQ: res = make_shared<CCSBinaryExp>(CCSBinaryExp::EQ, res, rhs); break;
            case CCSToken::TNEQ: res = make_shared<CCSBinaryExp>(CCSBinaryExp::NEQ, res, rhs); break;
            case CCSToken::TLT: res = make_shared<CCSBinaryExp>(CCSBinaryExp::LT, res, rhs); break;
            case CCSToken::TLEQ: res = make_shared<CCSBinaryExp>(CCSBinaryExp::LEQ, res, rhs); break;
            case CCSToken::TGT: res = make_shared<CCSBinaryExp>(CCSBinaryExp::GT, res, rhs); break;
            case CCSToken::TGEQ: res = make_shared<CCSBinaryExp>(CCSBinaryExp::GEQ, res, rhs); break;
            }
        }
    }
}

shared_ptr<CCSExp> CCSParser::parsePrimaryExp()
{
    CCSToken t = lex.peek(0);
    if(t.type == CCSToken::TEOF)
        throw CCSParserException(t, "unexpected end of file, expected `(`, identifier or constant");
    switch(t.type)
    {
    case CCSToken::TID:
    {
        lex.next();
        return make_shared<CCSIdExp>(t.str);
    }
    case CCSToken::TNUM:
    {
        try
        {
            lex.next();
            return make_shared<CCSConstExp>(stoi(t.str));
        }
        catch(exception& ex)
        {
            throw CCSParserException(t, "invalid number `" + t.str + "`");
        }
    }
    case CCSToken::TLPAR:
    {
        lex.next();
        shared_ptr<CCSExp> res = parseExp();
        t = lex.peek(0);
        if(t.type == CCSToken::TEOF)
            throw CCSParserException(t, "unexpected end of file, expected `)`");
        if(t.type != CCSToken::TRPAR)
            throw CCSParserException(t, "unexpected `" + t.str + "`, expected `)`");
        lex.next();
        return res;
    }
    default:
        throw CCSParserException(t, "unexpected `" + t.str + "`, expected `(`, identifier or constant");
    }
}

shared_ptr<CCSProcess> CCSParser::parseProcess(int prec, shared_ptr<CCSProcess> res)
{
    if(res == nullptr)
    {
        CCSToken t = lex.peek(0);
        CCSToken t2 = lex.peek(1);
        if(t.type == CCSToken::TWHEN)
        {
            lex.next();
            shared_ptr<CCSExp> cond = parseExp();
            res = make_shared<CCSWhen>(cond, parseProcess(pprec_i));
        }
        else if(t.type == CCSToken::TID && (t2.type == CCSToken::TDOT || t2.type == CCSToken::TQUESTIONMARK || t2.type == CCSToken::TBANG))
        {
            CCSAction act = parseAction();
            t = lex.peek(0);
            if(t.type == CCSToken::TEOF)
                throw CCSParserException(t, "unexpected end of file, expected `.`");
            if(t.type != CCSToken::TDOT)
                throw CCSParserException(t, "unexpected `" + t.str + "`, expected `.`");
            lex.next();
            res = make_shared<CCSPrefix>(act, parseProcess(pprec_i));
        }
        else
            res = parsePrimaryProcess();
    }

    CCSToken t = lex.peek(0);
    while(t.type == CCSToken::TBACKSLASH)
    {
        t = lex.next();
        if(t.type == CCSToken::TEOF)
            throw CCSParserException(t, "unexpected end of file, expected `{`");
        if(t.type != CCSToken::TLBRACE)
            throw CCSParserException(t, "unexpected `" + t.str + "`, expected `{`");

        bool comp = false;
        set<CCSAction> acts;

        t = lex.next();
        if(t.type == CCSToken::TSTAR)
        {
            comp = true;
            t = lex.next();
        }
        else if(t.type != CCSToken::TRBRACE)
        {
            acts.insert(parseAction());
            t = lex.peek(0);
        }

        while(t.type == CCSToken::TCOMMA)
        {
            lex.next();
            acts.insert(parseAction());
            t = lex.peek(0);
        }

        if(t.type == CCSToken::TEOF)
            throw CCSParserException(t, "unexpected end of file, expected `}`");
        if(t.type != CCSToken::TRBRACE)
            throw CCSParserException(t, "unexpected `" + t.str + "`, expected `}`");
        lex.next();

        res = make_shared<CCSRestrict>(res, acts, comp);
    }

    for(;;)
    {
        t = lex.peek(0);
        if(getLPPrec(t.type) < prec)
            return res;
        else
        {
            lex.next();
            shared_ptr<CCSProcess> rhs = parseProcess(getRPPrec(t.type));
            switch(t.type)
            {
            case CCSToken::TPLUS: res = make_shared<CCSChoice>(res, rhs); break;
            case CCSToken::TPIPE: res = make_shared<CCSParallel>(res, rhs); break;
            case CCSToken::TSEMICOLON: res = make_shared<CCSSequential>(res, rhs); break;
            }
        }
    }
}

shared_ptr<CCSProcess> CCSParser::parsePrimaryProcess()
{
    CCSToken t = lex.peek(0);
    if(t.type == CCSToken::TNUM && t.str == "0")
    {
        lex.next();
        return make_shared<CCSNull>();
    }
    else if(t.type == CCSToken::TNUM && t.str == "1")
    {
        lex.next();
        return make_shared<CCSTerm>();
    }
    else if(t.type == CCSToken::TID)
    {
        string name = t.str;
        vector<shared_ptr<CCSExp>> args;
        t = lex.next();
        if(t.type == CCSToken::TLSQBR)
        {
            lex.next();
            args.push_back(parseExp());
            t = lex.peek(0);
            while(t.type == CCSToken::TCOMMA)
            {
                lex.next();
                args.push_back(parseExp());
                t = lex.peek(0);
            }
            if(t.type == CCSToken::TEOF)
                throw CCSParserException(t, "unexpected end of file, expected `]`");
            if(t.type != CCSToken::TRSQBR)
                throw CCSParserException(t, "unexpected `" + t.str + "`, expected `]`");
            lex.next();
        }
        return make_shared<CCSProcessName>(name, args);
    }
    else if(t.type == CCSToken::TLPAR)
    {
        lex.next();
        shared_ptr<CCSProcess> res = parseProcess();
        t = lex.peek(0);
        if(t.type == CCSToken::TEOF)
            throw CCSParserException(t, "unexpected end of file, expected `)`");
        if(t.type != CCSToken::TRPAR)
            throw CCSParserException(t, "unexpected `" + t.str + "`, expected `)`");
        lex.next();
        return res;
    }
    else if(t.type == CCSToken::TEOF)
        throw CCSParserException(t, "unexpected end of file, expected `0`, `1`, identifier or `(`");
    else
        throw CCSParserException(t, "unexpected `" + t.str + "`, expected `0`, `1`, identifier or `(`");
}

CCSAction CCSParser::parseAction()
{
    CCSToken t = lex.peek(0);
    if(t.type == CCSToken::TEOF)
        throw CCSParserException(t, "unexpected end of file, expected identifier");
    if(t.type != CCSToken::TID)
        throw CCSParserException(t, "unexpected `" + t.str + "`, expected identifier");
    lex.next();

    if(t.str == "i")
        return CCSAction(CCSAction::Type::TAU);
    else if(t.str == "e")
        return CCSAction(CCSAction::Type::DELTA);

    string name = t.str;

    t = lex.peek(0);
    switch(t.type)
    {
    case CCSToken::TBANG:
        t = lex.next();
        if(t.type == CCSToken::TID || t.type == CCSToken::TNUM || t.type == CCSToken::TLPAR || t.type == CCSToken::TPLUS || t.type == CCSToken::TMINUS || t.type == CCSToken::TBANG)
            return CCSAction(CCSAction::SEND, name, parseExp());
        else
            return CCSAction(CCSAction::SEND, name);
    case CCSToken::TQUESTIONMARK:
        t = lex.next();
        if(t.type == CCSToken::TID)
        {
            lex.next();
            return CCSAction(CCSAction::RECV, name, t.str);
        }
        else if(t.type == CCSToken::TNUM || t.type == CCSToken::TLPAR || t.type == CCSToken::TPLUS || t.type == CCSToken::TMINUS || t.type == CCSToken::TBANG)
            return CCSAction(CCSAction::RECV, name, parseExp());
        else
            return CCSAction(CCSAction::RECV, name);
    default:
        return CCSAction(CCSAction::NONE, name);
    }
}
