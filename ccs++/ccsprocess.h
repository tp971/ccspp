#ifndef CCSPP_CCSPROCESS_H_INCLUDED
#define CCSPP_CCSPROCESS_H_INCLUDED

#include <memory>
#include <string>
#include <set>
#include <vector>
#include <iostream>

namespace ccspp
{
    /** @brief Represents a CCS process. */
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
            SEQUENTIAL,     /**< sequential operator */
            WHEN            /**< conditional process */
        };

        friend class CCSNull;
        friend class CCSTerm;
        friend class CCSProcessName;
        friend class CCSPrefix;
        friend class CCSChoice;
        friend class CCSParallel;
        friend class CCSRestrict;
        friend class CCSSequential;
        friend class CCSWhen;

    private:
        Type type;

    protected:
        /** @brief Internal comparison function.
            This should only be called if p is of the same type as this CCSProcess.
        */
        virtual int compare(CCSProcess* p) const = 0;

        /** @brief Internal method to calculates all possible transition of that process. */
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, bool fold, std::set<std::string> seen) = 0;

    public:
        /** @brief Constructor. */
        CCSProcess(Type type);

        /** @brief Returns the type of the process. */
        Type getType() const;

        /** @brief Calculates all possible transition of that process.
            @param program The CCSProgram of the process.
            @param fold True if constant expression should be folded to constants.
            @returns A set of CCSTransitions.
            @throws CCSRecursionException if there is an unguarded exception
                leading to a recursion in transition inference.
        */
        std::set<CCSTransition> getTransitions(CCSProgram& program, bool fold = true);

        /** @brief Substitutes an identifier by a value. */
        virtual std::shared_ptr<CCSProcess> subst(std::string id, int val, bool fold = true) = 0;

        /** @brief Prints the CCSProcess to an output stream. */
        virtual void print(std::ostream& out) const = 0;

        /** @brief Visitor acceptor method. */
        virtual void accept(CCSVisitor<void>* v) = 0;

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

    /** @brief Represents the null process. */
    class CCSNull : public CCSProcess, public std::enable_shared_from_this<CCSNull>
    {
    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, bool fold, std::set<std::string> seen);

    public:
        CCSNull();
        virtual std::shared_ptr<CCSProcess> subst(std::string id, int val, bool fold = true);
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    /** @brief Represents the terminated process. */
    class CCSTerm : public CCSProcess, public std::enable_shared_from_this<CCSTerm>
    {
    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, bool fold, std::set<std::string> seen);

    public:
        CCSTerm();
        virtual std::shared_ptr<CCSProcess> subst(std::string id, int val, bool fold = true);
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    /** @brief Represents a process instantiation. */
    class CCSProcessName : public CCSProcess, public std::enable_shared_from_this<CCSProcessName>
    {
    private:
        std::string name;
        std::vector<std::shared_ptr<CCSExp>> args;

    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, bool fold, std::set<std::string> seen);

    public:
        CCSProcessName(std::string name, std::vector<std::shared_ptr<CCSExp>> args);
        std::string getName() const;
        std::vector<std::shared_ptr<CCSExp>> getArgs();

        virtual std::shared_ptr<CCSProcess> subst(std::string id, int val, bool fold = true);
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    /** @brief Represents a prefix process. */
    class CCSPrefix : public CCSProcess, public std::enable_shared_from_this<CCSPrefix>
    {
    private:
        CCSAction act;
        std::shared_ptr<CCSProcess> p;

    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, bool fold, std::set<std::string> seen);

    public:
        CCSPrefix(CCSAction act, std::shared_ptr<CCSProcess> p);
        CCSAction getAction() const;
        std::shared_ptr<CCSProcess> getProcess() const;

        virtual std::shared_ptr<CCSProcess> subst(std::string id, int val, bool fold = true);
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    /** @brief Represents the choice operator. */
    class CCSChoice : public CCSProcess, public std::enable_shared_from_this<CCSChoice>
    {
    private:
        std::shared_ptr<CCSProcess> left;
        std::shared_ptr<CCSProcess> right;

    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, bool fold, std::set<std::string> seen);

    public:
        CCSChoice(std::shared_ptr<CCSProcess> left, std::shared_ptr<CCSProcess> right);
        std::shared_ptr<CCSProcess> getLeft() const;
        std::shared_ptr<CCSProcess> getRight() const;

        virtual std::shared_ptr<CCSProcess> subst(std::string id, int val, bool fold = true);
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    /** @brief Represents the parallel operator. */
    class CCSParallel : public CCSProcess, public std::enable_shared_from_this<CCSParallel>
    {
    private:
        std::shared_ptr<CCSProcess> left;
        std::shared_ptr<CCSProcess> right;

    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, bool fold, std::set<std::string> seen);

    public:
        CCSParallel(std::shared_ptr<CCSProcess> left, std::shared_ptr<CCSProcess> right);
        std::shared_ptr<CCSProcess> getLeft() const;
        std::shared_ptr<CCSProcess> getRight() const;

        virtual std::shared_ptr<CCSProcess> subst(std::string id, int val, bool fold = true);
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    /** @brief Represents the restriction operator. */
    class CCSRestrict : public CCSProcess, public std::enable_shared_from_this<CCSRestrict>
    {
    private:
        std::shared_ptr<CCSProcess> p;
        std::set<CCSAction> r;
        bool complement;

    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, bool fold, std::set<std::string> seen);

    public:
        CCSRestrict(std::shared_ptr<CCSProcess> p, std::set<CCSAction> r, bool complement = false);
        std::shared_ptr<CCSProcess> getProcess() const;
        std::set<CCSAction> getR() const;
        bool isComplement() const;

        virtual std::shared_ptr<CCSProcess> subst(std::string id, int val, bool fold = true);
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    /** @brief Represents the sequential operator. */
    class CCSSequential : public CCSProcess, public std::enable_shared_from_this<CCSSequential>
    {
    private:
        std::shared_ptr<CCSProcess> left;
        std::shared_ptr<CCSProcess> right;

    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, bool fold, std::set<std::string> seen);

    public:
        CCSSequential(std::shared_ptr<CCSProcess> left, std::shared_ptr<CCSProcess> right);
        std::shared_ptr<CCSProcess> getLeft() const;
        std::shared_ptr<CCSProcess> getRight() const;

        virtual std::shared_ptr<CCSProcess> subst(std::string id, int val, bool fold = true);
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };

    /** @brief Represents the when operator. */
    class CCSWhen : public CCSProcess, public std::enable_shared_from_this<CCSWhen>
    {
    private:
        std::shared_ptr<CCSExp> cond;
        std::shared_ptr<CCSProcess> p;

    protected:
        virtual int compare(CCSProcess* p) const;
        virtual std::set<CCSTransition> getTransitions(CCSProgram& program, bool fold, std::set<std::string> seen);

    public:
        CCSWhen(std::shared_ptr<CCSExp> cond, std::shared_ptr<CCSProcess> p);
        std::shared_ptr<CCSExp> getCond() const;
        std::shared_ptr<CCSProcess> getProcess() const;

        virtual std::shared_ptr<CCSProcess> subst(std::string id, int val, bool fold = true);
        virtual void print(std::ostream& out) const;
        virtual void accept(CCSVisitor<void>* v);
    };
}

#endif //CCSPP_CCSPROCESS_H_INCLUDED
