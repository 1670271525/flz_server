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
#include <sys/types.h>
#include <signal.h>




namespace flz{
	
	uint64_t getCurrentMS();
	uint64_t getCurrentUS();

	class FSUtil {
	public:
		static void ListAllFile(std::vector<std::string>& files
								,const std::string& path
								,const std::string& subfix);
		static bool Mkdir(const std::string& dirname);
		static bool IsRunningPidfile(const std::string& pidfile);
		static bool Rm(const std::string& path);
		static bool Mv(const std::string& from, const std::string& to);
		static bool Realpath(const std::string& path, std::string& rpath);
		static bool Symlink(const std::string& frm, const std::string& to);
		static bool Unlink(const std::string& filename, bool exist = false);
		static std::string Dirname(const std::string& filename);
		static std::string Basename(const std::string& filename);
		static bool OpenForRead(std::ifstream& ifs, const std::string& filename
						,std::ios_base::openmode mode);
		static bool OpenForWrite(std::ofstream& ofs, const std::string& filename
						,std::ios_base::openmode mode);
	};


	
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
