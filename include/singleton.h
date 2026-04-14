#pragma once
#ifndef __FLZ_SINGLETON_H
#define __FLZ_SINGLETON_H

#include <memory>


namespace flz {

	template<class T,class X = void,int N = 0>
	class Singleton{
	public:
		static T* GetInstance(){
			static T v;
			return &v;
		}
			
	};
	
	template<class T,class X = void,int N = 0>
	class SingletonPtr{
	public:
		static std::shared_ptr<T> GetInstance(){
			static std::shared_ptr<T> v(new T);
			return v;
		}
	};



}


#endif


