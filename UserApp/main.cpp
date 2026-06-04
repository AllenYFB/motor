#include "common_inc.h"

namespace
{
static const uint16_t kTcpServerPort = 5000;
static const uint16_t kTcpClientPort = 6000;
static const char *kTcpClientRemoteIp = "192.168.115.28";
static const uint32_t kTcpTaskStackSize = 2048;
static const int kSocketTimeoutMs = 2000;

void DebugPrint(const char *text)
{
    HAL_UART_Transmit(&huart1, (const uint8_t *)text, strlen(text), 100);
}

class SocketHandle
{
public:
    SocketHandle()
        : fd_(-1)
    {
    }

    explicit SocketHandle(int fd)
        : fd_(fd)
    {
    }

    ~SocketHandle()
    {
        Close();
    }

    bool IsValid() const
    {
        return fd_ >= 0;
    }

    int Get() const
    {
        return fd_;
    }

    void Reset(int fd)
    {
        Close();
        fd_ = fd;
    }

    void Close()
    {
        if (fd_ >= 0)
        {
            closesocket(fd_);
            fd_ = -1;
        }
    }

private:
    SocketHandle(const SocketHandle &);
    SocketHandle &operator=(const SocketHandle &);

    int fd_;
};

void ConfigureListenSocket(int fd)
{
    int enabled = 1;
    int timeoutMs = kSocketTimeoutMs;

    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &enabled, sizeof(enabled));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeoutMs, sizeof(timeoutMs));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeoutMs, sizeof(timeoutMs));
}

void ConfigureConnectedSocket(int fd)
{
    int enabled = 1;
    int timeoutMs = kSocketTimeoutMs;

    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &enabled, sizeof(enabled));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeoutMs, sizeof(timeoutMs));
}

bool SendAll(int fd, const char *data, int length)
{
    int sentTotal = 0;

    while (sentTotal < length)
    {
        int sent = send(fd, data + sentTotal, length - sentTotal, 0);
        if (sent <= 0)
        {
            return false;
        }

        sentTotal += sent;
    }

    return true;
}

class TcpServer
{
public:
    void Run()
    {
        for (;;)
        {
            SocketHandle listenSocket;
            if (!OpenListenSocket(listenSocket))
            {
                osDelay(1000);
                continue;
            }

            AcceptLoop(listenSocket);
            osDelay(1000);
        }
    }

private:
    bool OpenListenSocket(SocketHandle &listenSocket)
    {
        DebugPrint("[tcp server] socket\r\n");
        listenSocket.Reset(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        if (!listenSocket.IsValid())
        {
            DebugPrint("[tcp server] socket failed\r\n");
            return false;
        }

        ConfigureListenSocket(listenSocket.Get());

        int reuse = 1;
        setsockopt(listenSocket.Get(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        sockaddr_in localAddr;
        memset(&localAddr, 0, sizeof(localAddr));
        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);
        localAddr.sin_port = htons(kTcpServerPort);

        if (bind(listenSocket.Get(), reinterpret_cast<sockaddr *>(&localAddr), sizeof(localAddr)) < 0)
        {
            DebugPrint("[tcp server] bind failed\r\n");
            return false;
        }

        DebugPrint("[tcp server] bind ok\r\n");

        if (listen(listenSocket.Get(), 1) < 0)
        {
            DebugPrint("[tcp server] listen failed\r\n");
            return false;
        }

        DebugPrint("[tcp server] listen ok 0.0.0.0:5000\r\n");
        return true;
    }

    void AcceptLoop(SocketHandle &listenSocket)
    {
        for (;;)
        {
            DebugPrint("[tcp server] accept wait\r\n");
            SocketHandle clientSocket(accept(listenSocket.Get(), NULL, NULL));
            if (!clientSocket.IsValid())
            {
                DebugPrint("[tcp server] accept failed\r\n");
                continue;
            }

            ConfigureConnectedSocket(clientSocket.Get());
            DebugPrint("[tcp server] client connected\r\n");
            EchoLoop(clientSocket);
            DebugPrint("[tcp server] client closed\r\n");
        }
    }

    void EchoLoop(SocketHandle &clientSocket)
    {
        char rxBuffer[256];

        for (;;)
        {
            int received = recv(clientSocket.Get(), rxBuffer, sizeof(rxBuffer), 0);
            if (received <= 0)
            {
                break;
            }

            if (!SendAll(clientSocket.Get(), rxBuffer, received))
            {
                break;
            }
        }
    }
};

class TcpClient
{
public:
    void Run()
    {
        for (;;)
        {
            SocketHandle socketHandle(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
            if (!socketHandle.IsValid())
            {
                osDelay(1000);
                continue;
            }

            ConfigureConnectedSocket(socketHandle.Get());

            if (Connect(socketHandle))
            {
                SendLoop(socketHandle);
            }

            osDelay(1000);
        }
    }

private:
    bool Connect(SocketHandle &socketHandle)
    {
        sockaddr_in remoteAddr;
        memset(&remoteAddr, 0, sizeof(remoteAddr));
        remoteAddr.sin_family = AF_INET;
        remoteAddr.sin_port = htons(kTcpClientPort);
        remoteAddr.sin_addr.s_addr = inet_addr(kTcpClientRemoteIp);

        return connect(socketHandle.Get(), reinterpret_cast<sockaddr *>(&remoteAddr), sizeof(remoteAddr)) == 0;
    }

    void SendLoop(SocketHandle &socketHandle)
    {
        static const char message[] = "hello from stm32 tcp client\r\n";

        for (;;)
        {
            if (!SendAll(socketHandle.Get(), message, sizeof(message) - 1))
            {
                break;
            }

            osDelay(1000);
        }
    }
};

void ThreadTcpServer(void *argument)
{
    (void)argument;
    TcpServer server;
    server.Run();
}

void ThreadTcpClient(void *argument)
{
    (void)argument;
    TcpClient client;
    client.Run();
}
}

osThreadId_t tcpServerTaskHandle;
osThreadId_t tcpClientTaskHandle;

extern "C" void Main(void)
{
    osThreadAttr_t tcpServerTaskAttr;
    memset(&tcpServerTaskAttr, 0, sizeof(tcpServerTaskAttr));
    tcpServerTaskAttr.name = "TcpServer";
    tcpServerTaskAttr.stack_size = kTcpTaskStackSize;
    tcpServerTaskAttr.priority = osPriorityNormal;
    tcpServerTaskHandle = osThreadNew(ThreadTcpServer, NULL, &tcpServerTaskAttr);


    osThreadAttr_t tcpClientTaskAttr;
    memset(&tcpClientTaskAttr, 0, sizeof(tcpClientTaskAttr));
    tcpClientTaskAttr.name = "TcpClient";
    tcpClientTaskAttr.stack_size = kTcpTaskStackSize;
    tcpClientTaskAttr.priority = osPriorityBelowNormal;
    tcpClientTaskHandle = osThreadNew(ThreadTcpClient, NULL, &tcpClientTaskAttr);

}
