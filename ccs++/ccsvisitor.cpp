#include "ccs.h"
#include "ccsprocess.h"
#include "ccsvisitor.h"
#include <sstream>

using namespace std;
using namespace ccspp;

void CCSVisitor<void>::visit(CCSProcess& p)
{ p.accept(this); }

void CCSVisitor<void>::visit(CCSProcess* p)
{ p->accept(this); }

void CCSVisitor<void>::visit(shared_ptr<CCSProcess> p)
{ p->accept(this); }

void CCSExpVisitor<void>::visit(CCSExp& p)
{ p.accept(this); }

void CCSExpVisitor<void>::visit(CCSExp* p)
{ p->accept(this); }

void CCSExpVisitor<void>::visit(shared_ptr<CCSExp> p)
{ p->accept(this); }
