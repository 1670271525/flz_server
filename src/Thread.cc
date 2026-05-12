#include "include/thread.h"
#include <unistd.h>
#include <sys/syscall.h>

namespace flz {

	static thread_local Thread* t_thread = nullptr;
	static thread_local std::string t_thread_name = "UNKNOWN";

	const std::string& Thread::GetName(){
		return t_thread_name;
	}

	Thread* Thread::GetThis(){
		return t_thread;
	}

	void Thread::SetName(const std::string& name){
		if(t_thread){
			t_thread->m_name = name;
		}
		t_thread_name = name;
	}

	Thread::Thread(std::function<void()> cb,const std::string& name):m_name(name),m_cb(cb){
		if(name.empty()){
			m_name = "UNKNOWN";
		}
		int rt = pthread_create(&m_thread,nullptr,&Thread::run,this);
		if(rt){
			throw std::logic_error("pthread_create fail!"); 
		}
	}
	
	Thread::~Thread(){
		if(m_thread){
			pthread_detach(m_thread);
		}
	}

	void* Thread::run(void* args){
		Thread* thread = (Thread*)args;
		t_thread = thread;
		t_thread_name = thread->m_name;
		thread->m_id = syscall(SYS_gettid);
		pthread_setname_np(pthread_self(),thread->m_name.substr(0,15).c_str());
		std::function<void()> cb;
		cb.swap(thread->m_cb);
		thread->m_semaphore.notify();
		cb();
		return 0;
	}
	
	void Thread::join(){
		if(m_thread){
			int rt = pthread_join(m_thread,nullptr);
			if(rt){
				throw std::logic_error("pthread_join fail!");
			}
		}

	}



}
