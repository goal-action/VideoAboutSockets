#include <iostream>
#include <string>
#include <cstring> //for memset

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> //for addrinfo
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

class TcpClient
{
private:
    std::string m_sServerIp;
    uint16_t m_iServerPort;
    int m_iClientSocket;

private:
    void HandleConnection();

public:
    TcpClient();
    ~TcpClient();

    void Connect(const std::string csServerIp, const uint16_t ciServerPort);

};

TcpClient::TcpClient()
    :
    m_iServerPort{(uint16_t)-1},
    m_iClientSocket{-1}
{
}

TcpClient::~TcpClient()
{
    if(m_iClientSocket != -1)
    {
        close(m_iClientSocket);
    }
}

void TcpClient::HandleConnection()
{
    std::string sMsg(1024, '\0');
    while(true)
    {
        std::cout << "Message to the server: ";
        std::getline(std::cin, sMsg);

        send(m_iClientSocket, const_cast<char*>(sMsg.c_str()), sMsg.size(), 0);

        if(sMsg == "exit")
        {
            break;
        }
    }
}

void TcpClient::Connect(const std::string csServerIp, const uint16_t ciServerPort)
{
    m_sServerIp = csServerIp;
    m_iServerPort = ciServerPort;

    addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    const char* szPort = std::to_string(m_iServerPort).c_str();

    for(;*szPort; szPort++)
    {
        std::cout << *szPort;
    }
    std::cout << std::endl;

    int iRes = getaddrinfo(m_sServerIp.c_str(), std::to_string(m_iServerPort).c_str(), &hints, &servinfo);
    if(iRes != 0)
    {
        std::cout << "getaddrinfo error: " << gai_strerror(iRes) << std::endl;
        exit(-1);
    }

    m_iClientSocket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(m_iClientSocket == -1)
    {
        std::cout << "socket error: " << errno << std::endl;
        exit(-1);
    }

    iRes = connect(m_iClientSocket, servinfo->ai_addr, servinfo->ai_addrlen);
    if(iRes == -1)
    {
        std::cout << "connect error: " << errno << std::endl;
        exit(-1); 
    }
    std::cout << "connected to the server!!!!!!" << std::endl;

    //read hello message
    std::string sHelloMsg(1024, '\0');

    recv(m_iClientSocket, const_cast<char*>(sHelloMsg.c_str()), sHelloMsg.size(), 0);

    std::cout << "[From server] " << sHelloMsg << std::endl;

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