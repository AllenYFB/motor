#include "common_inc.h"

namespace
{
constexpr uint16_t kTcpServerPort = 5000;
constexpr uint16_t kTcpClientPort = 6000;
constexpr const char *kTcpClientRemoteIp = "192.168.115.100";
constexpr uint32_t kTcpTaskStackSize = 1024;

class SocketHandle
{
public:
    SocketHandle() = default;

    explicit SocketHandle(int fd)
        : fd_(fd)
    {
    }

    ~SocketHandle()
    {
        Close();
    }

    SocketHandle(const SocketHandle &) = delete;
    SocketHandle &operator=(const SocketHandle &) = delete;

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
    int fd_ = -1;
};

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
        listenSocket.Reset(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        if (!listenSocket.IsValid())
        {
            return false;
        }

        int reuse = 1;
        setsockopt(listenSocket.Get(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        sockaddr_in localAddr;
        std::memset(&localAddr, 0, sizeof(localAddr));
        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);
        localAddr.sin_port = htons(kTcpServerPort);

        if (bind(listenSocket.Get(), reinterpret_cast<sockaddr *>(&localAddr), sizeof(localAddr)) < 0)
        {
            return false;
        }

        return listen(listenSocket.Get(), 1) == 0;
    }

    void AcceptLoop(SocketHandle &listenSocket)
    {
        for (;;)
        {
            SocketHandle clientSocket(accept(listenSocket.Get(), nullptr, nullptr));
            if (!clientSocket.IsValid())
            {
                break;
            }

            EchoLoop(clientSocket);
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
        std::memset(&remoteAddr, 0, sizeof(remoteAddr));
        remoteAddr.sin_family = AF_INET;
        remoteAddr.sin_port = htons(kTcpClientPort);
        remoteAddr.sin_addr.s_addr = inet_addr(kTcpClientRemoteIp);

        return connect(socketHandle.Get(), reinterpret_cast<sockaddr *>(&remoteAddr), sizeof(remoteAddr)) == 0;
    }

    void SendLoop(SocketHandle &socketHandle)
    {
        constexpr char message[] = "hello from stm32 tcp client\r\n";

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
    static_cast<void>(argument);
    TcpServer server;
    server.Run();
}

void ThreadTcpClient(void *argument)
{
    static_cast<void>(argument);
    TcpClient client;
    client.Run();
}
}

osThreadId_t tcpServerTaskHandle;
osThreadId_t tcpClientTaskHandle;

extern "C" void Main(void)
{
    osThreadAttr_t tcpServerTaskAttr;
    std::memset(&tcpServerTaskAttr, 0, sizeof(tcpServerTaskAttr));
    tcpServerTaskAttr.name = "TcpServer";
    tcpServerTaskAttr.stack_size = kTcpTaskStackSize;
    tcpServerTaskAttr.priority = osPriorityNormal;
    tcpServerTaskHandle = osThreadNew(ThreadTcpServer, nullptr, &tcpServerTaskAttr);

    osThreadAttr_t tcpClientTaskAttr;
    std::memset(&tcpClientTaskAttr, 0, sizeof(tcpClientTaskAttr));
    tcpClientTaskAttr.name = "TcpClient";
    tcpClientTaskAttr.stack_size = kTcpTaskStackSize;
    tcpClientTaskAttr.priority = osPriorityBelowNormal;
    tcpClientTaskHandle = osThreadNew(ThreadTcpClient, nullptr, &tcpClientTaskAttr);
}
