# ------------------------------------------------------------------------------------------------
#  The MIT License
#  
#  Copyright (c) 2013 Tim Andersen
#  
#  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
#  and associated documentation files (the "Software"), to deal in the Software without
#  restriction, including without limitation the rights to use, copy, modify, merge, publish,
#  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#  
#  The above copyright notice and this permission notice shall be included in all copies or
#  substantial portions of the Software.
#  
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
#  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
#  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# ------------------------------------------------------------------------------------------------

# -----------------------------------------------------------------------------
#  python script to generate the templated registration classes
# -----------------------------------------------------------------------------

import sys
import os
import fileinput
import ctypes
from ctypes import *

# -----------------------------------------------------------------------------
def GenerateMacros(maxparamcount, outputfilename):
    
    #open the output file
    outputfile = open(outputfilename, 'w');
    
    outputfile.write("// ------------------------------------------------------------------------------------------------\n");
    outputfile.write("// Generated macros for function registration\n");
    outputfile.write("// ------------------------------------------------------------------------------------------------\n");

    paramcount = 0;
    while (paramcount <= maxparamcount):
        outputfile.write("\n\n// -- Parameter count: %d\n" % paramcount);
        
        #  global function
        regfuncstring = "#define REGISTER_FUNCTION_P%d" % paramcount + "(scriptname, funcname, R";
        i = 1;
        while (i <= paramcount):
            regfuncstring = regfuncstring + ", T%d" % i;
            i = i + 1;
        regfuncstring = regfuncstring + ") \\\n";
        outputfile.write(regfuncstring);
        
        regobjstring = "    static TinScript::CRegFunctionP%d" % paramcount + "<R";
        i = 1;
        while (i <= paramcount):
            regobjstring = regobjstring + ", T%d" % i;
            i = i + 1;
        regobjstring = regobjstring + "> _reg_##scriptname(#scriptname, funcname);";
        outputfile.write(regobjstring);

        #  method
        regfuncstring = "\n\n#define REGISTER_METHOD_P%d" % paramcount + "(classname, scriptname, methodname, R";
        i = 1;
        while (i <= paramcount):
            regfuncstring = regfuncstring + ", T%d" % i;
            i = i + 1;
        regfuncstring = regfuncstring + ")    \\\n";
        outputfile.write(regfuncstring);
        
        wrapperstring = "    static R classname##methodname(classname* obj"
        i = 1;
        while (i <= paramcount):
            wrapperstring = wrapperstring + ", T%d t%d" % (i, i);
            i = i + 1;
        wrapperstring = wrapperstring + ") {    \\\n";
        outputfile.write(wrapperstring);
        
        wrapperstring = "        return obj->methodname(";
        i = 1;
        while (i <= paramcount):
            if (i > 1):
                wrapperstring = wrapperstring + ", ";
            wrapperstring = wrapperstring + "t%d" % i;
            i = i + 1;
        wrapperstring = wrapperstring + ");    \\\n";
        outputfile.write(wrapperstring);
        outputfile.write("    }    \\\n");
        
        regobjstring = "    static TinScript::CRegMethodP%d" % paramcount + "<classname, R";
        i = 1;
        while (i <= paramcount):
            regobjstring = regobjstring + ", T%d" % i;
            i = i + 1;
        regobjstring = regobjstring + "> _reg_##classname##methodname(#scriptname, classname##methodname);";
        outputfile.write(regobjstring);

        # next class definition
        paramcount = paramcount + 1;
        
    outputfile.write("\n\n");
    outputfile.close();

    
