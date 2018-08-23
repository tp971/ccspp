#ifndef CCSPP_CCS_H_INCLUDED
#define CCSPP_CCS_H_INCLUDED

#include <memory>
#include <string>
#include <map>
#include <iostream>

namespace ccspp
{
    class CCSProcess;
    class CCSProcessName;

    template<typename T, typename V = void>
    class CCSVisitor;

    /** @brief Represents a CCS action. */
    class CCSAction
    {
    public:
        /** @brief Represents the type of a CCS action */
        enum Type
        {
            TAU = 0,    /**< internal action */
            DELTA,      /**< termination action */
            SEND,       /**< send action */
            RECV,       /**< receive action */
            NONE        /**< action */
        };

    private:
        Type type;
        std::string name;
        //std::string identifier;

    public:
        /** @brief Constructs a CCSAction */
        CCSAction(Type type = NONE, std::string name = "");

        /** @brief Constructs a CCSAction name?identifier */
        //CCSAction(std::string name, std::string identifier);

        /** @brief Constructs a CCSAction name?identifier */
        //CCSAction(std::string name, std::string identifier);

        /** @brief Returns the type of the CCSAction. */
        Type getType() const;

        /** @brief Returns the name of the CCSAction. */
        std::string getName() const;

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
        std::shared_ptr<CCSProcess> process;

    public:
        /** @brief Empty constructor (mainly for STL containers). */
        CCSBinding();

        /** @brief Constructs a CCSBinding from a name and a process. */
        CCSBinding(std::string name, std::shared_ptr<CCSProcess> process);

        /** @brief Returns the name of the process. */
        std::string getName() const;

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
        void addBinding(std::string name, std::shared_ptr<CCSProcess> process);

        /** @brief Set the main process. */
        void setProcess(std::shared_ptr<CCSProcess> process);

        /** @brief Get a named process. */
        std::shared_ptr<CCSProcess> get(std::string name) const;

        /** @brief Returns all bindings. */
        std::map<std::string, CCSBinding> getBindings() const;

        /** @brief Returns the main process. */
        std::shared_ptr<CCSProcess> getProcess() const;

        /** @brief Prints the CCSProgram to an output stream. */
        void print(std::ostream& out) const;
    };

    /** @brief Prints a CCSProgram to an output stream */
    std::ostream& operator<< (std::ostream& out, const CCSProgram& p);

    class CCSException : public std::runtime_error
    {
    private:
        std::shared_ptr<CCSProcess> process;

    public:
        CCSException(std::shared_ptr<CCSProcess> process, std::string message);
        std::shared_ptr<CCSProcess> getProcess() const;
    };

    class CCSRecursionException : public CCSException
    {
    private:
        std::string name;

    public:
        CCSRecursionException(std::shared_ptr<CCSProcessName> process, std::string message);
        std::string getName() const;
    };
}

#include "ccsprocess.h"
//#include "ccsexp.h"

#endif //CCSPP_CCS_H_INCLUDED
