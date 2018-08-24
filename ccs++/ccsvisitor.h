#ifndef CCSPP_CCSVISITOR_H_INCLUDED
#define CCSPP_CCSVISITOR_H_INCLUDED

#include "ccs.h"

namespace ccspp
{
    template<>
    class CCSVisitor<void,void>
    {
    public:
        virtual void visit(CCSNull* p) = 0;
        virtual void visit(CCSTerm* p) = 0;
        virtual void visit(CCSProcessName* p) = 0;
        virtual void visit(CCSPrefix* p) = 0;
        virtual void visit(CCSChoice* p) = 0;
        virtual void visit(CCSParallel* p) = 0;
        virtual void visit(CCSRestrict* p) = 0;
        virtual void visit(CCSSequential* p) = 0;
        virtual void visit(CCSWhen* p) = 0;

        void visit(CCSProcess& p);
        void visit(CCSProcess* p);
        void visit(std::shared_ptr<CCSProcess> p);
    };

    template<typename V>
    class CCSVisitor<void,V> : public CCSVisitor<void>
    {
    private:
        V* v;

    public:
        virtual void visit(CCSNull* p)
        { _visit(p, *v); }

        virtual void visit(CCSTerm* p)
        { _visit(p, *v); }

        virtual void visit(CCSProcessName* p)
        { _visit(p, *v); }

        virtual void visit(CCSPrefix* p)
        { _visit(p, *v); }

        virtual void visit(CCSChoice* p)
        { _visit(p, *v); }

        virtual void visit(CCSParallel* p)
        { _visit(p, *v); }

        virtual void visit(CCSRestrict* p)
        { _visit(p, *v); }

        virtual void visit(CCSSequential* p)
        { _visit(p, *v); }

        virtual void visit(CCSWhen* p)
        { _visit(p, *v); }

        virtual void _visit(CCSNull* p, V v) = 0;
        virtual void _visit(CCSTerm* p, V v) = 0;
        virtual void _visit(CCSProcessName* p, V v) = 0;
        virtual void _visit(CCSPrefix* p, V v) = 0;
        virtual void _visit(CCSChoice* p, V v) = 0;
        virtual void _visit(CCSParallel* p, V v) = 0;
        virtual void _visit(CCSRestrict* p, V v) = 0;
        virtual void _visit(CCSSequential* p, V v) = 0;
        virtual void _visit(CCSWhen* p, V v) = 0;

        void vvisit(CCSProcess& p, V v)
        { this->v = &v; p.accept(this); }

        void vvisit(CCSProcess* p, V v)
        { this->v = &v; p->accept(this); }

        void vvisit(std::shared_ptr<CCSProcess> p, V v)
        { this->v = &v; p->accept(this); }
    };

    template<typename T>
    class CCSVisitor<T,void> : public CCSVisitor<void>
    {
    private:
        T ret;

    public:
        virtual void visit(CCSNull* p)
        { ret = _visit(p); }

        virtual void visit(CCSTerm* p)
        { ret = _visit(p); }

        virtual void visit(CCSProcessName* p)
        { ret = _visit(p); }

        virtual void visit(CCSPrefix* p)
        { ret = _visit(p); }

        virtual void visit(CCSChoice* p)
        { ret = _visit(p); }

        virtual void visit(CCSParallel* p)
        { ret = _visit(p); }

        virtual void visit(CCSRestrict* p)
        { ret = _visit(p); }

        virtual void visit(CCSSequential* p)
        { ret = _visit(p); }

        virtual void visit(CCSWhen* p)
        { ret = _visit(p); }

        virtual T _visit(CCSNull* p) = 0;
        virtual T _visit(CCSTerm* p) = 0;
        virtual T _visit(CCSProcessName* p) = 0;
        virtual T _visit(CCSPrefix* p) = 0;
        virtual T _visit(CCSChoice* p) = 0;
        virtual T _visit(CCSParallel* p) = 0;
        virtual T _visit(CCSRestrict* p) = 0;
        virtual T _visit(CCSSequential* p) = 0;
        virtual T _visit(CCSWhen* p) = 0;

        T vvisit(CCSProcess& p)
        { p.accept(this); return ret; }

        T vvisit(CCSProcess* p)
        { p->accept(this); return ret; }

        T vvisit(std::shared_ptr<CCSProcess> p)
        { p->accept(this); return ret; }
    };

    template<typename T, typename V>
    class CCSVisitor : public CCSVisitor<void>
    {
    private:
        T ret;
        V* v;

    public:
        virtual void visit(CCSNull* p)
        { ret = _visit(p, *v); }

        virtual void visit(CCSTerm* p)
        { ret = _visit(p, *v); }

        virtual void visit(CCSProcessName* p)
        { ret = _visit(p, *v); }

        virtual void visit(CCSPrefix* p)
        { ret = _visit(p, *v); }

        virtual void visit(CCSChoice* p)
        { ret = _visit(p, *v); }

        virtual void visit(CCSParallel* p)
        { ret = _visit(p, *v); }

        virtual void visit(CCSRestrict* p)
        { ret = _visit(p, *v); }

        virtual void visit(CCSSequential* p)
        { ret = _visit(p, *v); }

        virtual void visit(CCSWhen* p)
        { ret = _visit(p, *v); }

        virtual T _visit(CCSNull* p, V v) = 0;
        virtual T _visit(CCSTerm* p, V v) = 0;
        virtual T _visit(CCSProcessName* p, V v) = 0;
        virtual T _visit(CCSPrefix* p, V v) = 0;
        virtual T _visit(CCSChoice* p, V v) = 0;
        virtual T _visit(CCSParallel* p, V v) = 0;
        virtual T _visit(CCSRestrict* p, V v) = 0;
        virtual T _visit(CCSSequential* p, V v) = 0;
        virtual T _visit(CCSWhen* p, V v) = 0;

        T vvisit(CCSProcess& p, V v)
        { this->v = &v; p.accept(this); return ret; }

        T vvisit(CCSProcess* p, V v)
        { this->v = &v; p->accept(this); return ret; }

        T vvisit(std::shared_ptr<CCSProcess> p, V v)
        { this->v = &v; p->accept(this); return ret; }
    };
}

#endif //CCSPP_CCSVISIOR_H_INCLUDED
