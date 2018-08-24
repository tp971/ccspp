#include "ccs++/ccs.h"
#include "ccs++/ccsparser.h"

#include "cmd_graph.h"
#include "cmd_random.h"
#include "cmd_actions.h"
#include "cmd_dead.h"
#include "cmd_ttr.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <cli++/cli.h>

using namespace std;
using namespace clipp;
using namespace ccspp;

int opt_max_depth = -1;
bool opt_ignore_error = false;
bool opt_no_fold = false;
bool opt_full_paths = false;
bool opt_omit_names = false;

void printUsage(char* argv0)
{
    cout << "Usage: " << argv0 << " [options] <command> [input-file]" << endl;
}

void printHelp(char* argv0)
{
    printUsage(argv0);
    cout <<
        "If no input file or \"-\" is given, reads from standard input." << endl <<
        endl <<
        "commands:" << endl <<
        "    graph" << endl <<
        "        Output a graph of the LTS in DOT format" << endl <<
        "    random" << endl <<
        "        Traverse a random path through the LTS" << endl <<
        "    actions" << endl <<
        "        Search for all actions" << endl <<
        "    dead" << endl <<
        "        Search for deadlocks (states with no outgoing transitions)" << endl <<
        "    ttr" << endl <<
        "        Search for terminating traces" << endl <<
        "    echo" << endl <<
        "        Outputs the CCS program (for debugging)" << endl <<
        endl <<
        "options (general):" << endl <<
        "    -d, --depth" << endl <<
        "        Limits the depth of LTS exploration" << endl <<
        "    -i, --ignore-error" << endl <<
        "        Ignores errors during LTS exploration" << endl <<
        "    --no-fold" << endl <<
        "        Do not fold constant expressions to constants" << endl <<
        "    --full-paths" << endl <<
        "        Show full paths instead traces (including all states)" << endl <<
        "    -h, --help" << endl <<
        "        Print this help message" << endl <<
        endl <<
        "options (graph):" << endl <<
        "    --omit-names" << endl <<
        "        Does not print the CCS process expressions into the nodes" << endl;
}

int main(int argc, char** argv)
{
    CLIParser cli;
    CLIOpt cli_depth = cli.addOpt('d', "depth", 1);
    CLIOpt cli_ignore_error = cli.addOpt('i', "ignore-error");
    CLIOpt cli_no_fold = cli.addOpt("no-fold");
    CLIOpt cli_full_paths = cli.addOpt("full-paths");
    CLIOpt cli_help = cli.addOpt('h', "help");
    CLIOpt cli_omit_names = cli.addOpt("omit-names");

    enum Command { NONE, GRAPH, RANDOM, ACTIONS, DEAD, TTR, ECHO };

    Command cmd = NONE;
    std::string inputfile;

    try
    {
        for(CLIArg arg : cli.parse(argc, argv))
            if(arg.opt == cli_help)
            {
                printHelp(argv[0]);
                return 0;
            }
            else if(arg.opt == cli_depth)
            {
                try
                {
                    opt_max_depth = stoi(arg.params[0]);
                }
                catch(exception& ex)
                {
                    cout << "invalid number: " << arg.params[0] << endl;
                    return 1;
                }
            }
            else if(arg.opt == cli_ignore_error)
                opt_ignore_error = true;
            else if(arg.opt == cli_no_fold)
                opt_no_fold = true;
            else if(arg.opt == cli_full_paths)
                opt_full_paths = true;
            else if(arg.opt == cli_omit_names)
                opt_omit_names = true;
            else if(arg.opt != CLINonOpt)
            {
                cerr << "error: command line option not implemented" << endl;
                return 1;
            }
            else if(cmd == NONE)
            {
                if(arg.str == "graph")
                    cmd = GRAPH;
                else if(arg.str == "random")
                    cmd = RANDOM;
                else if(arg.str == "actions")
                    cmd = ACTIONS;
                else if(arg.str == "dead")
                    cmd = DEAD;
                else if(arg.str == "ttr")
                    cmd = TTR;
                else if(arg.str == "echo")
                    cmd = ECHO;
                else
                {
                    cerr << "error: unknown command: " << arg.str << endl;
                    return 1;
                }
            }
            else if(inputfile == "")
                inputfile = arg.str;
            else
            {
                cerr << "error: more than one input file given" << endl;
                return 1;
            }
    }
    catch(CLIException& ex)
    {
        cerr << "error: " << ex.what() << endl;
        return 1;
    }

    if(cmd == NONE)
    {
        printUsage(argv[0]);
        return 0;
    }

    unique_ptr<CCSProgram> program;
    try
    {
        if(inputfile == "" || inputfile == "-")
        {
            CCSParser parser(cin, "stdin");
            program = parser.parse();
        }
        else
        {
            ifstream input(inputfile);
            if(!input)
            {
                cerr << "error: could not open input file" << endl;
                return 1;
            }
            CCSParser parser(input, inputfile);
            program = parser.parse();
        }
    }
    catch(CCSParserException& ex)
    {
        cout << ex.what() << endl;
        return 1;
    }

    switch(cmd)
    {
    case GRAPH:
        return cmd_graph(*program);
    case RANDOM:
        return cmd_random(*program);
    case ACTIONS:
        return cmd_actions(*program);
    case DEAD:
        return cmd_dead(*program);
    case TTR:
        return cmd_ttr(*program);
    case ECHO:
        cout << *program;
        return 0;
    default:
        cerr << "error: this should not happen!" << endl;
        return 1;
    }
    return 0;
}
