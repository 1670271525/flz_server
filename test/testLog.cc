#include "include/log.h"


flz::Logger::ptr g_logger = FLZ_LOG_ROOT();

void test01(){
	FLZ_LOG_DEBUG(g_logger)<<"test01";
}


int main(int argc,char** argv){
	test01();


	return 0;
}
