#ifndef CCSPP_CCSPROCESS_H_INCLUDED
#define CCSPP_CCSPROCESS_H_INCLUDED

#include <memory>
#include <string>
#include <set>
#include <iostream>

namespace ccspp
{
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

        friend class CCSNull;
        friend class CCSTerm;
        friend class CCSProcessName;
        friend class CCSPrefix;
        friend class CCSChoice;
        friend class CCSParallel;
        friend class CCSRestrict;
        friend class CCSSequential;

    private:
        Type type;

    protected:
        /** @brief Internal comparison function.
            This should only be called if p is of the same type as this CCSProcess.
        */
        virtual int compare(CCSProcess* p) const = 0;

        /** @brief Internal method to calculates all possible transition of that process. */
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen) = 0;

    public:
        /** @brief Constructor. */
        CCSProcess(Type type);

        /** @brief Prints the CCSProcess to an output stream. */
        virtual void print(std::ostream& out) const = 0;

        /** @brief Visitor acceptor method. */
        virtual void accept(CCSVisitor<void>* v) = 0;

        /** @brief Returns the type of the process. */
        Type getType() const;

        /** @brief Calculates all possible transition of that process.
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
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);

    public:
        CCSNull();
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    class CCSTerm : public CCSProcess, public std::enable_shared_from_this<CCSTerm>
    {
    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);

    public:
        CCSTerm();
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    class CCSProcessName : public CCSProcess, public std::enable_shared_from_this<CCSProcessName>
    {
    private:
        std::string name;

    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);

    public:
        CCSProcessName(std::string name);
        std::string getName() const;
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    class CCSPrefix : public CCSProcess, public std::enable_shared_from_this<CCSPrefix>
    {
    private:
        CCSAction act;
        std::shared_ptr<CCSProcess> p;

    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);

    public:
        CCSPrefix(CCSAction act, std::shared_ptr<CCSProcess> p);
        CCSAction getAction() const;
        std::shared_ptr<CCSProcess> getProcess() const;

        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    class CCSChoice : public CCSProcess, public std::enable_shared_from_this<CCSChoice>
    {
    private:
        std::shared_ptr<CCSProcess> left;
        std::shared_ptr<CCSProcess> right;

    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);

    public:
        CCSChoice(std::shared_ptr<CCSProcess> left, std::shared_ptr<CCSProcess> right);
        std::shared_ptr<CCSProcess> getLeft() const;
        std::shared_ptr<CCSProcess> getRight() const;

        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    class CCSParallel : public CCSProcess, public std::enable_shared_from_this<CCSParallel>
    {
    private:
        std::shared_ptr<CCSProcess> left;
        std::shared_ptr<CCSProcess> right;

    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);

    public:
        CCSParallel(std::shared_ptr<CCSProcess> left, std::shared_ptr<CCSProcess> right);
        std::shared_ptr<CCSProcess> getLeft() const;
        std::shared_ptr<CCSProcess> getRight() const;

        virtual void print(std::ostream& out) const;
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
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);

    public:
        CCSRestrict(std::shared_ptr<CCSProcess> p, std::set<CCSAction> r, bool complement = false);
        std::shared_ptr<CCSProcess> getProcess() const;
        std::set<CCSAction> getR() const;
        bool isComplement() const;

        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    class CCSSequential : public CCSProcess, public std::enable_shared_from_this<CCSSequential>
    {
    private:
        std::shared_ptr<CCSProcess> left;
        std::shared_ptr<CCSProcess> right;

    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, std::set<std::string> seen);

    public:
        CCSSequential(std::shared_ptr<CCSProcess> left, std::shared_ptr<CCSProcess> right);
        std::shared_ptr<CCSProcess> getLeft() const;
        std::shared_ptr<CCSProcess> getRight() const;

        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };
}

#endif //CCSPP_CCSPROCESS_H_INCLUDED