# -----------------------------------------------------------------------------
def GenerateClasses(maxparamcount, outputfilename):
    print("Max param count: %d" % maxparamcount);
    print("Output: %s" % outputfilename);
    
    #open the output file
    outputfile = open(outputfilename, 'w');
    
    outputfile.write("// ------------------------------------------------------------------------------------------------\n");
    outputfile.write("// Generated classes for function registration\n");
    outputfile.write("// ------------------------------------------------------------------------------------------------\n");
    outputfile.write("\n");

    paramcount = 0;
    while (paramcount <= maxparamcount):
        outputfile.write("\n");
        outputfile.write("// -------------------\n");
        outputfile.write("// Parameter count: %d\n" % paramcount);
        outputfile.write("// -------------------\n");
        outputfile.write("\n");
        
        template_string = "template<typename R";
        i = 1;
        while (i <= paramcount):
            template_string = template_string + ", typename T%d" % i;
            i = i + 1;
        template_string = template_string + ">\n";
            
        outputfile.write(template_string);
        outputfile.write("class CRegFunctionP%d : public CRegFunctionBase {\n" % paramcount);
        outputfile.write("public:\n");
        outputfile.write("\n");

        typedef_string = "    typedef R (*funcsignature)(";
        i = 1;
        while (i <= paramcount):
            if (i > 1):
                typedef_string = typedef_string + ", ";
            typedef_string = typedef_string + "T%d p%d" % (i, i);
            i = i + 1;
        typedef_string = typedef_string + ");\n";
        outputfile.write(typedef_string);
        outputfile.write("\n");

        outputfile.write("    // -- CRegisterFunctionP%d\n" % paramcount);
        outputfile.write("    CRegFunctionP%d(const char* _funcname, funcsignature _funcptr) :\n" % paramcount);
        outputfile.write("                    CRegFunctionBase(_funcname) {\n");
        outputfile.write("        funcptr = _funcptr;\n");
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- destructor\n");
        outputfile.write("    virtual ~CRegFunctionP%d() {\n" % paramcount);
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- virtual DispatchFunction wrapper\n");
        outputfile.write("    virtual void DispatchFunction(void* objaddr) {\n");
        i = 1;
        while (i <= paramcount):
            outputfile.write("        CVariableEntry* ve%d = GetContext()->GetParameter(%d);\n" % (i, i));
            i = i + 1;
            
        dispatch_string = "        Dispatch(";
        if(paramcount == 0):
            dispatch_string = dispatch_string + ");\n"
        else:
            i = 1;
            while (i <= paramcount):
                if (i > 1):
                    dispatch_string = dispatch_string + ",\n                 ";
                dispatch_string = dispatch_string + "convert_from_void_ptr<T%d>::Convert(ve%d->GetValueAddr(NULL))" % (i, i);
                i = i + 1;
            dispatch_string = dispatch_string + ");\n";
        outputfile.write(dispatch_string);
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- dispatch method\n");
        dispatch_string = "    R Dispatch(";
        i = 1;
        while (i <= paramcount):
            if (i > 1):
                dispatch_string = dispatch_string + ", ";
            dispatch_string = dispatch_string + "T%d p%d" % (i, i);
            i = i + 1;
        dispatch_string = dispatch_string + ") {\n";
        outputfile.write(dispatch_string);
        
        functioncall = "        R r = funcptr(";
        i = 1;
        while (i <= paramcount):
            if (i > 1):
                functioncall = functioncall + ", ";
            functioncall = functioncall + "p%d" % i;
            i = i + 1;
        functioncall = functioncall + ");\n";
        outputfile.write(functioncall);
        outputfile.write("        assert(GetContext()->GetParameter(0));\n");
        outputfile.write("        CVariableEntry* returnval = GetContext()->GetParameter(0);\n");
        outputfile.write("        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));\n");
        outputfile.write("        return (r);\n");
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- registration method\n");
        outputfile.write("    virtual void Register() {\n");
        outputfile.write("        CFunctionEntry* fe = new CFunctionEntry(0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);\n");
        outputfile.write("        SetContext(fe->GetContext());\n");
        outputfile.write("        GetContext()->AddParameter(\"__return\", Hash(\"__return\"), GetRegisteredType(GetTypeID<R>()));\n");
        i = 1;
        while (i <= paramcount):
            outputfile.write("        GetContext()->AddParameter(\"_p%d\", Hash(\"_p%d\"), GetRegisteredType(GetTypeID<T%d>()));\n" % (i, i, i));
            i = i + 1;
        outputfile.write("\n");
        outputfile.write("        unsigned int hash = fe->GetHash();\n");
        outputfile.write("        tFuncTable* globalfunctable = CNamespace::FindNamespace(0)->GetFuncTable();\n");
        outputfile.write("        globalfunctable->AddItem(*fe, hash);\n");
        outputfile.write("    }\n");
        outputfile.write("\n");
        outputfile.write("private:\n");
        outputfile.write("    funcsignature funcptr;\n");
        outputfile.write("};\n");
        outputfile.write("\n");
        
        # repeat for the void specialized template
        template_string = "template<";
        i = 1;
        while (i <= paramcount):
            if (i > 1):
                template_string = template_string + ", ";
            template_string = template_string + "typename T%d" % i;
            i = i + 1;
        template_string = template_string + ">\n";
            
        outputfile.write(template_string);
        classname_string = "class CRegFunctionP%d<void" % paramcount;
        i = 1;
        while (i <= paramcount):
            classname_string = classname_string + ", T%d" % i;
            i = i + 1;
        classname_string = classname_string + "> : public CRegFunctionBase {\n";
        outputfile.write(classname_string);
        outputfile.write("public:\n");
        outputfile.write("\n");

        typedef_string = "    typedef void (*funcsignature)(";
        i = 1;
        while (i <= paramcount):
            if (i > 1):
                typedef_string = typedef_string + ", ";
            typedef_string = typedef_string + "T%d p%d" % (i, i);
            i = i + 1;
        typedef_string = typedef_string + ");\n";
        outputfile.write(typedef_string);
        outputfile.write("\n");

        outputfile.write("    // -- CRegisterFunctionP%d\n" % paramcount);
        outputfile.write("    CRegFunctionP%d(const char* _funcname, funcsignature _funcptr) :\n" % paramcount);
        outputfile.write("                    CRegFunctionBase(_funcname) {\n");
        outputfile.write("        funcptr = _funcptr;\n");
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- destructor\n");
        outputfile.write("    virtual ~CRegFunctionP%d() {\n" % paramcount);
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- virtual DispatchFunction wrapper\n");
        outputfile.write("    virtual void DispatchFunction(void* objaddr) {\n");
        i = 1;
        while (i <= paramcount):
            outputfile.write("        CVariableEntry* ve%d = GetContext()->GetParameter(%d);\n" % (i, i));
            i = i + 1;
            
        dispatch_string = "        Dispatch(";
        if(paramcount == 0):
            dispatch_string = dispatch_string + ");\n"
        else:
            i = 1;
            while (i <= paramcount):
                if (i > 1):
                    dispatch_string = dispatch_string + ",\n                 ";
                dispatch_string = dispatch_string + "convert_from_void_ptr<T%d>::Convert(ve%d->GetValueAddr(NULL))" % (i, i);
                i = i + 1;
            dispatch_string = dispatch_string + ");\n";
        outputfile.write(dispatch_string);
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- dispatch method\n");
        dispatch_string = "    void Dispatch(";
        i = 1;
        while (i <= paramcount):
            if (i > 1):
                dispatch_string = dispatch_string + ", ";
            dispatch_string = dispatch_string + "T%d p%d" % (i, i);
            i = i + 1;
        dispatch_string = dispatch_string + ") {\n";
        outputfile.write(dispatch_string);
        
        functioncall = "        funcptr(";
        i = 1;
        while (i <= paramcount):
            if (i > 1):
                functioncall = functioncall + ", ";
            functioncall = functioncall + "p%d" % i;
            i = i + 1;
        functioncall = functioncall + ");\n";
        outputfile.write(functioncall);
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- registration method\n");
        outputfile.write("    virtual void Register() {\n");
        outputfile.write("        CFunctionEntry* fe = new CFunctionEntry(0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);\n");
        outputfile.write("        SetContext(fe->GetContext());\n");
        outputfile.write("        GetContext()->AddParameter(\"__return\", Hash(\"__return\"), TYPE_void);\n");
        i = 1;
        while (i <= paramcount):
            outputfile.write("        GetContext()->AddParameter(\"_p%d\", Hash(\"_p%d\"), GetRegisteredType(GetTypeID<T%d>()));\n" % (i, i, i));
            i = i + 1;
        outputfile.write("\n");
        outputfile.write("        unsigned int hash = fe->GetHash();\n");
        outputfile.write("        tFuncTable* globalfunctable = CNamespace::FindNamespace(0)->GetFuncTable();\n");
        outputfile.write("        globalfunctable->AddItem(*fe, hash);\n");
        outputfile.write("    }\n");
        outputfile.write("\n");
        outputfile.write("private:\n");
        outputfile.write("    funcsignature funcptr;\n");
        outputfile.write("};\n");
        outputfile.write("\n");
        
        
        # repeat for the methods templates
        template_string = "template<typename C, typename R";
        i = 1;
        while (i <= paramcount):
            template_string = template_string + ", typename T%d" % i;
            i = i + 1;
        template_string = template_string + ">\n";
        outputfile.write(template_string);
        
        outputfile.write("class CRegMethodP%d : public CRegFunctionBase {\n" % paramcount);
        outputfile.write("public:\n");
        outputfile.write("\n");

        typedef_string = "    typedef R (*methodsignature)(C* c";
        i = 1;
        while (i <= paramcount):
            typedef_string = typedef_string + ", T%d p%d" % (i, i);
            i = i + 1;
        typedef_string = typedef_string + ");\n";
        outputfile.write(typedef_string);
        outputfile.write("\n");

        outputfile.write("    // -- CRegisterMethodP%d\n" % paramcount);
        outputfile.write("    CRegMethodP%d(const char* _funcname, methodsignature _funcptr) :\n" % paramcount);
        outputfile.write("                  CRegFunctionBase(_funcname) {\n");
        outputfile.write("        funcptr = _funcptr;\n");
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- destructor\n");
        outputfile.write("    virtual ~CRegMethodP%d() {\n" % paramcount);
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- virtual DispatchFunction wrapper\n");
        outputfile.write("    virtual void DispatchFunction(void* objaddr) {\n");
        i = 1;
        while (i <= paramcount):
            outputfile.write("        CVariableEntry* ve%d = GetContext()->GetParameter(%d);\n" % (i, i));
            i = i + 1;
            
        dispatch_string = "        Dispatch(objaddr";
        if(paramcount == 0):
            dispatch_string = dispatch_string + ");\n"
        else:
            i = 1;
            while (i <= paramcount):
                dispatch_string = dispatch_string + ",\n                 ";
                dispatch_string = dispatch_string + "convert_from_void_ptr<T%d>::Convert(ve%d->GetValueAddr(NULL))" % (i, i);
                i = i + 1;
            dispatch_string = dispatch_string + ");\n";
        outputfile.write(dispatch_string);
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- dispatch method\n");
        dispatch_string = "    R Dispatch(void* objaddr";
        i = 1;
        while (i <= paramcount):
            dispatch_string = dispatch_string + ", T%d p%d" % (i, i);
            i = i + 1;
        dispatch_string = dispatch_string + ") {\n";
        outputfile.write(dispatch_string);
        
        outputfile.write("        C* objptr = (C*)objaddr;\n");
        functioncall = "        R r = funcptr(objptr";
        i = 1;
        while (i <= paramcount):
            functioncall = functioncall + ", p%d" % i;
            i = i + 1;
        functioncall = functioncall + ");\n";
        outputfile.write(functioncall);
        outputfile.write("        assert(GetContext()->GetParameter(0));\n");
        outputfile.write("        CVariableEntry* returnval = GetContext()->GetParameter(0);\n");
        outputfile.write("        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));\n");
        outputfile.write("        return (r);\n");
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- registration method\n");
        outputfile.write("    virtual void Register() {\n");
        outputfile.write("        CFunctionEntry* fe = new CFunctionEntry(Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);\n");
        outputfile.write("        SetContext(fe->GetContext());\n");
        outputfile.write("        GetContext()->AddParameter(\"__return\", Hash(\"__return\"), GetRegisteredType(GetTypeID<R>()));\n");
        i = 1;
        while (i <= paramcount):
            outputfile.write("        GetContext()->AddParameter(\"_p%d\", Hash(\"_p%d\"), GetRegisteredType(GetTypeID<T%d>()));\n" % (i, i, i));
            i = i + 1;
        outputfile.write("\n");
        outputfile.write("        unsigned int hash = fe->GetHash();\n");
        outputfile.write("        tFuncTable* methodtable = C::classnamespace->GetFuncTable();\n");
        outputfile.write("        methodtable->AddItem(*fe, hash);\n");
        outputfile.write("    }\n");
        outputfile.write("\n");
        outputfile.write("private:\n");
        outputfile.write("    methodsignature funcptr;\n");
        outputfile.write("};\n");
        outputfile.write("\n");
        
        # repeat for the void specialized methods
        template_string = "template<typename C";
        i = 1;
        while (i <= paramcount):
            template_string = template_string + ", typename T%d" % i;
            i = i + 1;
        template_string = template_string + ">\n";
        outputfile.write(template_string);
        
        classname_string = "class CRegMethodP%d<C, void" % paramcount;
        i = 1;
        while (i <= paramcount):
            classname_string = classname_string + ", T%d" % i;
            i = i + 1;
        classname_string = classname_string + "> : public CRegFunctionBase {\n";
        outputfile.write(classname_string);
        
        outputfile.write("public:\n");
        outputfile.write("\n");

        typedef_string = "    typedef void (*methodsignature)(C* c";
        i = 1;
        while (i <= paramcount):
            typedef_string = typedef_string + ", T%d p%d" % (i, i);
            i = i + 1;
        typedef_string = typedef_string + ");\n";
        outputfile.write(typedef_string);
        outputfile.write("\n");

        outputfile.write("    // -- CRegisterMethodP%d\n" % paramcount);
        outputfile.write("    CRegMethodP%d(const char* _funcname, methodsignature _funcptr) :\n" % paramcount);
        outputfile.write("                  CRegFunctionBase(_funcname) {\n");
        outputfile.write("        funcptr = _funcptr;\n");
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- destructor\n");
        outputfile.write("    virtual ~CRegMethodP%d() {\n" % paramcount);
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- virtual DispatchFunction wrapper\n");
        outputfile.write("    virtual void DispatchFunction(void* objaddr) {\n");
        i = 1;
        while (i <= paramcount):
            outputfile.write("        CVariableEntry* ve%d = GetContext()->GetParameter(%d);\n" % (i, i));
            i = i + 1;
            
        dispatch_string = "        Dispatch(objaddr";
        if(paramcount == 0):
            dispatch_string = dispatch_string + ");\n"
        else:
            i = 1;
            while (i <= paramcount):
                dispatch_string = dispatch_string + ",\n                 ";
                dispatch_string = dispatch_string + "convert_from_void_ptr<T%d>::Convert(ve%d->GetValueAddr(NULL))" % (i, i);
                i = i + 1;
            dispatch_string = dispatch_string + ");\n";
        outputfile.write(dispatch_string);
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- dispatch method\n");
        dispatch_string = "    void Dispatch(void* objaddr";
        i = 1;
        while (i <= paramcount):
            dispatch_string = dispatch_string + ", T%d p%d" % (i, i);
            i = i + 1;
        dispatch_string = dispatch_string + ") {\n";
        outputfile.write(dispatch_string);
        
        outputfile.write("        C* objptr = (C*)objaddr;\n");
        functioncall = "        funcptr(objptr";
        i = 1;
        while (i <= paramcount):
            functioncall = functioncall + ", p%d" % i;
            i = i + 1;
        functioncall = functioncall + ");\n";
        outputfile.write(functioncall);
        outputfile.write("    }\n");
        outputfile.write("\n");
        
        outputfile.write("    // -- registration method\n");
        outputfile.write("    virtual void Register() {\n");
        outputfile.write("        CFunctionEntry* fe = new CFunctionEntry(Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);\n");
        outputfile.write("        SetContext(fe->GetContext());\n");
        outputfile.write("        GetContext()->AddParameter(\"__return\", Hash(\"__return\"), TYPE_void);\n");
        i = 1;
        while (i <= paramcount):
            outputfile.write("        GetContext()->AddParameter(\"_p%d\", Hash(\"_p%d\"), GetRegisteredType(GetTypeID<T%d>()));\n" % (i, i, i));
            i = i + 1;
        outputfile.write("\n");
        outputfile.write("        unsigned int hash = fe->GetHash();\n");
        outputfile.write("        tFuncTable* methodtable = C::classnamespace->GetFuncTable();\n");
        outputfile.write("        methodtable->AddItem(*fe, hash);\n");
        outputfile.write("    }\n");
        outputfile.write("\n");
        outputfile.write("private:\n");
        outputfile.write("    methodsignature funcptr;\n");
        outputfile.write("};\n");
        outputfile.write("\n");

        # next class definition
        paramcount = paramcount + 1;
        
    outputfile.close();

