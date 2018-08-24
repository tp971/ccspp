#include "main.h"
#include "cmd_random.h"

#include <iostream>
#include <memory>
#include <random>

using namespace std;
using namespace ccspp;

int cmd_random(CCSProgram& program)
{
    random_device r;
    mt19937_64 rng(r());

    shared_ptr<CCSProcess> p = program.getProcess();
    cout << *p << endl;
    int depth = 0;
    while(opt_max_depth < 0 || depth < opt_max_depth)
    {
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
                cout << "error: " << ex.what() << endl;
                return 1;
            }
        }
        if(trans.empty())
            break;
        uniform_int_distribution<long> dist(0, trans.size() - 1);
        CCSTransition t = *next(trans.begin(), dist(rng));
        cout << "    --( " << t.getAction() << " )->" << endl;
        p = t.getTo();
        cout << *p << endl;
        depth++;
    }
    return 0;
}
