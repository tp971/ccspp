#include "main.h"
#include "cmd_graph.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace ccspp;

int cmd_actions(CCSProgram& program)
{
    set<CCSAction> actions;
    set<shared_ptr<CCSProcess>, PtrCmp> visited;
    set<shared_ptr<CCSProcess>, PtrCmp> frontier;
    
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
                    cerr << "warning: " << ex.what() << endl;
                else
                {
                    cerr << "error: " << ex.what() << endl;
                    return 1;
                }
            }

            for(CCSTransition t : trans)
            {
                CCSAction act = t.getAction();
                if(!actions.count(act))
                {
                    cout << act << endl;
                    actions.insert(act);
                }
                shared_ptr<CCSProcess> p2 = t.getTo();
                if(!visited.count(p2))
                    frontier2.insert(p2);
            }
        }

        depth++;
        frontier = move(frontier2);
    }
    return 0;
}
