// ------------------------------------------------------------------------------------------------
//  The MIT License
//  
//  Copyright (c) 2013 Tim Andersen
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without
//  restriction, including without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// unittest.cpp
// ------------------------------------------------------------------------------------------------


#include "stdafx.h"

// -- lib includes
#include "stdio.h"

// -- includes required by any system wanting access to TinScript
#include "TinScript.h"
#include "TinRegistration.h"

// -- use the DECLARE_FILE/REGISTER_FILE macros to prevent deadstripping
DECLARE_FILE(unittest_cpp);

// -- constants -----------------------------------------------------------------------------------
static const char* kUnitTestScriptName = "../scripts/unittest.ts";

// -- GLOBAL VARIABLES ----------------------------------------------------------------------------
int32 gCodeGlobalVariable = 17;
REGISTER_GLOBAL_VAR(gCodeGlobalVar, gCodeGlobalVariable);

// -- GLOBAL FUNCTIONS ----------------------------------------------------------------------------
int32 MultIntByTwo(int32 number) {
    return (number << 1);
}
REGISTER_FUNCTION_P1(MultIntByTwo, MultIntByTwo, int32, int32);

void MTPrint(const char* fmt, ...) {
    TinScript::CScriptContext* main_thread = ::TinScript::CScriptContext::GetMainThreadContext();
    va_list args;
    va_start(args, fmt);
    char buf[2048];
    vsprintf_s(buf, 2048, fmt, args);
    va_end(args);
    TinPrint(main_thread, "%s", buf);
}

// --  REGISTERED CLASS----------------------------------------------------------------------------
class CBase {
    public:
        CBase() {
            MTPrint("Enter constructor CBase()\n");
            floatvalue = 27.0;
            intvalue = 33;
            boolvalue = true;
        }
        virtual ~CBase() {
            MTPrint("Enter destructor ~CBase()\n");
        }

        DECLARE_SCRIPT_CLASS(CBase, VOID);

        float32 GetFloatValue() {
            return floatvalue;
        }

        void SetFloatValue(float32 val) {
            floatvalue = val;
        }

        int32 GetIntValue() {
            return intvalue;
        }

        virtual void SetIntValue(int32 val) {
            MTPrint("Enter CBase::SetIntValue()\n");
            intvalue = val;
        }

        bool8 GetBoolValue() {
            return boolvalue;
        }

        void SetBoolValue(bool8 val) {
            boolvalue = val;
        }

        float32 floatvalue;
        int32 intvalue;
        bool8 boolvalue;
};

IMPLEMENT_SCRIPT_CLASS_BEGIN(CBase, VOID)
    REGISTER_MEMBER(CBase, floatvalue, floatvalue);
    REGISTER_MEMBER(CBase, intvalue, intvalue);
    REGISTER_MEMBER(CBase, boolvalue, boolvalue);
IMPLEMENT_SCRIPT_CLASS_END()

REGISTER_METHOD_P0(CBase, GetFloatValue, GetFloatValue, float32);
REGISTER_METHOD_P0(CBase, GetIntValue, GetIntValue, int32);
REGISTER_METHOD_P0(CBase, GetBoolValue, GetBoolValue, bool8);

REGISTER_METHOD_P1(CBase, SetFloatValue, SetFloatValue, void, float32);
REGISTER_METHOD_P1(CBase, SetIntValue, SetIntValue, void, int32);
REGISTER_METHOD_P1(CBase, SetBoolValue, SetBoolValue, void, bool8);

class CChild : public CBase {
    public:
        CChild() : CBase() {
            MTPrint("Enter constructor CChild()\n");
            floatvalue = 19.0;
            intvalue = 11;
            boolvalue = false;
        }
        virtual ~CChild() {
            MTPrint("Enter destructor ~CChild()\n");
        }

        DECLARE_SCRIPT_CLASS(CChild, CBase);

        virtual void SetIntValue(int32 val) {
            MTPrint("Enter CChild::SetIntValue()\n");
            intvalue = 2 * val;
        }
};

IMPLEMENT_SCRIPT_CLASS_BEGIN(CChild, CBase)
IMPLEMENT_SCRIPT_CLASS_END()

REGISTER_METHOD_P1(CChild, SetIntValue, SetIntValue, void, int32);

// ------------------------------------------------------------------------------------------------
// -- Test weapon class
class CWeapon {
public:
    CWeapon() {
        MTPrint("CWeapon constructor\n");
        readytofire = true;

        // -- add to the linked list
        next = weaponlist;
        weaponlist = this;
    }

    virtual ~CWeapon() {
        if (weaponlist == this) {
            weaponlist = next;
        }
        else {
            CWeapon* curweapon = weaponlist;
            while(curweapon) {
                if(curweapon->next == this) {
                    curweapon->next = next;
                    break;
                }
            }
        }
    }

