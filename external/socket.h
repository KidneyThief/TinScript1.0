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

// -- sockets are only implemented in win32
#ifdef WIN32
    #include <Windows.h>
    #include <winsock.h>
#else
    #define SOCKET uint32*
#endif // WIN32

#include <vector>

// -- includes required by any system wanting access to TinScript
#include "../source/TinScript.h"
#include "../source/TinRegistration.h"

// ====================================================================================================================
// -- constants
// some random int32, just to ensure socket clients are compatible
const int32 k_PacketVersion = 0xdeadbeef;  
const int32 k_MaxPacketSize = 1024;

const int32 k_DefaultPort = 27069;
const int32 k_MaxBufferSize = 8 * 1024;

const int32 k_ThreadUpdateTimeMS = 5;
const int32 k_HeartbeatTimeMS = 10000;
const int32 k_HeartbeatTimeoutMS = 300000;

// ====================================================================================================================
// namespace SocketManager: managing remote connections
// ====================================================================================================================
namespace SocketManager
{

// ====================================================================================================================
// -- forward declarations
struct tPacketHeader;
struct tDataPacket;

// -- initialize the SocketManager
void Initialize();

// -- update loop for the SocketManager, run inside the thread
#ifdef WIN32
    DWORD WINAPI ThreadUpdate(LPVOID lpParam);
#endif

// -- perform all shutdown and cleanup of the SocketManager
void Terminate();

// -- request a connection
bool Listen();
bool Connect(const char* ipAddress);
void Disconnect();
bool IsConnected();
bool SendCommand(const char* command);
bool SendCommandf(const char* fmt, ...);

// -- This is not technically thread safe, but the chances of a collision are remote,
// -- and if you're using this, you're probably stuck in an infinte loop anyways
void SendDebuggerBreak();

// ====================================================================================================================
// -- only use this interface if you're sure you know what you're doing - e.g. the TinScript debugger methods
// -- send too much data for everything to use registered script functions
// -- first, declare a packet header on the stack, and fill it in (you must know the size already)
// -- second, create the data packet - the mData will already be allocated
// -- third, write to the mData buffer the exact size of data as specified in the header mSize
// -- finally, send the data packet.
// -- note:  if sending the data packet fails, the client is required to resend or deallocate the memory

typedef void (*ProcessRecvDataCallback)(tDataPacket* packet); 
void RegisterProcessRecvDataCallback(ProcessRecvDataCallback recvCallback);
tDataPacket* CreateDataPacket(tPacketHeader* header, void* data); 
bool SendDataPacket(tDataPacket* dataPacket);

// ====================================================================================================================
// struct tPacketHeader:  struct to organize the data being queued for send/recv
// ====================================================================================================================
struct tPacketHeader
{
    enum eType : int32
    {
        NONE = 0,

        // -- socket system packet types
        HEARTBEAT,

        // -- client data types
        SCRIPT,
        DATA,
        DEBUGGER_BREAK,

        DISCONNECT,
        COUNT,
    };

    tPacketHeader(int32 version = 0, int32 type = 0, int32 size = 0)
    {
        mVersion = version;
        mType = type;
        mSize = size;

        // -- initialize the send ptr to the start of the header
        mHeaderSent = false;
        mSendPtr = NULL;
    }

    int32 mVersion;
    int32 mType;
    int32 mSize;

    // -- send() can still send a partial byte stream - need to track how much still needs to be sent
    bool mHeaderSent;
    const char* mSendPtr;
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
        bool Enqueue(tDataPacket* packet, bool at_front = false);
        bool Dequeue(tDataPacket*& packet, bool peekOnly = false);

        // -- clear the entire queue
        void Clear();

    protected:
        std::vector<tDataPacket*> mQueue;
};

// ====================================================================================================================
// class CSocket:  an instance of a winsock connection
// ====================================================================================================================
#ifdef WIN32
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

        bool IsConnected() const
        {
            return (mConnected);
        }

        // -- see if we have anyone requesting a connection
        bool Listen();

        // -- request a connection
        bool Connect(const char* ipAddress);

        // -- disconnect methods
        void RequestDisconnect();
        void Disconnect();

        // -- handle the socket traffic
        bool ProcessSendPackets();
        bool ReceivePackets();
        bool ProcessRecvPackets();

        // -- send a script command
        bool SendScriptCommand(const char* command);

        // -- send raw data
        bool SendData(void* data, int dataSize);

        // -- send a pre-constructed data packet
        bool SendDataPacket(tDataPacket* packet);

        // -- update, to send/recv data
        bool Update();

        // -- Because sockets run in their own thread, they have to enqueue commands and statements through a mutex
        bool ScriptCommand(const char* fmt, ...);

        // -- sets the "force break" bool in the script context
        void DebuggerBreak();

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

        // -- timers to track last packet sent/received
        int32 mSendHeartbeatTimer;
        int32 mRecvHeartbeatTimer;

        // -- we need to ensure the queues are thread safe
        TinScript::CThreadMutex mThreadLock;
};
#endif // WIN32

} // SocketManager

#endif // __SOCKET_H

// ====================================================================================================================
// EOF
// ====================================================================================================================
