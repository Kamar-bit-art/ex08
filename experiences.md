# My Experience Working on the Logic Formula System (Blatt 7 & 8)

## Introduction

In this document, I want to share how I approached the logic formula project for Blatt 7 and 8. The goal was to build a system in C++ to represent logical formulas using gates like AND and OR, constants (True/False), and variables. At first, I wasn't sure where to start. I struggled a bit while trying things out, but with time and some debugging, things slowly started to make sense.

## What I Did in Week 1 (Blatt 7)

### 1. Logic_Node and Its Subclasses

I started by creating an abstract base class called `Logic_Node`. This class defines the basic functions that every node in the logic system needs — like `evaluate`, `arity`, and how to compare nodes. After that, I made three specific types of nodes:

- `Constant`: for representing True or False
- `Variable`: for variables like x1, x2, etc.
- `Gate`: for AND and OR gates that combine other formulas

### 2. Using shared_ptr

To make sure memory is handled safely, I used `std::shared_ptr`. At first, I didn't fully understand how shared pointers work, but after testing a few examples, I saw how they automatically manage memory and allow us to share nodes without copying them.

### 3. Logic_Builder

I made a class called `Logic_Builder` to help build formulas easily. It has functions like `make_variable`, `make_conjunction`, `make_disjunction`, etc. It also includes a normalization function that simplifies formulas by removing duplicates or unnecessary constants (like AND[x, True] → x).

### 4. Logger

Another part of the task was to implement a logger system. I created a base class `Logger` with three types:
- SilentLogger: does nothing
- StdoutLogger: prints to the console
- FileLogger: saves logs to a file

This helped me test and understand what was going on during formula evaluation and normalization.

## What I Did in Week 2 (Blatt 8)

### 1. Implementing Perfect Structural Sharing

In Week 2, I focused on implementing perfect structural sharing using the `simplify` method. The key idea was to ensure that structurally identical formulas would share the same memory location, which improves efficiency and reduces memory usage.

I implemented this by:
- Creating a cache (`simplified_representative`) that maps formulas to their simplified versions
- Using a custom hash function and equality operator for the cache
- Ensuring that the `simplify` method always returns the same pointer for structurally identical formulas

### 2. Deep Structural Equality with Logic_Node_Equal

I implemented the `Logic_Node_Equal` struct to perform deep structural equality checks between formulas. This was essential for the cache to work correctly. The implementation:
- First checks if the pointers are identical (fast path)
- Then checks if either pointer is null
- Finally performs a deep structural comparison using the overloaded `==` operator

### 3. Custom Hash Function with Logic_Node_Hash

I created a recursive hash function in `Logic_Node_Hash` that computes a hash based on the structure of the formula:
- For constants: returns 1 for True and 0 for False
- For variables: uses the literal value multiplied by a prime number
- For gates: combines the gate type with the hashes of all children

This hash function ensures that structurally identical formulas have the same hash value, which is crucial for the cache to work efficiently.

### 4. Optimized Simplification Rules

I implemented the specific simplification rules as required:
- AND[] = True
- OR[] = False
- AND[x] = x
- OR[x] = x

The implementation recursively simplifies all children first, then applies these rules to create a new formula or reuse an existing one from the cache.

### 5. Cache Management

I implemented the `clear_cache` method to reset the simplification cache, which is important for the fuzzer that regularly resets the cache. This ensures that the system can handle long-running processes without memory issues.

### 6. Extended Simplification Rules

For version g of the assignment, I implemented two additional simplification rules:

- `AND[... False ...] = False`: If any child of an AND gate is False, the entire expression evaluates to False. This was implemented by checking each child during simplification and immediately returning False if any False constant is found.

- `OR[... True ...] = True`: Similarly, if any child of an OR gate is True, the entire expression evaluates to True. This optimization short-circuits evaluation when the result is already determined.

### 7. Recursive Child Collection

I implemented the `collect_children` function that recursively collects all sub-nodes in a formula. This function traverses the formula tree and returns a vector containing all nodes (including the root and all descendants). This is useful for analyzing the structure of formulas and ensuring proper sharing.

### 8. Perfect Structural Sharing Verification

To ensure perfect structural sharing, I added verification checks in the fuzzer. After simplification, the code checks that:

1. No constants appear as children in the simplified formula (they should have been simplified away)
2. No duplicate nodes exist with the same logical structure but different memory addresses

This verification is crucial because it confirms that our simplification process is working correctly and efficiently. Perfect structural sharing means we never have two separate nodes in memory that represent the same logical formula, which saves memory and improves performance.

## Challenges I Faced

- Understanding how `shared_ptr` and `dynamic_cast` work in C++ took a bit of time.
- Writing the comparison (`==`) functions for the logic nodes was tricky — I had to compare the actual structure, not just the pointer values.
- Figuring out the normalization rules was confusing at first, especially the special cases like AND[] = True.
- Implementing a good hash function that correctly captures the structure of formulas was challenging.
- Ensuring that the cache correctly identifies structurally identical formulas required careful testing.
- Making sure that the simplification rules were applied correctly in all cases, especially with nested formulas.

## What I Learned

- How to use inheritance in C++ to build a clean design with abstract and concrete classes
- How to use shared pointers properly and why they are useful in this type of project
- How to represent and simplify logical formulas programmatically
- How to implement perfect structural sharing using caching techniques
- How to design custom hash functions and equality operators for complex data structures
- The importance of careful testing when implementing caching and sharing mechanisms

## Thoughts on the Project Documentation

At first, the documentation seemed very long and a bit overwhelming. But after reading it more carefully, I found it really helpful. It explained important details that I would've missed otherwise — like the edge cases, how to structure the code, and what future exercises might add.

I think the documentation helped me write better code and understand the whole goal of the project. The Week 2 documentation was particularly helpful in explaining the concept of perfect structural sharing and the specific simplification rules to implement.

## Conclusion

I spent a good amount of time on this project, and even though some parts were tough, I learned a lot about C++, memory management, and logical systems. The Week 2 implementation of caching and perfect structural sharing was particularly interesting and taught me valuable lessons about efficient data structures and algorithms.

I'm happy with how the system turned out, and I think I now understand how to build and simplify formulas much better than before. The combination of caching, structural sharing, and specific simplification rules has resulted in a highly optimized logic formula system that should perform well even with complex formulas.
