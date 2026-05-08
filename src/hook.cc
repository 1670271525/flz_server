#include "../include/hook.h"
#include "../include/log.h"
#include <dlfcn.h>
#include "../include/iomanager.h"
#include "../include/fd_manager.h"
#include <stdarg.h>

static flz::Logger::ptr g_logger = FLZ_LOG_ROOT();

namespace flz {
	
	static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX)\
	XX(sleep)\
	XX(usleep)\
	XX(nanosleep)\
	XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

	void hook_init(){
		static bool is_inited = false;
		if(is_inited){
			return;
		}
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
		HOOK_FUN(XX);
#undef XX

	}
	const uint64_t s_connect_timeout = -1;
	struct _Hook_Initer{
		_Hook_Initer(){
			hook_init();
		}
	};

	static _Hook_Initer s_hook_initer;

	bool is_hook_enable(){
		return t_hook_enable;
	}

	void set_hook_enable(bool flag){
		t_hook_enable = flag;
	}
	
}

struct timer_info{
	int cancelled = 0;
};

template<typename OriginFun,typename... Args>
static ssize_t do_io(int fd,OriginFun fun,const char* hook_fun_name,uint32_t event,int timeout_so,Args&&... args){
	if(!flz::t_hook_enable)return fun(fd,std::forward<Args>(args)...);
	flz::FdCtx::ptr ctx = flz::FdMgr::GetInstance()->get(fd);
	if(!ctx)return fun(fd,std::forward<Args>(args)...);
	if(ctx->isClose()){
		errno = EBADF;
		return -1;
	}
	if(!ctx->isSocket()||ctx->getUserNonblock())return fun(fd,std::forward<Args>(args)...);
	uint64_t to = ctx->getTimeout(timeout_so);
	std::shared_ptr<timer_info> tinfo(new timer_info);
retry:
	ssize_t n = fun(fd,std::forward<Args>(args)...);
	while(n == -1&&errno == EINTR){
		n = fun(fd,std::forward<Args>(args)...);
	}
	if(n == -1 && errno == EAGAIN){
		flz::IOManager* iom = flz::IOManager::GetThis();
		flz::Timer::ptr timer;
		std::weak_ptr<timer_info> winfo(tinfo);
		if(to!=(uint64_t)-1){
			timer = iom->addConditionTimer(to,[winfo,fd,iom,event](){
						auto t = winfo.lock();
						if(!t||t->cancelled){
							return;
						}
						t->cancelled = ETIMEDOUT;
						iom->cancelEvent(fd,(flz::IOManager::Event)(event));
					},winfo);
		}
		int rt = iom->addEvent(fd,(flz::IOManager::Event)(event));
		if(rt){
			FLZ_LOG_ERROR(g_logger)<<hook_fun_name<<" addEvent("<<fd<<", "<<event<<")";
			if(timer){
				timer->cancel();
			}
			return -1;
		}else{
			flz::Fiber::YieldToHold();
			if(timer){
				timer->cancel();
			}
			if(tinfo->cancelled){
				errno = tinfo->cancelled;
				return -1;
			}
			goto retry;
		}
	}
	return n;
}



