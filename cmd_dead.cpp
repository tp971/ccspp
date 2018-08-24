#include "main.h"
#include "cmd_dead.h"

#include <iostream>
#include <memory>
#include <stack>

using namespace std;
using namespace ccspp;

int cmd_dead(CCSProgram& program)
{
    set<shared_ptr<CCSProcess>, PtrCmp> visited;
    set<shared_ptr<CCSProcess>, PtrCmp> frontier;
    map<shared_ptr<CCSProcess>, CCSTransition, PtrCmp> pred;
    
    frontier.insert(program.getProcess());

    int depth = 0;
    while((opt_max_depth < 0 || depth < opt_max_depth) && !frontier.empty())
    {
        set<shared_ptr<CCSProcess>, PtrCmp> frontier2;
        for(shared_ptr<CCSProcess> p : frontier)
        {
            visited.insert(p);

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
                    continue;
                }
                else
                {
                    cerr << "error: " << ex.what() << endl;
                    return 1;
                }
            }

            if(trans.empty())
            {
                stack<CCSTransition> path;
                while(pred.count(p))
                {
                    CCSTransition next = pred[p];
                    path.push(next);
                    p = next.getFrom();
                }

                if(opt_full_paths)
                {
                    cout << *path.top().getFrom();
                    while(!path.empty())
                    {
                        CCSTransition next = path.top();
                        path.pop();
                        cout << "   --( " << next.getAction() << " )->   " << *next.getTo();
                    }
                    cout << endl;
                }
                else
                {
                    cout << "[";
                    bool first = true;
                    while(!path.empty())
                    {
                        if(!first)
                            cout << ", ";
                        first = false;
                        cout << path.top().getAction();
                        p = path.top().getTo();
                        path.pop();
                    }
                    cout << "] ~> " << *p << endl;
                }
            }

            for(CCSTransition t : trans)
            {
                shared_ptr<CCSProcess> p2 = t.getTo();
                if(!visited.count(p2))
                {
                    frontier2.insert(p2);
                    pred[p2] = t;
                }
            }
        }

        depth++;
        frontier = move(frontier2);
    }
    return 0;
}
