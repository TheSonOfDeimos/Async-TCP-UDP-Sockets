#ifndef ACCEPTOR
#define ACCEPTOR

#include <utility>
#include "baseSocket.hpp"
#include "../threadService/taskPool.hpp"
#include "tcpSocket.hpp"
#include "udpSocket.hpp"

class TCPAcceptor : public BaseSocket
{
public:
    using BaseSocket::connect_function_handler;

    TCPAcceptor(TaskPool &t_task_pool, const EndPoint &t_end_point);

    int accept(TCPSocket *t_socket);

    template <class... Args>
    void acceptAsync(TCPSocket *t_socket, connect_function_handler<Args &&...> &&t_handler);

private:
    TaskPool &m_task_pool;
};

class UDPAcceptor : public UDPSocket
{
public:
    using BaseSocket::connect_function_handler;

    UDPAcceptor(TaskPool &t_task_pool, const EndPoint &t_end_point);
};

// ======================================= Implementation ==================================================

TCPAcceptor::TCPAcceptor(TaskPool &t_task_pool, const EndPoint &t_end_point)
    : BaseSocket(BaseSocket::TCP),
      m_task_pool(t_task_pool)
{
    if (inet_pton(AF_INET, t_end_point.m_host.c_str(), &this->BaseSocket::m_address.sin_addr) <= 0)
    {
        throw std::logic_error("Invalid address. Address type not supported.");
    }

    this->BaseSocket::m_address.sin_family = AF_INET;
    this->BaseSocket::m_address.sin_port = htons(t_end_point.m_port);

    if (bind(this->BaseSocket::m_raw_sock, (const sockaddr *)&this->BaseSocket::m_address, sizeof(this->BaseSocket::m_address)) < 0)
    {
        throw std::logic_error("Cant bind socket!");
    }
}

int TCPAcceptor::accept(TCPSocket *t_socket)
{
    if (listen(this->BaseSocket::m_raw_sock, 10) < 0)
    {
        //onError(errno, "Error: Server can't listen the socket.");
        return -1;
    }

    sockaddr_in newSocketInfo;
    socklen_t newSocketInfoLength = sizeof(newSocketInfo);

    int newSock;

    while ((newSock = ::accept(this->BaseSocket::m_raw_sock, (sockaddr *)&newSocketInfo, &newSocketInfoLength)) < 0)
    {
        if (errno == EBADF || errno == EINVAL)
            return -1;

        //onError(errno, "Error while accepting a new connection.");
        return -1;
    }

    if (!this->BaseSocket::isClosed() && newSock >= 0)
    {
        t_socket->BaseSocket::m_raw_sock = newSock;
        t_socket->BaseSocket::m_address = newSocketInfo;
    }

    return 0;
}

template <class... Args>
void TCPAcceptor::acceptAsync(TCPSocket *t_socket, connect_function_handler<Args &&...> &&t_handler)
{
    std::function<void()> accept_lambda = [=]() {
        int res = -1;
        while (res < 0)
        {
            res = this->accept(t_socket);
        }

        t_handler(res);
    };

    m_task_pool.push(accept_lambda);
    return;
}

// ==================================================================

UDPAcceptor::UDPAcceptor(TaskPool &t_task_pool, const EndPoint &t_end_point)
    : UDPSocket(t_task_pool)
{
    if (inet_pton(AF_INET, t_end_point.m_host.c_str(), &this->BaseSocket::m_address.sin_addr) <= 0)
    {
        throw std::logic_error("Invalid address. Address type not supported.");
    }

    this->BaseSocket::m_address.sin_family = AF_INET;
    this->BaseSocket::m_address.sin_port = htons(t_end_point.m_port);

    if (bind(this->BaseSocket::m_raw_sock, (const sockaddr *)&this->BaseSocket::m_address, sizeof(this->BaseSocket::m_address)) < 0)
    {
        throw std::logic_error("Cannot bind the socket.");
    }
    
}

#endif