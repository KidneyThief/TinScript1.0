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
// Generated classes for function registration
// ------------------------------------------------------------------------------------------------


// -------------------
// Parameter count: 0
// -------------------

template<typename R>
class CRegFunctionP0 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)();

    // -- CRegisterFunctionP0
    CRegFunctionP0(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP0() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        Dispatch();
    }

    // -- dispatch method
    R Dispatch() {
        R r = funcptr();
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<>
class CRegFunctionP0<void> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)();

    // -- CRegisterFunctionP0
    CRegFunctionP0(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP0() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        Dispatch();
    }

    // -- dispatch method
    void Dispatch() {
        funcptr();
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename R>
class CRegContextFunctionP0 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(CScriptContext*);

    // -- CRegisterFunctionP0
    CRegContextFunctionP0(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP0() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        Dispatch();
    }

    // -- dispatch method
    R Dispatch() {
        R r = funcptr(GetScriptContext());
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<>
class CRegContextFunctionP0<void> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(CScriptContext*);

    // -- CRegisterFunctionP0
    CRegContextFunctionP0(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP0() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        Dispatch();
    }

    // -- dispatch method
    void Dispatch() {
        funcptr(GetScriptContext());
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename C, typename R>
class CRegMethodP0 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(C* c);

    // -- CRegisterMethodP0
    CRegMethodP0(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP0() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        Dispatch(objaddr);
    }

    // -- dispatch method
    R Dispatch(void* objaddr) {
        C* objptr = (C*)objaddr;
        R r = funcptr(objptr);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C>
class CRegMethodP0<C, void> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(C* c);

    // -- CRegisterMethodP0
    CRegMethodP0(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP0() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        Dispatch(objaddr);
    }

    // -- dispatch method
    void Dispatch(void* objaddr) {
        C* objptr = (C*)objaddr;
        funcptr(objptr);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename R>
class CRegContextMethodP0 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(CScriptContext*, C* c);

    // -- CRegisterMethodP0
    CRegContextMethodP0(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP0() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        Dispatch(objaddr);
    }

    // -- dispatch method
    R Dispatch(void* objaddr) {
        C* objptr = (C*)objaddr;
        R r = funcptr(GetScriptContext(), objptr);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C>
class CRegContextMethodP0<C, void> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(CScriptContext*, C* c);

    // -- CRegisterMethodP0
    CRegContextMethodP0(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP0() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        Dispatch(objaddr);
    }

    // -- dispatch method
    void Dispatch(void* objaddr) {
        C* objptr = (C*)objaddr;
        funcptr(GetScriptContext(), objptr);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};


// -------------------
// Parameter count: 1
// -------------------

template<typename R, typename T1>
class CRegFunctionP1 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(T1 p1);

    // -- CRegisterFunctionP1
    CRegFunctionP1(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP1() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1) {
        R r = funcptr(p1);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1>
class CRegFunctionP1<void, T1> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(T1 p1);

    // -- CRegisterFunctionP1
    CRegFunctionP1(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP1() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1) {
        funcptr(p1);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename R, typename T1>
class CRegContextFunctionP1 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(CScriptContext*, T1 p1);

    // -- CRegisterFunctionP1
    CRegContextFunctionP1(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP1() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1) {
        R r = funcptr(GetScriptContext(), p1);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1>
class CRegContextFunctionP1<void, T1> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(CScriptContext*, T1 p1);

    // -- CRegisterFunctionP1
    CRegContextFunctionP1(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP1() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1) {
        funcptr(GetScriptContext(), p1);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename C, typename R, typename T1>
class CRegMethodP1 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(C* c, T1 p1);

    // -- CRegisterMethodP1
    CRegMethodP1(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP1() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1) {
        C* objptr = (C*)objaddr;
        R r = funcptr(objptr, p1);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1>
class CRegMethodP1<C, void, T1> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(C* c, T1 p1);

    // -- CRegisterMethodP1
    CRegMethodP1(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP1() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1) {
        C* objptr = (C*)objaddr;
        funcptr(objptr, p1);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename R, typename T1>
class CRegContextMethodP1 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(CScriptContext*, C* c, T1 p1);

    // -- CRegisterMethodP1
    CRegContextMethodP1(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP1() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1) {
        C* objptr = (C*)objaddr;
        R r = funcptr(GetScriptContext(), objptr, p1);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1>
class CRegContextMethodP1<C, void, T1> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(CScriptContext*, C* c, T1 p1);

    // -- CRegisterMethodP1
    CRegContextMethodP1(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP1() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1) {
        C* objptr = (C*)objaddr;
        funcptr(GetScriptContext(), objptr, p1);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};


// -------------------
// Parameter count: 2
// -------------------

template<typename R, typename T1, typename T2>
class CRegFunctionP2 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(T1 p1, T2 p2);

    // -- CRegisterFunctionP2
    CRegFunctionP2(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP2() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2) {
        R r = funcptr(p1, p2);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2>
class CRegFunctionP2<void, T1, T2> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(T1 p1, T2 p2);

    // -- CRegisterFunctionP2
    CRegFunctionP2(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP2() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2) {
        funcptr(p1, p2);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename R, typename T1, typename T2>
class CRegContextFunctionP2 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(CScriptContext*, T1 p1, T2 p2);

    // -- CRegisterFunctionP2
    CRegContextFunctionP2(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP2() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2) {
        R r = funcptr(GetScriptContext(), p1, p2);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2>
class CRegContextFunctionP2<void, T1, T2> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(CScriptContext*, T1 p1, T2 p2);

    // -- CRegisterFunctionP2
    CRegContextFunctionP2(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP2() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2) {
        funcptr(GetScriptContext(), p1, p2);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2>
class CRegMethodP2 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(C* c, T1 p1, T2 p2);

    // -- CRegisterMethodP2
    CRegMethodP2(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP2() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2) {
        C* objptr = (C*)objaddr;
        R r = funcptr(objptr, p1, p2);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2>
class CRegMethodP2<C, void, T1, T2> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(C* c, T1 p1, T2 p2);

    // -- CRegisterMethodP2
    CRegMethodP2(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP2() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2) {
        C* objptr = (C*)objaddr;
        funcptr(objptr, p1, p2);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2>
class CRegContextMethodP2 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2);

    // -- CRegisterMethodP2
    CRegContextMethodP2(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP2() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2) {
        C* objptr = (C*)objaddr;
        R r = funcptr(GetScriptContext(), objptr, p1, p2);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2>
class CRegContextMethodP2<C, void, T1, T2> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2);

    // -- CRegisterMethodP2
    CRegContextMethodP2(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP2() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2) {
        C* objptr = (C*)objaddr;
        funcptr(GetScriptContext(), objptr, p1, p2);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};


// -------------------
// Parameter count: 3
// -------------------

template<typename R, typename T1, typename T2, typename T3>
class CRegFunctionP3 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(T1 p1, T2 p2, T3 p3);

    // -- CRegisterFunctionP3
    CRegFunctionP3(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP3() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2, T3 p3) {
        R r = funcptr(p1, p2, p3);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2, typename T3>
class CRegFunctionP3<void, T1, T2, T3> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(T1 p1, T2 p2, T3 p3);

    // -- CRegisterFunctionP3
    CRegFunctionP3(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP3() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2, T3 p3) {
        funcptr(p1, p2, p3);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename R, typename T1, typename T2, typename T3>
class CRegContextFunctionP3 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(CScriptContext*, T1 p1, T2 p2, T3 p3);

    // -- CRegisterFunctionP3
    CRegContextFunctionP3(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP3() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2, T3 p3) {
        R r = funcptr(GetScriptContext(), p1, p2, p3);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2, typename T3>
class CRegContextFunctionP3<void, T1, T2, T3> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(CScriptContext*, T1 p1, T2 p2, T3 p3);

    // -- CRegisterFunctionP3
    CRegContextFunctionP3(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP3() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2, T3 p3) {
        funcptr(GetScriptContext(), p1, p2, p3);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2, typename T3>
class CRegMethodP3 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(C* c, T1 p1, T2 p2, T3 p3);

    // -- CRegisterMethodP3
    CRegMethodP3(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP3() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3) {
        C* objptr = (C*)objaddr;
        R r = funcptr(objptr, p1, p2, p3);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2, typename T3>
class CRegMethodP3<C, void, T1, T2, T3> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(C* c, T1 p1, T2 p2, T3 p3);

    // -- CRegisterMethodP3
    CRegMethodP3(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP3() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3) {
        C* objptr = (C*)objaddr;
        funcptr(objptr, p1, p2, p3);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2, typename T3>
class CRegContextMethodP3 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2, T3 p3);

    // -- CRegisterMethodP3
    CRegContextMethodP3(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP3() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3) {
        C* objptr = (C*)objaddr;
        R r = funcptr(GetScriptContext(), objptr, p1, p2, p3);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2, typename T3>
class CRegContextMethodP3<C, void, T1, T2, T3> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2, T3 p3);

    // -- CRegisterMethodP3
    CRegContextMethodP3(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP3() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3) {
        C* objptr = (C*)objaddr;
        funcptr(GetScriptContext(), objptr, p1, p2, p3);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};


// -------------------
// Parameter count: 4
// -------------------

template<typename R, typename T1, typename T2, typename T3, typename T4>
class CRegFunctionP4 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(T1 p1, T2 p2, T3 p3, T4 p4);

    // -- CRegisterFunctionP4
    CRegFunctionP4(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP4() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2, T3 p3, T4 p4) {
        R r = funcptr(p1, p2, p3, p4);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2, typename T3, typename T4>
class CRegFunctionP4<void, T1, T2, T3, T4> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(T1 p1, T2 p2, T3 p3, T4 p4);

    // -- CRegisterFunctionP4
    CRegFunctionP4(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP4() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2, T3 p3, T4 p4) {
        funcptr(p1, p2, p3, p4);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename R, typename T1, typename T2, typename T3, typename T4>
class CRegContextFunctionP4 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(CScriptContext*, T1 p1, T2 p2, T3 p3, T4 p4);

    // -- CRegisterFunctionP4
    CRegContextFunctionP4(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP4() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2, T3 p3, T4 p4) {
        R r = funcptr(GetScriptContext(), p1, p2, p3, p4);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2, typename T3, typename T4>
class CRegContextFunctionP4<void, T1, T2, T3, T4> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(CScriptContext*, T1 p1, T2 p2, T3 p3, T4 p4);

    // -- CRegisterFunctionP4
    CRegContextFunctionP4(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP4() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2, T3 p3, T4 p4) {
        funcptr(GetScriptContext(), p1, p2, p3, p4);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2, typename T3, typename T4>
class CRegMethodP4 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(C* c, T1 p1, T2 p2, T3 p3, T4 p4);

    // -- CRegisterMethodP4
    CRegMethodP4(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP4() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4) {
        C* objptr = (C*)objaddr;
        R r = funcptr(objptr, p1, p2, p3, p4);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2, typename T3, typename T4>
class CRegMethodP4<C, void, T1, T2, T3, T4> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(C* c, T1 p1, T2 p2, T3 p3, T4 p4);

    // -- CRegisterMethodP4
    CRegMethodP4(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP4() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4) {
        C* objptr = (C*)objaddr;
        funcptr(objptr, p1, p2, p3, p4);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2, typename T3, typename T4>
class CRegContextMethodP4 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2, T3 p3, T4 p4);

    // -- CRegisterMethodP4
    CRegContextMethodP4(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP4() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4) {
        C* objptr = (C*)objaddr;
        R r = funcptr(GetScriptContext(), objptr, p1, p2, p3, p4);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2, typename T3, typename T4>
class CRegContextMethodP4<C, void, T1, T2, T3, T4> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2, T3 p3, T4 p4);

    // -- CRegisterMethodP4
    CRegContextMethodP4(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP4() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4) {
        C* objptr = (C*)objaddr;
        funcptr(GetScriptContext(), objptr, p1, p2, p3, p4);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};


// -------------------
// Parameter count: 5
// -------------------

template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
class CRegFunctionP5 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5);

    // -- CRegisterFunctionP5
    CRegFunctionP5(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP5() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5) {
        R r = funcptr(p1, p2, p3, p4, p5);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2, typename T3, typename T4, typename T5>
class CRegFunctionP5<void, T1, T2, T3, T4, T5> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5);

    // -- CRegisterFunctionP5
    CRegFunctionP5(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP5() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5) {
        funcptr(p1, p2, p3, p4, p5);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
class CRegContextFunctionP5 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(CScriptContext*, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5);

    // -- CRegisterFunctionP5
    CRegContextFunctionP5(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP5() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5) {
        R r = funcptr(GetScriptContext(), p1, p2, p3, p4, p5);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2, typename T3, typename T4, typename T5>
class CRegContextFunctionP5<void, T1, T2, T3, T4, T5> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(CScriptContext*, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5);

    // -- CRegisterFunctionP5
    CRegContextFunctionP5(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP5() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5) {
        funcptr(GetScriptContext(), p1, p2, p3, p4, p5);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
class CRegMethodP5 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5);

    // -- CRegisterMethodP5
    CRegMethodP5(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP5() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5) {
        C* objptr = (C*)objaddr;
        R r = funcptr(objptr, p1, p2, p3, p4, p5);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2, typename T3, typename T4, typename T5>
class CRegMethodP5<C, void, T1, T2, T3, T4, T5> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5);

    // -- CRegisterMethodP5
    CRegMethodP5(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP5() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5) {
        C* objptr = (C*)objaddr;
        funcptr(objptr, p1, p2, p3, p4, p5);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
class CRegContextMethodP5 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5);

    // -- CRegisterMethodP5
    CRegContextMethodP5(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP5() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5) {
        C* objptr = (C*)objaddr;
        R r = funcptr(GetScriptContext(), objptr, p1, p2, p3, p4, p5);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2, typename T3, typename T4, typename T5>
class CRegContextMethodP5<C, void, T1, T2, T3, T4, T5> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5);

    // -- CRegisterMethodP5
    CRegContextMethodP5(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP5() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5) {
        C* objptr = (C*)objaddr;
        funcptr(GetScriptContext(), objptr, p1, p2, p3, p4, p5);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};


// -------------------
// Parameter count: 6
// -------------------

template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class CRegFunctionP6 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6);

    // -- CRegisterFunctionP6
    CRegFunctionP6(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP6() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6) {
        R r = funcptr(p1, p2, p3, p4, p5, p6);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class CRegFunctionP6<void, T1, T2, T3, T4, T5, T6> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6);

    // -- CRegisterFunctionP6
    CRegFunctionP6(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP6() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6) {
        funcptr(p1, p2, p3, p4, p5, p6);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class CRegContextFunctionP6 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(CScriptContext*, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6);

    // -- CRegisterFunctionP6
    CRegContextFunctionP6(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP6() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6) {
        R r = funcptr(GetScriptContext(), p1, p2, p3, p4, p5, p6);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class CRegContextFunctionP6<void, T1, T2, T3, T4, T5, T6> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(CScriptContext*, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6);

    // -- CRegisterFunctionP6
    CRegContextFunctionP6(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP6() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6) {
        funcptr(GetScriptContext(), p1, p2, p3, p4, p5, p6);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class CRegMethodP6 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6);

    // -- CRegisterMethodP6
    CRegMethodP6(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP6() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6) {
        C* objptr = (C*)objaddr;
        R r = funcptr(objptr, p1, p2, p3, p4, p5, p6);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class CRegMethodP6<C, void, T1, T2, T3, T4, T5, T6> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6);

    // -- CRegisterMethodP6
    CRegMethodP6(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP6() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6) {
        C* objptr = (C*)objaddr;
        funcptr(objptr, p1, p2, p3, p4, p5, p6);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class CRegContextMethodP6 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6);

    // -- CRegisterMethodP6
    CRegContextMethodP6(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP6() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6) {
        C* objptr = (C*)objaddr;
        R r = funcptr(GetScriptContext(), objptr, p1, p2, p3, p4, p5, p6);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class CRegContextMethodP6<C, void, T1, T2, T3, T4, T5, T6> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6);

    // -- CRegisterMethodP6
    CRegContextMethodP6(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP6() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6) {
        C* objptr = (C*)objaddr;
        funcptr(GetScriptContext(), objptr, p1, p2, p3, p4, p5, p6);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};


// -------------------
// Parameter count: 7
// -------------------

template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class CRegFunctionP7 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7);

    // -- CRegisterFunctionP7
    CRegFunctionP7(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP7() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7) {
        R r = funcptr(p1, p2, p3, p4, p5, p6, p7);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class CRegFunctionP7<void, T1, T2, T3, T4, T5, T6, T7> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7);

    // -- CRegisterFunctionP7
    CRegFunctionP7(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP7() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7) {
        funcptr(p1, p2, p3, p4, p5, p6, p7);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class CRegContextFunctionP7 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(CScriptContext*, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7);

    // -- CRegisterFunctionP7
    CRegContextFunctionP7(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP7() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7) {
        R r = funcptr(GetScriptContext(), p1, p2, p3, p4, p5, p6, p7);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class CRegContextFunctionP7<void, T1, T2, T3, T4, T5, T6, T7> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(CScriptContext*, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7);

    // -- CRegisterFunctionP7
    CRegContextFunctionP7(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP7() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7) {
        funcptr(GetScriptContext(), p1, p2, p3, p4, p5, p6, p7);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class CRegMethodP7 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7);

    // -- CRegisterMethodP7
    CRegMethodP7(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP7() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7) {
        C* objptr = (C*)objaddr;
        R r = funcptr(objptr, p1, p2, p3, p4, p5, p6, p7);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class CRegMethodP7<C, void, T1, T2, T3, T4, T5, T6, T7> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7);

    // -- CRegisterMethodP7
    CRegMethodP7(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP7() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7) {
        C* objptr = (C*)objaddr;
        funcptr(objptr, p1, p2, p3, p4, p5, p6, p7);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class CRegContextMethodP7 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7);

    // -- CRegisterMethodP7
    CRegContextMethodP7(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP7() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7) {
        C* objptr = (C*)objaddr;
        R r = funcptr(GetScriptContext(), objptr, p1, p2, p3, p4, p5, p6, p7);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class CRegContextMethodP7<C, void, T1, T2, T3, T4, T5, T6, T7> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7);

    // -- CRegisterMethodP7
    CRegContextMethodP7(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP7() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7) {
        C* objptr = (C*)objaddr;
        funcptr(GetScriptContext(), objptr, p1, p2, p3, p4, p5, p6, p7);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};


// -------------------
// Parameter count: 8
// -------------------

template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class CRegFunctionP8 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8);

    // -- CRegisterFunctionP8
    CRegFunctionP8(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP8() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        CVariableEntry* ve8 = GetContext()->GetParameter(8);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)),
                 convert_from_void_ptr<T8>::Convert(ve8->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8) {
        R r = funcptr(p1, p2, p3, p4, p5, p6, p7, p8);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));
        GetContext()->AddParameter("_p8", Hash("_p8"), GetRegisteredType(GetTypeID<T8>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class CRegFunctionP8<void, T1, T2, T3, T4, T5, T6, T7, T8> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8);

    // -- CRegisterFunctionP8
    CRegFunctionP8(const char* _funcname, funcsignature _funcptr) :
                    CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegFunctionP8() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        CVariableEntry* ve8 = GetContext()->GetParameter(8);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)),
                 convert_from_void_ptr<T8>::Convert(ve8->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8) {
        funcptr(p1, p2, p3, p4, p5, p6, p7, p8);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));
        GetContext()->AddParameter("_p8", Hash("_p8"), GetRegisteredType(GetTypeID<T8>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class CRegContextFunctionP8 : public CRegFunctionBase {
public:

    typedef R (*funcsignature)(CScriptContext*, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8);

    // -- CRegisterFunctionP8
    CRegContextFunctionP8(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP8() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        CVariableEntry* ve8 = GetContext()->GetParameter(8);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)),
                 convert_from_void_ptr<T8>::Convert(ve8->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8) {
        R r = funcptr(GetScriptContext(), p1, p2, p3, p4, p5, p6, p7, p8);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));
        GetContext()->AddParameter("_p8", Hash("_p8"), GetRegisteredType(GetTypeID<T8>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class CRegContextFunctionP8<void, T1, T2, T3, T4, T5, T6, T7, T8> : public CRegFunctionBase {
public:

    typedef void (*funcsignature)(CScriptContext*, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8);

    // -- CRegisterFunctionP8
    CRegContextFunctionP8(const char* _funcname, funcsignature _funcptr) :
                           CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextFunctionP8() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        CVariableEntry* ve8 = GetContext()->GetParameter(8);
        Dispatch(convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)),
                 convert_from_void_ptr<T8>::Convert(ve8->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8) {
        funcptr(GetScriptContext(), p1, p2, p3, p4, p5, p6, p7, p8);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, 0, GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));
        GetContext()->AddParameter("_p8", Hash("_p8"), GetRegisteredType(GetTypeID<T8>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* globalfunctable = script_context->FindNamespace(0)->GetFuncTable();
        globalfunctable->AddItem(*fe, hash);
    }

private:
    funcsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class CRegMethodP8 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8);

    // -- CRegisterMethodP8
    CRegMethodP8(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP8() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        CVariableEntry* ve8 = GetContext()->GetParameter(8);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)),
                 convert_from_void_ptr<T8>::Convert(ve8->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8) {
        C* objptr = (C*)objaddr;
        R r = funcptr(objptr, p1, p2, p3, p4, p5, p6, p7, p8);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));
        GetContext()->AddParameter("_p8", Hash("_p8"), GetRegisteredType(GetTypeID<T8>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class CRegMethodP8<C, void, T1, T2, T3, T4, T5, T6, T7, T8> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8);

    // -- CRegisterMethodP8
    CRegMethodP8(const char* _funcname, methodsignature _funcptr) :
                  CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegMethodP8() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        CVariableEntry* ve8 = GetContext()->GetParameter(8);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)),
                 convert_from_void_ptr<T8>::Convert(ve8->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8) {
        C* objptr = (C*)objaddr;
        funcptr(objptr, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));
        GetContext()->AddParameter("_p8", Hash("_p8"), GetRegisteredType(GetTypeID<T8>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class CRegContextMethodP8 : public CRegFunctionBase {
public:

    typedef R (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8);

    // -- CRegisterMethodP8
    CRegContextMethodP8(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP8() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        CVariableEntry* ve8 = GetContext()->GetParameter(8);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)),
                 convert_from_void_ptr<T8>::Convert(ve8->GetValueAddr(NULL)));
    }

    // -- dispatch method
    R Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8) {
        C* objptr = (C*)objaddr;
        R r = funcptr(GetScriptContext(), objptr, p1, p2, p3, p4, p5, p6, p7, p8);
        assert(GetContext()->GetParameter(0));
        CVariableEntry* returnval = GetContext()->GetParameter(0);
        returnval->SetValueAddr(NULL, convert_to_void_ptr<R>::Convert(r));
        return (r);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), GetRegisteredType(GetTypeID<R>()));
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));
        GetContext()->AddParameter("_p8", Hash("_p8"), GetRegisteredType(GetTypeID<T8>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

template<typename C, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class CRegContextMethodP8<C, void, T1, T2, T3, T4, T5, T6, T7, T8> : public CRegFunctionBase {
public:

    typedef void (*methodsignature)(CScriptContext*, C* c, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8);

    // -- CRegisterMethodP8
    CRegContextMethodP8(const char* _funcname, methodsignature _funcptr) :
                         CRegFunctionBase(_funcname) {
        funcptr = _funcptr;
    }

    // -- destructor
    virtual ~CRegContextMethodP8() {
    }

    // -- virtual DispatchFunction wrapper
    virtual void DispatchFunction(void* objaddr) {
        CVariableEntry* ve1 = GetContext()->GetParameter(1);
        CVariableEntry* ve2 = GetContext()->GetParameter(2);
        CVariableEntry* ve3 = GetContext()->GetParameter(3);
        CVariableEntry* ve4 = GetContext()->GetParameter(4);
        CVariableEntry* ve5 = GetContext()->GetParameter(5);
        CVariableEntry* ve6 = GetContext()->GetParameter(6);
        CVariableEntry* ve7 = GetContext()->GetParameter(7);
        CVariableEntry* ve8 = GetContext()->GetParameter(8);
        Dispatch(objaddr,
                 convert_from_void_ptr<T1>::Convert(ve1->GetValueAddr(NULL)),
                 convert_from_void_ptr<T2>::Convert(ve2->GetValueAddr(NULL)),
                 convert_from_void_ptr<T3>::Convert(ve3->GetValueAddr(NULL)),
                 convert_from_void_ptr<T4>::Convert(ve4->GetValueAddr(NULL)),
                 convert_from_void_ptr<T5>::Convert(ve5->GetValueAddr(NULL)),
                 convert_from_void_ptr<T6>::Convert(ve6->GetValueAddr(NULL)),
                 convert_from_void_ptr<T7>::Convert(ve7->GetValueAddr(NULL)),
                 convert_from_void_ptr<T8>::Convert(ve8->GetValueAddr(NULL)));
    }

    // -- dispatch method
    void Dispatch(void* objaddr, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8) {
        C* objptr = (C*)objaddr;
        funcptr(GetScriptContext(), objptr, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    // -- registration method
    virtual void Register(CScriptContext* script_context) {
        CFunctionEntry* fe = new CFunctionEntry(script_context, Hash(C::GetClassName()), GetName(), Hash(GetName()), eFuncTypeGlobal, this);
        SetScriptContext(script_context);
        SetContext(fe->GetContext());
        GetContext()->AddParameter("__return", Hash("__return"), TYPE_void);
        GetContext()->AddParameter("_p1", Hash("_p1"), GetRegisteredType(GetTypeID<T1>()));
        GetContext()->AddParameter("_p2", Hash("_p2"), GetRegisteredType(GetTypeID<T2>()));
        GetContext()->AddParameter("_p3", Hash("_p3"), GetRegisteredType(GetTypeID<T3>()));
        GetContext()->AddParameter("_p4", Hash("_p4"), GetRegisteredType(GetTypeID<T4>()));
        GetContext()->AddParameter("_p5", Hash("_p5"), GetRegisteredType(GetTypeID<T5>()));
        GetContext()->AddParameter("_p6", Hash("_p6"), GetRegisteredType(GetTypeID<T6>()));
        GetContext()->AddParameter("_p7", Hash("_p7"), GetRegisteredType(GetTypeID<T7>()));
        GetContext()->AddParameter("_p8", Hash("_p8"), GetRegisteredType(GetTypeID<T8>()));

        unsigned int hash = fe->GetHash();
        tFuncTable* methodtable = C::classnamespace->GetFuncTable();
        methodtable->AddItem(*fe, hash);
    }

private:
    methodsignature funcptr;
};

