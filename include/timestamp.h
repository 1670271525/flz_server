#pragma once
#ifndef __FLZ_TIMESTAMP_H__
#define __FLZ_TIMESTAMP_H__

#include <iostream>
#include <string>



namespace flz {

	class Timestamp{
	public:
		Timestamp();
		explicit Timestamp(int64_t microSecondsSinceEpoch);
		static Timestamp now();
		std::string toString() const;

	private:
		int64_t m_microSecondsSinceEpoch;
	};


}
#endif
