#include "baseSocket.hpp"

BaseSocket::BaseSocket(const SocketType &t_sock_type)
{
    m_raw_sock = socket(AF_INET, t_sock_type, 0);
    if (m_raw_sock < 0)
    {
        throw std::runtime_error("Socket creating error.");
    }
}

int BaseSocket::rawSocket()
{
    return m_raw_sock;
}

bool BaseSocket::isClosed()
{
    return m_is_closed;
}

int BaseSocket::closeRaw()
{
    if(m_is_closed){
        return -1;
    }

    m_is_closed = true;
    return ::close(this->m_raw_sock); 


}