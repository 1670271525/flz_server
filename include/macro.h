#pragma once
#ifndef __FLZ_MACRO_H__
#define __FLZ_MACRO_H__

#include <assert.h>
#include "include/log.h"
#include <string.h>

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define FLZ_LIKELY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define FLZ_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define FLZ_LIKELY(x)      (x)
#   define FLZ_UNLIKELY(x)      (x)
#endif



#define FLZ_ASSERT(x)\
 	if(!(x)){\
		FLZ_LOG_ERROR(FLZ_LOG_ROOT())<<"ASSERTION: "#x<<"\nbacktrace:\n"<<flz::BacktraceToString(100,2,"     ");\
		assert(x);\
	}

#define FLZ_ASSERT2(x,w)\
	if(!(x)){\
		FLZ_LOG_ERROR(FLZ_LOG_ROOT())<<"ASSERTION: "#x<<"\n"<<w<<"\nBACKTRACE:\n"<<flz::BacktraceToString(100,2,"     ");\
		assert(x);\
	}



#endif
