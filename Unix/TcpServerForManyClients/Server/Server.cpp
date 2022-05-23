#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <unistd.h>
#include <cstring> //for memset

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/poll.h>


class TcpServer 
{
private:
    std::vector<pollfd> m_fds; //first struct is server accept socket

    addrinfo* m_pAddr;
    std::string m_sIp;
    uint16_t m_iPort;
    int m_iSocket;

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
    m_pAddr{nullptr},
    m_sIp{csIp},
    m_iPort{ciPort},
    m_iSocket{-1}
{
}

TcpServer::~TcpServer()
{
}

void TcpServer::Init()
{
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    int iRes = -1;
    if((iRes = getaddrinfo(m_sIp.c_str(), std::to_string(m_iPort).c_str(), &hints, &m_pAddr)) != 0)
    {
        std::cout << "getaddrinfo error: " << iRes << std::endl;
        exit(-1);
    }
    std::cout << "getaddrinfo success!\n";

    m_iSocket = socket(m_pAddr->ai_family, m_pAddr->ai_socktype, m_pAddr->ai_protocol);
    if(m_iSocket == -1)
    {
        std::cout << "socket error: " << errno << std::endl;
        exit(-1);
    }
    std::cout << "socket success!\n";

    if(bind(m_iSocket, m_pAddr->ai_addr, m_pAddr->ai_addrlen) == -1)
    {
        std::cout << "bind error: " << errno << std::endl;
        exit(-1);
    }
    std::cout << "bind success!\n";

    if(listen(m_iSocket, SOMAXCONN) == -1)
    {
        std::cout << "listen error: " << errno << std::endl;
        exit(-1);
    }
    std::cout << "listen success!\n";

    pollfd serverPollFd = {0};
    serverPollFd.fd = m_iSocket;
    serverPollFd.events = POLLIN;
    m_fds.push_back(serverPollFd);

}

void TcpServer::HandleClients()
{
    sockaddr_in clientAddr = {0};
    socklen_t iClientAddrSize = sizeof(clientAddr);

    int iRes = -1;
    while(true)
    {
        iRes = poll(m_fds.data(), m_fds.size(), 200);
        
        if(iRes == 0)
        {
            //timeout, no read descriptors
            continue;
        }
        
        if(iRes == -1)
        {
            std::cout << "poll error: " << errno << std::endl;
            continue;
        }

        if(m_fds.at(0).revents & POLLIN) //if there are new client(s)
        {
            //process new connection
            pollfd newClientPollFd = {0};
            newClientPollFd.fd = accept(m_iSocket, reinterpret_cast<sockaddr*>(&clientAddr), &iClientAddrSize);
            
            if(newClientPollFd.fd == -1)
            {
                std::cout << "error accepting new client: " << errno << std::endl;
                continue;
            }

            //get new client ip address
            char clientIp[16];
            inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIp, 16);

            std::cout << "NEW CONNECTION: ";
            for(int i = 0; i < 16; i++)
            {  
                std::cout << clientIp[i];
            }
            std::cout << std::endl;

            //send hello message to the new client
            std::string sHello = "Hello from server!";
            send(newClientPollFd.fd, sHello.c_str(), sHello.size(), 0);

            //add info about client to the vector
            newClientPollFd.events = POLLIN;
            m_fds.push_back(newClientPollFd);
        }

        for(int i = 1; i < m_fds.size(); i++)
        {
            if(m_fds.at(i).revents & POLLIN)
            {
                std::string sClientMsg;
                sClientMsg.resize(1024);

                iRes = recv(m_fds.at(i).fd, const_cast<char*>(sClientMsg.c_str()), sClientMsg.size(), 0);
                if(iRes == 0)
                {
                    std::cout << "Client disconnection fd = [" << m_fds.at(i).fd << "]" << std::endl;
                    close(m_fds.at(i).fd);
                    m_fds.erase(m_fds.begin() + i);
                    
                    continue;
                }

                std::cout << "[From client] " << sClientMsg << std::endl;
            }
        }
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