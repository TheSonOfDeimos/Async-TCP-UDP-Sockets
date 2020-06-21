#include <thread>
#include <iostream>

#include "threadService/threadPool.hpp"
#include "threadService/taskPool.hpp"
#include "socket/tcpSocket.hpp"
#include "socket/acceptor.hpp"
#include "socket/udpSocket.hpp"


char buff[1024];
TaskPool task_pool;
TCPSocket sock(task_pool);
TCPAcceptor acceptor(task_pool, {"0.0.0.0", 8888});

void startAccept();

void acceptHandle(TCPSocket *new_session, const std::size_t error)
{
    if (!error)
    {
        std::cout << std::this_thread::get_id() << " New Connection\n";
		std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    else
    {
        delete new_session;
    }

    startAccept();
}

void startAccept()
{
    TCPSocket *new_session = new TCPSocket(task_pool);
    acceptor.acceptAsync(new_session,
                           std::bind(&acceptHandle, new_session,
                                       std::placeholders::_1));
}



int main()
{
	ThreadPool thread_pool;
	thread_pool.newThread(&TaskPool::run, &task_pool);
	thread_pool.newThread(&TaskPool::run, &task_pool);
	thread_pool.newThread(&TaskPool::run, &task_pool);

	startAccept();
	

	while (1)
	{
		/* code */
	}
	
	//std::this_thread::sleep_for(std::chrono::seconds(3));
	task_pool.stop();
	thread_pool.joinAll();

	return 0;
}