# -----------------------------------------------------------------------------
def main():

    print ("\n***********************************");
    print ("*  Generate registration classes  *");
    print ("***********************************\n");
    
    classesfilename = "registrationclasses.h";
    macrosfilename = "registrationmacros.h";
    maxparamcount = 8;
    
    # parse the command line arguments
    argcount = len(sys.argv);
    printhelp = (argcount <= 1);
    currentarg = 1;
    while (currentarg < argcount):
        if (sys.argv[currentarg] == "-h" or sys.argv[currentarg] == "-help" or sys.argv[currentarg] == "-?"):
            printhelp = 1;
        elif (sys.argv[currentarg] == "-maxparam"):
            if(currentarg + 1 >= argcount):
                printhelp = 1;
                currentarg = argcount;
            else:
                maxparamcount = int(sys.argv[currentarg + 1]);
                currentarg = currentarg + 2;
            
        elif (sys.argv[currentarg] == "-oc" or sys.argv[currentarg] == "-outputclasses"):
            if(currentarg + 1 >= argcount):
                printhelp = 1;
                currentarg = argcount;
            else:
                classesfilename = sys.argv[currentarg + 1];
                currentarg = currentarg + 2;
        
        elif (sys.argv[currentarg] == "-om" or sys.argv[currentarg] == "-outputmacros"):
            if(currentarg + 1 >= argcount):
                printhelp = 1;
                currentarg = argcount;
            else:
                macrosfilename = sys.argv[currentarg + 1];
                currentarg = currentarg + 2;
        
        else:
            printhelp = 1;
            break;
        
    if (printhelp == 1):
        help_width = 36;
        print ("Usage:  python genregclasses.py [option]");
        print (str("-h | -help").rjust(help_width) + " :  " + str("This help menu."));
        print (str("-maxparam <count>").rjust(help_width) + " :  " + str("maximum number or parameters"));
        print (str("-oc | -outputclasses <filename>").rjust(help_width) + " :  " + str("templated classes output file."));
        print (str("-om | -outputmacros <filename>").rjust(help_width) + " :  " + str("macros output file."));
        exit(0);
        
    else:
        GenerateClasses(maxparamcount, classesfilename);
        GenerateMacros(maxparamcount, macrosfilename);
        
    print ("\n**********************************");
    print ("*  Finished generating classes.  *");
    print ("**********************************\n");

# -----------------------------------------------------------------------------
# start of the script execution

main(); 
exit();

# -----------------------------------------------------------------------------
# EOF
# -----------------------------------------------------------------------------
