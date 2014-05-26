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

// -- system includes
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>

// -- includes
#include "socket.h"

// -- includes required by any system wanting access to TinScript
#include "TinScript.h"
#include "TinRegistration.h"

// -- use the DECLARE_FILE/REGISTER_FILE macros to prevent deadstripping
DECLARE_FILE(socket_cpp);

// --------------------------------------------------------------------------------------------------------------------
// -- statics
const uint32 SCK_VERSION2 = 0x0202;

// == namespace SocketManager =========================================================================================

namespace SocketManager
{

// --------------------------------------------------------------------------------------------------------------------
// -- statics
static bool mInitialized = false;
static DWORD mThreadID = 0;
static HANDLE mThreadHandle = NULL;
static CSocket* mThreadSocket = NULL;
static int mThreadSocketID = 1;

// -- WSA variables
bool mWSAInitialized = false;
static WSADATA mWSAdata;

// ====================================================================================================================
// Initialize():  Initialize the SocketManager
// ====================================================================================================================
void Initialize()
{
    // -- create the thread
    mThreadHandle = CreateThread(NULL,                      // default security attributes
                                 0,                         // use default stack size  
                                 ThreadUpdate,              // thread function name
                                 TinScript::GetContext(),   // argument to thread function 
                                 0,                         // use default creation flags 
                                 &mThreadID);               // returns the thread identifier 
}

// ====================================================================================================================
// ThreadUpdate():  Update loop for the SocketManager, run inside the thread
// ====================================================================================================================
DWORD WINAPI ThreadUpdate(void* script_context)
{
    // -- see if we need to create the socket
    if (mThreadSocket == NULL)
    {
        mThreadSocket = new CSocket((TinScript::CScriptContext*)(script_context));

        // -- initialize the WSAData
        int error = WSAStartup(SCK_VERSION2, &mWSAdata);
        if (error)
        {
            // -- fail to start Winsock
            return (false);
        }

        // -- wrong Winsock version?
        if (mWSAdata.wVersion != SCK_VERSION2)
        {
            WSACleanup();
            return (false);
        }

        // -- we were successfully able to initialize WSA
        mWSAInitialized = true;
    }

    // -- update the socket until the thread is terminated
    while (true)
    {
        // -- listen for connections (internally, will actually listen, or simply ignore)
        if (!mThreadSocket->Listen())
        {
            return (0);
        }

        // -- update the socket (send/recv)
        if (!mThreadSocket->Update())
        {
            return (0);
        }

        Sleep(5);
    }

    // -- no errors
    return (0);
}

// ====================================================================================================================
// Termintate():  Perform all shutdown and cleanup of the SocketManager
// ====================================================================================================================
void Terminate()
{
    // -- kill the thread
    TerminateThread(mThreadHandle, 0);

    // -- kill the socket
    if (mThreadSocket)
    {
        delete mThreadSocket;
        mThreadSocket = NULL;
    }

    // -- see if we need to shutdown WSA
    if (mWSAInitialized)
    {
        WSACleanup();
        mWSAInitialized = false;
    }
}

// ====================================================================================================================
// Listen(): Set the connection to listen for incoming connect requests
// ====================================================================================================================
bool Listen()
{
    // -- see if we can enable listening
    if (mThreadSocket && !mThreadSocket->isConnected())
    {
        mThreadSocket->SetListen(true);
        return (true);
    }

    // -- unable to listen for new connections
    return (false);
}

// ====================================================================================================================
// Connect():  send a winsock connection request
// ====================================================================================================================
bool Connect(const char* ipAddress)
{
    if (!mThreadSocket)
    {
        TinPrint(TinScript::GetContext(), "Error - Connect(): SocketManager has not been initialized.\n");
    }
    else if (mThreadSocket->GetListen())
    {
        TinPrint(TinScript::GetContext(), "Error - Connect(): SocketManager is set to listen.\n");
        return (false);
    }

    // -- default address is loopback
    if (!ipAddress || !ipAddress[0])
        ipAddress = "127.0.0.1";

    bool result = mThreadSocket->Connect(ipAddress);
    if (!result)
    {
        TinPrint(TinScript::GetContext(),
                 "Error - Connect(): unable to connect - execute SocketListen() on target IP.\n");
    }

    return (result);
}

// ====================================================================================================================
// SendCommand():  send a script command to a connected socket
// ====================================================================================================================
bool SendCommand(const char* command)
{
    if (!mThreadSocket)
    {
        return (false);
    }

    bool result = mThreadSocket->SendScriptCommand(command);
    return (result);
}

// == class DataQueue =================================================================================================

// ====================================================================================================================
// Constructure
// ====================================================================================================================
DataQueue::DataQueue()
{
}

// ====================================================================================================================
// Enqueue():  Add data to the queue
// ====================================================================================================================
bool DataQueue::Enqueue(tDataPacket* packet)
{
    // -- sanity check - validate the packet
    if (!packet || !packet->mData)
    {
        return (false);
    }
    if (packet->mHeader.mType <= tPacketHeader::NONE || packet->mHeader.mType >= tPacketHeader::COUNT)
    {
        return (false);
    }
    if (packet->mHeader.mVersion != k_PacketVersion)
    {
        return (false);
    }

    // -- we want to insert acknowledge and requst packet types at the front
    if (packet->mHeader.mType == tPacketHeader::REQUEST || packet->mHeader.mType == tPacketHeader::ACKNOWLEDGE)
    {
        mQueue.insert(mQueue.begin(), packet);
    }

    // -- the remaining are all "in order" requirements, and should be sorted by ID
    else
    {
        bool inserted = false;
        for (int i = 0; i < (int)mQueue.size(); ++i)
        {
            tDataPacket* temp = mQueue[i];

            // -- skip past all the request/acknowledge packet types
            if (temp->mHeader.mType == tPacketHeader::REQUEST || temp->mHeader.mType == tPacketHeader::ACKNOWLEDGE)
            {
                continue;
            }

            // -- if we find a packet that we should be inserted before...
            else if (temp->mHeader.mID > packet->mHeader.mID)
            {
                mQueue.insert(mQueue.begin() + i, packet);
                inserted = true;
                break;
            }
        }

        // -- if we didn't already insert it, add it to the end of the queue
        if (!inserted)
        {
            mQueue.push_back(packet);
        }
    }

    // -- success
    return (true);
}

// ====================================================================================================================
// Dequeue():  return the next data block and size
// ====================================================================================================================
bool DataQueue::Dequeue(tDataPacket*& packet, int32 packetID)
{
    if (mQueue.size() == 0)
    {
        return (false);
    }

    // -- if we didn't request a specific packet ID, return the first element
    if (packetID < 0)
    {
        packet = mQueue[0];
        mQueue.erase(mQueue.begin());
        return (true);
    }

    // -- see if we can find an entry for the given ID
    for (int i = 0; i < (int)mQueue.size(); ++i)
    {
        tDataPacket* peek = mQueue[i];
        if (peek->mHeader.mID == packetID)
        {
            packet = peek;
            mQueue.erase(mQueue.begin() + i);
            return (true);
        }
    }

    // -- not found
    return (false);
}

// ====================================================================================================================
// Peek():  return the packet in the queue either with the packetID, or at the front
// ====================================================================================================================
bool DataQueue::Peek(tDataPacket*& packet, int32 packetID)
{
    if (mQueue.size() == 0)
    {
        return (false);
    }

    // -- if no ID was specified, simply return the front of the queue
    if (packetID < 0)
    {
        packet = mQueue[0];
        return (true);
    }

    // -- see if we can find an entry for the given ID
    for (int i = 0; i < (int)mQueue.size(); ++i)
    {
        tDataPacket* peek = mQueue[i];
        if (peek->mHeader.mID == packetID)
        {
            packet = peek;
            return (true);
        }
    }

    // -- not found
    return (false);
}

// ====================================================================================================================
// Clear();
// ====================================================================================================================
void DataQueue::Clear()
{
    // -- delete all queued packets
    while (mQueue.size() > 0)
    {
        tDataPacket* packet = mQueue[0];
        mQueue.erase(mQueue.begin());
        delete packet;
    }
}

// == CSocket =========================================================================================================

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CSocket::CSocket(TinScript::CScriptContext* script_context)
    : mListen(false)
    , mConnected(false)
    , mListenSocket(INVALID_SOCKET)
    , mConnectSocket(INVALID_SOCKET)
    , mScriptContext(script_context)
{
    // -- initialize the incoming packet members
    mRecvHeader[0] = '\0';
    mRecvPtr = mRecvHeader;
    mRecvPacket = NULL;
}

// ====================================================================================================================
// Destructor
// ====================================================================================================================
CSocket::~CSocket()
{
    // -- clear the queues
    mSendQueue.Clear();
    mRecvQueue.Clear();
    if (mRecvPacket != NULL)
    {
        delete mRecvPacket;
    }

    // -- close the socket if it exists
    if (mListenSocket != INVALID_SOCKET)
        closesocket(mConnectSocket);

    // -- if we have an open connection socket, we also need to cleanup WSA
    if (mConnectSocket != INVALID_SOCKET)
        closesocket(mConnectSocket);
}

// ====================================================================================================================
// ListenForConnection():  See if anyone is trying to connect
// ====================================================================================================================
bool CSocket::Listen()
{
    // -- if we're already connected, we're done
    if (mConnected || !mListen)
    {
        return (true);
    }

    // -- if we don't have a listen socket, create one
    if (mListenSocket == INVALID_SOCKET)
    {
        // -- this is executed within a thread - enqueue a thread command to print the message
        ScriptCommand("Print('CSocket::Listen(): listening for connection.\n');");

        // The address structure for a TCP socket
        SOCKADDR_IN addr;

        // -- address family
        addr.sin_family = AF_INET;      
        // -- assign port to this socket
        addr.sin_port = htons(k_DefaultPort);   

        //Accept a connection from any IP using INADDR_ANY
        //You could pass inet_addr("0.0.0.0") instead to accomplish the 
        //same thing. If you want only to watch for a connection from a 
        //specific IP, specify that instead.
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        // Create socket
        mListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (mListenSocket == INVALID_SOCKET)
        {
            // -- failed to create a socket
            return (false); 
        }

        // -- bind the socket to this address
        if (bind(mListenSocket, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            // -- failed to bind
            closesocket(mListenSocket);
            mListenSocket = INVALID_SOCKET;
            return (false);
        }

        // -- only one connection - use SOMAXCONN if you want multiple
        if (listen(mListenSocket, 1) != 0)
        {
            // -- failed to listen
            ScriptCommand("Print('Error - CSocket: listen() failed with error %d\n');", WSAGetLastError());
            closesocket(mListenSocket);
            mListenSocket = INVALID_SOCKET;
            return (false);
        }

        // -- accept a socket request
        mConnectSocket = accept(mListenSocket, NULL, NULL);
        if (mConnectSocket == INVALID_SOCKET)
        {
            // -- failed to listen
            ScriptCommand("Print('Error - CSocket: accept() failed with error %d\n');", WSAGetLastError());
            closesocket(mListenSocket);
            mListenSocket = INVALID_SOCKET;
            return (false);
        }

        // -- ensure the connect socket is non-blocking
        u_long iMode=1;
        ioctlsocket(mConnectSocket, FIONBIO, &iMode);

        // -- we're connected
        ScriptCommand("Print('CSocket: Connected.');");
        closesocket(mListenSocket);
        mListenSocket = INVALID_SOCKET;
        mConnected = true;
    }

    // -- done
    return (true);
}

// ====================================================================================================================
// RequestConnection():  See if we can establish a connection
// ====================================================================================================================
bool CSocket::Connect(const char* ipAddress)
{
    // -- this only works if we're not already connected, and we have a listening socket, and a valid address
    if (mConnected || !ipAddress || !ipAddress[0])
    {
        return (false);
    }

    struct addrinfo* addressResult = NULL;
    struct addrinfo addressHints;
    struct addrinfo *addresses = NULL;

    // -- get the connection info
    char defaultPortStr[8];
    sprintf_s(defaultPortStr, "%d", k_DefaultPort);
    
    memset(&addressHints, 0, sizeof(addrinfo));
    int addrResult = getaddrinfo(ipAddress, defaultPortStr, &addressHints, &addressResult);
    if (addrResult != 0)
    {
        ScriptCommand("Print('Error - CSocket: getaddrinfo failed with error: %d\n');", addrResult);
        return (false);
    }

    // -- create the connect socket
    mConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mConnectSocket == INVALID_SOCKET)
    {
        // -- failed to create a socket
        return (false); 
    }

    // -- see if we can request a conenction
    int connectResult = connect(mConnectSocket, addressResult->ai_addr, addressResult->ai_addrlen);
    if (connectResult == SOCKET_ERROR)
    {
        ScriptCommand("Print('Error CSocket: connect() failed.\n');");
        closesocket(mConnectSocket);
        mConnectSocket = INVALID_SOCKET;
        return (false);
    }

    // -- cleanup the addr info
    freeaddrinfo(addressResult);

    // -- see if we actually created a socket
    if (mConnectSocket == INVALID_SOCKET)
    {
        ScriptCommand("Print('Error CSocket: connect() failed.\n');");
        return (false);
    }

    // -- ensure the socket is non-blocking
    u_long iMode=1;
    ioctlsocket(mConnectSocket, FIONBIO, &iMode);

    // -- success
    ScriptCommand("Print('CSocket: Connected.\n');");

    // -- connection was successful - close down the listening connection
    if (mListenSocket != INVALID_SOCKET)
    {
        closesocket(mListenSocket);
        mListenSocket = INVALID_SOCKET;
    }

    // -- set the bool, return the result
    mConnected = true;
    mListen = false;
    return (true);
}

// ====================================================================================================================
// SendScriptCommand():  Send a script command through a connected socket
// ====================================================================================================================
bool CSocket::SendScriptCommand(const char* command)
{
    // -- ensure we're connected, and have something to send
    if (!mConnected || !command || !command[0])
    {
        return (false);
    }

    // -- lock the thread before modifying the queues
    mThreadLock.Lock();

    // -- fill in the packet header
    int length = strlen(command) + 1;
    tPacketHeader header(k_PacketVersion, 0, tPacketHeader::SCRIPT, length);
    tDataPacket* newPacket = new tDataPacket(&header, (void*)command);

    // -- enqueue the packet
    bool result = mSendQueue.Enqueue(newPacket);

    // -- unlock the thread
    mThreadLock.Unlock();

    // -- return the result
    return (result);
}

// ====================================================================================================================
// Update():  Update the socket
// ====================================================================================================================
bool CSocket::Update()
{
    // -- if we're not connected, we're done
    if (!mConnected)
    {
        return (true);
    }

    // -- in an update, we want to lock any access to the send and recv queues, from outside this thread
    mThreadLock.Lock();

    // -- see if we have something to recv
    char recvbuf[k_MaxBufferSize];
    int recvbuflen = k_MaxBufferSize;

    // -- recv data from the socket
    while (true)
    {
        int bytesRecv = recv(mConnectSocket, recvbuf, recvbuflen, 0);

        // -- check for a disconnecct
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK && error != 0)
        {
            ScriptCommand("Print('CSocket: Recv error %d\n');", error);
            closesocket(mConnectSocket);
            mConnectSocket = INVALID_SOCKET;
            mConnected = false;

            // -- clear the queues
            mSendQueue.Clear();
            mRecvQueue.Clear();

            // -- this is only a disconnection - no reason to kill the thread
            mThreadLock.Unlock();
            return (true);
        }

        else if (bytesRecv > 0)
        {
            // -- if we fail to process the data, something very bad has happened
            if (!ProcessRecvData(recvbuf, bytesRecv))
            {
                ScriptCommand("Print('CSocket: Recv error %d\n');", error);
                closesocket(mConnectSocket);
                mConnectSocket = INVALID_SOCKET;
                mConnected = false;

                // -- clear the queues
                mSendQueue.Clear();
                mRecvQueue.Clear();

                // -- this is only a disconnection - no reason to kill the thread
                mThreadLock.Unlock();
                return (true);
            }
        }

        // -- else we're done receiving
        else
        {
            break;
        }
    }

