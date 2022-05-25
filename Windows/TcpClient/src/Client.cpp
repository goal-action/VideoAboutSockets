#include <iostream>
#include <string>
#include <cstring> //for memset

#include <WinSock2.h> //for sockets
#include <WS2tcpip.h>

class TcpClient
{
private:
    WSADATA m_wData;

    std::string m_sServerIp;
    uint16_t m_iServerPort;
    SOCKET m_iClientSocket;

private:
    void HandleConnection();

public:
    TcpClient();
    ~TcpClient();

    void Connect(const std::string csServerIp, const uint16_t ciServerPort);

};

TcpClient::TcpClient()
    :
    m_wData{0},
    m_iServerPort{ (uint16_t)-1 },
    m_iClientSocket{ INVALID_SOCKET }
{
}

TcpClient::~TcpClient()
{
    if (m_iClientSocket != INVALID_SOCKET)
    {
        closesocket(m_iClientSocket);
    }
    if (WSACleanup() == SOCKET_ERROR)
    {
        std::cout << "WSACleanup error: " << WSAGetLastError() << std::endl;
    }
}

void TcpClient::HandleConnection()
{
    std::string sMsg(1024, '\0');
    while (true)
    {
        std::cout << "Message to the server: ";
        std::getline(std::cin, sMsg);

        if (sMsg == "exit")
        {
            break;
        }

        send(m_iClientSocket, const_cast<char*>(sMsg.c_str()), sMsg.size(), 0);
    }
}

void TcpClient::Connect(const std::string csServerIp, const uint16_t ciServerPort)
{
    if (WSAStartup(MAKEWORD(2, 2), &m_wData) != 0)
    {
        std::cout << "WSAStartup erron: " << WSAGetLastError() << std::endl;
        exit(-1);
    }
    std::cout << "WSAStartup success!" << std::endl;

    m_sServerIp = csServerIp;
    m_iServerPort = ciServerPort;

    addrinfo hints, * servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    const char* szPort = std::to_string(m_iServerPort).c_str();

    int iRes = getaddrinfo(m_sServerIp.c_str(), std::to_string(m_iServerPort).c_str(), &hints, &servinfo);
    if (iRes != 0)
    {
        std::cout << "getaddrinfo error: " << gai_strerror(iRes) << std::endl;
        exit(-1);
    }

    m_iClientSocket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (m_iClientSocket == INVALID_SOCKET)
    {
        std::cout << "socket error: " << WSAGetLastError() << std::endl;
        exit(-1);
    }

    iRes = connect(m_iClientSocket, servinfo->ai_addr, servinfo->ai_addrlen);
    if (iRes == SOCKET_ERROR)
    {
        std::cout << "connect error: " << WSAGetLastError() << std::endl;
        exit(-1);
    }
    std::cout << "connected to the server!!!!!!" << std::endl;

    //read hello message
    std::string sHelloMsg(1024, '\0');

    iRes = recv(m_iClientSocket, const_cast<char*>(sHelloMsg.c_str()), sHelloMsg.size(), 0);
    if (iRes <= 0)
    {
        std::cout << "server closed connection or error occured. Code: " << iRes << std::endl;
        exit(-1);
    }

    std::cout << "[From server] " << sHelloMsg.substr(0, iRes) << std::endl;

    HandleConnection();
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

    TcpClient client;
    client.Connect(csIp, iPort);

    std::cout << "End for Client..." << std::endl;

    return 0;
}