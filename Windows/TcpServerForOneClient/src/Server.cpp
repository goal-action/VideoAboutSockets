#include <iostream>
#include <string>
#include <cstring> //for memset

#include <WinSock2.h>
#include <WS2tcpip.h>


class TcpServer
{
private:
    WSADATA m_wData;

    addrinfo* m_pAddr;
    std::string m_sIp;
    uint16_t m_iPort;
    SOCKET m_iSocket;

private:
    void Init();
    void HandleClients();

public:
    void Start();

public:
    TcpServer(const std::string csIp, const uint16_t ciPort);
    ~TcpServer();
};

TcpServer::TcpServer(const std::string csIp, const uint16_t ciPort)
    :
    m_pAddr{ nullptr },
    m_sIp{ csIp },
    m_iPort{ ciPort },
    m_iSocket{ INVALID_SOCKET }
{
}

TcpServer::~TcpServer()
{
    if (m_pAddr)
    {
        freeaddrinfo(m_pAddr);
    }
    if (m_iSocket != -1)
    {
        closesocket(m_iSocket);
    }
    if (WSACleanup() == SOCKET_ERROR)
    {
        std::cout << "WSACleanup error: " << WSAGetLastError() << std::endl;
    }
}

void TcpServer::Init()
{
    if (WSAStartup(MAKEWORD(2, 2), &m_wData) != 0)
    {
        std::cout << "WSAStartup error: " << WSAGetLastError() << std::endl;
        exit(-1);
    }
    std::cout << "WSAStartup success!" << std::endl;

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int iRes = -1;
    if ((iRes = getaddrinfo(m_sIp.c_str(), std::to_string(m_iPort).c_str(), &hints, &m_pAddr)) != 0)
    {
        std::cout << "getaddrinfo error: " << gai_strerror(iRes) << std::endl;
        exit(-1);
    }
    std::cout << "getaddrinfo success!\n";

    m_iSocket = socket(m_pAddr->ai_family, m_pAddr->ai_socktype, m_pAddr->ai_protocol);
    if (m_iSocket == INVALID_SOCKET)
    {
        std::cout << "socket error: " << WSAGetLastError() << std::endl;
        exit(-1);
    }
    std::cout << "socket success!\n";

    if (bind(m_iSocket, m_pAddr->ai_addr, m_pAddr->ai_addrlen) == SOCKET_ERROR)
    {
        std::cout << "bind error: " << WSAGetLastError() << std::endl;
        exit(-1);
    }
    std::cout << "bind success!\n";

    if (listen(m_iSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cout << "listen error: " << WSAGetLastError() << std::endl;
        exit(-1);
    }
    std::cout << "listen success!\n";

}

void TcpServer::HandleClients()
{
    sockaddr_in clientAddr = { 0 };
    socklen_t iClientAddrSize = sizeof(clientAddr);
    int iClientSocket = accept(m_iSocket, reinterpret_cast<sockaddr*>(&clientAddr), &iClientAddrSize);

    if (iClientSocket == INVALID_SOCKET)
    {
        std::cout << "accept error: " << WSAGetLastError() << std::endl;
        exit(-1);
    }

    char clientIp[16];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIp, 16);

    std::cout << "NEW CONNECTION: ";
    for (int i = 0; i < 15; i++)
    {
        std::cout << clientIp[i];
    }
    std::cout << std::endl;

    std::string sHello = "Hello from server!";
    send(iClientSocket, sHello.c_str(), sHello.size(), 0);

    int iRes = -1;
    while (true)
    {
        std::string sClientMsg;
        sClientMsg.resize(1024);

        iRes = recv(iClientSocket, const_cast<char*>(sClientMsg.c_str()), sClientMsg.size(), 0);
        if (iRes <= 0)
        {
            std::cout << "client [" << iClientSocket << "] closed connection or error occured. Code: " << iRes << std::endl;

            closesocket(iClientSocket);
            break;
        }

        std::cout << "[From client] " << sClientMsg.substr(0, iRes) << std::endl;
    }

}


void TcpServer::Start()
{
    Init();
    HandleClients();
}


int main(int argc, char** argv)
{
    if (argc <= 2)
    {
        std::cout << "usage: ./prog xxx.xxx.xxx.xxx port" << std::endl;
        exit(-1);
    }

    const std::string csIp(argv[1]);
    const uint16_t iPort = std::atoi(argv[2]);

    TcpServer tcpServer(csIp, iPort);
    tcpServer.Start();

    return 0;
}