    // send the messages we've queued
    tDataPacket* packetToSend = NULL;
    while (mSendQueue.Dequeue(packetToSend, -1))
    {
        // -- send the header
        bool sendError = false;
        int error = send(mConnectSocket, (const char*)&(packetToSend->mHeader), sizeof(tPacketHeader), 0);
        if (error == SOCKET_ERROR)
            sendError = true;

        // -- send the data
        error = send(mConnectSocket, (const char*)packetToSend->mData, packetToSend->mHeader.mSize, 0);
        if (error == SOCKET_ERROR)
            sendError = true;

        // -- if we encountered an error sending, disconnect
        if (sendError)
        {
            ScriptCommand("Print('Error - CSocket::Send(): failed with error: %d\n');", WSAGetLastError());
            closesocket(mConnectSocket);

            // -- clear the queues
            mSendQueue.Clear();
            mRecvQueue.Clear();

            // -- also just a disconnect - no reason to kill the thread
            mThreadLock.Unlock();
            return (true);
        }
    }

    // -- process our recv queue
    tDataPacket* recvPacket = NULL;
    while (mRecvQueue.Dequeue(recvPacket, -1))
    {
        if (recvPacket->mHeader.mType == tPacketHeader::SCRIPT)
        {
            // -- execute whatever command we received
            ScriptCommand((const char*)recvPacket->mData);
        }
    }

