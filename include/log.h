#ifndef __FLZ_LOG_H__
#define __FLZ_LOG_H__


#include <iostream>
#include <string>
#include <memory>
#include <stdint.h>
#include <sstream>
#include <vector>
#include <functional>
#include <map>
#include <ostream>
#include <fstream>
#include <unordered_map>

#include "include/util.h"
#include "include/singleton.h"
#include "include/mutex.h"

#define FLZ_LOG_LEVEL(logger,level)\
	if(logger->getLevel()<=level)\
		flz::LogEventWrap(flz::LogEvent::ptr(new flz::LogEvent(logger,level,__FILE__,__LINE__,0,flz::GetThreadId(),0,time(0),"UNKNOW"))).getSS()
#define FLZ_LOG_INFO(logger) FLZ_LOG_LEVEL(logger,flz::LogLevel::INFO)
#define FLZ_LOG_DEBUG(logger) FLZ_LOG_LEVEL(logger,flz::LogLevel::DEBUG)
#define FLZ_LOG_ERROR(logger) FLZ_LOG_LEVEL(logger,flz::LogLevel::ERROR)
#define FLZ_LOG_WARN(logger) FLZ_LOG_LEVEL(logger,flz::LogLevel::WARN)
#define FLZ_LOG_FATAL(logger) FLZ_LOG_LEVEL(logger,flz::LogLevel::FATAL)

#define FLZ_LOG_ROOT() flz::LoggerMgr::GetInstance()->getRoot()

#define FLZ_LOG_NAME(name) flz::LoggerMgr::GetInstance()->getLogger(name)

namespace flz {


	class Logger;
	class LogLevel{
	public:
		enum Level{
			UNKNOW = 0,
			DEBUG = 1,
			INFO = 2,
			WARN = 3,
			ERROR = 4,
			FATAL = 5
		};
		static const char* toString(LogLevel::Level level);
		static const LogLevel::Level fromString(const std::string& str);
	};
	

	class LogEvent{
	public:
		LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char* file,int32_t line,uint32_t elapse,uint32_t threadId,uint32_t fiberId,uint32_t time,const std::string& threadName );
		typedef std::shared_ptr<LogEvent> ptr;
		uint32_t getLine()const{return m_line;}
		const char* getFile()const{return m_file;}
		uint32_t getElapse()const{return m_elapse;}
		std::string getThreadName()const {return m_threadName;}
		uint32_t getThreadId()const{return m_threadId;}
		uint32_t getFiberId()const{return m_fiberId;}
		uint32_t getTime()const{return m_time;}
		std::string getContent()const{return m_ss.str();}
		std::shared_ptr<Logger> getLogger()const{return m_logger;}
		LogLevel::Level getLevel()const{return m_level;}
		std::stringstream& getSS(){return m_ss;}

		void format(const char* fmt,...);
		void format(const char* fmt,va_list al);
	private:
		const char* m_file = nullptr;
		int32_t m_line = 0;
		uint32_t m_elapse = 0;
		uint32_t m_threadId = 0;
		uint32_t m_fiberId = 0;
		uint32_t m_time = 0;
		std::string m_threadName;
		std::stringstream m_ss;
		std::shared_ptr<Logger> m_logger;
		LogLevel::Level m_level;
	};
	
	class LogFormatter{
	public:
		typedef std::shared_ptr<LogFormatter> ptr;
		void init();
		LogFormatter(const std::string& pattern);
		std::ostream& format(std::ostream& ofs,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event);
		bool isError()const{return m_error;}
	public:
		class FormatItem{
		public:
			typedef std::shared_ptr<FormatItem> ptr;
			virtual ~FormatItem(){};
			virtual void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) = 0;
		};
	private:
		std::string m_pattern;
		std::vector<FormatItem::ptr> m_items;
		bool m_error = false;
	};
	

	class LogAppender{
	friend class Logger;
	public:
		typedef std::shared_ptr<LogAppender> ptr;
		virtual void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) = 0;
		void setFormatter(LogFormatter::ptr val);
		LogFormatter::ptr getFormatter();
	protected:
		LogLevel::Level m_level = LogLevel::DEBUG;
		LogFormatter::ptr m_formatter;
		bool m_hasFormatter = false;
	};

	class LoggerManager;
	class Logger:public std::enable_shared_from_this<Logger>{
	friend class LoggerManager;
	public:
		typedef std::shared_ptr<Logger> ptr;
		Logger(const std::string& name = "root");
		void log(LogLevel::Level level,LogEvent::ptr event);
		void info(LogEvent::ptr event);
		void debug(LogEvent::ptr event);
		void warn(LogEvent::ptr event);
		void error(LogEvent::ptr event);
		void fatal(LogEvent::ptr event);
		void addAppender(LogAppender::ptr appender);
		void delAppender(LogAppender::ptr appender);
		std::string getName()const {return m_name;}
		LogLevel::Level getLevel()const{return m_level;}
		void setLevel(LogLevel::Level level){m_level = level;}
		void clearAppenders(){m_appenders.clear();}
		void setFormatter(LogFormatter::ptr val);
		void setFormatter(const std::string& val);
	private:
		std::string m_name;
		std::vector<LogAppender::ptr> m_appenders;
		Logger::ptr m_root;
		LogLevel::Level m_level;
		LogFormatter::ptr m_formatter;
	};

	class StdoutLogAppender:public LogAppender{
	public:
		StdoutLogAppender();
		void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event);
	};

	class FileLogAppender:public LogAppender{
	public:
		FileLogAppender(const std::string& name);
		void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event)override;
		bool reopen();
		typedef std::shared_ptr<FileLogAppender> ptr;
	private:
		uint64_t m_lastTime = 0;
		std::string m_filename;
		std::ofstream m_filestream;
	};

	class LoggerManager{
	public:
		typedef Spinlock MutexType;
		LoggerManager();
		Logger::ptr getRoot() const{return m_root;}
		Logger::ptr getLogger(const std::string& name);

		void init();


	private:
		MutexType m_mutex;
		std::unordered_map<std::string,Logger::ptr> m_loggers;
		Logger::ptr m_root;



	};

	class LogEventWrap{
	public:
		LogEventWrap(LogEvent::ptr event):m_event(event){};
		~LogEventWrap(){
			m_event->getLogger()->log(m_event->getLevel(),m_event);
		}
		LogEvent::ptr getEvent()const{return m_event;}
		std::stringstream& getSS(){return m_event->getSS();}
	private:
		LogEvent::ptr m_event;
	};



	typedef Singleton<LoggerManager> LoggerMgr;

}


#endif
