// ------------------------------------------------------------------------------------------------
//  The MIT License
//  
//  Copyright (c) 2014 Tim Andersen
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

#ifndef __SOCKET_H
#define __SOCKET_H

#include <Windows.h>
#include <vector>

// -- includes required by any system wanting access to TinScript
#include "TinScript.h"
#include "TinRegistration.h"

// ====================================================================================================================
// -- constants
const int k_PacketVersion = 1;
const int k_MaxPacketSize = 1024;

const int k_DefaultPort = 27069;
const int k_MaxBufferSize = 8 * 1024;

// ====================================================================================================================
// namespace SocketManager: managing remote connections
// ====================================================================================================================
namespace SocketManager
{

// -- initialize the SocketManager
void Initialize();

// -- update loop for the SocketManager, run inside the thread
DWORD WINAPI ThreadUpdate(LPVOID lpParam);

// -- perform all shutdown and cleanup of the SocketManager
void Terminate();

// -- request a connection
bool Listen();
bool Connect(const char* ipAddress);
bool SendCommand(const char* command);

// ====================================================================================================================
// struct tPacketHeader:  struct to organize the data being queued for send/recv
// ====================================================================================================================
struct tPacketHeader
{
    enum eType : int32
    {
        NONE = 0,

        REQUEST,
        ACKNOWLEDGE,
        SCRIPT,
        DEBUG,

        COUNT,
    };

    tPacketHeader(int32 version = 0, int32 id = 0, int32 type = 0, int32 size = 0)
    {
        mVersion = version;
        mID = id;
        mType = type;
        mSize = size;
    }

    int32 mVersion;
    int32 mID;
    int32 mType;
    int32 mSize;
};

// ====================================================================================================================
// struct tDataPacket:  struct to contain a complete packet (including a header) to send/recv
// ====================================================================================================================
struct tDataPacket
{
    // -- constructor
    explicit tDataPacket(tPacketHeader* header, void* data)
    {
        // -- copy the header
        memcpy(&mHeader, header, sizeof(tPacketHeader));

        // -- allocate the buffer
        mData = NULL;
        if (mHeader.mSize > 0)
        {
            mData = new char[mHeader.mSize];

            // -- if we already have data, fill that in as well
            if (data != NULL)
            {
                memcpy(mData, data, mHeader.mSize);
            }
        }
    }

    // -- destructor
    ~tDataPacket()
    {
        if (mData)
            delete [] mData;
    }

    // -- members, a header and the packet data
    tPacketHeader mHeader;
    char* mData;

    private:
        // -- no empty packets
        tDataPacket() { }
};

// ====================================================================================================================
// class DataQueue:  helper class to manage a circular queue to send and recv data
// ====================================================================================================================
class DataQueue
{
    public:
        DataQueue();
        bool Enqueue(tDataPacket* packet);
        bool Dequeue(tDataPacket*& packet, int32 packetID);

        // -- access to specific packets by ID
        bool Peek(tDataPacket*& packet, int32 packetID);

        // -- clear the entire queue
        void Clear();

    protected:
        std::vector<tDataPacket*> mQueue;
};

// ====================================================================================================================
// class CSocket:  an instance of a winsock connection
// ====================================================================================================================
class CSocket
{
    public:
        // -- constructor / destructor
        explicit CSocket(TinScript::CScriptContext* script_context);
        ~CSocket();

        // -- public interface to create, update, and destroy sockets by name
        static CSocket* Create();
        static void UpdateSockets();

        void SetListen(bool torf)
        {
            mListen = torf;
        }

        bool GetListen() const
        {
            return (mListen);
        }

        bool isConnected() const
        {
            return (mConnected);
        }

        // -- see if we have anyone requesting a connection
        bool Listen();

        // -- request a connection
        bool Connect(const char* ipAddress);

        // -- send a script command
        bool SendScriptCommand(const char* command);

        // -- send data
        bool Send(void* data, int dataSize);

        // -- update, to send/recv data
        bool Update();

        // -- Because sockets run in their own thread, they have to enqueue commands and statements through a mutex
        void ScriptCommand(const char* fmt, ...);

    protected:
        bool mListen;
        bool mConnected;
        SOCKET mListenSocket;
        SOCKET mConnectSocket;

        // -- we need access to the script context for which this socket was created
        TinScript::CScriptContext* mScriptContext;

        // -- we need to reconstitute packets as they arrive
        bool ProcessRecvData(void* data, int dataSize);
        char mRecvHeader[sizeof(tPacketHeader)];
        char* mRecvPtr;
        tDataPacket* mRecvPacket;

        // -- we need both send and a recv packet queues
        DataQueue mSendQueue;
        DataQueue mRecvQueue;

        // -- we need to ensure the queues are thread safe
        TinScript::CThreadMutex mThreadLock;
};

} // SocketManager

#endif // __SOCKET_H

// ====================================================================================================================
// EOF
// ====================================================================================================================
