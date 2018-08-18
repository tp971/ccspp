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
    case ':':
        getch();
        if(input.eof())
            throw CCSParserException(name, line, col, "unexpected end of file");

        if(ch == '=')
            tokens.emplace_back(CCSToken::TCOLONEQ, ":=", name, line, col);
        else if(ch == '\r' || ch == '\n')
            throw CCSParserException(name, line, col, "unexpected end of line");
        else
            throw CCSParserException(name, line, col, string("unexpected character: `") + ch + "`");
        getch();
        break;
    case '.': tokens.emplace_back(CCSToken::TDOT, ".", name, line, col); getch(); break;
    case '!': tokens.emplace_back(CCSToken::TSEND, "!", name, line, col); getch(); break;
    case '?': tokens.emplace_back(CCSToken::TRECV, "?", name, line, col); getch(); break;
    case '+': tokens.emplace_back(CCSToken::TPLUS, "+", name, line, col); getch(); break;
    case '|': tokens.emplace_back(CCSToken::TPIPE, "|", name, line, col); getch(); break;
    case ';': tokens.emplace_back(CCSToken::TSEMICOLON, ";", name, line, col); getch(); break;
    case '\\': tokens.emplace_back(CCSToken::TBACKSLASH, "\\", name, line, col); getch(); break;
    case '{': tokens.emplace_back(CCSToken::TLBRACE, "{", name, line, col); getch(); break;
    case '}': tokens.emplace_back(CCSToken::TRBRACE, "}", name, line, col); getch(); break;
    case ',': tokens.emplace_back(CCSToken::TCOMMA, ",", name, line, col); getch(); break;
    case '*': tokens.emplace_back(CCSToken::TSTAR, "*", name, line, col); getch(); break;
    case '(': tokens.emplace_back(CCSToken::TLPAR, "(", name, line, col); getch(); break;
    case ')': tokens.emplace_back(CCSToken::TRPAR, ")", name, line, col); getch(); break;
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
    addOp(CCSToken::TSEMICOLON);
    
    addPrec(1);
    addOp(CCSToken::TPIPE);
    
    addPrec(1);
    addOp(CCSToken::TPLUS);
    
    //unary
    addPrec(1);
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

unique_ptr<CCSProgram> CCSParser::parse()
{
    unique_ptr<CCSProgram> res = make_unique<CCSProgram>();
    
    CCSToken t;
    while((t = lex.peek(0)).type == CCSToken::TID && lex.peek(1).type == CCSToken::TCOLONEQ)
    {
        lex.next();
        lex.next();
        res->addBinding(t.str, parseProcess());
    }

    res->setProcess(parseProcess());

    t = lex.peek(0);
    if(t.type != CCSToken::TEOF)
        throw CCSParserException(t, "unexpected `" + t.str + "`, expected end of file");

    return res;
}

shared_ptr<CCSProcess> CCSParser::parseProcess(int prec)
{
    shared_ptr<CCSProcess> res = parsePrimary();

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
        CCSToken t = lex.peek(0);
        if(getLPrec(t.type) < prec)
            return res;
        else
        {
            lex.next();
            shared_ptr<CCSProcess> rhs = parseProcess(getRPrec(t.type));
            switch(t.type)
            {
            case CCSToken::TPLUS: res = make_shared<CCSChoice>(res, rhs); break;
            case CCSToken::TPIPE: res = make_shared<CCSParallel>(res, rhs); break;
            case CCSToken::TSEMICOLON: res = make_shared<CCSSequential>(res, rhs); break;
            }
        }
    }
}

shared_ptr<CCSProcess> CCSParser::parsePrimary()
{
    stack<CCSAction> acts;
    CCSToken t = lex.peek(0);
    CCSToken t2 = lex.peek(1);
    while(t.type == CCSToken::TID && (t2.type == CCSToken::TDOT || t2.type == CCSToken::TRECV || t2.type == CCSToken::TSEND))
    {
        acts.push(parseAction());
        t = lex.peek(0);
        if(t.type == CCSToken::TEOF)
            throw CCSParserException(t, "unexpected end of file, expected `.`");
        if(t.type != CCSToken::TDOT)
            throw CCSParserException(t, "unexpected `" + t.str + "`, expected `.`");
        t = lex.next();
        t2 = lex.peek(1);
    }

    shared_ptr<CCSProcess> res;
    if(t.type == CCSToken::TNUM && t.str == "0")
    {
        res = make_shared<CCSNull>();
        lex.next();
    }
    else if(t.type == CCSToken::TNUM && t.str == "1")
    {
        res = make_shared<CCSTerm>();
        lex.next();
    }
    else if(t.type == CCSToken::TID)
    {
        res = make_shared<CCSProcessName>(t.str);
        lex.next();
    }
    else if(t.type == CCSToken::TLPAR)
    {
        lex.next();
        res = parseProcess();
        t = lex.peek(0);
        if(t.type == CCSToken::TEOF)
            throw CCSParserException(t, "unexpected end of file, expected `)`");
        if(t.type != CCSToken::TRPAR)
            throw CCSParserException(t, "unexpected `" + t.str + "`, expected `)`");
        lex.next();
    }
    else if(t.type == CCSToken::TEOF)
        throw CCSParserException(t, "unexpected end of file, expected `0`, `1`, identifier or `(`");
    else
        throw CCSParserException(t, "unexpected `" + t.str + "`, expected `0`, `1`, identifier or `(`");

    while(!acts.empty())
    {
        res = make_shared<CCSPrefix>(acts.top(), res);
        acts.pop();
    }
    return res;
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

    CCSAction::Type type = CCSAction::NONE;
    string name = t.str;

    t = lex.peek(0);
    switch(t.type)
    {
    case CCSToken::TSEND: type = CCSAction::SEND; lex.next(); break;
    case CCSToken::TRECV: type = CCSAction::RECV; lex.next(); break;
    }

    return CCSAction(type, name);
}