extern "C"{
#define XX(name) name ## _fun name ## _f = nullptr;
	HOOK_FUN(XX);
#undef XX
	unsigned int sleep(unsigned int seconds){
		if(!flz::t_hook_enable)return sleep_f(seconds);
		flz::Fiber::ptr fiber = flz::Fiber::GetThis();
		flz::IOManager* iom = flz::IOManager::GetThis();
		//iom->addTimer(seconds*1000,std::bind((void(flz::Scheduler::*)(flz::Fiber::ptr,int thread))&flz::IOManager::schedule,iom,fiber,-1));
		iom->addTimer(seconds * 1000,[iom,fiber](){
			iom->schedule(fiber);
				});
		flz::Fiber::YieldToHold();
		return 0;
	}

	 int usleep(useconds_t usec){
		if(!flz::t_hook_enable)return usleep_f(usec);
		flz::Fiber::ptr fiber = flz::Fiber::GetThis();
		flz::IOManager* iom = flz::IOManager::GetThis();
		iom->addTimer(usec/1000,std::bind((void(flz::Scheduler::*)(flz::Fiber::ptr,int thread))&flz::IOManager::schedule,iom,fiber,-1));
		flz::Fiber::YieldToHold();
		return 0;
	}

	 int nanosleep(const struct timespec *req,struct timespec *rem){
		 if(!flz::t_hook_enable){
			 return nanosleep_f(req,rem);
		 }
		 int timeout_ms = req->tv_sec * 1000 + req->tv_nsec/1000/1000;
		 flz::Fiber::ptr fiber = flz::Fiber::GetThis();
		 flz::IOManager* iom = flz::IOManager::GetThis();
		 iom->addTimer(timeout_ms,std::bind((void(flz::Scheduler::*)(flz::Fiber::ptr,int thread))&flz::IOManager::schedule,iom,fiber,-1));
		 flz::Fiber::YieldToHold();
		 return 0;
	 }

	 int socket(int domain,int type,int protocol){
		 if(!flz::t_hook_enable){
			 return socket_f(domain,type,protocol);
		 }
		int fd = socket_f(domain,type,protocol);
		if(fd == -1){
			return fd;
		}
		flz::FdMgr::GetInstance()->get(fd,true);
		return fd;
	 }

	 int connect_with_timeout(int fd,const struct sockaddr* addr,socklen_t addrlen,uint64_t timeout_ms){
		 FLZ_LOG_INFO(g_logger)<<"t_hook_enable="<<flz::t_hook_enable;
		if(!flz::t_hook_enable)return connect_f(fd,addr,addrlen);
		flz::FdCtx::ptr ctx = flz::FdMgr::GetInstance()->get(fd);
		if(!ctx || ctx->isClose()){
			errno = EBADF;
			return -1;
		}
		if(!ctx->isSocket())return connect_f(fd,addr,addrlen);
		if(ctx->getUserNonblock())return connect_f(fd,addr,addrlen);
		
		int n = connect_f(fd,addr,addrlen);
		if(n==0)return 0;
		else if(n!=-1&&errno!=EINPROGRESS){
			return n;
		}
		flz::IOManager* iom = flz::IOManager::GetThis();
		flz::Timer::ptr timer;
		std::shared_ptr<timer_info> tinfo(new timer_info);
		std::weak_ptr<timer_info> winfo(tinfo);
		if(timeout_ms != (uint64_t)-1){
			timer = iom->addConditionTimer(timeout_ms,[winfo,fd,iom](){
						auto t = winfo.lock();
						if(!t || t->cancelled){
							return;
						}
						t->cancelled = ETIMEDOUT;
						iom->cancelEvent(fd,flz::IOManager::WRITE);
					},winfo);
		}
		int rt = iom->addEvent(fd,flz::IOManager::WRITE);
		if(rt == 0){
			flz::Fiber::YieldToHold();
			if(timer){
				timer->cancel();
			}
			if(tinfo->cancelled){
				errno = tinfo->cancelled;
				return -1;
			}
		}else{
			if(timer){
				timer->cancel();
			}
			FLZ_LOG_ERROR(g_logger)<<"connect addEvent("<<fd<<",WRITE) error";
		}
		int error = 0;
		socklen_t len = sizeof(int);
		if(getsockopt(fd,SOL_SOCKET,SO_ERROR,&error,&len)==-1){
			return -1;
	 	}
	 	if(!error){
			return 0;
		}else{
			errno = error;
			return -1;
		}

	 }

	 int connect(int sockfd,const struct sockaddr* addr,socklen_t addrlen){
		 return connect_with_timeout(sockfd,addr,addrlen,flz::s_connect_timeout);
	 }

	 int accept(int s,struct sockaddr *addr,socklen_t *addrlen){
		int fd = do_io(s,accept_f,"accept",flz::IOManager::READ,SO_RCVTIMEO,addr,addrlen);
		if(fd>0){
			flz::FdMgr::GetInstance()->get(fd,true);
		}
		return fd;
	 }
	
	 ssize_t read(int fd,void* buf,size_t count){
		 return do_io(fd,read_f,"read",flz::IOManager::READ,SO_RCVTIMEO,buf,count);
	 }

	 ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
		return do_io(fd, readv_f, "readv", flz::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
	}

	ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
		return do_io(sockfd, recv_f, "recv", flz::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
	}

	ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
		return do_io(sockfd, recvfrom_f, "recvfrom", flz::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
	}

	ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
		return do_io(sockfd, recvmsg_f, "recvmsg", flz::IOManager::READ, SO_RCVTIMEO, msg, flags);
	}

	ssize_t write(int fd, const void *buf, size_t count) {
		return do_io(fd, write_f, "write", flz::IOManager::WRITE, SO_SNDTIMEO, buf, count);
	}

	ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
		return do_io(fd, writev_f, "writev", flz::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
	}

	ssize_t send(int s, const void *msg, size_t len, int flags) {
		return do_io(s, send_f, "send", flz::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
	}

	ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
		return do_io(s, sendto_f, "sendto", flz::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
	}

	ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
		return do_io(s, sendmsg_f, "sendmsg", flz::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
	}

	int close(int fd){
		if(!flz::t_hook_enable){
			return close_f(fd);
		}
		flz::FdCtx::ptr ctx = flz::FdMgr::GetInstance()->get(fd);
		if(ctx){
			flz::IOManager* iom = flz::IOManager::GetThis();
			if(iom){
				iom->cancelAll(fd);
			}
			flz::FdMgr::GetInstance()->del(fd);
		}
		return close_f(fd);
	}
	
	int fcntl(int fd,int cmd,...){
		va_list va;
		va_start(va,cmd);
		switch(cmd){
			case F_SETFL:
				{
					int arg = va_arg(va,int);
					va_end(va);
					flz::FdCtx::ptr ctx = flz::FdMgr::GetInstance()->get(fd);
					if(!ctx||ctx->isClose()||!ctx->isSocket()){
						return fcntl_f(fd,cmd,arg);
					}
					ctx->setUserNonblock(arg & O_NONBLOCK);
					if(ctx->getSysNonblock()){
						arg |= O_NONBLOCK;
					}else{
						arg &= ~O_NONBLOCK;
					}
					return fcntl_f(fd,cmd,arg);
				}
				break;
			case F_GETFL:
				{
					va_end(va);
					int arg = fcntl_f(fd,cmd);
					flz::FdCtx::ptr ctx = flz::FdMgr::GetInstance()->get(fd);
					if(!ctx || ctx->isClose() || !ctx->isSocket()){
						return arg;
					}
					if(ctx->getUserNonblock()){
						return arg |= O_NONBLOCK;
					}else{
						return arg & ~O_NONBLOCK;
					}
				}
				break;
			case F_DUPFD:
			case F_DUPFD_CLOEXEC:
			case F_SETFD:
			case F_SETOWN:
			case F_SETSIG:
			case F_SETLEASE:
			case F_NOTIFY:
#ifdef F_SETPIPE_SZ
			case F_SETPIPE_SZ:
#endif
				{
					int arg = va_arg(va,int);
					va_end(va);
					return fcntl_f(fd,cmd,arg);
				}
				break;
			case F_GETFD:
			case F_GETOWN:
			case F_GETSIG:
			case F_GETLEASE:
#ifdef F_GETPIPE_SZ
			case F_GETPIPE_SZ:
#endif
				{
					va_end(va);
					return fcntl_f(fd,cmd);
				}
				break;
			case F_SETLK:
			case F_SETLKW:
			case F_GETLK:
				{
					struct flock* arg = va_arg(va, struct flock*);
					va_end(va);
					return fcntl_f(fd, cmd, arg);
				}
				break;
			case F_GETOWN_EX:
			case F_SETOWN_EX:
				{
					struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
					va_end(va);
					return fcntl_f(fd, cmd, arg);
				}
				break;
			default:
				va_end(va);
				return fcntl_f(fd, cmd);
		}
	}

	int ioctl(int d,unsigned long int request,...){
		va_list va;
		va_start(va,request);
		void* arg = va_arg(va,void*);
		va_end(va);
		if(FIONBIO == request){
			bool user_nonblock = !!*(int*)arg;
			flz::FdCtx::ptr ctx = flz::FdMgr::GetInstance()->get(d);
			if(!ctx || ctx->isClose() || !ctx->isSocket()){
				return ioctl_f(d,request,arg);
			}
			ctx->setUserNonblock(user_nonblock);
		}
		return ioctl_f(d,request,arg);
	}
	
	int getsockopt(int sockfd,int level,int optname,void *optval,socklen_t *optlen){
		return getsockopt_f(sockfd,level,optname,optval,optlen);
	}

	int setsockopt(int sockfd,int level,int optname,const void *optval,socklen_t optlen){
		if(!flz::t_hook_enable){
			return setsockopt_f(sockfd,level,optname,optval,optlen);
		}
		if(level == SOL_SOCKET){
			if(optname == SO_RCVTIMEO||optname == SO_SNDTIMEO){
				flz::FdCtx::ptr ctx = flz::FdMgr::GetInstance()->get(sockfd);
				if(ctx){
					const timeval* v = (const timeval*)optval;
					ctx->setTimeout(optname,v->tv_sec*1000+v->tv_usec/1000);
				}
			}
		}
		return setsockopt_f(sockfd,level,optname,optval,optlen);
	}
}





