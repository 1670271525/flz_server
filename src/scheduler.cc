#include "../include/scheduler.h"
#include "../include/log.h"
#include "../include/hook.h"




namespace flz {

	static thread_local Scheduler* t_scheduler = nullptr;
	static thread_local flz::Fiber* t_scheduler_fiber = nullptr;
	static flz::Logger::ptr g_logger = FLZ_LOG_ROOT();

	Scheduler::Scheduler(size_t threads,bool use_caller,const std::string& name):m_name(name){
		if(use_caller){
			flz::Fiber::GetThis();
			threads--;
			t_scheduler = this;
			m_rootFiber.reset(new flz::Fiber(std::bind(&Scheduler::run,this),0,true));
			flz::Thread::SetName(m_name);
			t_scheduler_fiber = m_rootFiber.get();
			m_threadIds.push_back(m_rootThread);
		}else{
			m_rootThread = -1;
		}
		m_threadCount = threads;
	}
	Scheduler::~Scheduler(){
		if(GetThis()==this){
			t_scheduler = nullptr;
		}
	}
	Scheduler* Scheduler::GetThis(){
		return t_scheduler;
	}
	flz::Fiber* Scheduler::GetMainFiber(){
		return t_scheduler_fiber;
	}
	void Scheduler::start(){
		MuteType::Lock lock(m_mutex);
		if(!m_stopping){return;}
		m_stopping = false;
		m_threads.resize(m_threadCount);
		for(size_t i = 0;i<m_threadCount;i++){
			m_threads[i].reset(new flz::Thread(std::bind(&flz::Scheduler::run,this),m_name+"_"+std::to_string(i)));
			m_threadIds.push_back(m_threads[i]->getId());
		}
		lock.unlock();
	}
	void Scheduler::stop(){
		m_autoStop = true;
		if(m_rootFiber&&m_threadCount==0&&(m_rootFiber->getState()==Fiber::State::END||m_rootFiber->getState()==Fiber::State::INIT)){
			m_stopping = true;
			if(stopping())return;
		}
		if(m_rootThread!=-1){
			FLZ_ASSERT(GetThis()==this);
		}else{
			FLZ_ASSERT(GetThis()!=this);
		}
		m_stopping = true;
		for(size_t i = 0;i<m_threadCount;i++){
			tickle();
		}
		if(m_rootFiber)tickle();
		if(m_rootFiber){
			if(!stopping())m_rootFiber->call();
		}
		std::vector<flz::Thread::ptr> thrs;
		{
			MuteType::Lock lock(m_mutex);
			thrs.swap(m_threads);
		}
		for(auto& t:thrs){
			t->join();
		}
	}

	void Scheduler::setThis(){
		t_scheduler = this;
	}
	
	void Scheduler::run(){
		FLZ_LOG_DEBUG(g_logger)<<m_name<<" run";
		set_hook_enable(true);
		setThis();
		if(flz::GetThreadId()!=m_rootThread){
			t_scheduler_fiber = Fiber::GetThis().get();
		}
		Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle,this)));
		Fiber::ptr cb_fiber;

		FiberAndThread ft;
		while(true){
			ft.reset();
			bool tickle_me = false;
			bool is_active = false;
			{
				MuteType::Lock lock(m_mutex);
				auto it = m_fibers.begin();
				while(it!=m_fibers.end()){
					if(it->thread != -1 && it->thread != flz::GetThreadId()){
						it++;
						tickle_me = true;
						continue;
					}
					FLZ_ASSERT(it->fiber||it->cb);
					if(it->fiber && it->fiber->getState() == Fiber::State::RUNNING){
						it++;
						continue;
					}
					ft = *it;
					m_fibers.erase(it++);
					m_activeThreadCount++;
					is_active = true;
					break;
				}
				tickle_me |= it != m_fibers.end();
			}
			if(tickle_me)tickle();
			if(ft.fiber && (ft.fiber->getState()!=Fiber::State::END &&ft.fiber->getState()!=Fiber::State::EXCEPT)){
				ft.fiber->swapIn();
				m_activeThreadCount--;
				if(ft.fiber->getState()==Fiber::State::READY){
					schedule(ft.fiber);
				}else if(ft.fiber->getState()!=Fiber::State::END&&ft.fiber->getState()!=Fiber::State::EXCEPT){
					ft.fiber->setState(Fiber::State::HOLD);
				}
				ft.reset();
			}else if(ft.cb){
				if(cb_fiber){
					cb_fiber->reset(ft.cb);
				}else{
					cb_fiber.reset(new Fiber(ft.cb));
				}
				ft.reset();
				cb_fiber->swapIn();
				m_activeThreadCount--;
				if(cb_fiber->getState() == Fiber::READY){
					schedule(cb_fiber);
					cb_fiber.reset();
				}else if(cb_fiber->getState()==Fiber::State::EXCEPT||cb_fiber->getState()==Fiber::State::END){
					cb_fiber->reset(nullptr);
				}else{
					cb_fiber->setState(Fiber::State::HOLD);
					cb_fiber.reset();
				}
			}else{
				if(is_active){
					m_activeThreadCount--;
					continue;
				}
				if(idle_fiber->getState() == Fiber::State::END) {
					FLZ_LOG_INFO(g_logger) << "idle fiber term";
					break;
				}

				++m_idleThreadCount;
				idle_fiber->swapIn();
				--m_idleThreadCount;
				if(idle_fiber->getState() != Fiber::State::END
						&& idle_fiber->getState() != Fiber::State::EXCEPT) {
					idle_fiber->setState(Fiber::State::HOLD);
				}
			}
		}
	}
	
	void Scheduler::tickle(){
		FLZ_LOG_INFO(g_logger)<<"tickle";
	}
	
	bool Scheduler::stopping(){
		MuteType::Lock lock(m_mutex);
		return m_stopping && m_autoStop && m_fibers.empty() && m_threadCount==0;
	}
	
	void Scheduler::idle(){
		FLZ_LOG_INFO(g_logger)<<"idle";
		while(!stopping()){
			flz::Fiber::YieldToHold();
		}
	}


	void Scheduler::switchTo(int thread) {
		FLZ_ASSERT(Scheduler::GetThis() != nullptr);
		if(Scheduler::GetThis() == this) {
			if(thread == -1 || thread == flz::GetThreadId()) {
				return;
			}
		}
		schedule(Fiber::GetThis(), thread);
		Fiber::YieldToHold();
	}

	std::ostream& Scheduler::dump(std::ostream& os) {
		os << "[Scheduler name=" << m_name
		   << " size=" << m_threadCount
		   << " active_count=" << m_activeThreadCount
		   << " idle_count=" << m_idleThreadCount
		   << " stopping=" << m_stopping
		   << " ]" << std::endl << "    ";
		for(size_t i = 0; i < m_threadIds.size(); ++i) {
			if(i) {
				os << ", ";
			}
			os << m_threadIds[i];
		}
		return os;
	}	

}
