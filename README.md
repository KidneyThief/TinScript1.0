TinScript1.0
============

A lightweight scripting language, using a C-lite syntax (no pointers, references, or other
memory complications), designed to be integrated into a C++ project to facilitate runtime development.

I haven't even begun to document this library, but the fastest way to explore it is:

*  Download the entire depot
*  Open up the solution:  tinconsole/tinconsole.sln
*  Build the executable, and run it.
*  A cmd shell should open up, with a "Console =>" prompt
*  Try executing (precisely, case sensitive):
  *  Print("hello world");
*  You should see both the console input echo'd, as well as the actual "hello world" text.
*  Finally, execute the command:
  *  BeginUnitTests();
*  The spam of about 20 unit tests should stream by.  Open up files:
  *  source/TinScript.h  - at the bottom, are the function prototypes for using tinscript
  *  source/unittest.cpp - for an example of just about every aspect of the integration
  *  scripts/unittest.ts - for the script side of the unit test examples.

Enjoy,

~Tim Andersen

