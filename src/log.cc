#include "../include/log.h"
#include <cstdarg>

namespace flz {

	const char* LogLevel::toString(LogLevel::Level level){
		switch(level){
#define XX(name)\
			case LogLevel::name :\
				return #name;

			XX(DEBUG);
			XX(INFO);
			XX(ERROR);
			XX(WARN);
			XX(FATAL);
#undef XX			
			default:
				return "UNKNOW";
		}
		return "UNKNOW";
	}

	const LogLevel::Level LogLevel::fromString(const std::string& str){
#define XX(level,v)\
		if(str == #v){\
			return LogLevel::level;\
		}
		XX(DEBUG,debug);
		XX(INFO,info);
		XX(WARN,warn);
		XX(ERROR,error);
		XX(FATAL,fatal);

		XX(DEBUG,DEBUG);
		XX(INFO,INFO);
		XX(WARN,WARN);
		XX(ERROR,ERROR);
		XX(FATAL,FATAL);
#undef XX
		return LogLevel::UNKNOW;

	}
	LogEvent::LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char* file,int32_t line,uint32_t elapse,uint32_t threadId,uint32_t fiberId,uint32_t time,const std::string& threadName ):m_logger(logger),m_level(level),m_file(file),m_line(line),m_elapse(elapse),m_threadId(threadId),m_fiberId(fiberId),m_time(time),m_threadName(threadName){
	}

	void LogEvent::format(const char* fmt,...){
		va_list vl;
		va_start(vl,fmt);
		format(fmt,vl);
		va_end(vl);
	}

	void LogEvent::format(const char* fmt,va_list vl){
		char* buf = nullptr;
		int len = vasprintf(&buf,fmt,vl);
		if(len!=-1){
			m_ss<<std::string(buf,len);
			free(buf);
		}
	}
	LogFormatter::LogFormatter(const std::string& pattern):m_pattern(pattern){
		init();
	}



	class MessageFormatItem : public LogFormatter::FormatItem {
	public:
		MessageFormatItem(const std::string& str = "") {};
		void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)override {
			os << event->getContent();
		}
	};

	class LevelFormatItem : public LogFormatter::FormatItem {
	public:
		LevelFormatItem(const std::string& str = "") {};
		void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
			os << LogLevel::toString(level);
		}
	};

	class ElapseFormatItem : public LogFormatter::FormatItem {
	public:
		ElapseFormatItem(const std::string& str = "") {};
		void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)override {
			os << event->getElapse();
		}
	};

	class NameFormatItem : public LogFormatter::FormatItem {
	public:
		NameFormatItem(const std::string& str = "") {};
		void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
			os << logger->getName();
		}
	};

	class ThreadIdFormatItem : public LogFormatter::FormatItem {
	public:
		ThreadIdFormatItem(const std::string& str = "") {};
		void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
			os << event->getThreadId();
		}
	};

	class FiberIdFormatItem : public LogFormatter::FormatItem {
	public:
		FiberIdFormatItem(const std::string& str = "") {};
		void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)override {
			os << event->getFiberId();
		}
	};

	class ThreadNameFormatItem : public LogFormatter::FormatItem {
	public:
		ThreadNameFormatItem(const std::string& str = "") {};
		void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)override {
			os << event->getThreadName();
		}
	};

	class DateTimeFormatItem : public LogFormatter::FormatItem {
	public:
		DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S") :m_format(format) {};
		void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)override {
			struct tm tm;
			time_t time = event->getTime();
			localtime_r(&time, &tm);
			char buf[64];
			strftime(buf, sizeof(buf), m_format.c_str(), &tm);
			os << buf;
		}
	private:
		const std::string m_format;
	};

	class FilenameFormatItem : public LogFormatter::FormatItem {
	public:
		FilenameFormatItem(const std::string& str = "") {};
		void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)override {
			os << event->getFile();
		}
	};

	class LineFormatItem : public LogFormatter::FormatItem {
	public:
		LineFormatItem(const std::string& str = "") {};
		void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)override {
			os << event->getLine();
		}
	};

	class NewLineFormatItem : public LogFormatter::FormatItem {
	public:
		NewLineFormatItem(const std::string& str = "") {};
		void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)override {
			os << std::endl;
		}
	};

	class StringFormatItem : public LogFormatter::FormatItem {
	public:
		StringFormatItem(const std::string& str):m_string(str){};
		void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)override {
			os << m_string;
		}
	private:
		std::string m_string;
	};

	class TabFormatItem : public LogFormatter::FormatItem {
	public:
		TabFormatItem(const std::string& str = "") {};
		void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)override {
			os << '\t';
		}
	};

	void LogFormatter::init() {
		std::vector<std::tuple<std::string, std::string, int>> vec;
		std::string nstr;
		for (size_t i = 0;i < m_pattern.size();i++) {
			if (m_pattern[i] != '%') {
				nstr.append(1,m_pattern[i]);
				continue;
			}
			if ((i + 1) < m_pattern.size()) {
				if (m_pattern[i + 1] == '%') {
					nstr.append(1, '%');
					continue;
				}
			}
			size_t n = i + 1;
			int fmt_status = 0;
			size_t fmt_begin = 0;

			std::string str;
			std::string fmt;
			while (n < m_pattern.size()) {
				if (!fmt_status && !isalpha(m_pattern[n]) && m_pattern[n] != '{'
					&& m_pattern[n] != '}') {
					str = m_pattern.substr(i + 1, n - i - 1);
					break;
				}
				if (!fmt_status && m_pattern[n] == '{') {
					str = m_pattern.substr(i + 1, n - i - 1);
					fmt_status = 1;
					fmt_begin = n;
					++n;
					continue;
				}
				else if (fmt_status == 1 && m_pattern[n] == '}') {
					fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
					fmt_status = 0;
					++n;
					break;
				}
				++n;
				if (n == m_pattern.size()) {
					if (str.empty()) {
						str = m_pattern.substr(i + 1);
					}
				}
			}
			if (fmt_status == 0) {
				if (!nstr.empty()) {
					vec.push_back(std::make_tuple(nstr, "", 0));
					nstr.clear();
				}
				vec.push_back(std::make_tuple(str, fmt, 1));
				i = n - 1;
			}
			else if (fmt_status == 1) {
				std::cout << "pattern parse error: " << m_pattern << '-' << m_pattern.substr(i) << std::endl;
				m_error = true;
				vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
			}
		}
		if (!nstr.empty()) {
			vec.push_back(std::make_tuple(nstr, "", 0));
		}
		static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> s_format_items = {
#define		XX(str,C) \
		{#str,[](const std::string& fmt){return FormatItem::ptr(new C(fmt));}}

			XX(m, MessageFormatItem),           //m:消息
			XX(p, LevelFormatItem),             //p:日志级别
			XX(r, ElapseFormatItem),            //r:累计毫秒数
			XX(c, NameFormatItem),              //c:日志名称
			XX(t, ThreadIdFormatItem),          //t:线程id
			XX(n, NewLineFormatItem),           //n:换行
			XX(d, DateTimeFormatItem),          //d:时间
			XX(f, FilenameFormatItem),          //f:文件名
			XX(l, LineFormatItem),              //l:行号
			XX(T, TabFormatItem),               //T:Tab
			XX(F, FiberIdFormatItem),           //F:协程id
			XX(N, ThreadNameFormatItem),        //N:线程名称
#undef XX


		};
		for (auto& i : vec) {
			if (std::get<2>(i) == 0) {
				m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
			}
			else {
				auto it = s_format_items.find(std::get<0>(i));
				if (it == s_format_items.end()) {
					m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i))));

				}
				else {
					m_items.push_back(it->second(std::get<1>(i)));
				}
			}
		}
	}


	void LogAppender::setFormatter(LogFormatter::ptr val) {
		m_formatter = val;
		if (m_formatter) {
			m_hasFormatter = true;
		}
		else {
			m_hasFormatter = false;
		}
	}
	LogFormatter::ptr LogAppender::getFormatter() {
		return m_formatter;
	}

	StdoutLogAppender::StdoutLogAppender(){
	}

	void StdoutLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
		if(level>=m_level){

			m_formatter->format(std::cout,logger,level,event);
			
		}
	}

	FileLogAppender::FileLogAppender(const std::string& name):m_filename(name){
		reopen();
	};

	void FileLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
		if(level>=m_level){
			uint64_t now = event->getTime();
			if(now >= (m_lastTime+3)){
				reopen();
				m_lastTime = now;
			}
			if(!m_formatter->format(m_filestream,logger,level,event)){
				perror("FileLogAppender format fail");
			}
		}
	}
	bool FileLogAppender::reopen(){
		if(m_filestream){
			m_filestream.close();
		}
		return File::OpenForWrite(m_filestream,m_filename,std::ios::app); 
	}
	std::ostream& LogFormatter::format(std::ostream& ofs,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
		for(auto i:m_items){
			i->format(ofs,logger,level,event);
		}
		return ofs;
	}

	void Logger::log(LogLevel::Level level,LogEvent::ptr event){
		if(level>=m_level){
			auto self = shared_from_this();
			if(!m_appenders.empty()){
				for(auto& a:m_appenders){
					a->log(self,level,event);
				}
			}else{
				m_root->log(level,event);
			}

		}
	}
	void  Logger::debug(LogEvent::ptr event) {
		log(LogLevel::DEBUG, event);
	}

	void  Logger::info(LogEvent::ptr event) {
		log(LogLevel::INFO, event);
	}

	void  Logger::warn(LogEvent::ptr event) {
		log(LogLevel::WARN, event);
	}

	void  Logger::error(LogEvent::ptr event) {
		log(LogLevel::ERROR, event);
	}

	void  Logger::fatal(LogEvent::ptr event) {
		log(LogLevel::FATAL, event);
	}
	Logger::Logger(const std::string& name):m_name(name),m_level(LogLevel::DEBUG) {
		m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
	}

	void Logger::setFormatter(LogFormatter::ptr val) {
		m_formatter = val;
		for (auto& i : m_appenders) {
			if (!i->m_hasFormatter) {
				i->setFormatter(m_formatter);
			}
		}
	}

	void Logger::setFormatter(const std::string& val) {
		LogFormatter::ptr new_val(new LogFormatter(val));
		if (new_val->isError()) {
			std::cout << "Logger setFormatter name=" << m_name
				<< " value=" << val << " invalid formatter"
				<< std::endl;
			return;
		}
		setFormatter(new_val);
	}
	void Logger::addAppender(LogAppender::ptr appender){
		if(!appender->m_hasFormatter){
			appender->setFormatter(m_formatter);
		}
		m_appenders.push_back(appender);
	}
	void Logger::delAppender(LogAppender::ptr appender){
		for(auto it = m_appenders.begin();it!=m_appenders.end();it++){
			if(*it == appender){
				m_appenders.erase(it);
				break;
			}
		}
	}
	LoggerManager::LoggerManager(){
		m_root.reset(new Logger);
		m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
		m_root->addAppender(LogAppender::ptr(new FileLogAppender("/home/fang/Desktop/CppProject/TcpServerProject01/Log/Log.txt")));
		
	}
	
	Logger::ptr LoggerManager::getLogger(std::string& name)const{
		auto it = m_loggers.find(name);
		if(it == m_loggers.end())return nullptr;
		return it->second;
	}
	
	


}
