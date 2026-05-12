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
#include "include/crypto_util.h"
#include "include/hash_util.h"
#include <boost/lexical_cast.hpp>



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
		
	class StringUtil {
	public:
		static std::string Format(const char* fmt, ...);
		static std::string Formatv(const char* fmt, va_list ap);

		static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
		static std::string UrlDecode(const std::string& str, bool space_as_plus = true);

		static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
		static std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");
		static std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");


		static std::string WStringToString(const std::wstring& ws);
		static std::wstring StringToWString(const std::string& s);

	};


	std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");
	time_t Str2Time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S");


	template<class V, class Map, class K>
	V GetParamValue(const Map& m, const K& k, const V& def = V()) {
		auto it = m.find(k);
		if(it == m.end()) {
			return def;
		}
		try {
			return boost::lexical_cast<V>(it->second);
		} catch (...) {
		}
		return def;
	}




}





#endif
