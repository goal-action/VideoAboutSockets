#include <iostream>
#include <string>
#include <cstring> //for memset

#include <WinSock2.h>
#include <WS2tcpip.h>

class UdpClient
{
private:
    WSADATA m_wData;

    addrinfo* m_pServInfo;
    std::string m_sServerIp;
    uint16_t m_iServerPort;
    SOCKET m_iClientSocket;

private:
    void Init();
    void HandleConnection();

public:
    UdpClient(const std::string csServerIp, const uint16_t ciServerPort);
    ~UdpClient();

    void Start();

};

UdpClient::UdpClient(const std::string csServerIp, const uint16_t ciServerPort)
    :
    m_wData{0},
    m_pServInfo{ nullptr },
    m_sServerIp{ csServerIp },
    m_iServerPort{ ciServerPort },
    m_iClientSocket{ INVALID_SOCKET }
{
}

UdpClient::~UdpClient()
{
    if (m_iClientSocket != -1)
    {
        closesocket(m_iClientSocket);
    }
    if (WSACleanup() == SOCKET_ERROR)
    {
        std::cout << "WSACleanup error: " << WSAGetLastError() << std::endl;
    }
}

void UdpClient::Init()
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
    hints.ai_socktype = SOCK_DGRAM;

    int iRes = getaddrinfo(m_sServerIp.c_str(), std::to_string(m_iServerPort).c_str(), &hints, &m_pServInfo);
    if (iRes != 0)
    {
        std::cout << "getaddrinfo error: " << WSAGetLastError() << std::endl;
        exit(-1);
    }

    m_iClientSocket = socket(m_pServInfo->ai_family, m_pServInfo->ai_socktype, m_pServInfo->ai_protocol);
    if (m_iClientSocket == INVALID_SOCKET)
    {
        std::cout << "socket error: " << WSAGetLastError() << std::endl;
        exit(-1);
    }
     
    std::cout << "Initialized!" << std::endl;
}

void UdpClient::HandleConnection()
{
    std::string sMsg(1024, '\0');
    int iRes = -1;

    while (true)
    {
        std::cout << "Message to the server: ";
        std::getline(std::cin, sMsg);

        if (sMsg == "exit")
        {
            break;
        }

        iRes = sendto(m_iClientSocket, const_cast<char*>(sMsg.c_str()), sMsg.size(), 0, m_pServInfo->ai_addr, m_pServInfo->ai_addrlen);
        if (iRes == SOCKET_ERROR)
        {
            std::cout << "sendto error: " << WSAGetLastError() << std::endl;
            continue;
        }
    }
}

void UdpClient::Start()
{
    Init();
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

    UdpClient client(csIp, iPort);
    client.Start();

    std::cout << "End for Client..." << std::endl;

    return 0;
}