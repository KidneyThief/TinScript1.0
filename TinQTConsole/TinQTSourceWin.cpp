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
// TinQTConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <QListWidget>

#include "TinScript.h"
#include "TinRegistration.h"

#include "TinQTConsole.h"
#include "TinQTSourceWin.h"
#include "TinQTBreakpointsWin.h"
#include "mainwindow.h"

// ------------------------------------------------------------------------------------------------
char* ReadFileAllocBuf(const char* filename) {

	// -- open the file
	FILE* filehandle = NULL;
	if(filename) {
		 int32 result = fopen_s(&filehandle, filename, "r");
		 if (result != 0)
			 return NULL;
	}
	if(!filehandle)
		return NULL;

	// -- get the size of the file
	int32 result = fseek(filehandle, 0, SEEK_END);
	if(result != 0) {
		fclose(filehandle);
		return NULL;
	}
	int32 filesize = ftell(filehandle);
	if(filesize <= 0) {
		fclose(filehandle);
		return NULL;
	}
	fseek(filehandle, 0, SEEK_SET);

	// -- allocate a buffer and read the file into it (will null terminate)
	char* filebuf = (char*)TinAllocArray(ALLOC_FileBuf, char, filesize + 1);
	fseek(filehandle, 0, SEEK_SET);
	int32 bytesread = fread(filebuf, 1, filesize, filehandle);

    // $$$TZA for some reason, my text file is taking more space on disk than what is actually read...
	//if (bytesread != filesize) {
	if (bytesread <= 0) {
		delete[] filebuf;
		fclose(filehandle);
		return NULL;
	}
	filebuf[bytesread] = '\0';

	// -- close the file before we leave
	fclose(filehandle);

	// -- success
	return filebuf;
}

// ------------------------------------------------------------------------------------------------
CSourceLine::CSourceLine(QByteArray& text, int line_number) : QListWidgetItem()
{
    setText(text);
    mLineNumber = line_number;
    mBreakpointSet = false;
}

// ------------------------------------------------------------------------------------------------
CDebugSourceWin::CDebugSourceWin(QWidget* parent) : QListWidget(parent)
{
    mCurrentCodeblockHash = 0;
    mCurrentLineNumber = -1;

    // -- initalize the debugger directory
    mDebuggerDir[0] = '\0';
}

CDebugSourceWin::~CDebugSourceWin() {
    // -- clear any old text
    clear();
    while(mSourceText.size() > 0) {
        CSourceLine* line_item = mSourceText.at(0);
        mSourceText.removeAt(0);
        delete line_item;
    }
}

void CDebugSourceWin::NotifyCurrentDir(const char* cwd)
{
    if (!cwd)
        cwd = "./";

    // -- get the length - ensure we don't copy some randomly long directory
    int length = strlen(cwd);
    if (length >= kMaxArgLength - 2)
    {
        mDebuggerDir[0] = '\0';
        return;
    }

    // -- copy the cwd
    strcpy_s(mDebuggerDir, cwd);

    // -- ensure the directory ends in a '/'
    if (mDebuggerDir[length - 1] != '/' && mDebuggerDir[length - 1] != '\\')
    {
        mDebuggerDir[length] = '/';
        mDebuggerDir[length + 1] = '\0';
    }

    // -- because communication is remote, we must be sure our string table is up to date with our target's
    char remoteStringTableName[256];
    sprintf_s(remoteStringTableName, "%s%s", mDebuggerDir, TinScript::GetStringTableName());
    TinScript::LoadStringTable(remoteStringTableName);
}

bool CDebugSourceWin::OpenSourceFile(const char* filename, bool reload)
{
    // -- sanity check
    if(!filename || !filename[0])
        return (false);

    // -- see if we actually need to reload this file
    const char* fileNamePtr = GetFileName(filename);
    uint32 filehash = TinScript::Hash(fileNamePtr);
    if (filehash == mCurrentCodeblockHash && !reload)
        return (true);

    // -- create the full path
    char fullPath[kMaxArgLength];
    int length = sprintf_s(fullPath, "%s%s", mDebuggerDir, filename);
    if (length >= kMaxArgLength)
        return (false);

    return (OpenFullPathFile(fullPath, reload));
}

