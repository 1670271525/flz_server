#pragma once
#ifndef __FLZ_SCHEDULER_H__
#define __FLZ_SCHEDULER_H__

#include <iostream>
#include "thread.h"
#include "fiber.h"
#include <memory>
#include <functional>
#include <vector>
#include <queue>
#include <atomic>
#include <list>

namespace flz {

	class Scheduler{
	public:
		using ptr = std::shared_ptr<Scheduler>;
		using MuteType = flz::Mutex;
		
		Scheduler(size_t threads = 1,bool use_caller = true,const std::string& name = "");
		virtual ~Scheduler();
		const std::string& getName()const{return m_name;}
		
		static Scheduler* GetThis();
		static flz::Fiber* GetMainFiber();
		void start();
		void stop();
		
		template<class FiberOrCb>
		void schedule(FiberOrCb fc,int thread = -1){
			bool need_tickle = false;
			{
				MuteType::Lock lock(m_mutex);
				need_tickle = scheduleNoLock(fc,thread);
			}
			if(need_tickle)tickle();
		}
		template<class InputIterator>
		void schedule(InputIterator begin,InputIterator end){
			bool need_tickle = false;
			{
				MuteType::Lock lock(m_mutex);
				while(begin != end){
					need_tickle = scheduleNoLock(&*begin,-1)||need_tickle;
					begin++;
				}
			}
			if(need_tickle)tickle();
		}

		void switchTo(int thread = -1);
   		std::ostream& dump(std::ostream& os);
	protected:
		void run();
		virtual void tickle();
		virtual bool stopping();
		virtual void idle();
		void setThis();
		bool hasIdleThreads(){return m_idleThreadCount > 0;}
	private:
		template<class FiberOrCb>
			bool scheduleNoLock(FiberOrCb fc,int thread){
				bool need_tickle = m_fibers.empty();
				FiberAndThread ft(fc,thread);
				if(ft.fiber||ft.cb){
					m_fibers.push_back(ft);
				}
				return need_tickle;
			}
	private:
		struct FiberAndThread{
			flz::Fiber::ptr fiber;
			std::function<void()> cb;
			int thread;

			FiberAndThread(flz::Fiber::ptr f,int thr):fiber(f),thread(thr){}

			FiberAndThread(flz::Fiber::ptr* f,int thr):thread(thr){
				fiber.swap(*f);
			}

			FiberAndThread(std::function<void()> f,int thr):cb(f),thread(thr){}
			
			FiberAndThread(std::function<void()> *f,int thr):thread(thr){
				cb.swap(*f);
			}
			
			FiberAndThread():thread(-1){}

			void reset(){
				fiber = nullptr;
				cb = nullptr;
				thread = -1;
			}
		};
	private:
		MuteType m_mutex;
		std::vector<flz::Thread::ptr> m_threads;
		std::list<FiberAndThread> m_fibers;
		flz::Fiber::ptr m_rootFiber;
		std::string m_name;
	protected:
		std::vector<int> m_threadIds;
		size_t m_threadCount = 0;
		std::atomic<size_t> m_activeThreadCount = {0};
		std::atomic<size_t> m_idleThreadCount = {0};
		bool m_stopping = true;
		bool m_autoStop = false;
		int m_rootThread = 0;
	};


}

#endif


