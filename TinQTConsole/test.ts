Print("Hello world!");

void CScriptObject::OnCreate()
{
    Print("Did this work??");
}

void Foo::OnCreate()
{
    Print("I'm a foo!");
    CScriptObject::OnCreate();
}

vector3f TestV3f(vector3f in_v)
{
    vector3f local_v;
    local_v:x = in_v:z;
    local_v:y = in_v:y;
    local_v:z = in_v:x;
    
    return (local_v);
}

void Test2V3f()
{
    object v0 = create CVector3f("v0");
    v0.x = 1; v0.y = 2; v0.z = 3;
    
    object v1 = create CVector3f("v1");
    v1.x = 4; v1.y = 5; v1.z = 6;
    
    object result = create CVector3f("result");
    
    destroy v1;
    
    ObjCross(result, v0, v1);
    result.ListMembers();
}

void SocketTest(int count)
{
    if (count < 0)
        return;
        
        
    string command = StringCat("Print('Foobar: ", count, "');");
    SocketSend(Print(command));
    schedule(0, 1000, Hash("SocketTest"), count -1);
}

void TestDebugObj::AddNumber(int number)
{
    object moar_test = create CScriptObject();
    self.foo += number;
    destroy moar_test;
}   

void TestDebugObj::OnCreate()
{
    int self.foo = 72;
}

void DebuggerTest(int num)
{
    object test_object = create CScriptObject("TestDebugObj");
    Print(test_object.foo);
    test_object.AddNumber(num);
    Print(test_object.foo);
    destroy test_object;
}