bool CDebugSourceWin::OpenFullPathFile(const char* fullPath, bool reload)
{
    const char* fileNamePtr = GetFileName(fullPath);
    uint32 filehash = TinScript::Hash(fileNamePtr);
    char* filebuf = ReadFileAllocBuf(fullPath);
    if (filebuf)
    {
        // -- set the file line edit
        CConsoleWindow::GetInstance()->GetFileLineEdit()->setText(fileNamePtr);

        // -- reset the current hash
        mCurrentCodeblockHash = 0;
        mCurrentLineNumber = -1;

        // -- clear any old text
        clear();
        mSourceText.clear();

        // -- set the hash
        mCurrentCodeblockHash = filehash;

        // --read each line of the document, and add it to the 
        int line_number = 1;
        char* filebufptr = filebuf;
        char* eol = strchr(filebufptr, '\n');
        while(eol)
        {
            *eol = '\0';

            // -- Handle the breakpoint icon as "B  " (3 chars) , and the PC as a "--> " (4 chars)
            char line_buf[8];
            sprintf_s(line_buf, "%5d", line_number++);
            QString newline(line_buf);
            newline.append("       ");
            newline.append(filebufptr);

            CSourceLine* list_item = new CSourceLine(newline.toUtf8(), mSourceText.size());
            addItem(list_item);
            mSourceText.append(list_item);

            filebufptr = eol + 1;
            eol = strchr(filebufptr, '\n');
        }
        addItem(filebufptr);

        // -- delete the buffer
        delete [] filebuf;
    }

    // -- unable to open file
    else
    {
        return (false);
    }

    // -- notify the break points window, so we can transmit all breakpoints for this file
    CConsoleWindow::GetInstance()->GetDebugBreakpointsWin()->NotifySourceFile(filehash);

    // -- success
    return (true);
}

// ------------------------------------------------------------------------------------------------
bool CDebugSourceWin::SetSourceView(uint32 codeblock_hash, int32 line_number) {
    const char* filename = TinScript::UnHash(codeblock_hash);
    if(filename) {
        bool result = OpenSourceFile(filename);
        if(result) {
            // -- set the selected line
            if(line_number >= 0 && line_number < mSourceText.size()) {
                mSourceText.at(line_number)->setSelected(true);
                if(line_number < mSourceText.size() - 6)
                    scrollToItem(mSourceText.at(line_number + 5));
                else
                    scrollToItem(mSourceText.at(mSourceText.size() - 1));

                if(line_number >= 5)
                    scrollToItem(mSourceText.at(line_number - 5));
                else
                    scrollToItem(mSourceText.at(0));

                scrollToItem(mSourceText.at(line_number));
            }
        }
        return (result);
    }
    return (false);
}

