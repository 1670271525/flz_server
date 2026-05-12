#pragma once
#ifndef __FLZ_IOMANAGER_H__
#define __FLZ_IOMANAGER_H__

#include "include/scheduler.h"
#include "include/timer.h"


namespace flz{

	class IOManager:public Scheduler,public TimerManager{
	public:
		using ptr = std::shared_ptr<IOManager>;
		using RWMutexType = RWMutex;
		enum Event {
			NONE = 0x0,
			READ = 0x1,
			WRITE = 0x4,
			
		};

	private:
		struct FdContext{
			using MutexType = Mutex;
			struct EventContext{
				Scheduler* scheduler = nullptr;
				Fiber::ptr fiber;
				func_t cb;
			};
			EventContext& getContext(Event event);
			void resetContext(EventContext& ctx);
			void triggerEvent(Event event);
			EventContext read;
			EventContext write;
			int fd = 0;
			Event events = NONE;
			MutexType mutex;
		};
	public:
		IOManager(int threads = 1,bool use_caller = true,const std::string& name = "");
		~IOManager();
		int addEvent(int fd,Event event,func_t cb = nullptr);
		bool delEvent(int fd,Event event);
		bool cancelEvent(int fd,Event event);
		bool cancelAll(int fd);
		static IOManager* GetThis();
	protected:
		void tickle() override;
		bool stopping() override;
		void idle() override;
		void onTimerInsertedAtFront() override;
		void contextResize(size_t size);
		bool stopping(uint64_t& timeout);
	private:
		int m_epfd = 0;
		int m_tickleFds[2];
		std::atomic<size_t> m_pendingEventCount = {0};
		RWMutexType m_mutex;
		std::vector<FdContext*> m_fdContexts;

	};

}

#endif
