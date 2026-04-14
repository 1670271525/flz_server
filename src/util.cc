#include "../include/util.h"
#include <iostream>
#include <execinfo.h>
#include "../include/log.h"
#include <sys/time.h>


namespace flz{
	
	uint64_t getCurrentMS(){
		struct timeval tv;
		gettimeofday(&tv,NULL);
		return tv.tv_sec * 1000ul + tv.tv_usec/1000;
	}
	uint64_t getCurrentUS(){
		struct timeval tv;
		gettimeofday(&tv,NULL);
		return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
	}


	flz::Logger::ptr g_logger = FLZ_LOG_ROOT();

	pid_t GetThreadId(){
		return syscall(SYS_gettid);
	}


	bool File::exists(const std::string& name){
		struct stat st;
		return stat(name.c_str(),&st) == 0;
	}

	std::string File::path(const std::string& name){
		if(name.empty())return ".";
		size_t pos = name.find_last_of("/\\");
		if(pos == std::string::npos)return ".";
		return name.substr(0,pos+1);
	}
	
	void File::create_direction(const std::string& path){
		if(path.empty())return;
		if(exists(path))return;
		size_t pos,idx = 0;
		while(idx < path.size()){
			pos = path.find_first_of("/\\",idx);
			if(pos == std::string::npos){
				mkdir(path.c_str(),0777);
				return;
			}
			if(pos == idx){
				idx = pos+1;
				continue;
			}
			std::string str = path.substr(0,pos);
			if(str == "." || str == ".."){
				idx = pos + 1;
				continue;
			}
			if(exists(str)){
				idx = pos + 1;
				continue;
			}
			mkdir(str.c_str(),0777);
			idx = pos + 1;
		}
	}
	
	bool File::OpenForWrite(std::ofstream& ofs,const std::string& filename,std::ios_base::openmode mode){
		ofs.open(filename.c_str(),mode);
		if(!ofs.is_open()){
			create_direction(filename);
			ofs.open(filename.c_str(),mode);
		}
		return ofs.is_open();
	}

	std::string File::getContent(const std::string& path){
		std::ifstream in(path,std::ios::binary);
		if(!in.is_open())return std::string();
		in.seekg(0,in.end);
		int fileSize = in.tellg();
		in.seekg(0,in.beg);
		std::string content;
		content.resize(fileSize);
		in.read((char*)content.c_str(),fileSize);
		in.close();
		return content;

	}

	void Backtrace(std::vector<std::string>& bt,int size,int skip){
		void** array = (void**)malloc(sizeof(void*)*size);
		size_t s = ::backtrace(array,size);
		char** strings = ::backtrace_symbols(array,s);
		if(strings == NULL){
			FLZ_LOG_ERROR(g_logger)<<"backtrace_symbols error\n";
			return;
		}
		for(size_t i = skip;i<s;i++){
			bt.push_back(strings[i]);
		}
		free(array);
		free(strings);
	}
	std::string BacktraceToString(int size,int skip,const std::string& prefix){
		std::vector<std::string> bt;
		Backtrace(bt,size,skip);
		std::stringstream ss;
		for(int i = 0;i<bt.size();i++){
			ss<<prefix<<bt[i]<<"\n";
		}
		return ss.str();
	}
}