    // -- unlock the thread, allowing access to the queues
    mThreadLock.Unlock();

    // -- no errors
    return (true);
}

// -- Because sockets run in their own thread, they have to enqueue commands and statements through a mutex
// ====================================================================================================================
// ScriptCommand():  Create a text command, to be enqueued in the script context thread
// ====================================================================================================================
void CSocket::ScriptCommand(const char* fmt, ...)
{
    // -- sanity check
    if (!mScriptContext || !fmt || !fmt[0])
        return;

    // -- create the script command
    va_list args;
    va_start(args, fmt);
    char cmdBuf[2048];
    vsprintf_s(cmdBuf, 2048, fmt, args);
    va_end(args);

    // -- add the command to the thread buffer
    mScriptContext->AddThreadCommand(cmdBuf);
}

// ====================================================================================================================
// ProcessRecvData():  Reconstitute a data stream back into packets
// ====================================================================================================================
bool CSocket::ProcessRecvData(void* data, int dataSize)
{
    // -- keep track of the data left to process
    char* dataPtr = (char*)data;
    int bytesToProcess = dataSize;

    // -- keep processing, as long as we have data
    while (bytesToProcess)
    {
        // -- set up the pointers to fill in either the header, of the data of a packet
        char* basePtr = (mRecvPacket == NULL) ? mRecvHeader : mRecvPacket->mData;
        int bufferSize = (mRecvPacket == NULL) ? sizeof(tPacketHeader) : mRecvPacket->mHeader.mSize;
        int bytesRequired = bufferSize - ((int)mRecvPtr - (int)basePtr);

        // -- find out how many bytes we've received, and copy as many as we're able
        int bytesToCopy = (bytesToProcess >= bytesRequired) ? bytesRequired : bytesToProcess;
        if (bytesToCopy <= 0)
        {
            // -- should be impossible - something went wrong
            return (false);
        }

        // -- copy the data, and modify the counts and pointers
        memcpy(mRecvPtr, dataPtr, bytesToCopy);
        dataPtr += bytesToCopy;
        mRecvPtr += bytesToCopy;
        bytesRequired -= bytesToCopy;
        bytesToProcess -= bytesToCopy;

        // -- see if we have filled our required bytes
        if (bytesRequired == 0)
        {
            // -- if we don't have a recv packet, then we've completed a header, and can construct one
            if (mRecvPacket == NULL)
            {
                // -- verify the new packet is valid
                tPacketHeader* newPacketHeader = (tPacketHeader*)mRecvHeader;
                if (newPacketHeader->mVersion != k_PacketVersion)
                {
                    // -- invalid version
                    return (false);
                }
                if (newPacketHeader->mType <= tPacketHeader::NONE || newPacketHeader->mType >= tPacketHeader::COUNT)
                {
                    // -- invalid type
                    return (false);
                }

                if (newPacketHeader->mSize <= 0 || newPacketHeader->mSize > 1024)
                {
                    // -- invalid size
                    return (false);
                }

                // -- create the new packet
                mRecvPacket = new tDataPacket((tPacketHeader*)mRecvHeader, NULL);

                // -- reset the recv pointer to the data of the new packet
                mRecvPtr = mRecvPacket->mData;
            }

            // -- otherwise, we've got complete packet
            else
            {
                // -- enqueue, and prepare for the next
                if (!mRecvQueue.Enqueue(mRecvPacket))
                {
                    // -- unable to enqueue - no more space
                    return (false);
                }

                // -- reset the recv members
                mRecvPacket = NULL;
                mRecvPtr = mRecvHeader;
            }
        }
    }

    // -- success
    return (true);
}

} // namespace SocketManager

// ====================================================================================================================
// -- TinScript Registration
REGISTER_FUNCTION_P0(SocketListen, SocketManager::Listen, bool);
REGISTER_FUNCTION_P1(SocketConnect, SocketManager::Connect, bool, const char*);
REGISTER_FUNCTION_P1(SocketSend, SocketManager::SendCommand, bool, const char*);

// ====================================================================================================================
// EOF
// ====================================================================================================================
