#include "../include/fiber.h"
#include <atomic>
#include <ucontext.h>
#include <stdlib.h>
#include <stdexcept>


namespace flz {

	static uint32_t const STACKSIZE = 32*128;

	static std::atomic<uint64_t> s_fiber_id{0};
	static std::atomic<uint64_t> s_fiber_count{0};

	static thread_local Fiber *t_fiber = nullptr;
	static thread_local Fiber::ptr t_thread_fiber = nullptr;

	Fiber::Fiber(){
		m_state = RUNNING;
		SetThis(this);
		getcontext(&m_ctx);
		s_fiber_count++;

	}

	class MallocStackAllocator{
	public:
		static void* Alloc(size_t size){
			return malloc(size);
		}
		static void Dealloc(void* vp,size_t size){
			return free(vp);	
		}
	};


	Fiber::Fiber(std::function<void()> cb,size_t stacksize,bool use_caller):m_id(++s_fiber_id),m_cb(cb){
		m_stacksize = stacksize?stacksize:STACKSIZE;
		m_stack = MallocStackAllocator::Alloc(m_stacksize);
		++s_fiber_count;
		getcontext(&m_ctx);
		m_ctx.uc_stack.ss_sp = m_stack;
		m_ctx.uc_stack.ss_size = m_stacksize;
		m_ctx.uc_link = nullptr;
		if(!use_caller)makecontext(&m_ctx,&Fiber::MainFunc,0);
		else makecontext(&m_ctx,&Fiber::CallerMainFunc,0);

	}

	Fiber::~Fiber(){
		--s_fiber_count;
		if(m_stack){
			MallocStackAllocator::Dealloc(m_stack,m_stacksize);
		}else{
			Fiber* cur = t_fiber;
			if(cur == this)SetThis(nullptr);
		
		}
	}


	void Fiber::SetThis(Fiber* fiber){
		t_fiber = fiber;
	}

	Fiber::ptr Fiber::GetThis(){
		if(t_fiber)return t_fiber->shared_from_this();
		Fiber::ptr main_fiber(new Fiber);
		t_thread_fiber = main_fiber;
		return t_fiber->shared_from_this();
	}
	
	void Fiber::call(){
		SetThis(this);
		m_state = State::RUNNING;
		swapcontext(&t_thread_fiber->m_ctx,&m_ctx);
	}
	
	void Fiber::back(){
		SetThis(t_thread_fiber.get());
		swapcontext(&m_ctx,&t_thread_fiber->m_ctx);
	}

	void Fiber::swapIn(){
		SetThis(this);
		if(m_state == RUNNING){
			perror("fiber is running");
			return;
		}
		m_state = RUNNING;
		swapcontext(&t_thread_fiber->m_ctx,&m_ctx);
	}

	void Fiber::swapOut(){
		SetThis(t_thread_fiber.get());
		swapcontext(&m_ctx,&t_thread_fiber->m_ctx);
	}
	
	void Fiber::CallerMainFunc(){
		Fiber::ptr cur = GetThis();
		try{
			cur->m_cb();
			cur->m_cb = nullptr;
			cur->m_state = END;
		}catch(std::exception& ex){
			cur->m_state = EXCEPT;
		}catch(...){
			cur->m_state = EXCEPT;
		}
		auto raw_ptr = cur.get();
		cur.reset();
		raw_ptr->back();
	}

	void Fiber::MainFunc(){
		Fiber::ptr cur = GetThis();
		try{
			cur->m_cb();
			cur->m_cb = nullptr;
			cur->m_state = END;
		}catch(std::exception& ex){
			cur->m_state = EXCEPT;
		}catch(...){
			cur->m_state = EXCEPT;
		}
		
		auto raw_ptr = cur.get();
		cur.reset();
		raw_ptr->swapOut();
	
	}

	void Fiber::YieldToReady(){
		Fiber::ptr cur = GetThis();
		if(cur->m_state != RUNNING){
			perror("fiber is running");
			return;
		}
		cur->m_state = READY;
		cur->swapOut();
	}

	void Fiber::YieldToHold(){
		Fiber::ptr cur = GetThis();
		if(cur->m_state != RUNNING){
			perror("fiber is running");
			return;
		}
		//cur->m_state = HOLD;
		cur->swapOut();
	}

	void Fiber::reset(std::function<void()> cb){
		m_cb = cb;
		getcontext(&m_ctx);
		m_ctx.uc_link = nullptr;
		m_ctx.uc_stack.ss_size = m_stacksize;
		m_ctx.uc_stack.ss_sp = m_stack;
		makecontext(&m_ctx,&Fiber::MainFunc,0);
		m_state = INIT;
	}

}
