#pragma once
#ifndef __FLZ_TIMER_H__
#define __FLZ_TIMER_H__

#include <memory>
#include <functional>
#include "include/thread.h"
#include <vector>
#include <set>



namespace flz {
	
	using func_t = std::function<void()>;


	class TimerManager;
	class Timer:public std::enable_shared_from_this<Timer>{
	friend class TimerManager;
	public:
		using ptr = std::shared_ptr<Timer>;
		bool cancel();
		bool refresh();
		bool reset(uint64_t ms,bool from_now);
	
	private:
		Timer(uint64_t ms,func_t cb,bool recurring,TimerManager* manager);
		Timer(uint64_t next);

	private:
		bool m_recurring = false;
		uint64_t m_ms = 0;
		uint64_t m_next = 0;
		func_t m_cb;
		TimerManager* m_manager = nullptr;
	private:
		struct Comparator{
			bool operator()(const Timer::ptr& lhs,const Timer::ptr& rhs)const;
		};
	};

	class TimerManager{
	friend class Timer;
	public:
		using RWMutexType = RWMutex;
		TimerManager();
		virtual ~TimerManager();
		Timer::ptr addTimer(uint64_t ms,func_t cb,bool recurring = false);
		Timer::ptr addConditionTimer(uint64_t ms,func_t cb,std::weak_ptr<void> cond,bool recurring = false);
		uint64_t getNextTimer();
		void listExpiredCb(std::vector<func_t>& cbs);
		bool hasTimer();
	protected:
		virtual void onTimerInsertedAtFront() = 0;
		void addTimer(Timer::ptr val,RWMutexType::WriteLock& lock);
	private:
		bool detectClockRollover(uint64_t now_ms);
	private:
		RWMutexType m_mutex;
		std::set<Timer::ptr,Timer::Comparator> m_timers;
		bool m_tickled = false;
		uint64_t m_previouseTime = 0;

	};
}

#endif
