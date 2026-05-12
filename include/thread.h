#ifndef __FLZ_THREAD_H__
#define __FLZ_THREAD_H__


#include <iostream>
#include <pthread.h>
#include <memory>
#include <functional>

#include "include/mutex.h"
#include "include/nocopyable.h"


namespace flz {
	
	class Thread:Nocopyable{
	public:
		using ptr = std::shared_ptr<Thread>;
		Thread(std::function<void()> cb,const std::string& name);
		~Thread();
		pid_t getId()const {return m_id;}
		const std::string& getName() const{return m_name;}
		void join();
		
	public:
		static const std::string& GetName();
		static void SetName(const std::string& name);
		static Thread* GetThis();


	private:
		
		static void* run(void* args);
		
	private:
		pid_t m_id;
		pthread_t m_thread;
		std::function<void()> m_cb;
		std::string m_name;
		Semaphore m_semaphore;

	};


}

#endif 
