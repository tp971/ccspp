#ifndef CCSPP_CCS_H_INCLUDED
#define CCSPP_CCS_H_INCLUDED

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <iostream>

namespace ccspp
{
    class CCSExp;
    class CCSProcess;
    class CCSProcessName;

    template<typename T, typename V = void>
    class CCSVisitor;

    template<typename T, typename V = void>
    class CCSExpVisitor;

    /** @brief Represents a CCS action.

        A CCS action can be:
        1. `i` (the internal action)
        2. `e` (the termination action)
        3. `name(param)`
        4. `name(param)!`
        5. `name(param)!(exp)`
        6. `name(param)?identifier`
        7. `name(param)?(exp)`

        where param and exp are CCS expressions.
        In all cases, (param) can be omitted.

        The action `i` has type TAU, the action `e` has type DELTA.
        Actions with a `!` have type SEND and actions with `?` have type RECV.
        All other actions have type NONE.
    */
    class CCSAction
    {
    public:
        /** @brief Represents the type of a CCS action */
        enum Type
        {
            NONE = 0,   /**< action */
            TAU,        /**< internal action */
            DELTA,      /**< termination action */
            SEND,       /**< send action */
            RECV        /**< receive action */
        };

    private:
        Type type;
        std::string name;
        std::shared_ptr<CCSExp> param;
        std::string input;
        std::shared_ptr<CCSExp> exp;

        CCSAction(Type type, std::string name, std::shared_ptr<CCSExp> param, std::string input, std::shared_ptr<CCSExp> exp);

    public:
        /** @brief Constructs an empty CCSAction, i or e. */
        CCSAction(Type type = NONE);

        /** @brief Constructs a CCSAction name(param), name(param)! or name(param)? */
        CCSAction(Type type, std::string name, std::shared_ptr<CCSExp> param = nullptr);

        /** @brief Constructs a CCSAction name(param)?input */
        CCSAction(Type type, std::string name, std::shared_ptr<CCSExp> param, std::string input);

        /** @brief Constructs a CCSAction name(param)?exp or name(param)!exp */
        CCSAction(Type type, std::string name, std::shared_ptr<CCSExp> param, std::shared_ptr<CCSExp> exp);

        /** @brief Returns the type of the CCSAction. */
        Type getType() const;

        /** @brief Returns the name of the CCSAction. */
        std::string getName() const;

        /** @brief Returns the parameter expression in act(param) */
        std::shared_ptr<CCSExp> getParam() const;

        /** @brief Returns the input in act?input */
        std::string getInput() const;

        /** @brief Returns the expression in act?exp or act!exp */
        std::shared_ptr<CCSExp> getExp() const;

        /** @brief Returns the action without expression or input. */
        CCSAction getBase() const;

        /** @brief Returns the action without parameter and expression or input. */
        CCSAction getPlain() const;

        /** @brief Returns the action without type, parameter and expression or input. */
        CCSAction getNone() const;

        /** @brief Substitutes variable to constant in expressions. */
        CCSAction subst(std::string id, int v, bool fold = true) const;

        /** @brief Returns the action with evaluated expressions. */
        CCSAction eval() const;

        /** @brief Prints the CCSAction to an output stream. */
        void print(std::ostream& out) const;

        /** @brief Returns a string representing this CCSAction. */
        operator std::string() const;

        /** @brief Returns the complementary action (i.e. exchanges SEND and RECV). */
        CCSAction operator~ () const;

        /** @brief Compares this CCSAction to another instance.
            \returns -1 if this < p, 1 if this > p, 0 else.
        */
        int compare(const CCSAction& act) const;

        /** @brief Comparator to use in STL containers. */
        bool operator< (const CCSAction& act) const;

        /** @brief Comparator to use in STL containers. */
        bool operator== (const CCSAction& act) const;
    };

    /** @brief Outputs a CCSAction to an output stream */
    std::ostream& operator<< (std::ostream& out, const CCSAction& act);

    /** @brief Represents a CCS transition.
        CCS processes can perform transitions to other processes.

        If a process P can perform a transition to Q with the action act, we write "from --( act )-> to", where from is the left hand side and to is the right hand side.
    */
    class CCSTransition
    {
    private:
        CCSAction act;
        std::shared_ptr<CCSProcess> from;
        std::shared_ptr<CCSProcess> to;

    public:
        /** @brief Empty constructor. */
        CCSTransition();

        /** @brief Constructs a CCSTransition.
            Constructs a CCSTransition "from --( act )-> to".

            @param act The action performed by the transititon.
            @param from The process on the left hand side.
            @param to The process on the right hand side.
        */
        CCSTransition(CCSAction act, std::shared_ptr<CCSProcess> from, std::shared_ptr<CCSProcess> to);

