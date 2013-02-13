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

bool8 gUnitTestIncludeMe = false;

// -- constants -----------------------------------------------------------------------------------
static const char* kUnitTestScriptName = "../scripts/unittest.cs";

// -- GLOBAL VARIABLES ----------------------------------------------------------------------------
int32 gCodeGlobalVariable = 17;
REGISTER_GLOBAL_VAR(gCodeGlobalVar, gCodeGlobalVariable);

// -- GLOBAL FUNCTIONS ----------------------------------------------------------------------------
int32 MultIntByTwo(int32 number) {
    return (number << 1);
}
REGISTER_FUNCTION_P1(MultIntByTwo, MultIntByTwo, int32, int32);

// --  REGISTERED CLASS----------------------------------------------------------------------------
class CBase {
    public:
        CBase() {
            printf("Enter constructor CBase()\n");
            floatvalue = 27.0;
            intvalue = 33;
            boolvalue = true;
        }
        virtual ~CBase() {
            printf("Enter destructor ~CBase()\n");
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
            printf("Enter CBase::SetIntValue()\n");
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

IMPLEMENT_SCRIPT_CLASS(CBase, VOID) {
    REGISTER_MEMBER(CBase, floatvalue, floatvalue);
    REGISTER_MEMBER(CBase, intvalue, intvalue);
    REGISTER_MEMBER(CBase, boolvalue, boolvalue);
}

REGISTER_METHOD_P0(CBase, GetFloatValue, GetFloatValue, float32);
REGISTER_METHOD_P0(CBase, GetIntValue, GetIntValue, int32);
REGISTER_METHOD_P0(CBase, GetBoolValue, GetBoolValue, bool8);

REGISTER_METHOD_P1(CBase, SetFloatValue, SetFloatValue, void, float32);
REGISTER_METHOD_P1(CBase, SetIntValue, SetIntValue, void, int32);
REGISTER_METHOD_P1(CBase, SetBoolValue, SetBoolValue, void, bool8);

class CChild : public CBase {
    public:
        CChild() : CBase() {
            printf("Enter constructor CChild()\n");
            floatvalue = 19.0;
            intvalue = 11;
            boolvalue = false;
        }
        virtual ~CChild() {
            printf("Enter destructor ~CChild()\n");
        }

        DECLARE_SCRIPT_CLASS(CChild, CBase);

        virtual void SetIntValue(int32 val) {
            printf("Enter CChild::SetIntValue()\n");
            intvalue = 2 * val;
        }
};

IMPLEMENT_SCRIPT_CLASS(CChild, CBase) {
}

REGISTER_METHOD_P1(CChild, SetIntValue, SetIntValue, void, int32);

// ------------------------------------------------------------------------------------------------
// -- Test weapon class
class CWeapon {
public:
    CWeapon() {
        printf("CWeapon constructor\n");
        readytofire = true;

        // -- add to the linked list
        next = weaponlist;
        weaponlist = this;
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
        uint32 objectid = GetObjectID();
        int32 dummy = 0;
        TinScript::ObjExecF(objectid, dummy, "OnUpdate();");
    }

    static CWeapon* weaponlist;
    CWeapon* next;

private:
    bool8 readytofire;
};

CWeapon* CWeapon::weaponlist = NULL;

IMPLEMENT_SCRIPT_CLASS(CWeapon, VOID) {
    REGISTER_MEMBER(CWeapon, readytofire, readytofire);
}

REGISTER_FUNCTION_P0(UpdateWeaponList, CWeapon::UpdateWeaponList, void);

// ------------------------------------------------------------------------------------------------
void BeginUnitTests(int32 teststart, int32 testend)
{
    // -- initialize if we have no test range
    if(teststart == 0 && testend == 0) {
        teststart = 0;
        testend = 9999;
    }

    // -- banner
    printf("\n****************************\n");
    printf("*** TinScript Unit Tests ***\n");
    printf("****************************\n");

    printf("\nExecuting unittest.cs\n");
	if(!TinScript::ExecScript(kUnitTestScriptName)) {
		printf("Error - unable to parse file: %s\n", kUnitTestScriptName);
		return;
	}

    int32 testindex = 0;
    printf("***  VARIABLES, FLOW:  ************\n");

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        gCodeGlobalVariable = 17;
        printf("Paren Expr: (((3 + 4) * 17) - (3 + 6)) %% (42 / 3) - next line prints: 12.0f\n");
        TinScript::ExecCommand("TestParenthesis();");
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        gCodeGlobalVariable = 17;
        printf("Test 'if' statement - next line prints: 'Equal to 9'\n");
        TinScript::ExecCommand("TestIfStatement();");
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        gCodeGlobalVariable = 17;
        printf("Test 'while' statement - next line prints the sequence: 5 to 1\n");
        TinScript::ExecCommand("TestWhileStatement();");
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        gCodeGlobalVariable = 17;
        printf("Test 'for' loop - next line prints the sequence 0 to 4\n");
        TinScript::ExecCommand("TestForLoop();");
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        gCodeGlobalVariable = 17;
        printf("Script access to registered global - next line prints: %d\n", gCodeGlobalVariable);
        TinScript::ExecCommand("TestScriptAccessToGlobal();");
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        printf("Script access to modify registered global - next line prints: 43\n");
        TinScript::ExecCommand("SetGlobalVarTo43();");
        printf("gCodeGlobalVariable: %d\n", gCodeGlobalVariable);
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        printf("Code access to script global - next line prints: 12\n");
        int32 testscriptglobal = 0;
        if(!TinScript::GetGlobalVar("gScriptGlobalVar", testscriptglobal)) {
		    printf("Error - failed to find script global: gScriptGlobalVar\n");
		    return;
        }
        printf("Found: %d\n", testscriptglobal);
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n\n***  GLOBAL FUNCTIONS:  ************\n");
        printf("\n%d.  ", testindex);
        printf("Script access to call a global function with a return value -\nnext line prints: 34\n");
        TinScript::ExecCommand("CallMultIntByTwo(17);");
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        printf("Code access to call a script function with a return value -\nnext line prints: 4\n");
        int32 scriptreturn = 0;
        if(!TinScript::ExecF(scriptreturn, "ScriptMod9(%d);", 49)) {
		    printf("Error - failed to execute script function CallScriptMod9()\n");
		    return;
        }
        printf("Script result: %d\n", scriptreturn);
    }

    // -- 
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        printf("Nested function calls - next lines print: 32 and 13\n");
        int32 scriptreturn = 0;
        if(!TinScript::ExecF(scriptreturn, "TestNestedFunctions(%d);", 4)) {
		    printf("Error - failed to execute script function TestNestedFunctions()\n");
		    return;
        }
    }

