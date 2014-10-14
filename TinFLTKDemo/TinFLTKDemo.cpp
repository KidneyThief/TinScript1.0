// --------------------------------------------------------------------------------------------------------------------
// TinFLTKDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// -- system includes
#include <vector>
#include <math.h>
#include <stdio.h>
#include <time.h>

// -- TinScript includes
#include "TinScript.h"
#include "TinRegistration.h"

// -- external includes
#include "cmdshell.h"
#include "mathutil.h"
#include "socket.h"

// -- FLTK inclues
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

// -- forward declarations --------------------------------------------------------------------------------------------
class Canvas;
void ScriptNotifyEvent(int32 keypress);

// --------------------------------------------------------------------------------------------------------------------
// struct Line:  simple wrapper to submit a line to FLTK
// --------------------------------------------------------------------------------------------------------------------
struct tLine
{
    tLine(int _id, const CVector3f& _start, const CVector3f& _end, int _color)
    {
        id = _id;
        start = _start;
        end = _end;
        color = _color;
        expired = false;
    }

    // -- object_id is the owner of the request (if there is one)
    int id;
    CVector3f start;
    CVector3f end;
    int color;
    bool expired;
};

// --------------------------------------------------------------------------------------------------------------------
// struct tCircle:  simple wrapper to submit a circle to FLTK
// --------------------------------------------------------------------------------------------------------------------
struct tCircle
{
    tCircle(int _id, const CVector3f& _center, float _radius, int _color)
    {
        id = _id;
        center = _center;
        radius = _radius;
        color = _color;
        expired = false;
    }

    int id;
    CVector3f center;
    float radius;
    int color;
    bool expired;
};

// --------------------------------------------------------------------------------------------------------------------
// struct tCircle:  simple wrapper to submit a circle to FLTK
// --------------------------------------------------------------------------------------------------------------------
struct tText
{
    tText(int _id, const CVector3f& _position, const char* _text, int _color)
    {
        id = _id;
        position = _position;
        TinScript::SafeStrcpy(text, _text, TinScript::kMaxNameLength);
        color = _color;
        expired = false;
    }

    int id;
    CVector3f position;
    char text[TinScript::kMaxNameLength];
    int color;
    bool expired;
};

// -- statics ---------------------------------------------------------------------------------------------------------
static CCmdShell* gCmdShell = NULL;
static Canvas* gCanvas = NULL;

// --------------------------------------------------------------------------------------------------------------------
// class Canvas: Simple canvas using an FLTK Box, receives input, and displays printf output
// --------------------------------------------------------------------------------------------------------------------
class Canvas : public Fl_Box
{
public:
    // -- constructor
    Canvas(int X = 0, int Y = 0, int W = 640, int H = 480, const char*L=0) : Fl_Box(X,Y,W,H,L)
    {
        box(FL_FLAT_BOX);
        color(255);

        // -- this starts the thread to 
        Fl::add_idle(Update_CB, (void*)this);

        // -- set the current font
        fl_font(0, 10);
    }

    // -- event handler
    int handle(int event)
    {
        switch (event)
        {
            case FL_SHORTCUT:
            {
                int event_key = Fl::event_key();
                if ((event_key >= 'a' && event_key <= 'z') ||
                    (event_key >= '0' && event_key <= '9') ||
                    (event_key == 0x20))
                {
                    ScriptNotifyEvent(event_key);
                    return (1);
                }
            }
            break;
        }

        return (Fl_Box::handle(event));
    }

    // -- draw method for the canvas
    void draw()
    {
        // TELL BASE WIDGET TO DRAW ITS BACKGROUND
        Fl_Box::draw();

        // -- draw all the submitted lines
        int line_count = mDrawLines.size();
        for (int i = 0; i < line_count; ++i)
        {
            tLine& line = mDrawLines[i];
            if (!line.expired)
            {
                fl_color(line.color);
                fl_line((int)line.start.x, (int)line.start.y, (int)line.end.x, (int)line.end.y);
            }
        }

        // -- draw all the submitted circles
        int circle_count = mDrawCircles.size();
        for (int i = 0; i < circle_count; ++i)
        {
            tCircle& circle = mDrawCircles[i];
            if (!circle.expired)
            {
                fl_color(circle.color);
                fl_circle(circle.center.x, circle.center.y, circle.radius);
            }
        }

        // -- draw all the submitted text
        int text_count = mDrawText.size();
        for (int i = 0; i < text_count; ++i)
        {
            tText& text = mDrawText[i];
            if (!text.expired)
            {
                fl_color(text.color);
                fl_draw(text.text, (int)text.position.x, (int)text.position.y);
            }
        }
    }

