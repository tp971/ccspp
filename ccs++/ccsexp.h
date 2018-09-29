#ifndef CCSPP_CCSEXP_H_INCLUDED
#define CCSPP_CCSEXP_H_INCLUDED

#include <memory>
#include <string>
#include <set>
#include <map>
#include <iostream>

namespace ccspp
{
    /** @brief Represents a CCS expression used in CCSvp. */
    class CCSExp
    {
    public:
        /** @brief The type of the CCS expression */
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

        /** @brief Destructor. */
        virtual ~CCSExp() = default;

        /** @brief Returns the type of the expression. */
        Type getType() const;

        /** @brief Substitute identifier by a value. */
        virtual std::shared_ptr<CCSExp> subst(std::string id, int val, bool fold = true) = 0;

        /** @brief Evaluates the expression.
            @throws CCSUnboundException if there is an identifier in the expression.
            @throws CCSUndefinedException if the expression is undefined (e.g. division by zero)
        */
        virtual int eval() = 0;

        /** @brief Prints the CCSExp to an output stream. */
        virtual void print(std::ostream& out) const = 0;

        /** @brief Visitor acceptor method. */
        virtual void accept(CCSExpVisitor<void>* v) = 0;

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

    /** @brief Represents a constant. */
    class CCSConstExp : public CCSExp, public std::enable_shared_from_this<CCSConstExp>
    {
    private:
        int val;

    protected:
        virtual int compare(CCSExp* e) const;

    public:
        CCSConstExp(int val);
        int getVal() const;
        virtual std::shared_ptr<CCSExp> subst(std::string id, int val, bool fold = true);
        virtual int eval();
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSExpVisitor<void>* v);
    };

    /** @brief Represents an identifier. */
    class CCSIdExp : public CCSExp, public std::enable_shared_from_this<CCSIdExp>
    {
    private:
        std::string id;

    protected:
        virtual int compare(CCSExp* e) const;

    public:
        CCSIdExp(std::string id);
        std::string getId() const;
        virtual std::shared_ptr<CCSExp> subst(std::string id, int val, bool fold = true);
        virtual int eval();
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSExpVisitor<void>* v);
    };

    /** @brief Represents a unary expression. */
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
        virtual std::shared_ptr<CCSExp> subst(std::string id, int val, bool fold = true);
        virtual int eval();
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSExpVisitor<void>* v);
    };

    /** @brief Represents a binary expression. */
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
        virtual std::shared_ptr<CCSExp> subst(std::string id, int val, bool fold = true);
        virtual int eval();
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSExpVisitor<void>* v);
    };
}

#endif //CCSPP_CCSEXP_H_INCLUDED
