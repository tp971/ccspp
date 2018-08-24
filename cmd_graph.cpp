#include "main.h"
#include "cmd_graph.h"

#include <iostream>
#include <memory>
#include <iomanip>

using namespace std;
using namespace ccspp;

void printNode(int id, CCSProcess& p, bool error, bool explored, bool term)
{
    cout << "    p" << id << " [";
    if(opt_omit_names)
        cout << "label=\"\"";
    else
        cout << "label=" << quoted((string)p);
    if(term)
        cout << ",shape=box";
    if(!explored)
        cout << ",style=dashed";
    if(error)
        cout << ",color=red";
    cout << "];" << endl;
}

int cmd_graph(CCSProgram& program)
{
    cout << "digraph lts {" << endl;

    int nodes_id = 0;
    map<shared_ptr<CCSProcess>, int, PtrCmp> nodes;
    set<shared_ptr<CCSProcess>, PtrCmp> frontier;
    
    shared_ptr<CCSProcess> start = program.getProcess();
    nodes[start] = nodes_id++;
    frontier.insert(start);
    cout << "    start [shape=point];" << endl;
    cout << "    start -> p0;" << endl;

    int depth = 0;
    while((opt_max_depth < 0 || depth < opt_max_depth) && !frontier.empty())
    {
        set<shared_ptr<CCSProcess>, PtrCmp> frontier2;
        for(shared_ptr<CCSProcess> p : frontier)
        {
            int id = nodes[p];
            set<CCSTransition> trans;
            try
            {
                trans = p->getTransitions(program, !opt_no_fold);
                printNode(id, *p, false, true, trans.empty());
            }
            catch(CCSException& ex)
            {
                if(opt_ignore_error)
                {
                    cerr << "warning: " << ex.what() << endl;
                    printNode(id, *p, true, true, false);
                }
                else
                {
                    cerr << "error: " << ex.what() << endl;
                    return 1;
                }
            }

            for(CCSTransition t : trans)
            {
                shared_ptr<CCSProcess> p2 = t.getTo();
                int id2;
                if(nodes.count(p2))
                    id2 = nodes[p2];
                else
                {
                    id2 = nodes_id++;
                    nodes[p2] = id2;
                    frontier2.insert(p2);
                }

                cout << "    p" << id << " -> p" << id2 << " [label=" << quoted((string)t.getAction()) << "];" << endl;
            }
        }

        depth++;
        frontier = move(frontier2);
    }
    for(shared_ptr<CCSProcess> p : frontier)
        printNode(nodes[p], *p, false, false, false);
    cout << "}" << endl;
    return 0;
}
