#pragma once

#include <ucontext.h>
#include <memory>
#include <stdint.h>
#include <functional>
#include <iostream>

#include "../include/macro.h"



namespace flz {

	class Fiber:public std::enable_shared_from_this<Fiber>{
	public:
		typedef std::shared_ptr<Fiber> ptr;
		enum State{
			INIT,HOLD,RUNNING,END,READY,EXCEPT
		};
		Fiber(std::function<void()> cb,size_t stacksize = 0,bool use_caller = true);
		~Fiber();
		void reset(std::function<void()> cb);
		void start();
		void call();
		void back();
		void swapIn();
		void swapOut();
		State getState(){return m_state;}
		void setState(State state){m_state = state;}
	public:
		static void SetThis(Fiber* fiber);
		static Fiber::ptr GetThis();
		static void MainFunc();
		static void CallerMainFunc();
		static void YieldToReady();
		static void YieldToHold();
	private:
		Fiber();
	private:
		uint64_t m_id;
		ucontext_t m_ctx;
		State m_state = INIT;
		void* m_stack = nullptr;
		uint32_t m_stacksize;
		std::function<void()> m_cb;

	};

}