    DECLARE_SCRIPT_CLASS(CWeapon, VOID);

    static void UpdateWeaponList() {
        CWeapon* weapon = weaponlist;
        while(weapon) {
            weapon->Update();
            weapon = weapon->next;
        }
    }

    void Update() {
        int32 dummy = 0;
        TinScript::ObjExecF(TinScript::CScriptContext::GetMainThreadContext(), this, dummy,
                            "OnUpdate();");
    }

    static CWeapon* weaponlist;
    CWeapon* next;

private:
    bool8 readytofire;
};

CWeapon* CWeapon::weaponlist = NULL;

IMPLEMENT_SCRIPT_CLASS_BEGIN(CWeapon, VOID)
    REGISTER_MEMBER(CWeapon, readytofire, readytofire);
IMPLEMENT_SCRIPT_CLASS_END()

REGISTER_FUNCTION_P0(UpdateWeaponList, CWeapon::UpdateWeaponList, void);

// ------------------------------------------------------------------------------------------------
void BeginUnitTests(int32 teststart, int32 testend)
{
    // -- initialize if we have no test range
    if(teststart == 0 && testend == 0) {
        teststart = 0;
        testend = 9999;
    }

    // -- unit tests are run on the main thread
    TinScript::CScriptContext* script_context = TinScript::CScriptContext::GetMainThreadContext();

    // -- banner
    MTPrint("\n****************************\n");
    MTPrint("*** TinScript Unit Tests ***\n");
    MTPrint("****************************\n");

    MTPrint("\nExecuting unittest.ts\n");
	if(!script_context->ExecScript(kUnitTestScriptName)) {
		MTPrint("Error - unable to parse file: %s\n", kUnitTestScriptName);
		return;
	}

    int32 testindex = 0;
    MTPrint("***  VARIABLES, FLOW:  ************\n");

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Paren Expr: (((3 + 4) * 17) - (3 + 6)) %% (42 / 3) - next line prints: 12.667f\n");
        script_context->ExecCommand("TestParenthesis();");
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Test 'if' statement - next line prints: 'Equal to 9'\n");
        script_context->ExecCommand("TestIfStatement();");
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Test 'while' statement - next line prints the sequence: 5 to 1\n");
        script_context->ExecCommand("TestWhileStatement();");
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Test 'for' loop - next line prints the sequence 0 to 4\n");
        script_context->ExecCommand("TestForLoop();");
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        gCodeGlobalVariable = 17;
        MTPrint("Script access to registered global - next line prints: %d\n", gCodeGlobalVariable);
        script_context->ExecCommand("TestScriptAccessToGlobal();");
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Script access to modify registered global - next line prints: 43\n");
        script_context->ExecCommand("SetGlobalVarTo43();");
        MTPrint("gCodeGlobalVariable: %d\n", gCodeGlobalVariable);
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Code access to script global - next line prints: 12\n");
        int32 testscriptglobal = 0;
        if(!TinScript::GetGlobalVar(script_context, "gScriptGlobalVar", testscriptglobal)) {
		    MTPrint("Error - failed to find script global: gScriptGlobalVar\n");
		    return;
        }
        MTPrint("Found: %d\n", testscriptglobal);
    }

    // --
    /*
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("C3Vector cross((v0 + v1), v2) - next two lines print: 3.5355 and (-2.0 0.0 -1.5)\n");
        int32 dummy = 0;
        if(!TinScript::ExecF(script_context, dummy, "TestC3Vector();")) {
            MTPrint("Error - failed to find execute TestC3Vector()\n");
            return;
        }
    }
    */

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Test Hashtable variables - next two lines print: goodbye and 17.5\n");
        int32 dummy = 0;
        if(!TinScript::ExecF(script_context, dummy, "TestHashtables();")) {
            MTPrint("Error - failed to find execute TestHashtables()\n");
            return;
        }
    }
    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n\n***  GLOBAL FUNCTIONS:  ************\n");
        MTPrint("\n%d.  ", testindex);
        MTPrint("Script access to call a global function with a return value -\nnext line prints: 34\n");
        script_context->ExecCommand("CallMultIntByTwo(17);");
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Code access to call a script function with a return value -\nnext line prints: 4\n");
        int32 scriptreturn = 0;
        if(!TinScript::ExecF(script_context, scriptreturn, "ScriptMod9(%d);", 49)) {
		    MTPrint("Error - failed to execute script function CallScriptMod9()\n");
		    return;
        }
        MTPrint("Script result: %d\n", scriptreturn);
    }

    // -- 
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Nested function calls - next lines print: 32 and 13\n");
        int32 scriptreturn = 0;
        if(!TinScript::ExecF(script_context, scriptreturn, "TestNestedFunctions(%d);", 4)) {
		    MTPrint("Error - failed to execute script function TestNestedFunctions()\n");
		    return;
        }
    }

    // -- 
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Recursive function call - next lines prints 21\n");
        int32 scriptreturn = 0;
        if(!TinScript::ExecF(script_context, scriptreturn, "Fibonacci(%d);", 7)) {
		    MTPrint("Error - failed to execute script function Fibonacci()\n");
		    return;
        }
        MTPrint("Script result: %d\n", scriptreturn);
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n\n***  OBJECTS:  ************\n");
        MTPrint("\n%d.  ", testindex);
        MTPrint("Create an instance of a registered class from script -\n");
        MTPrint("next line prints:  Entering constructor CBase()\n");
        if(!script_context->ExecCommand("ScriptCreateObject();")) {
		    MTPrint("Error - failed to execute ScriptCreateObject()\n");
		    return;
        }
    }

    // --
    ++testindex;
    CBase* testobj = NULL;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Find the object from code by ID, cast and read the float32 member-\n");
        MTPrint("next line prints: 27.0\n");
        int32 testobjectid = 0;
        if(!TinScript::GetGlobalVar(script_context, "gScriptBaseObject", testobjectid)) {
		    MTPrint("Error - failed to find script global: gScriptBaseObject\n");
		    return;
        }
        testobj = static_cast<CBase*>(script_context->FindObject(testobjectid));
        if(!testobj) {
		    MTPrint("Error - code failed to find object: %d\n", testobjectid);
		    return;
        }
        MTPrint("%.2f\n", testobj->GetFloatValue());
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend && testobj) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Access the object member from script-\n");
        MTPrint("next two lines print: 35.0\n");
        if(!script_context->ExecCommand("ScriptModifyMember();")) {
		    MTPrint("Error - failed to execute ScriptModifyMember()\n");
		    return;
        }
        MTPrint("%.2f\n", testobj->GetFloatValue());
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend && testobj) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Access the object method from script-\n");
        MTPrint("next two lines print: 91\n");
        if(!script_context->ExecCommand("ScriptModifyMethod();")) {
		    MTPrint("Error - failed to execute ScriptModifyMethod()\n");
		    return;
        }
        MTPrint("%d\n", testobj->GetIntValue());
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Call a scripted method from script\n");
        MTPrint("next line prints: -65.0f\n");
        if(!script_context->ExecCommand("ScriptCallTestBaseMethod();")) {
		    MTPrint("Error - failed to execute ScriptCallTestBaseMethod()\n");
		    return;
        }
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Create a derived CChild instance from script\n");
        MTPrint("next line identifies both CBase and CChild constructors being called\n");
        if(!script_context->ExecCommand("ScriptCreateChildObject();")) {
		    MTPrint("Error - failed to execute ScriptCreateChildObject()\n");
		    return;
        }
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Call a derived method from script\n");
        MTPrint("next line prints: 24\n");
        if(!script_context->ExecCommand("ScriptCallChildMethod();")) {
		    MTPrint("Error - failed to execute ScriptCallChildMethod()\n");
		    return;
        }
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n%d.  ", testindex);
        MTPrint("Call the same method for both base and child objects\n");
        MTPrint("Notice the base object calls base method, the derived method calls the derived\n");
        MTPrint("next lines print: 18, followed by 36\n");
        if(!script_context->ExecCommand("ScriptCallBothObjectMethods();")) {
		    MTPrint("Error - failed to execute ScriptCallBothObjectMethods()\n");
		    return;
        }
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        MTPrint("\n\n***  NAMED OBJECTS:  ************\n");
        MTPrint("\n%d.  ", testindex);
        MTPrint("Create a CChild instance, but named 'TestNS'\n");
        MTPrint("Notice the constructors for both CBase, then CChild are called,\n");
        MTPrint("followed by the Scripted TestNS::OnCreate()\n");
        if(!script_context->ExecCommand("ScriptCreateNamedObject();")) {
		    MTPrint("Error - failed to execute ScriptCreateNamedObject()\n");
		    return;
        }
    }

    MTPrint("\n\n****************************\n");
    MTPrint("*** Unit Tests Complete ****\n");
    MTPrint("****************************\n");
}

REGISTER_FUNCTION_P2(BeginUnitTests, BeginUnitTests, void, int32, int32);

// ------------------------------------------------------------------------------------------------
// $$$TZA temporary C3Vector implementation
float32 C3Length(C3Vector v) {
    return (v.Length());
}

REGISTER_FUNCTION_P1(C3Length, C3Length, float32, C3Vector);

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
