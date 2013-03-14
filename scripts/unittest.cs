
// ------------------------------------------------------------------------------------------------
// unittest.cs
// ------------------------------------------------------------------------------------------------

// -- VARIABLES, FLOW -----------------------------------------------------------------------------
int gScriptGlobalVar = 12;

void TestScriptAccessToGlobal() {
    Print(gCodeGlobalVar);
}

void TestIfStatement() {
    int testvar = 9;
    if(testvar > 9)
        Print("Greater than 9");
    else if(testvar < 9)
        Print("Less than 9");
    else
        Print("Equal to 9");
}

void TestWhileStatement() {
    int testvar = 5;
    while(testvar > 0) {
        Print(testvar);
        testvar = testvar - 1;
    }
}

void TestForLoop() {
    int testvar;
    for(testvar = 0; testvar < 5; testvar = testvar + 1)
        Print(testvar);
}

void SetGlobalVarTo43() {
    gCodeGlobalVar = 43;
}

void TestParenthesis() {
    float result = (((3 + 4) * 17) - (3 + 6)) % (42 / 3);
    Print(result);
}

// -- GLOBAL FUNCTIONS ----------------------------------------------------------------------------
void CallMultIntByTwo(int number) {
    int multresult = MultIntByTwo(number);
    Print(multresult);
}

int ScriptMod9(int number) {
    return (number % 9);
}

int AddThree(int num) {
    return (num + 3);
}

int TestNestedFunctions(int value) {
    int result1 = MultIntByTwo(MultIntByTwo(MultIntByTwo(value)));
    Print(result1);
    int result2 = AddThree(AddThree(AddThree(value)));
    Print(result2);
}

int Fibonacci(int num) {
    if (num <= 1) {
        return 1;
    }
    else {
        int prev =Fibonacci(num - 1);
        int prevprev = Fibonacci(num - 2);
        int result = prev + prevprev;
        return result;
    }
}

// -- OBJECTS FUNCTIONS ---------------------------------------------------------------------------

object gScriptBaseObject;
void ScriptCreateObject() {
    gScriptBaseObject = create CBase();
}

void ScriptModifyMember() {
    gScriptBaseObject.floatvalue = 35.0f;
    Print(gScriptBaseObject.floatvalue);
}

void ScriptModifyMethod() {
    gScriptBaseObject.SetIntValue(91);
    Print(gScriptBaseObject.GetIntValue());
}

// -- OBJECTS METHODS -----------------------------------------------------------------------------

void CBase::CallBaseMethod() {
    Print("Enter CBase::CallBaseMethod()");
    self.floatvalue = self.floatvalue - 100.0f;
    Print(self.GetFloatValue());
}

void ScriptCallTestBaseMethod() {
    gScriptBaseObject.floatvalue = 35.0f;
    gScriptBaseObject.CallBaseMethod();
}

object gScriptChildObject;
void ScriptCreateChildObject() {
    gScriptChildObject = create CChild();
}

void ScriptCallChildMethod() {
    gScriptChildObject.SetIntValue(12);
    Print(gScriptChildObject.intvalue);
}

void CChild::CallChildMethod() {
    Print("Enter CBase::CallChildMethod()");
    self.SetFloatValue(123.0f);
    CBase::CallBaseMethod();
}

void ScriptCallChildScriptMethod() {
    gScriptChildObject.CallChildMethod();
    Print(gScriptChildObject.floatvalue);
}

void ScriptCallBothObjectMethods() {
    gScriptBaseObject.SetIntValue(18);
    gScriptChildObject.SetIntValue(18);
    Print(gScriptBaseObject.intvalue);
    Print(gScriptChildObject.intvalue);
}

// -- NAMED OBJECT METHODS ------------------------------------------------------------------------
object gScriptNamedObject;
void ScriptCreateNamedObject() {
    gScriptNamedObject = create CChild("ScriptNS");
}

void ScriptNS::OnCreate() {
    Print("Entering TestNS::OnCreate()");
    int self.schedrequest;
}

void ScriptNS::OnDestroy() {
    Print("Entering TestNS::OnDestroy()");
}

void ScriptNS::TestMethod() {
    self.floatvalue = 58.0f;
    Print("Entering TestNS::TestMethod()");
    Print(self.floatvalue);
}

void ScriptNS::TestThread() {
    self.intvalue = self.intvalue - 1;
    Print(" ");
    Print(self.intvalue);
    if(self.intvalue > 0) {
        //self.schedrequest = Schedule(self, 1000, "TestThread();");
        self.schedrequest = schedule(self, 1000, TestThread);
    }
    else {
        Print(" ");
        Print("Countdown complete!");
    }
}

void ThreadTestCount(int num) {
    Print(num);
    if(num > 0)
        schedule(0, 1000, ThreadTestCount, num - 1);
}

void BeginThreadTest() {
    gScriptNamedObject.intvalue = 30;
    gScriptNamedObject.TestThread();
}

// -- DEBUGGER Test Functions ----------------------------------------------------------------------
int MultBy3(int value) {
    int result = value * 3;
    return (result);
}

int MultBy6(int value) {
    int result = MultBy3(value) * 2;
    return (result);
}

int MultBy24(int value) {
    int result = MultBy6(value) * 4;
    return (result);
}

// -- LAMBDA? EXPRESSIONS --------------------------------------------------------------------------
void BeginLambdaTest(int value) {
    void decrement(int dec) {
        Print(dec);
        BeginLambdaTest(dec - 1);
    }
    Schedule(0, 1000, decrement(value));
}

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
