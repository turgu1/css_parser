# A bare CSS Parser in C++

This is a simple, single-source code, CSS Parser written in C++. It implements lexical and syntax analysis, ideal for someone who wants to add its own semantic processing. It is based on the grammar as defined [here](https://www.w3.org/TR/CSS21/grammar.html) and was coded by hand.

The following are the main differences from the 2.1 grammar:

- No support for Unicode
- Supports numbers starting with '.'
- Supports @font-style 

A minimal testing tool is included in the file and is enabled/disabled through a single #define constant. 