    // -- drawing interface
    void DrawLine(int32 id, const CVector3f& start, const CVector3f& end, int color)
    {
        // -- find an expired line, to avoid thrashing memory
        bool found = false;
        int count = mDrawLines.size();
        for (int i = 0; i < count; ++i)
        {
            tLine& item = mDrawLines[i];
            if (item.expired)
            {
                item.id = id;
                item.start = start;
                item.end = end;
                item.color = color;
                item.expired = false;

                // -- we found an expired entry
                found = true;
                break;
            }
        }

        // -- if we didn't find an expired entry, add a new one
        if (!found)
        {
            tLine line(id, start, end, color);
            mDrawLines.push_back(line);
        }
    }

    void DrawCircle(int32 id, const CVector3f& center, float radius, int color)
    {
        // -- find an expired item, to avoid thrashing memory
        bool found = false;
        int count = mDrawCircles.size();
        for (int i = 0; i < count; ++i)
        {
            tCircle& item = mDrawCircles[i];
            if (item.expired)
            {
                item.id = id;
                item.center = center;
                item.radius = radius;
                item.color = color;
                item.expired = false;

                // -- we found an expired entry
                found = true;
                break;
            }
        }

        // -- if we didn't find an expired entry, add a new one
        if (!found)
        {
            tCircle circle(id, center, radius, color);
            mDrawCircles.push_back(circle);
        }
    }

    void DrawText(int32 id, const CVector3f& position, const char* _text, int color)
    {
        // -- find an expired item, to avoid thrashing memory
        bool found = false;
        int count = mDrawText.size();
        for (int i = 0; i < count; ++i)
        {
            tText& item = mDrawText[i];
            if (item.expired)
            {
                item.id = id;
                item.position = position;
                TinScript::SafeStrcpy(item.text, _text, TinScript::kMaxNameLength);
                item.color = color;
                item.expired = false;

                // -- we found an expired entry
                found = true;
                break;
            }
        }

        // -- if we didn't find an expired entry, add a new one
        if (!found)
        {
            tText text(id, position, _text, color);
            mDrawText.push_back(text);
        }
    }

    void CancelDrawRequests(int draw_request_id)
    {
        // -- mark all associated lines as expired
        std::vector<tLine>::iterator line_it;
        for (line_it = mDrawLines.begin(); line_it != mDrawLines.end(); ++line_it)
        {
            tLine& item = *line_it;
            if (draw_request_id < 0 || item.id == draw_request_id)
            {
                item.expired = true;
            }
        }

        std::vector<tCircle>::iterator circle_it;
        for (circle_it = mDrawCircles.begin(); circle_it != mDrawCircles.end(); ++circle_it)
        {
            tCircle& item = *circle_it;
            if (draw_request_id < 0 || item.id == draw_request_id)
            {
                item.expired = true;
                break;
            }
        }

        std::vector<tText>::iterator text_it;
        for (text_it = mDrawText.begin(); text_it != mDrawText.end(); ++text_it)
        {
            tText& item = *text_it;
            if (draw_request_id < 0 || item.id == draw_request_id)
            {
                item.expired = true;
            }
        }
    }

    void MainUpdateLoop()
    {
        // -- update the command shell, execute any command statement returned
        if (gCmdShell)
        {
            const char* command = gCmdShell->Update();
            if (command)
            {
                TinScript::ExecCommand(command);

                // -- once handled, refresh the prompt
                gCmdShell->RefreshConsoleInput(true, "");
            }
        }

        // -- use the system GetTickCount to attempt some accuracy in the current time
        DWORD current_tick = GetTickCount();
        if (gSystemTickCount == 0)
        {
            gSystemTickCount = current_tick;
        }
        int delta_ms = current_tick - gSystemTickCount;

        // -- scale the elapsed time
        delta_ms = int(gTimeScale * float(delta_ms));

        // -- limit the sim update frequency
        if (delta_ms >= 3)
        {
            gSystemTickCount = current_tick;

            if (!gPaused)
            {
                gCurrentTimeMS += delta_ms;
            }

            // -- limit the updates
            TinScript::UpdateContext(gCurrentTimeMS);
        }
    }

