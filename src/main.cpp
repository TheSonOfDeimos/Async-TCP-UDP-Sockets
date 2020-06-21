#include <thread>
#include <iostream>

#include "threadService/threadPool.hpp"
#include "threadService/taskPool.hpp"
#include "socket/tcpSocket.hpp"
#include "socket/acceptor.hpp"
#include "socket/udpSocket.hpp"


char buff[1024];

int main()
{

	TaskPool task_pool;
	ThreadPool thread_pool;
	thread_pool.newThread(&TaskPool::run, &task_pool);
	thread_pool.newThread(&TaskPool::run, &task_pool);
	thread_pool.newThread(&TaskPool::run, &task_pool);

	UDPAcceptor acceptor(task_pool, {"0.0.0.0", 8888});

	acceptor.readAsync(buff, 1024, std::bind(

		[&](char* buff, std::size_t buff_size, const std::size_t err, int bt)
		{
			std::cout << "New Message: " << buff << "\n";
		},
		buff, 1024, std::placeholders::_1, std::placeholders::_1));
	acceptor.write(buff, 1024);
	
	
	std::this_thread::sleep_for(std::chrono::seconds(3));
	task_pool.stop();
	thread_pool.joinAll();

	return 0;
}