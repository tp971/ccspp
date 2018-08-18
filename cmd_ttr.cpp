#include "cmd_ttr.h"
#include "main.h"
#include <stack>
#include <sstream>

using namespace std;
using namespace ccspp;

bool dfs_limit(CCSProgram& program, shared_ptr<CCSProcess> p, int depth, set<string>& seen,
               set<shared_ptr<CCSProcess>, PtrCmp>& visited, deque<CCSTransition>& trace)
{
    if(depth <= 0)
        return false;
    if(visited.count(p))
        return true;

    set<CCSTransition>&& trans = p->getTransitions(program);
    if(trans.empty())
    {
        stringstream ss;
        ss << "[";
        bool first = true;
        for(CCSTransition t : trace)
        {
            if(!first)
                ss << ", ";
            first = false;
            ss << t.getAction();
        }
        ss << "]";
        string res = ss.str();
        if(!seen.count(res))
        {
            cout << res << endl;
            seen.insert(res);
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

bool dfs_limit(CCSProgram& program, shared_ptr<CCSProcess> p, int depth, set<string>& seen)
{
    set<shared_ptr<CCSProcess>, PtrCmp> visited;
    deque<CCSTransition> trace;
    return dfs_limit(program, p, depth, seen, visited, trace);
}

int cmd_ttr(CCSProgram& program)
{
    int depth = 0;
    set<string> seen;
    while(depth <= opt_max_depth || opt_max_depth < 1)
        if(dfs_limit(program, program.getProcess(), depth++, seen))
            break;
    return 0;
}