    static void Update_CB(void *userdata)
    {
        if (gCanvas)
            gCanvas->MainUpdateLoop();

        Canvas *o = (Canvas*)userdata;
        o->redraw();
    }

    static int gCurrentTimeMS;
    static DWORD gSystemTickCount;
    static bool gPaused;
    static float gTimeScale;

private:
    // -- store a vector of lines
    std::vector<tLine> mDrawLines;
    std::vector<tCircle> mDrawCircles;
    std::vector<tText> mDrawText;
};

// -- statics ---------------------------------------------------------------------------------------------------------
int Canvas::gCurrentTimeMS = 0;
DWORD Canvas::gSystemTickCount = 0;
bool Canvas::gPaused = false;
float Canvas::gTimeScale = 1.0f;

// -- application entry point -----------------------------------------------------------------------------------------

int main() {

    // -- ensure the mathutil file isn't deadstripped
    REGISTER_FILE(unittest_cpp);
    REGISTER_FILE(mathutil_cpp);
    REGISTER_FILE(socket_cpp);

    Fl_Double_Window window(640, 480, "TinScript Demo");
    gCanvas = new Canvas(0, 0, window.w(), window.h());

    // -- Create the TinScript context, using the default printf, and no assert handler
    TinScript::CScriptContext* thread_context = TinScript::CScriptContext::Create(printf, CmdShellAssertHandler);

    // -- create a command shell
    gCmdShell = new CCmdShell();

    // -- create a socket, so we can allow a remote debugger to connect
    SocketManager::Initialize();

    // -- execute our demo script
    TinScript::ExecScript("TinScriptDemo.ts");

    window.show();
    int return_value = Fl::run();

    // -- destroy the command shell and canvas
    delete gCmdShell;
    delete gCanvas;

    // shutdown TinScript
    SocketManager::Terminate();
    TinScript::DestroyContext();
}

// --------------------------------------------------------------------------------------------------------------------
// Registered wrappers for submitting draw requests to the Canvas
void DrawLine(int32 id, CVector3f start, CVector3f end, int32 color)
{
    if (gCanvas)
    {
        gCanvas->DrawLine(id, start, end, color);
    }
}

void DrawCircle(int32 id, CVector3f center, float32 radius, int32 color)
{
    if (gCanvas)
    {
        gCanvas->DrawCircle(id, center, radius, color);
    }
}

void DrawText(int32 id, CVector3f position, const char* text, int32 color)
{
    if (gCanvas)
    {
        gCanvas->DrawText(id, position, text, color);
    }
}

void CancelDrawRequests(int32 id)
{
    if (gCanvas)
    {
        gCanvas->CancelDrawRequests(id);
    }
}

REGISTER_FUNCTION_P4(DrawLine, DrawLine, void, int32, CVector3f, CVector3f, int32);
REGISTER_FUNCTION_P4(DrawCircle, DrawCircle, void, int32, CVector3f, float32, int32);
REGISTER_FUNCTION_P4(DrawText, DrawText, void, int32, CVector3f, const char*, int32);

REGISTER_FUNCTION_P1(CancelDrawRequests, CancelDrawRequests, void, int32);

// --------------------------------------------------------------------------------------------------------------------
// Event Handlers
void ScriptNotifyEvent(int32 keypress)
{
    int32 dummy = 0;
    TinScript::ExecF(dummy, "NotifyEvent(%d);", keypress);
}

void SimPause()
{
    Canvas::gPaused = true;
}

void SimUnpause()
{
    Canvas::gPaused = false;
}

bool8 SimIsPaused()
{
    return (Canvas::gPaused);
}

int32 GetSimTime()
{
    return (Canvas::gCurrentTimeMS);
}

void SimSetTimeScale(float scale)
{
    Canvas::gTimeScale = scale;
}

REGISTER_FUNCTION_P0(SimPause, SimPause, void);
REGISTER_FUNCTION_P0(SimUnpause, SimUnpause, void);
REGISTER_FUNCTION_P0(SimIsPaused, SimIsPaused, bool8);
REGISTER_FUNCTION_P0(GetSimTime, GetSimTime, int32);
REGISTER_FUNCTION_P1(SimSetTimeScale, SimSetTimeScale, void, float);

// --------------------------------------------------------------------------------------------------------------------
// EOF
// --------------------------------------------------------------------------------------------------------------------
