#ifndef __FLZ__CHANNEL_H__
#define __FLZ__CHANNEL_H__

#include "include/nocopyable.h"
#include <functional>
#include "include/timestamp.h"
#include <memory>

namespace flz {
	

	class Channel:Nocopyable{
	public:
		using EventCallback = std::function<void()>;
		using ReadEventCallback = std::function<void(flz::Timestamp)>;
		Channel();
		~Channel();
		
		void handleEvent(flz::Timestamp receiveTime);

		void setReadCallback(ReadEventCallback cb){m_readCallback = std::move(cb);}
		void setWriteCallback(EventCallback cb){m_writeCallback = std::move(cb);}
		void setCloseCallback(EventCallback cb){m_closeCallback = std::move(cb);}
		void setErrorCallback(EventCallback cb){m_errorCallback = std::move(cb);}

		void tie(const std::shared_ptr<void>&);
		
		int getFd(){return m_fd;}
		int getEvents(){return m_events;}
		int getIndex(){return m_index;}
		void setIndex(int index){m_index = index;}
		void setRevents(int revt){m_revents = revt;}

		void enableReading(){m_events |= m_kReadEvent;update();}
		void disableReading(){m_events &= ~m_kReadEvent;update();}
		void enableWriting(){m_events |= m_kWriteEvent;update();}
		void disableWriting(){m_events &= ~m_kWriteEvent;update();}
		void disableAll(){m_events = m_kNoneEvent;update();}

		bool isNoneEvent(){return m_events == m_kNoneEvent;}
		bool isReading(){return m_events & m_kReadEvent;}
		bool isWriting(){return m_events & m_kWriteEvent;}

	private:
		void update();
		void handleEventWithGuard(flz::Timestamp receiveTime);

		static const int m_kNoneEvent;
		static const int m_kReadEvent;
		static const int m_kWriteEvent;

		const int m_fd;
		int m_events;
		int m_revents;
		int m_index;

		std::weak_ptr<void> m_tie;
		bool tied;

		ReadEventCallback m_readCallback;
		EventCallback m_writeCallback;
		EventCallback m_closeCallback;
		EventCallback m_errorCallback;

	};


}



#endif
