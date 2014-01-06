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

// -- includes
#include "mathutil.h"

// -- includes required by any system wanting access to TinScript
#include "integration.h"
#include "TinScript.h"
#include "TinRegistration.h"

// -- use the DECLARE_FILE/REGISTER_FILE macros to prevent deadstripping
DECLARE_FILE(mathutil_cpp);

// == CVector3f =======================================================================================================

const CVector3f CVector3f::zero(0.0f, 0.0f, 0.0f);
const CVector3f CVector3f::realmax(1e8f, 1e8f, 1e8f);

// ====================================================================================================================
// Set():  Sets the x,y,z values
// ====================================================================================================================
void CVector3f::Set(float32 _x, float32 _y, float32 _z)
{
    x = _x;
    y = _y;
    z = _z;
}

// ====================================================================================================================
// Length():  Returns the length of the vector
// ====================================================================================================================
float32 CVector3f::Length()
{
    float32 length = sqrt(x*x + y*y + z*z);
    return (length);
}

// ====================================================================================================================
// Length():  Normalizes the vector, returns the length
// ====================================================================================================================
float32 CVector3f::Normalize()
{
    float32 length = sqrt(x*x + y*y + z*z);
    if (length > 0.0f)
    {
        x /= length;
        y /= length;
        z /= length;
    }

    // -- return the length
    return (length);
}

// ====================================================================================================================
// Cross():  Returns the cross product of two vectors
// ====================================================================================================================
CVector3f CVector3f::Cross(CVector3f v0, CVector3f v1)
{
    CVector3f result(v0.y * v1.z - v0.z * v1.y,
                     v0.z * v1.x - v0.x * v1.z,
                     v0.x * v1.y - v0.y * v1.x);
    return (result);
}

// ====================================================================================================================
// Dot():  Returns the dot product of two vectors
// ====================================================================================================================
float32 CVector3f::Dot(CVector3f v0, CVector3f v1)
{
    float dot = v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
    return dot;
}

// ====================================================================================================================
// Normalized():  Returns the value of the input vector normalized
// ====================================================================================================================
CVector3f CVector3f::Normalized(CVector3f v0)
{
    v0.Normalize();
    return (v0);
}

// ====================================================================================================================
// Length():  Returns the length of the input vector
// ====================================================================================================================
float32 CVector3f::Length(CVector3f v0)
{
    return (v0.Length());
}

// --------------------------------------------------------------------------------------------------------------------
// -- registration

IMPLEMENT_SCRIPT_CLASS_BEGIN(CVector3f, VOID)
    REGISTER_MEMBER(CVector3f, x, x);
    REGISTER_MEMBER(CVector3f, y, y);
    REGISTER_MEMBER(CVector3f, z, z);
IMPLEMENT_SCRIPT_CLASS_END()

REGISTER_METHOD_P3(CVector3f, Set, Set, void, float32, float32, float32);
REGISTER_METHOD_P0(CVector3f, Length, Length, float32);
REGISTER_METHOD_P0(CVector3f, Normalize, Normalize, float32);

// --------------------------------------------------------------------------------------------------------------------
// -- registered functions
bool8 TS_Cross(TinScript::CScriptContext* script_context, uint32 obj_result, uint32 obj_v0, uint32 obj_v1)
{
    CVector3f* result = static_cast<CVector3f*>(script_context->FindObject(obj_result));
    CVector3f* v0 = static_cast<CVector3f*>(script_context->FindObject(obj_v0));
    CVector3f* v1 = static_cast<CVector3f*>(script_context->FindObject(obj_v1));
    if (!result || !v0 || !v1)
    {
        ScriptAssert_(script_context, result && v0 && v1, "<internal>", -1,
                      "Error - Cross():  Unable to find the result/v0/v1 objects");
        return (false);
    }

    // -- cross the vectors, assign the result
    *result = CVector3f::Cross(*v0, *v1);

    // -- success
    return (true);
}

// --------------------------------------------------------------------------------------------------------------------
// -- registered functions
float32 TS_Dot(TinScript::CScriptContext* script_context, uint32 obj_v0, uint32 obj_v1)
{
    CVector3f* v0 = static_cast<CVector3f*>(script_context->FindObject(obj_v0));
    CVector3f* v1 = static_cast<CVector3f*>(script_context->FindObject(obj_v1));
    if (!v0 || !v1)
    {
        ScriptAssert_(script_context, v0 && v1, "<internal>", -1,
                      "Error - Dot():  Unable to find the v0/v1 objects");
        return (false);
    }

    // -- return the dot product
    float dot = CVector3f::Dot(*v0, *v1);
    return (dot);
}

// --------------------------------------------------------------------------------------------------------------------
// -- registered functions
float32 TS_Normalized(TinScript::CScriptContext* script_context, uint32 obj_result, uint32 obj_v0)
{
    CVector3f* result = static_cast<CVector3f*>(script_context->FindObject(obj_result));
    CVector3f* v0 = static_cast<CVector3f*>(script_context->FindObject(obj_v0));
    if (!result || !v0)
    {
        ScriptAssert_(script_context, result && v0, "<internal>", -1,
                      "Error - Normalized():  Unable to find the result/v0 objects");
        return (false);
    }

    // -- return the dot product
    *result = *v0;
    float32 length = result->Normalize();
    return (length > 0.0f);
}

CONTEXT_FUNCTION_P3(ObjCross, TS_Cross, bool8, uint32, uint32, uint32);
CONTEXT_FUNCTION_P2(ObjDot, TS_Dot, float32, uint32, uint32);
CONTEXT_FUNCTION_P2(ObjNormalized, TS_Normalized, float32, uint32, uint32);

// --------------------------------------------------------------------------------------------------------------------
// -- Re-registered using the registered type, no longer requiring a script context
// -- the function signature must pass by 
REGISTER_FUNCTION_P1(V3fLength, CVector3f::Length, float32, CVector3f);
REGISTER_FUNCTION_P2(V3fCross, CVector3f::Cross, CVector3f, CVector3f, CVector3f);
REGISTER_FUNCTION_P2(V3fDot, CVector3f::Dot, float32, CVector3f, CVector3f);
REGISTER_FUNCTION_P1(V3fNormalized, CVector3f::Normalized, CVector3f, CVector3f);

// ====================================================================================================================
// EOF
// ====================================================================================================================
