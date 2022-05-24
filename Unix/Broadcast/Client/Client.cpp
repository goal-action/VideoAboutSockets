#include <iostream>
#include <string>
#include <cstring> //for memset
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> //for addrinfo
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

class UdpClient
{
private:
    addrinfo* m_pServInfo;
    std::string m_sServerIp;
    uint16_t m_iServerPort;
    int m_iClientSocket;

private:
    void Init();
    void SendBroadcastMessages(const int ciCount = 10, const int ciMilliseconds = 5000);

public:
    UdpClient(const std::string csServerIp, const uint16_t ciServerPort);
    ~UdpClient();

    void Start();

};

UdpClient::UdpClient(const std::string csServerIp, const uint16_t ciServerPort)
    :
    m_pServInfo{nullptr},
    m_sServerIp{csServerIp},
    m_iServerPort{ciServerPort},
    m_iClientSocket{-1}
{
}

UdpClient::~UdpClient()
{
    if(m_iClientSocket != -1)
    {
        close(m_iClientSocket);
    }
}

void UdpClient::Init()
{
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int iRes = getaddrinfo(m_sServerIp.c_str(), std::to_string(m_iServerPort).c_str(), &hints, &m_pServInfo);
    if(iRes != 0)
    {
        std::cout << "getaddrinfo error: " << gai_strerror(iRes) << std::endl;
        exit(-1);
    }

    m_iClientSocket = socket(m_pServInfo->ai_family, m_pServInfo->ai_socktype, m_pServInfo->ai_protocol);
    if(m_iClientSocket == -1)
    {
        std::cout << "socket error: " << errno << std::endl;
        exit(-1);
    }

    int broadcast = 1;
    if (setsockopt(m_iClientSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1)
    {
        std::cout << "setsockopt error: " << errno << std::endl;
        exit(1);
    }

    std::cout << "Initialized!" << std::endl;
}

void UdpClient::SendBroadcastMessages(const int ciCount, const int ciMilliseconds)
{
    std::string sMsg("Broadcast message to the server");
    int iRes = -1;

    for(int i = 0; i < ciCount; i++)
    {
        iRes = sendto(m_iClientSocket, const_cast<char*>(sMsg.c_str()), sMsg.size(), 0, m_pServInfo->ai_addr, m_pServInfo->ai_addrlen);
        if(iRes == -1)
        {
            std::cout << "sendto error: " << errno << std::endl;
            continue;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(ciMilliseconds)); //wait for ciMilliseconds milliseconds
    }
}

void UdpClient::Start()
{
    Init();
    SendBroadcastMessages();
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