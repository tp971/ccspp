# ccs++
ccs++ is an implementation of CCS (Calculus of communicating systems) as a C++ library
including a cli for exploring the semantics of CCS processes.

CCS is a process algebra for modeling concurrent processes.
It has a well defined operational semantics and is even turing complete.

# Build & Install
Dependencies:
* a c++17 compliant c++ compiler
* [cli++](https://github.com/tp971/clipp)

The project can be built with

    make

and installed with

    make install

into `/usr/local`. To specify an own installation prefix, use

    make install PREFIX=path/to/prefix

# CCS
A brief introduction to CCS:
CCS is a language aimed to formalize and study the behaviour of concurrent systems.
The semantics of CCS is a labelled transition system:
CCS processes perform transitions to other CCS processes, labelled with an action.
For that, we write

    P   --( a )->   Q

which means "P makes a transition to Q with the action a".

As an example, let's consider the following process:

    a.(b.0 + c.(x.0 | y.0))

This process is able to perform the transition

    a.(b.0 + c.(x.0 | y.0))   --( a )->   b.0 + c.(x.0 | y.0)

This is because the prefix operator "a.P" is defined such that it always makes a transition with a to P.

The process on the right side can now perform the two transitions

    b.0 + c.(x.0 | y.0)   --( b )->   0
    b.0 + c.(x.0 | y.0)   --( c )->   x.0 | y.0

So the choice operator "P + Q" either behaves like the left or the right process.

The process "0" can perform no transitions.

The process "x.0 | y.0" can perform two transitions:

    x.0 | y.0   --( x )->   0 | y.0
    x.0 | y.0   --( y )->   x.0 | 0

So the parallel operator "P | Q" either makes a transition of the left or right process while keeping the other process (in contrast to "P + Q"). This represents the parallel execution of two processes.

# Syntax

A CCS program consists of named (and possibly parameterized) processes and one process:

    P1 := process
    P2[i] := process
    P3[x, y] := process
    ...
    process

A CCS process has the following syntax (given in ebnf):

    process = "0"                   (* null process *)
            | "1"                   (* terminated process *)
            | identifier [ "[" exp { "," exp } "]" ]
                                    (* process instantiation *)
            | act "." process       (* prefix *)
            | process "\" "{" [ act { "," act } ] "}"
                                    (* restriction operator *)
            | process "\" "{" "*" { "," act } "}"
                                    (* restriction of complement *)
            | process "+" process   (* choice operator *)
            | process "|" process   (* parallel operator *)
            | process ";" process   (* sequential operator *)
            | "(" process ")"

    act = identifier [ "(" identifier ")" ]                         (* action *)
        | identifier [ "(" identifier ")" ] "!" [ "(" exp ")" ]     (* send action *)
        | identifier [ "(" identifier ")" ] "?" [ identifier ]      (* receive action *)
        | identifier [ "(" identifier ")" ] "?" [ "(" exp ")" ]     (* receive action *)

    alpha = "a" | ... | "z" | "A" | ... | "Z"
    digitnonzero = "1" | ... | "9"
    digit = "0" | digitnonzero

    identifier = ( alpha | "_" ) { alpha | num | "_" }

    int = "0" | digitnonzero { digit } | "-" digitnonzero { digit }
    exp = identifier
        | int
        | "-" exp
        | exp "*" exp | exp "/" exp | exp "%" exp
        | exp "+" exp | exp "-" exp

The precedence is given in descending order in the grammar above (i.e. prefix has the highest precedence, followed by restriction, then the choice operator etc).
Operators in the same row have the same precedence and all binary operators are left associative.

# Semantics

## Actions
There are five kinds of actions:
1. Internal action "i".
2. Termination acion "e".
3. Sending action "identifier!"
4. Receiving action "identifier?"
5. "Normal" action "identifier"

Let act be an action. We define the complementary action ~act of act as follows:

1. ~act? := act!
2. ~act! := act?

## Null process
The null process "0" has no transitions.

## Terminating process
The terminating process "1" has one transition:

    1   --( e )->   0

## Restriction operator
Let "P \ H" be a process, where H is the set of actions, e.g. "P \ {a,b,c,...}", and H should not contain "i" or "e".

If H contains a normal action act, it behaves as H would also contain act! and act?.

If P has a transition

    P   --( act )->   Q

to some Q with some action act and act is "i", "e" or act is not in H (\*), then

    P\H   --( act )->   Q\H

So a restriction can prevent a process from doing certain transitions, but it cannot prevent "i" or "e".

The restriction of complement "P \ {*,a,b,c,...}" inverts the condition (*): act has to be "i", "e" or act must be in H.

## Choice operator
Let "P + Q" be a process. If P has a transition

    P   --( act )->   P'

then

    P + Q   --( act )->   P'

Similarly, if

    Q   --( act )->   Q'

then

    P + Q   --( act )->   Q'

So the choice operator a transition from either P or Q, while discarding the other process.
This operator represents a non-deterministical choice between P or Q.

## Parallel operator
Let "P | Q" be a process. If P has a transition

    P   --( act )->   P'

where act is no termination action, then

    P | Q   --( act )->   P' | Q

Similarly, if

    Q   --( act )->   Q'

then

    P | Q   --( act )->   P | Q'

So the parallel operator performs a transition on either the left or the right side, while retaining the other process (in contract to the choice operator, which discards the other side).
This operator represents the parallel execution of P and Q.

To enable communication of processes, we also have the following rule:
Let act be a send or receive action. Then, if

    P   --( act )->   P'

and

    Q   --( ~act )->   Q'

then

    P | Q   --( i )->   P' | Q'

We call that "synchronization" of the two processes.

There is also a rule for termination: if

    P   --( e )->   P'

and

    Q   --( e )->   Q'

then

    P | Q   --( e )->   P' | Q'

So "P | Q" only terminates (i.e. performs the action "e"), if both processes are terminated (i.e. P and Q can perform the action "e").

## Sequential operator
Let "P; Q" be a process. If P has a transition

    P   --( e )->   Q

then

    P; Q   --( i )->   Q

So the sequential operator waits for the termination of P and continues with Q.
Note that you cannot write something like "P.Q", because the prefix operator has an action on the left hand side and no process.

## Execution and Traces

Let

    P0   --( a1 )->   P1   --( a2 )->   P2   --( a3 )->   ...   --( an )->   Pn

be valid transitions. Then the sequence of states and actions and executions is called an execution of P0.
The sequence of actions alone is called the trace of this execution. The trace of this would be

    a1, a2, a3, ..., an

## Examples

### Synchronization
In this example we want to model two processes: one process gets some input, caluclates something and gives it to the second process, which calculates something, and outputs it.
The first approach for this in CCS could be:

    input?.i.pass!.0 | pass?.i.output!.0

So the "giving something to the second process" is solved by using "pass!" and "pass?", which will synchronize using the synchronization rule.
(We do not model the actual values or calculation here, this is just an abstraction of what is happening.)

This process can behave as follows:

    input?.i.pass!.0 | pass?.i.output!.0

        --( input? )->

    i.pass!.0 | pass?.i.output!.0

        --( i )->

    pass!.0 | pass?.i.output!.0

        --( i )->       (synchronization here)

    0 | i.output!.0

        --( i )->

    0 | output!.0

        --( output! )->

    0 | 0

But the parallel operator does not force the processes to wait for each other, so this process could also do this:

    input?.i.pass!.0 | pass?.i.output!.0

        --( pass? )->

    input?.i.pass!.0 | i.output!.0

        --( input? )->

    i.pass!.0 | i.output!.0

        --( i )->

    i.pass!.0 | output!.0

        --( output! )->

    i.pass!.0 | 0

        --( i )->

    pass!.0 | 0

        --( pass! )->

    0 | 0

which is certanly not what we intended.
To prevent this, we have to explicitly disallow pass? without pass! and vice versa.
We can do this with the restriction operator:

    (input?.i.pass!.0 | pass?.i.output!.0) \ {pass}

The restriction "(...)\{pass}" will surpress the actions pass? and pass!, so the above example cannot happen anymore, because "(...)\{pass}" cannot perform any "pass" action.
On the other hand, the synchronization of both processes will not be surpressed, because processes synchronize with an "i" action, which is not affected by restriction.
This means that this process will now behave as follows:

    (input?.i.pass!.0 | pass?.i.output!.0) \ {pass}

        --( input? )->

    (i.pass!.0 | pass?.i.output!.0) \ {pass}

        --( i )->

    (pass!.0 | pass?.i.output!.0) \ {pass}

        --( i )->       (synchronization here)

    (0 | i.output!.0) \ {pass}

        --( i )->

    (0 | output!.0) \ {pass}

        --( output! )->

    (0 | 0) \ {pass}

This is also the only possible execution of this process.

### Data races
Let's look at a typical problem of concurrent programs, data-races:
Two processes want to increment a variable X.
We can write this as follows:

    getX?.i.setX!.0 | getX?.i.setX!.0

This process could behave as follows:

    getX?.i.setX!.0 | getX?.i.setX!.0
    
        --( getX? )->

    i.setX!.0 | getX?.i.setX!.0

        --( i )->

    setX!.0 | getX?.i.setX!.0

        --( setX! )->

    0 | getX?.i.setX!.0

        --( getX? )->

    0 | i.setX!.0

        --( i )->

    0 | .setX!.0

        --( setX! )->

    0 | 0

Which would be a good case, since X would now have been incremented by 2.
But it could also be like this:

    getX?.i.setX!.0 | getX?.i.setX!.0
    
        --( getX? )->

    i.setX!.0 | getX?.i.setX!.0
    
        --( getX? )->

    i.setX!.0 | i.setX!.0
    
        --( i )->

    setX!.0 | i.setX!.0
    
        --( i )->

    setX!.0 | setX!.0
    
        --( setX! )->

    0 | setX!.0
    
        --( setX! )->

    0 | 0

Which would be a bad case, because X would only have been incremented by 1.

### Locks
A common solution for the problem above are locks (or mutexes):
Only process can acquire a lock, other processes must wait for the process to release it.
The first approach for the example above could be:

    (lock!.getX?.i.setX!.unlock!.0 | lock!.getX?.i.setX!.unlock!.0 | lock?.unlock?.lock?.unlock?.0) \ {lock,unlock}

We model the lock as a seperate process, which can only receive "lock?", followed from "unlock?" etc.
This would now prevent a setX! from one process between a getX! and setX! from the other process,
because both processes first need to perform a synchronized lock! and later a synchronized unlock!.
(note that the synchronization of lock and unlock is ensured by the restriction operator).
There are now exactly two executions:

    (lock!.getX?.i.setX!.unlock!.0 | lock!.getX?.i.setX!.unlock!.0 | lock?.unlock?.lock?.unlock?.0) \ {lock,unlock}

        --( i )->

    (getX?.i.setX!.unlock!.0 | lock!.getX?.i.setX!.unlock!.0 | unlock?.lock?.unlock?.0) \ {lock,unlock}

        ...

    (unlock!.0 | lock!.getX?.i.setX!.unlock!.0 | unlock?.lock?.unlock?.0) \ {lock,unlock}

        --( i )->

    (0 | lock!.getX?.i.setX!.unlock!.0 | lock?.unlock?.0) \ {lock,unlock}

        --( i )->

    (0 | getX?.i.setX!.unlock!.0 | unlock?.0) \ {lock,unlock}

        ...

    (0 | unlock!.0 | unlock?.0) \ {lock,unlock}

        --( i )->

    (0 | 0 | 0) \ {lock,unlock}

The second execution begins with the second process getting the lock before the first process.

The problem with this solution is that the lock process can only lock and unlock two times.
We can solve this by using a recursive process definition:

    Lock := lock?.unlock?.Lock
    Inc := lock!.getX?.i.setX!.unlock!.0

    (Inc | Inc | Lock) \ {lock,unlock}

One execution of this process would now be:

    (Inc | Inc | Lock) \ {lock,unlock}

        --( i )->

    (Inc | getX?.i.setX!.unlock!.0 | unlock?.Lock!) \ {lock,unlock}

        --( getX? )->

    (Inc | i.setX!.unlock!.0 | unlock?.Lock!) \ {lock,unlock}

        --( i )->

    (Inc | setX!.unlock!.0 | unlock?.Lock!) \ {lock,unlock}

        --( setX! )->

    (Inc | unlock!.0 | unlock?.Lock) \ {lock,unlock}

        --( i )->

    (Inc | 0 | Lock) \ {lock,unlock}

        --( i )->

    (getX?.i.setX!.0 | 0 | unlock?.Lock) \ {lock,unlock}

        ...

    (0 | 0 | Lock) \ {lock,unlock}

## Value passing

_Note: This is not yet implemented._

_Note: This is not yet documented._
