#ifndef CCSPP_CCSEXP_H_INCLUDED
#define CCSPP_CCSEXP_H_INCLUDED

#include <memory>
#include <string>
#include <set>
#include <map>
#include <iostream>

namespace ccspp
{
    class CCSExp
    {
    public:
        enum Type
        {
            CONST = 0,
            ID,
            UNARY,
            BINARY
        };

        friend class CCSConstExp;
        friend class CCSIdExp;
        friend class CCSUnaryExp;
        friend class CCSBinaryExp;

    private:
        Type type;

    protected:
        /** @brief Internal comparison function.
            This should only be called if p is of the same type as this CCSExp.
        */
        virtual int compare(CCSExp* e) const = 0;

    public:
        /** @brief Constructor. */
        CCSExp(Type type);

        /** @brief Returns the type of the expression. */
        Type getType() const;

        /** @brief Prints the CCSExp to an output stream. */
        virtual void print(std::ostream& out) const = 0;

        ///** @brief Visitor acceptor method. */
        //virtual void accept(CCSVisitor<void>* v) = 0;

        ///** @brief Substitute identifier by a value. */
        virtual std::shared_ptr<CCSExp> subst(std::string id, int val, bool fold = true) = 0;

        ///** @brief Evaluate expression. */
        virtual int eval() = 0;

        /** @brief Returns a string representing this CCSExp. */
        operator std::string() const;

        /** @brief Compares this CCSExp to another instance.
            \returns -1 if this < p, 1 if this > p, 0 else.
        */
        int compare(CCSExp& e) const;

        /** @brief Comparator to use in STL containers. */
        bool operator< (CCSExp& e) const;
    };

    /** @brief Prints a CCSExp to an output stream */
    std::ostream& operator<< (std::ostream& out, const CCSExp& p);

    class CCSConstExp : public CCSExp, public std::enable_shared_from_this<CCSConstExp>
    {
    private:
        int val;

    protected:
        virtual int compare(CCSExp* e) const;

    public:
        CCSConstExp(int val);
        int getVal() const;
        virtual void print(std::ostream& out) const;
        //virtual void accept(CCSVisitor<void>* v);
        virtual std::shared_ptr<CCSExp> subst(std::string id, int val, bool fold = true);
        virtual int eval();
    };

    class CCSIdExp : public CCSExp, public std::enable_shared_from_this<CCSIdExp>
    {
    private:
        std::string id;

    protected:
        virtual int compare(CCSExp* e) const;

    public:
        CCSIdExp(std::string id);
        std::string getId() const;
        virtual void print(std::ostream& out) const;
        //virtual void accept(CCSVisitor<void>* v);
        virtual std::shared_ptr<CCSExp> subst(std::string id, int val, bool fold = true);
        virtual int eval();
    };

    class CCSUnaryExp : public CCSExp, public std::enable_shared_from_this<CCSUnaryExp>
    {
    public:
        enum Op
        {
            PLUS = 0,
            MINUS,
            NOT
        };
    private:
        Op op;
        std::shared_ptr<CCSExp> exp;

        int eval(int val);

    protected:
        virtual int compare(CCSExp* e) const;

    public:
        CCSUnaryExp(Op op, std::shared_ptr<CCSExp> exp);
        Op getOp() const;
        std::shared_ptr<CCSExp> getExp() const;
        virtual void print(std::ostream& out) const;
        //virtual void accept(CCSVisitor<void>* v);
        virtual std::shared_ptr<CCSExp> subst(std::string id, int val, bool fold = true);
        virtual int eval();
    };

    class CCSBinaryExp : public CCSExp, public std::enable_shared_from_this<CCSBinaryExp>
    {
    public:
        enum Op
        {
            PLUS = 0,
            MINUS,
            MUL,
            DIV,
            MOD,
            AND,
            OR,
            EQ,
            NEQ,
            LT,
            LEQ,
            GT,
            GEQ
        };
    private:
        Op op;
        std::shared_ptr<CCSExp> lhs;
        std::shared_ptr<CCSExp> rhs;

        int eval(int lval, int rval);

    protected:
        virtual int compare(CCSExp* e) const;

    public:
        CCSBinaryExp(Op op, std::shared_ptr<CCSExp> lhs, std::shared_ptr<CCSExp> rhs);
        Op getOp() const;
        std::shared_ptr<CCSExp> getLhs() const;
        std::shared_ptr<CCSExp> getRhs() const;
        virtual void print(std::ostream& out) const;
        //virtual void accept(CCSVisitor<void>* v);
        virtual std::shared_ptr<CCSExp> subst(std::string id, int val, bool fold = true);
        virtual int eval();
    };
}

#endif //CCSPP_CCSEXP_H_INCLUDED
