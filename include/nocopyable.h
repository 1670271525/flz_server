#ifndef __FLZ_NOCOPYABLE_H__
#define __FLZ_NOCOPYABLE_H__

namespace flz {
	class Nocopyable{
	public:
		Nocopyable()=default;
		~Nocopyable()=default;
		Nocopyable(const Nocopyable&&)=delete;
		Nocopyable(const Nocopyable&)=delete;
		Nocopyable& operator=(const Nocopyable&)=delete;
	};
}



#endif
