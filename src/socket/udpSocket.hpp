#ifndef UDP_SOCKET
#define UDP_SOCKET

#include "baseSocket.hpp"
#include "../threadService/taskPool.hpp"

class UDPSocket : private BaseSocket
{
public:
    using BaseSocket::connect_function_handler;
    using BaseSocket::io_function_handler;

    UDPSocket(TaskPool &t_task_pool);
    virtual ~UDPSocket() = default;

    // Synchonous operations
    void setEndPoint(const EndPoint &t_end_point);

    int write(const std::string t_message);
    int write(const char *t_bytes, const size_t t_bytes_length);
    int read(char *t_bytes, const size_t t_bytes_length);

    // Asynchronous operations
    template <class... Args>
    void readAsync(char *t_bytes, const size_t t_bytes_length, io_function_handler<Args &&...> t_handler);

    template <class... Args>
    void writeAsync(const std::string &t_buffer, io_function_handler<Args &&...> t_handler);

private:
    friend class UDPAcceptor;

    TaskPool &m_task_pool;
    std::mutex m_mtx;
};

// ======================================= Implementation ==================================================

UDPSocket::UDPSocket(TaskPool &t_task_pool)
    : BaseSocket(BaseSocket::UDP),
      m_task_pool(t_task_pool)
{
}

void UDPSocket::setEndPoint(const EndPoint &t_end_point)
{
    this->BaseSocket::m_end_point = t_end_point;
    return;
}

int UDPSocket::write(const std::string t_message)
{
    return this->write(t_message.c_str(), t_message.length());
}

int UDPSocket::write(const char *t_bytes, const size_t t_bytes_length)
{
    sockaddr_in hostAddr;

    struct addrinfo hints, *res, *it;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int status;
    if ((status = getaddrinfo(this->BaseSocket::m_end_point.m_host.c_str(), NULL, &hints, &res)) != 0)
    {
        //onError(errno, "Invalid address." + std::string(gai_strerror(status)));
        return -1;
    }

    for (it = res; it != NULL; it = it->ai_next)
    {
        if (it->ai_family == AF_INET)
        { // IPv4
            memcpy((void *)(&hostAddr), (void *)it->ai_addr, sizeof(sockaddr_in));
            break; // for now, just get first ip (ipv4).
        }
    }

    freeaddrinfo(res);

    hostAddr.sin_port = htons(this->BaseSocket::m_end_point.m_port);
    hostAddr.sin_family = AF_INET;

    if (sendto(this->BaseSocket::m_raw_sock, t_bytes, t_bytes_length, 0, (sockaddr *)&hostAddr, sizeof(hostAddr)) < 0)
    {
        //onError(errno, "Cannot send message to the address.");
        return -1;
    }

    return 0;
}

int UDPSocket::read(char *t_bytes, const size_t t_bytes_length)
{
    sockaddr_in hostAddr;
    socklen_t hostAddrSize = sizeof(hostAddr);

    auto res = recvfrom(this->BaseSocket::m_raw_sock, t_bytes, t_bytes_length, 0, (sockaddr *)&hostAddr, &hostAddrSize);
    t_bytes[res] = '\0';
    return res;
}

template <class... Args>
void UDPSocket::readAsync(char *t_bytes, const size_t t_bytes_length, io_function_handler<Args &&...> t_handler)
{
    std::function<void()> read_lambda = [=]() {
        int res = -1;
        while (res < 0)
        {
            res = this->read(t_bytes, t_bytes_length);
        }
        t_handler(errno, res);
    };

    this->m_task_pool.push(read_lambda);
    return;
}

template <class... Args>
void UDPSocket::writeAsync(const std::string &t_buffer, io_function_handler<Args &&...> t_handler)
{
    std::function<void()> write_lambda = [=]() {
        this -> m_mtx.lock();
        int res = this -> write(t_buffer);
        this -> m_mtx.unlock();
        t_handler(errno, res);
    };

    m_task_pool.push(write_lambda);
    return;
}

#endif