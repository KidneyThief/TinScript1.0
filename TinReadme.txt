//----------------------------------------------------------------------
// Notes:

1.  *FIXED* Order of operations is not implemented - use parenthesis to enforce.

2.  There is no variable scoping - all variables are local to the entire function, or global
    Nested statement blocks no meaning.

3.  'For loops' do not allow variable declarations as part of the opening expression.

4.  Function definitions require all parameters to be named (cannot just specify type).

5.  Function paths are not verified - a default "" is returned from any function missing
a return statement.

6.  All registered methods must be public

7.  Took hook up a new object to the namespace created by it's name, at least one function in that namespace has to have been defined before the object is created.

8.  Namespaced functions are by definition methods...  You cannot call NS::Func() without an object who's using the NS namespace.

9.  Temporary systems and should be replaced implemented include:  CTimer/systemtime, CHashTable, CStringTable, Hash() function, the entire Console is a getch() hack.

10.  Currently the language is fully functional as case-sensitive, but not completely enforced.  Need to tighten up the #ifdef to properly define this.

11.  No support for hashtable type variables outside of the global scope

12.  No support for post increment/decrement...  pre increment/decrement only.