        /** @brief Return the action of the CCSTransition. */
        CCSAction getAction() const;

        /** @brief Return the left hand side process of the CCSTransition. */
        std::shared_ptr<CCSProcess> getFrom() const;

        /** @brief Return the right hand side process of the CCSTransition. */
        std::shared_ptr<CCSProcess> getTo() const;

        /** @brief Prints the CCSTransition to an output stream. */
        void print(std::ostream& out) const;

        /** @brief Compares this CCSTransition to another instance.
            \returns -1 if this < p, 1 if this > p, 0 else.
        */
        int compare(const CCSTransition& act) const;

        /** @brief Comparator to use in STL containers. */
        bool operator< (const CCSTransition& act) const;
    };

    /** @brief Prints a CCSAction to an output stream */
    std::ostream& operator<< (std::ostream& out, const CCSTransition& t);

    /** @brief Represents a binding to a named process */
    class CCSBinding
    {
    private:
        std::string name;
        std::vector<std::string> params;
        std::shared_ptr<CCSProcess> process;

    public:
        /** @brief Empty constructor (mainly for STL containers). */
        CCSBinding();

        /** @brief Constructs a CCSBinding name[params] := process */
        CCSBinding(std::string name, std::vector<std::string> params, std::shared_ptr<CCSProcess> process);

        /** @brief Returns the name of the process. */
        std::string getName() const;

        /** @brief Returns the parameters of the process. */
        std::vector<std::string> getParams() const;

        /** @brief Returns the process. */
        std::shared_ptr<CCSProcess> getProcess() const;

        /** @brief Prints the CCSBinding to an output stream. */
        void print(std::ostream& out) const;
    };

    /** @brief Prints a CCSBinding to an output stream */
    std::ostream& operator<< (std::ostream& out, const CCSBinding& b);

    /** @brief Represents a CCS program, consisting of a process and bindings to named processes.*/
    class CCSProgram
    {
    private:
        std::map<std::string, CCSBinding> bindings;
        std::shared_ptr<CCSProcess> process;

    public:
        /** @brief Add a binding to a named process. */
        void addBinding(std::string name, std::vector<std::string> params, std::shared_ptr<CCSProcess> process);

        /** @brief Set the main process. */
        void setProcess(std::shared_ptr<CCSProcess> process);

        /** @brief Get a named process. */
        std::shared_ptr<CCSProcess> get(std::string name, std::vector<int> args, bool fold = true) const;

        /** @brief Returns all bindings. */
        std::map<std::string, CCSBinding> getBindings() const;

        /** @brief Returns the main process. */
        std::shared_ptr<CCSProcess> getProcess() const;

        /** @brief Prints the CCSProgram to an output stream. */
        void print(std::ostream& out) const;
    };

    /** @brief Prints a CCSProgram to an output stream */
    std::ostream& operator<< (std::ostream& out, const CCSProgram& p);

    /** @brief Comparator functor to use CCS object pointers in STL containers. */
    template<typename T>
    class PtrCmp
    {
    public:
        bool operator() (const std::shared_ptr<T>& p1, const std::shared_ptr<T>& p2) const
        {
            return *p1 < *p2;
        }
    };

    class CCSException : public std::runtime_error
    {
    public:
        CCSException(std::string message);
    };

    class CCSProcessException : public CCSException
    {
    private:
        std::shared_ptr<CCSProcess> process;

    public:
        CCSProcessException(std::shared_ptr<CCSProcess> process, std::string message);
        std::shared_ptr<CCSProcess> getProcess() const;
    };

    class CCSRecursionException : public CCSProcessException
    {
    private:
        std::string name;

    public:
        CCSRecursionException(std::shared_ptr<CCSProcessName> process, std::string message);
        std::string getName() const;
    };

    class CCSExpException : public CCSException
    {
    private:
        std::shared_ptr<CCSExp> exp;

    public:
        CCSExpException(std::shared_ptr<CCSExp> exp, std::string message);
        std::shared_ptr<CCSExp> getExp() const;
        void setExp(std::shared_ptr<CCSExp> exp);
    };

    class CCSUnboundException : public CCSExpException
    {
    private:
        std::string id;

    public:
        CCSUnboundException(std::shared_ptr<CCSExp> exp, std::string id, std::string message);
        std::string getId() const;
    };

    class CCSUndefinedException : public CCSExpException
    {
    public:
        CCSUndefinedException(std::shared_ptr<CCSExp> exp, std::string message);
    };
}

#include "ccsexp.h"
#include "ccsprocess.h"

#endif //CCSPP_CCS_H_INCLUDED
