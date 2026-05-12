#include "include/threadpool.h"
#include <chrono>
#include "include/mutex.h"
#include <iostream>

flz::Mutex m_mutex;

int main(){
	
	
	
	flz::ThreadPool threadpool(2);
	for(int i = 0;i<5;i++){
		threadpool.enqueue([i]{
			auto thread = flz::Thread::GetThis();
			flz::Mutex::Lock lock(m_mutex);
			std::cout<<thread->getName()<<" is running"<<std::endl;
			lock.unlock();
			std::this_thread::sleep_for(std::chrono::seconds(1));
			lock.lock();
			std::cout<<thread->getName()<<" is done"<<std::endl;
			lock.unlock();
			});
	}



	return 0;
}

