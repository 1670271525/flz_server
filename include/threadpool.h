#ifndef __FLZ_THREADPOOL_H__
#define __FLZ_THREADPOOL_H__

#include <stdint.h>
#include "thread.h"
#include "mutex.h"
#include <queue>
#include <vector>
#include <memory.h>
#include "fiber.h"


namespace flz {

	class ThreadPool{
	public:
		typedef std::shared_ptr<ThreadPool> ptr;
		ThreadPool(uint16_t threadNums = 0);
		~ThreadPool();
		bool init();
		bool shutdown();
		template<class F,class... Args>
		void enqueue(F&& f,Args&&... args);
		
	private:
		uint16_t m_thread_nums;
		enum State{STOP,RUNNING,STOPPING};
		State m_state = STOP;
		std::vector<std::shared_ptr<flz::Thread>> m_threads;
		std::queue<std::function<void()>> m_tasks;
		void m_loop();
		flz::Mutex m_mutex;
		flz::Semaphore m_sem;
	};
	

	template <class F,class... Args>
	void ThreadPool::enqueue(F&& f,Args&&... args){
		if(m_state != RUNNING)return;
		std::function<void()> task;
		task = std::bind(std::forward<F>(f),std::forward<Args>(args)...);
		Mutex::Lock lock(m_mutex);
		m_tasks.push(task);
		lock.unlock();
		m_sem.notify();
	}

}

#endif
