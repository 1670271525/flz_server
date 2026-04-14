#ifndef __FLZ_UTIL_H__
#define __FLZ_UTIL_H__
#include <stdint.h>
#include <cstddef>
#include <ctime>
#include <sys/stat.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <sys/syscall.h>
#include <vector>


namespace flz{
	
	uint64_t getCurrentMS();
	uint64_t getCurrentUS();



	
	void Backtrace(std::vector<std::string>& bt,int size,int skip);
	std::string BacktraceToString(int size,int skip,const std::string& prefix);


	pid_t GetThreadId();
	

	class Data{
	public:
		static std::size_t now(){return (std::size_t)time(nullptr);}

	};
	class File{
	public:
		static bool exists(const std::string& name);
		static std::string path(const std::string& name);
		static void create_direction(const std::string& path);
		static bool OpenForWrite(std::ofstream& ofs, const std::string& filename, std::ios_base::openmode mode);	
		static std::string getContent(const std::string& path);

	};
		
}





#endif
