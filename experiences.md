# My Experience Working on the Logic Formula System (Blatt 7)

## Introduction

This note is about how I worked on the logic formula project for Blatt 7. The goal was to build a system in C++ to represent logical formulas using gates like AND and OR, constants (True/False), and variables. At first, I wasn’t sure where to start, but after reading the documentation and trying things out, I slowly got the hang of it.

## What I Did

### 1. Logic_Node and Its Subclasses

I started by creating an abstract base class called `Logic_Node`. This class defines the basic functions that every node in the logic system needs — like `evaluate`, `arity`, and how to compare nodes. After that, I made three specific types of nodes:

- `Constant`: for representing True or False
- `Variable`: for variables like x1, x2, etc.
- `Gate`: for AND and OR gates that combine other formulas

### 2. Using shared_ptr

To make sure memory is handled safely, I used `std::shared_ptr`. At first, I didn’t fully understand how shared pointers work, but after testing a few examples, I saw how they automatically manage memory and allow us to share nodes without copying them.

### 3. Logic_Builder

I made a class called `Logic_Builder` to help build formulas easily. It has functions like `make_variable`, `make_conjunction`, `make_disjunction`, etc. It also includes a normalization function that simplifies formulas by removing duplicates or unnecessary constants (like AND[x, True] → x).

### 4. Logger

Another part of the task was to implement a logger system. I created a base class `Logger` with three types:
- SilentLogger: does nothing
- StdoutLogger: prints to the console
- FileLogger: saves logs to a file

This helped me test and understand what was going on during formula evaluation and normalization.

## Challenges I Faced

- Understanding how `shared_ptr` and `dynamic_cast` work in C++ took a bit of time.
- Writing the comparison (`==`) functions for the logic nodes was tricky — I had to compare the actual structure, not just the pointer values.
- Figuring out the normalization rules was confusing at first, especially the special cases like AND[] = True.

## What I Learned

- How to use inheritance in C++ to build a clean design with abstract and concrete classes
- How to use shared pointers properly and why they are useful in this type of project
- How to represent and simplify logical formulas programmatically

## Thoughts on the Project Documentation

At first, the documentation seemed very long and a bit overwhelming. But after reading it more carefully, I found it really helpful. It explained important details that I would’ve missed otherwise — like the edge cases, how to structure the code, and what future exercises might add.

I think the documentation helped me write better code and understand the whole goal of the project.

## Conclusion

I spent a good amount of time on this project, and even though some parts were tough, I learned a lot about C++, memory management, and logical systems. I'm happy with how the system turned out, and I think I now understand how to build and simplify formulas much better than before.