// ------------------------------------------------------------------------------------------------
void CDebugSourceWin::SetCurrentPC(uint32 codeblock_hash, int32 line_number) {
    const char* filename = TinScript::UnHash(codeblock_hash);
    if(filename) {
        bool result = OpenSourceFile(filename);
        if(result) {
            // -- if we have a current line number, and it's different, we need to clear it
            if(mCurrentLineNumber >= 0 && mCurrentLineNumber != line_number) {
                // -- find the CSourceLine for the actual_line
                CSourceLine* source_line = mSourceText.at(mCurrentLineNumber);
                QString source_text = source_line->text();
                source_text[7] = ' ';
                source_text[8] = ' ';
                source_text[9] = ' ';
                source_line->setText(source_text);
            }

            // -- now set the new current line
            if(line_number >= 0 && line_number < mSourceText.size()) {
                mCurrentLineNumber = line_number;
                CSourceLine* source_line = mSourceText.at(mCurrentLineNumber);
                QString source_text = source_line->text();
                source_text[7] = '-';
                source_text[8] = '-';
                source_text[9] = '>';
                source_line->setText(source_text);

                // -- set the selected line
                SetSourceView(codeblock_hash, line_number);
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
void CDebugSourceWin::OnDoubleClicked(QListWidgetItem * item) {
    CSourceLine* source_line = static_cast<CSourceLine*>(item);
    int actual_line = -1;
    bool set_breakpoint = false;
    if(! source_line->mBreakpointSet) {
        CConsoleWindow::GetInstance()->ToggleBreakpoint(mCurrentCodeblockHash,
                                                        source_line->mLineNumber, true, true);
    }
    else {
        CConsoleWindow::GetInstance()->ToggleBreakpoint(mCurrentCodeblockHash,
                                                        source_line->mLineNumber, false, false);
    }
}

void CDebugSourceWin::ToggleBreakpoint(uint32 codeblock_hash, int32 line_number,
                                       bool add, bool enable) {
    // -- sanity check
    if(line_number < 0 || line_number >= mSourceText.size())
        return;

    // -- ignore, if the current source view is a different file
    if(codeblock_hash != mCurrentCodeblockHash)
        return;

    // -- find the CSourceLine for the actual_line
    CSourceLine* actual_source_line = mSourceText.at(line_number);
    QString source_text = actual_source_line->text();
    source_text[6] = add && enable ? 'B' : add ? 'b' : ' ';
    actual_source_line->setText(source_text);
    actual_source_line->mBreakpointSet = add;
}

void CDebugSourceWin::NotifyCodeblockLoaded(uint32 codeblock_hash)
{
    // -- get the matching filename
    const char* filename = TinScript::UnHash(codeblock_hash);

    // -- open the file in the source window, unless it's already open
    if(mCurrentCodeblockHash == 0 && mCurrentCodeblockHash != codeblock_hash)
    {
        // -- get the file name, and open the file
        OpenSourceFile(filename, true);
    }

    // -- add an action to the main menu
    char fullPath[kMaxArgLength];
    int length = sprintf_s(fullPath, "%s%s", mDebuggerDir, filename);
    if (length >= kMaxArgLength)
        return;

    // -- add an entry to the Scripts menu
    CConsoleWindow::GetInstance()->GetMainWindow()->AddScriptOpenAction(fullPath);
}

void CDebugSourceWin::NotifyCodeblockLoaded(const char* filename)
{
    // -- sanity check
    if (!filename || !filename[0])
        return;

    // -- get the codeblock_hash
    uint32 codeblock_hash = TinScript::Hash(filename);

    // -- do nothing if we've already got a file open,
    // -- unless we're reloading the current file
    if (mCurrentCodeblockHash == 0 && mCurrentCodeblockHash != codeblock_hash)
    {
        // -- open the file
        OpenSourceFile(filename, true);
    }

    // -- add an action to the main menu
    char fullPath[kMaxArgLength];
    int length = sprintf_s(fullPath, "%s%s", mDebuggerDir, filename);
    if (length >= kMaxArgLength)
        return;

    // -- add an entry to the Scripts menu
    CConsoleWindow::GetInstance()->GetMainWindow()->AddScriptOpenAction(fullPath);
}

// --------------------------------------------------------------------------------------------------------------------
// GetFileName():  Return just the file name, given a full path.
// --------------------------------------------------------------------------------------------------------------------
const char* CDebugSourceWin::GetFileName(const char* fullPath)
{
    if (!fullPath)
        return ("");

    // -- see if we actually need to reload this file
    const char* fileNamePtr = fullPath;
    const char* pathPtr0 = strrchr(fullPath, '/');
    const char* pathPtr1 = strrchr(fullPath, '\\');
    if (pathPtr0 || pathPtr1)
    {
        // -- if we actually found both path delineators in the same name (ugh)
        if (pathPtr0 && pathPtr1)
        {
            if ((uint32)pathPtr0 > (uint32)pathPtr1)
                fileNamePtr = &pathPtr0[1];
            else
                fileNamePtr = &pathPtr1[1];
        }
        else
            fileNamePtr = pathPtr0 ? &pathPtr0[1] : &pathPtr1[1];
    }

    // -- return the result
    return (fileNamePtr);
}

// ------------------------------------------------------------------------------------------------
#include "TinQTSourceWinMoc.cpp"

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
