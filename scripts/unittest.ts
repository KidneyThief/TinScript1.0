
// ------------------------------------------------------------------------------------------------
// unittest.cs
// ------------------------------------------------------------------------------------------------

// -- VARIABLES, FLOW -----------------------------------------------------------------------------
int gScriptGlobalVar = 12;

void UnitTest_RegisteredIntAccess()
{
    // -- This registered int was set in code - retrieving here
    gUnitTestScriptResult = StringCat(gUnitTestRegisteredInt);
}

void UnitTest_RegisteredIntModify()
{
    // -- This registered int was set in code - modifying the value
    gUnitTestRegisteredInt = 23;
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

//void SetGlobalVarTo43() {
//    gCodeGlobalVar = 43;
//}

void TestParenthesis() {
    float result = (((3 + 4) * 17) - (3.0f + 6)) % (42 / 3);
    gUnitTestScriptResult = StringCat(result);
}

hashtable gHashTable;
string gHashTable["hello"] = "goodbye";
float gHashTable["goodbye"] = 17.5f;
void TestHashtables() {
    Print(gHashTable["hello"]);
    Print(gHashTable[gHashTable["hello"]]);
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
int FibIterative(int num) {
    if(num < 2) {
        return num;
    }
    int cur = 1;
    int prev = 1;
    
    // -- the first two numbers have already been taken care of
    num = num - 2;
    while (num > 0) {
        int temp = cur;
        cur = cur + prev;
        prev = temp;
        num = num - 1;
    }
    return (cur);
}

// -- CVector3f functions--------------------------------------------------------------------------
void TestObjCVector3f()
{
    object v0 = create CVector3f();
    v0.Set(1.0f, 2.0f, 3.0f);
    Print(v0.Length());
    
    object v1 = create CVector3f();
    v1.Set(4.0f, 5.0f, 6.0f);
    
    object cross_result = create CVector3f();
    if (ObjCross(cross_result, v0, v1))
    {
        Print("The cross product is: (", cross_result.x, ", ", cross_result.y, ", ", cross_result.z, ")");
    }
    
    float dot = ObjDot(v0, v1);
    Print("The dot product is: ", dot);
    
    // -- cleanup
    destroy v0;
    destroy v1;
    destroy cross_result;
}

void TestTypeCVector3f()
{
    vector3f v0 = "1 2 3";
    Print(V3fLength(v0));
    
    vector3f v1 = "4 5 6";
    
    vector3f cross_result = V3fCross(v0, v1);
    Print("The cross product is: (", cross_result:x, ", ", cross_result:y, ", ", cross_result:z, ")");
    
    float dot = V3fDot(v0, v1);
    Print("The dot product is: ", dot);
}

vector3f TestReturnV3f(vector3f in_v)
{
    vector3f local_v = in_v;
    local_v:y = 0.0f;
    return (local_v);
}

float TestReturnPODMember(vector3f in_v)
{
    float y_value = in_v:y;
    return (y_value);
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
        self.schedrequest = schedule(self, 1000, Hash("TestThread"));
    }
    else {
        Print(" ");
        Print("Countdown complete!");
    }
}

void ThreadTestCount(int num) {
    Print(num);
    if(num > 0)
        schedule(0, 1000, Hash("ThreadTestCount"), num - 1);
}

void BeginThreadTest() {
    gScriptNamedObject.intvalue = 30;
    gScriptNamedObject.TestThread();
}

// -- MultiThread test -----------------------------------------------------------------------------
// -- declaring a global script variable - this must be unique to each thead
string gMultiThreadVariable = "<uninitialized>";
void MultiThreadTestFunction(string value)
{
    gMultiThreadVariable = value;
	ListObjects();
}


// -- DEBUGGER Test Functions ----------------------------------------------------------------------
int MultBy3(int value) {
    int test_watch_var = 17;
    int result = value * 3;
    return (result);
}

int MultBy6(int value) {
    int test_watch_var2 = 29;
    int result = MultBy3(value) * 2;
    return (result);
}

int MultBy24(int value) {
    int test_watch_var3 = 5;
    object test_obj = gScriptNamedObject;
    int result = MultBy6(value) * 4;
    return (result);
}

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
