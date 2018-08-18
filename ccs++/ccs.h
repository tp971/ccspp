#ifndef CCSPP_CCS_H_INCLUDED
#define CCSPP_CCS_H_INCLUDED

#include <memory>
#include <string>
#include <set>
#include <map>
#include <iostream>

//TODO: implement CCSvp

namespace ccspp
{
    class CCSProcess;

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

    public:
        /** @brief Constructs a CCSAction */
        CCSAction(Type type, std::string name = "");

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

    /** @brief An abstract class which represents a CCS process */
    class CCSProcess
    {
    public:
        /** @brief The type of the CCS process */
        enum Type
        {
            CCSNULL = 0,    /**< the null process */
            TERM,           /**< the terminating process */
            PROCESSNAME,    /**< a process name */
            PREFIX,         /**< prefix operator */
            CHOICE,         /**< choice operator */
            PARALLEL,       /**< parallel operator */
            RESTRICT,       /**< restriction operator */
            SEQUENTIAL      /**< sequential operator */
        };

    private:
        Type type;

    protected:
        /** @brief Internal comparison function.
            This should only be called if p is of the same type as this CCSProcess.
        */
        virtual int compare(CCSProcess* p) const = 0;

    public:
        /** @brief Prints the CCProcess to an ouptu stream. */
        virtual void print(std::ostream& out) const = 0;

        /** @brief Internal method to calculates all possible transition of that process.
            \note This method should not be used outside the class. This method is only public due to some restriction of C++ to the protected access modifier.
        */
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen) = 0;

        /** @brief Visitor acceptor method. */
        virtual void accept(CCSVisitor<void>* v) = 0;

        /** @brief Constructor. */
        CCSProcess(Type type);

        /** @brief Returns the type of the process. */
        Type getType() const;

        /** @brief Calculates all possible transition of that process.
            Hello?
            @param program The CCSProgram of the process.
            @returns A set of CCSTransitions.
            @throws CCSRecursionException if there is an unguarded exception
                leading to a recursion in transition inference.
        */
        std::set<CCSTransition> getTransitions(CCSProgram& program);

        /** @brief Returns a string representing this CCSProcess. */
        operator std::string() const;

        /** @brief Compares this CCSProcess to another instance.
            \returns -1 if this < p, 1 if this > p, 0 else.
        */
        int compare(CCSProcess& p) const;

        /** @brief Comparator to use in STL containers. */
        bool operator< (CCSProcess& p) const;
    };

    /** @brief Prints a CCSProcess to an output stream */
    std::ostream& operator<< (std::ostream& out, const CCSProcess& p);

    /** @brief Comparator functor to use CCSProcess pointers in STL containers. */
    class PtrCmp
    {
    public:
        bool operator() (const std::shared_ptr<CCSProcess>& p1, const std::shared_ptr<CCSProcess>& p2) const;
    };

    class CCSNull : public CCSProcess, public std::enable_shared_from_this<CCSNull>
    {
    protected:
        virtual int compare(CCSProcess* p) const;

    public:
        CCSNull();
        virtual void print(std::ostream& out) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);
        virtual void accept(CCSVisitor<void>* v);
    };

    class CCSTerm : public CCSProcess, public std::enable_shared_from_this<CCSTerm>
    {
    protected:
        virtual int compare(CCSProcess* p) const;

    public:
        CCSTerm();
        virtual void print(std::ostream& out) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);
        virtual void accept(CCSVisitor<void>* v);
    };

    class CCSProcessName : public CCSProcess, public std::enable_shared_from_this<CCSProcessName>
    {
    private:
        std::string name;

    protected:
        virtual int compare(CCSProcess* p) const;

    public:
        CCSProcessName(std::string name);
        std::string getName() const;
        virtual void print(std::ostream& out) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);
        virtual void accept(CCSVisitor<void>* v);
    };

    class CCSPrefix : public CCSProcess, public std::enable_shared_from_this<CCSPrefix>
    {
    private:
        CCSAction act;
        std::shared_ptr<CCSProcess> p;

    protected:
        virtual int compare(CCSProcess* p) const;

    public:
        CCSPrefix(CCSAction act, std::shared_ptr<CCSProcess> p);
        CCSAction getAction() const;
        std::shared_ptr<CCSProcess> getProcess() const;

        virtual void print(std::ostream& out) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);
        virtual void accept(CCSVisitor<void>* v);
    };

    class CCSChoice : public CCSProcess, public std::enable_shared_from_this<CCSChoice>
    {
    private:
        std::shared_ptr<CCSProcess> left;
        std::shared_ptr<CCSProcess> right;

    protected:
        virtual int compare(CCSProcess* p) const;

    public:
        CCSChoice(std::shared_ptr<CCSProcess> left, std::shared_ptr<CCSProcess> right);
        std::shared_ptr<CCSProcess> getLeft() const;
        std::shared_ptr<CCSProcess> getRight() const;

        virtual void print(std::ostream& out) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);
        virtual void accept(CCSVisitor<void>* v);
    };

    class CCSParallel : public CCSProcess, public std::enable_shared_from_this<CCSParallel>
    {
    private:
        std::shared_ptr<CCSProcess> left;
        std::shared_ptr<CCSProcess> right;

    protected:
        virtual int compare(CCSProcess* p) const;

    public:
        CCSParallel(std::shared_ptr<CCSProcess> left, std::shared_ptr<CCSProcess> right);
        std::shared_ptr<CCSProcess> getLeft() const;
        std::shared_ptr<CCSProcess> getRight() const;

        virtual void print(std::ostream& out) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);
        virtual void accept(CCSVisitor<void>* v);
    };

    class CCSRestrict : public CCSProcess, public std::enable_shared_from_this<CCSRestrict>
    {
    private:
        std::shared_ptr<CCSProcess> p;
        std::set<CCSAction> r;
        bool complement;

    protected:
        virtual int compare(CCSProcess* p) const;

    public:
        CCSRestrict(std::shared_ptr<CCSProcess> p, std::set<CCSAction> r, bool complement = false);
        std::shared_ptr<CCSProcess> getProcess() const;
        std::set<CCSAction> getR() const;
        bool isComplement() const;

        virtual void print(std::ostream& out) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);
        virtual void accept(CCSVisitor<void>* v);
    };

    class CCSSequential : public CCSProcess, public std::enable_shared_from_this<CCSSequential>
    {
    private:
        std::shared_ptr<CCSProcess> left;
        std::shared_ptr<CCSProcess> right;

    protected:
        virtual int compare(CCSProcess* p) const;

    public:
        CCSSequential(std::shared_ptr<CCSProcess> left, std::shared_ptr<CCSProcess> right);
        std::shared_ptr<CCSProcess> getLeft() const;
        std::shared_ptr<CCSProcess> getRight() const;

        virtual void print(std::ostream& out) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);
        virtual void accept(CCSVisitor<void>* v);
    };

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

#endif //CCSPP_CCS_H_INCLUDED
