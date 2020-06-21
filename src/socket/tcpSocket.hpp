#ifndef TCP_SOCKET
#define TCP_SOCKET

#include "baseSocket.hpp"
#include "../threadService/taskPool.hpp"

class TCPSocket : private BaseSocket
{
public:
    using BaseSocket::connect_function_handler;
    using BaseSocket::io_function_handler;

    TCPSocket(TaskPool &t_task_pool);

    // Synchonous operations
    int connect(const EndPoint &t_point_to_connect);
    int write(const std::string t_message);
    int write(const char *t_bytes, const size_t t_bytes_length);
    int read(char *t_bytes, const size_t t_bytes_length);

    // Asynchronous operations
    template <class... Args>
    void connectAsync(const EndPoint &t_point_to_connect, connect_function_handler<Args &&...>&& t_handler);

    template <class... Args>
    void readAsync(char *t_bytes, const size_t t_bytes_length, io_function_handler<Args &&...> t_handler);

    template <class... Args>
    void writeAsync(const std::string &t_buffer, io_function_handler<Args &&...> t_handler);

private:
    friend class TCPAcceptor;

    TaskPool &m_task_pool;
    std::mutex m_mtx;
};

// ======================================= Implementation ==================================================
TCPSocket::TCPSocket(TaskPool &t_task_pool)
    : BaseSocket(BaseSocket::TCP),
      m_task_pool(t_task_pool)
{
}

int TCPSocket::connect(const EndPoint &t_point_to_connect)
{
    struct addrinfo hints, *res, *it;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status;
    if ((status = getaddrinfo(t_point_to_connect.m_host.c_str(), NULL, &hints, &res)) != 0)
    {
        return errno;
    }

    for (it = res; it != NULL; it = it->ai_next)
    {
        if (it->ai_family == AF_INET)
        { // IPv4
            memcpy((void *)(&this->m_address), (void *)it->ai_addr, sizeof(sockaddr_in));
            break;
        }
    }

    freeaddrinfo(res);

    this->m_address.sin_family = AF_INET;
    this->m_address.sin_port = htons(t_point_to_connect.m_port);
    this->m_address.sin_addr.s_addr = (uint32_t)this->m_address.sin_addr.s_addr;

    if (::connect(this->BaseSocket::m_raw_sock, (const sockaddr *)&this->BaseSocket::m_address, sizeof(sockaddr_in)) < 0)
    {
        return errno;
    }

    return 0;
}

int TCPSocket::write(const std::string t_message)
{
    return this->write(t_message.c_str(), t_message.length());
}

int TCPSocket::write(const char *t_bytes, const size_t t_bytes_length)
{
    int bytes_transferred = 0;
    if ((bytes_transferred = send(this->m_raw_sock, t_bytes, t_bytes_length, 0)) < 0)
    {
        return -1;
    }
    return bytes_transferred;
}

int TCPSocket::read(char *t_bytes, const size_t t_bytes_length)
{
    auto res = recv(this -> BaseSocket::m_raw_sock, t_bytes, t_bytes_length, 0);
    t_bytes[res] = '\0';
    return res;
}

template <class... Args>
void TCPSocket::connectAsync(const EndPoint &t_point_to_connect, connect_function_handler<Args &&...>&& t_handler)
{

    std::function<void()> connect_lambda = [=]() {
        this->m_mtx.lock();
        int res = this->connect(t_point_to_connect);
        this->m_mtx.unlock();
        t_handler(res);
    };

    this->m_task_pool.push(connect_lambda);
    return;
}

template <class... Args>
void TCPSocket::readAsync(char *t_bytes, const size_t t_bytes_length, io_function_handler<Args &&...> t_handler)
{
    std::function<void()> read_lambda = [=]() {
        int res = -1;
        while (res < 0) {
            res = this -> read(t_bytes, t_bytes_length);
        }
        t_handler(errno, res);
    };

    this->m_task_pool.push(read_lambda);
    return;
}

template <class... Args>
void TCPSocket::writeAsync(const std::string &t_buffer, io_function_handler<Args &&...> t_handler)
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
