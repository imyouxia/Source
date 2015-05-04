SGI-STL V3.3

The STL was developed on SGI MIPSproTM C++ 7.0, 7.1, 7.2, and 7.2.1. If you are using the 7.0 compiler, you must compile using either the -n32 or the -64 flag; if you are using 7.1 or later, you may use -o32, -n32, or -64. The STL has also been tested on Microsoft Visual C++ 5.0, on g++ 2.8.1, and on recent egcs snapshots. All of the STL except for the <string>, <bitset>, and <valarray> headers has also been tested on Borland 5.02.

It should not be difficult to use the STL with other compilers that have good support for templates, especially compilers that use the EDG front end.

This distribution of the STL consists entirely of header files: there is no need to link to any library files. You can view or download a header file individually, or you can download all of them as a tar or zip file. (Note that v3 has many more header files than v1 and v2 did, because it provides both old-style and new-style header names. Many of the files in v3 are very short, and do little other than forwarding.)
