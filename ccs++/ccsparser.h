#ifndef CCSPP_CCSPARSER_H_INCLUDED
#define CCSPP_CCSPARSER_H_INCLUDED

#include "ccs.h"
#include <iostream>
#include <deque>
#include <exception>

namespace ccspp
{
    class CCSLexer;
    class CCSToken;

    class CCSParserException : public std::runtime_error
    {
    private:
        std::string source;
        int line;
        int col;
        std::string msg;

    public:
        CCSParserException(std::string source, int line, int col, std::string msg);
        CCSParserException(CCSToken t, std::string msg);
    };

    class CCSToken
    {
    public:
        enum Type
        {
            TNONE = 0, TEOF, TID, TNUM, TLPAR, TRPAR, TLSQBR, TRSQBR,
            TPLUS, TMINUS, TSTAR, TSLASH, TPERCENT,
            TANDAND, TPIPEPIPE, TEQEQ, TNEQ, TLT, TLEQ, TGT, TGEQ,
            TCOLONEQ, TDOT, TBANG, TQUESTIONMARK, TPIPE, TSEMICOLON, TBACKSLASH, TLBRACE, TRBRACE, TCOMMA, TWHEN
        };

        Type type;
        std::string str;
        std::string source;
        int line;
        int col;

        CCSToken(Type type = TNONE, std::string str = "", std::string source = "", int line = 0, int col = 0);
    };

    class CCSLexer
    {
    private:
        std::istream& input;
        std::string name;
        std::deque<CCSToken> tokens;
        int lookahead;
        int line;
        int col;
        char ch;

        int getch();
        void read();
        void update();

    public:
        CCSLexer(std::istream& input, std::string name, int lookahead);
        CCSToken& peek(int i = 0);
        CCSToken& next();
    };

    class CCSParser
    {
    private:
        CCSLexer lex;

        int prec_i;
        bool prec_left;
        std::map<CCSToken::Type, int> prec_l;
        std::map<CCSToken::Type, int> prec_r;

        void addPrec(int assoc);
        void addOp(CCSToken::Type type);
        int getLPrec(CCSToken::Type type);
        int getRPrec(CCSToken::Type type);

        int pprec_i;
        bool pprec_left;
        std::map<CCSToken::Type, int> pprec_l;
        std::map<CCSToken::Type, int> pprec_r;

        void addPPrec(int assoc);
        void addPOp(CCSToken::Type type);
        int getLPPrec(CCSToken::Type type);
        int getRPPrec(CCSToken::Type type);

        std::shared_ptr<CCSExp> parseExp(int prec = 0);
        std::shared_ptr<CCSExp> parsePrimaryExp();

        std::shared_ptr<CCSProcess> parseProcess(int prec = 0, std::shared_ptr<CCSProcess> res = nullptr);
        std::shared_ptr<CCSProcess> parsePrimaryProcess();
        CCSAction parseAction();

    public:
        CCSParser(std::istream& input, std::string name = "input");
        std::unique_ptr<CCSProgram> parse();
    };
}

#endif //CCSPP_CCSPARSER_H_INCLUDED