    // -- 
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        printf("Recursive function call - next lines prints 21\n");
        int32 scriptreturn = 0;
        if(!TinScript::ExecF(scriptreturn, "Fibonacci(%d);", 7)) {
		    printf("Error - failed to execute script function Fibonacci()\n");
		    return;
        }
        printf("Script result: %d\n", scriptreturn);
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n\n***  OBJECTS:  ************\n");
        printf("\n%d.  ", testindex);
        printf("Create an instance of a registered class from script -\n");
        printf("next line prints:  Entering constructor CBase()\n");
        if(!TinScript::ExecCommand("ScriptCreateObject();")) {
		    printf("Error - failed to execute ScriptCreateObject()\n");
		    return;
        }
    }

    // --
    ++testindex;
    CBase* testobj = NULL;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        printf("Find the object from code by ID, cast and read the float32 member-\n");
        printf("next line prints: 27.0\n");
        int32 testobjectid = 0;
        if(!TinScript::GetGlobalVar("gScriptBaseObject", testobjectid)) {
		    printf("Error - failed to find script global: gScriptBaseObject\n");
		    return;
        }
        testobj = static_cast<CBase*>(TinScript::FindObject(testobjectid));
        if(!testobj) {
		    printf("Error - code failed to find object: %d\n", testobjectid);
		    return;
        }
        printf("%.2f\n", testobj->GetFloatValue());
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend && testobj) {
        printf("\n%d.  ", testindex);
        printf("Access the object member from script-\n");
        printf("next two lines print: 35.0\n");
        if(!TinScript::ExecCommand("ScriptModifyMember();")) {
		    printf("Error - failed to execute ScriptModifyMember()\n");
		    return;
        }
        printf("%.2f\n", testobj->GetFloatValue());
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend && testobj) {
        printf("\n%d.  ", testindex);
        printf("Access the object method from script-\n");
        printf("next two lines print: 91\n");
        if(!TinScript::ExecCommand("ScriptModifyMethod();")) {
		    printf("Error - failed to execute ScriptModifyMethod()\n");
		    return;
        }
        printf("%d\n", testobj->GetIntValue());
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        printf("Call a scripted method from script\n");
        printf("next line prints: -65.0f\n");
        if(!TinScript::ExecCommand("ScriptCallTestBaseMethod();")) {
		    printf("Error - failed to execute ScriptCallTestBaseMethod()\n");
		    return;
        }
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        printf("Create a derived CChild instance from script\n");
        printf("next line identifies both CBase and CChild constructors being called\n");
        if(!TinScript::ExecCommand("ScriptCreateChildObject();")) {
		    printf("Error - failed to execute ScriptCreateChildObject()\n");
		    return;
        }
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        printf("Call a derived method from script\n");
        printf("next line prints: 24\n");
        if(!TinScript::ExecCommand("ScriptCallChildMethod();")) {
		    printf("Error - failed to execute ScriptCallChildMethod()\n");
		    return;
        }
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n%d.  ", testindex);
        printf("Call the same method for both base and child objects\n");
        printf("Notice the base object calls base method, the derived method calls the derived\n");
        printf("next lines print: 18, followed by 36\n");
        if(!TinScript::ExecCommand("ScriptCallBothObjectMethods();")) {
		    printf("Error - failed to execute ScriptCallBothObjectMethods()\n");
		    return;
        }
    }

    // --
    ++testindex;
    if(testindex >= teststart && testindex <= testend) {
        printf("\n\n***  NAMED OBJECTS:  ************\n");
        printf("\n%d.  ", testindex);
        printf("Create a CChild instance, but named 'TestNS'\n");
        printf("Notice the constructors for both CBase, then CChild are called,\n");
        printf("followed by the Scripted TestNS::OnCreate()\n");
        if(!TinScript::ExecCommand("ScriptCreateNamedObject();")) {
		    printf("Error - failed to execute ScriptCreateNamedObject()\n");
		    return;
        }
    }

    printf("\n\n****************************\n");
    printf("*** Unit Tests Complete ****\n");
    printf("****************************\n");
}

REGISTER_FUNCTION_P2(BeginUnitTests, BeginUnitTests, void, int32, int32);

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
