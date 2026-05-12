#include "include/threadpool.h"

namespace flz {

	ThreadPool::ThreadPool(uint16_t threadNums):m_thread_nums(threadNums){}
	ThreadPool::~ThreadPool(){
		printf("ThreadPool destory\n");
		Mutex::Lock lock(m_mutex);
		if(m_state != RUNNING)return;
		m_state = STOPPING;
		lock.unlock();
		for(int i = 0;i<m_threads.size();i++)m_sem.notify();
		for(auto& t:m_threads){
			t->join();
		}
		m_state = STOP;
		printf("real destory!\n");
	}
	
	bool flz::ThreadPool::init(){
		printf("ThreadPool init\n");
		Mutex::Lock lock(m_mutex);
		if(m_state != STOP)return false;
		m_state = RUNNING;
		lock.unlock();
		for(int i = 0;i<m_thread_nums;i++){
			std::string name = "thread-"+std::to_string(i);
			m_threads.push_back(std::make_shared<flz::Thread>(std::bind(&ThreadPool::m_loop,this),name));
		}
		return true;
	}	
	void ThreadPool::m_loop(){

		Fiber::GetThis();

		while(true){
			m_sem.wait();
			std::function<void()> task;
			Mutex::Lock lock(m_mutex);
			if(m_state == STOPPING && m_tasks.empty()){
				lock.unlock();
				return;
			}
			if(m_state == STOP){
				lock.unlock();
				return;
			}
			task = std::move(m_tasks.front());
			m_tasks.pop();
			lock.unlock();
			task();
		}
	}

}



