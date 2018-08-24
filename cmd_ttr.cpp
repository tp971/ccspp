#include "cmd_ttr.h"
#include "main.h"
#include <stack>
#include <vector>

using namespace std;
using namespace ccspp;

bool dfs_limit(CCSProgram& program, shared_ptr<CCSProcess> p, int depth, set<vector<CCSAction>>& seen,
               set<shared_ptr<CCSProcess>, PtrCmp>& visited, deque<CCSTransition>& trace)
{
    if(depth <= 0)
        return false;
    if(visited.count(p))
        return true;

    set<CCSTransition> trans;

    try
    {
        trans = p->getTransitions(program, !opt_no_fold);
    }
    catch(CCSException& ex)
    {
        if(opt_ignore_error)
        {
            cerr << "warning: " << ex.what() << endl;
            return true;
        }
        else
        {
            cerr << "error: " << ex.what() << endl;
            throw;
        }
    }

    if(trans.empty())
    {
        vector<CCSAction> next;
        for(CCSTransition t : trace)
            next.push_back(t.getAction());
        if(!seen.count(next))
        {
            seen.insert(next);
            if(opt_full_paths)
            {
                cout << *trace.front().getFrom();
                for(CCSTransition t : trace)
                    cout << "   --( " << t.getAction() << " )->   " << *t.getTo();
                cout << endl;
            }
            else
            {
                cout << "[";
                bool first = true;
                for(CCSTransition t : trace)
                {
                    if(!first)
                        cout << ", ";
                    first = false;
                    cout << t.getAction();
                }
                cout << "] ~> " << *p << endl;
            }
        }
        return true;
    }

    visited.insert(p);
    bool res = true;
    for(const CCSTransition& next : trans)
    {
        trace.push_back(next);
        res &= dfs_limit(program, next.getTo(), depth - 1, seen, visited, trace);
        trace.pop_back();
    }
    visited.erase(p);
    return res;
}

bool dfs_limit(CCSProgram& program, shared_ptr<CCSProcess> p, int depth, set<vector<CCSAction>>& seen)
{
    set<shared_ptr<CCSProcess>, PtrCmp> visited;
    deque<CCSTransition> trace;
    return dfs_limit(program, p, depth, seen, visited, trace);
}

int cmd_ttr(CCSProgram& program)
{
    try
    {
        int depth = 0;
        set<vector<CCSAction>> seen;
        while(depth <= opt_max_depth || opt_max_depth < 1)
            if(dfs_limit(program, program.getProcess(), depth++, seen))
                break;
    }
    catch(CCSException& ex)
    {
        return 1;
    }
    return 0;
}
