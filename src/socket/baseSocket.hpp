#ifndef BASE_SOCKET
#define BASE_SOCKET

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string>

struct EndPoint
{
    std::string m_host;
    uint16_t m_port;
};

class BaseSocket
{
public:
    enum SocketType
    {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };

    using error_code = int;

    template <class... Args>
    using io_function_handler = std::function<void(Args&& ..., const error_code, std::size_t)>;

    template <class... Args>
    using connect_function_handler = std::function<void(Args&& ..., const error_code)>;

    BaseSocket(const SocketType& t_sock_type);
    virtual ~BaseSocket() = default;

    int& rawSocket();
    bool isClosed() const;
    int closeRaw();

    sockaddr_in& getAddress();
    EndPoint& getEndPoint();

private:
    int m_raw_sock = 0;
    sockaddr_in m_address;
    bool m_is_closed = false;
    EndPoint m_end_point;
};



#endif