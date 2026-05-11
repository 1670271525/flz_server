#ifndef __FLZ_MUTEX_H__
#define __FLZ_MUTEX_H__

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <atomic>
#include <list>

#include "nocopyable.h"
#include "fiber.h"

namespace flz {
	class Semaphore :public flz::Nocopyable{
	public:
		Semaphore(uint32_t count = 0);
		~Semaphore();
		virtual void wait();
		virtual void notify();
	private:
		sem_t m_semaphore;
	};

	template<class T>
	struct ScopedLockImpl{
	public:
		ScopedLockImpl(T& mutex):m_mutex(mutex),m_lock(true){
			m_mutex.lock();
		}
		~ScopedLockImpl(){
			m_lock = false;
			m_mutex.unlock();
		}
		void lock(){
			if(!m_lock){
				m_mutex.lock();
				m_lock = true;
			}
		}
		void unlock(){
			if(m_lock){
				m_mutex.unlock();
				m_lock = false;
			}
		}
	private:
		T& m_mutex;
		bool m_lock;
	};
	template <class T>
	struct ReadScopedLockImpl{
	public:
		ReadScopedLockImpl(T& mutex):m_mutex(mutex){
			m_mutex.rdlock();
			m_lock = true;
		};
		~ReadScopedLockImpl(){
			unlock();
		}
		void lock(){
			if(!m_lock){
				m_mutex.rdlock();
				m_lock = true;
			}
		}
		void unlock(){
			if(m_lock){
				m_mutex.unlock();
				m_lock = false;
			}
		}
	private:
		T& m_mutex;
		bool m_lock;
	};
	template<class T>
	struct WriteScopedLockImpl{
	public:
		WriteScopedLockImpl(T& mutex):m_mutex(mutex){
			m_mutex.wrlock();
			m_lock = true;
		}
		~WriteScopedLockImpl(){
			unlock();
		}
		void lock(){
			if(!m_lock){
				m_mutex.wrlock();
				m_lock = true;
			}
		}
		void unlock(){
			if(m_lock){
				m_mutex.unlock();
				m_lock = false;
			}
		}
	private:
		T& m_mutex;
		bool m_lock;
	};
	class Mutex:Nocopyable{
	public:
		typedef ScopedLockImpl<Mutex> Lock; 
		Mutex(){
			pthread_mutex_init(&m_mutex,nullptr);
		}
		~Mutex(){
			pthread_mutex_destroy(&m_mutex);
		}
		void lock(){
			pthread_mutex_lock(&m_mutex);
		}
		void unlock(){
			pthread_mutex_unlock(&m_mutex);
		}
	private:
		pthread_mutex_t m_mutex;
	};


	class RWMutex:Nocopyable{
	public:
		using ReadLock = ReadScopedLockImpl<RWMutex>;
		using WriteLock = WriteScopedLockImpl<RWMutex>;
		RWMutex(){
			pthread_rwlock_init(&m_lock,nullptr);
		}
		~RWMutex(){
			pthread_rwlock_destroy(&m_lock);
		}
		void rdlock(){
			pthread_rwlock_rdlock(&m_lock);
		}
		void wrlock(){
			pthread_rwlock_wrlock(&m_lock);
		}
		void unlock(){
			pthread_rwlock_unlock(&m_lock);
		}
	private:
		pthread_rwlock_t m_lock;
	};

	/**
	 * @brief 自旋锁
	 */
	class Spinlock : Nocopyable {
	public:
		/// 局部锁
		typedef ScopedLockImpl<Spinlock> Lock;

		/**
		 * @brief 构造函数
		 */
		Spinlock() {
			pthread_spin_init(&m_mutex, 0);
		}

		/**
		 * @brief 析构函数
		 */
		~Spinlock() {
			pthread_spin_destroy(&m_mutex);
		}

		/**
		 * @brief 上锁
		 */
		void lock() {
			pthread_spin_lock(&m_mutex);
		}

		/**
		 * @brief 解锁
		 */
		void unlock() {
			pthread_spin_unlock(&m_mutex);
		}
	private:
		/// 自旋锁
		pthread_spinlock_t m_mutex;
	};


	class Scheduler;
	class FiberSemaphore : Nocopyable {
	public:
		typedef Spinlock MutexType;

		FiberSemaphore(size_t initial_concurrency = 0);
		~FiberSemaphore();

		bool tryWait();
		void wait();
		void notify();

		size_t getConcurrency() const { return m_concurrency;}
		void reset() { m_concurrency = 0;}
	private:
		MutexType m_mutex;
		std::list<std::pair<Scheduler*, Fiber::ptr> > m_waiters;
		size_t m_concurrency;
	};




}







#endif
