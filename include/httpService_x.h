#ifndef __HTTP_SERVICE_H__
#define __HTTP_SERVICE_H__

#include <sstream>
#include <unordered_map>
#include <string.h>
#include <vector>
#include <iostream>
#include <memory>
#include <functional>
#include "include/util.h"

namespace flz {
	

	const static std::string base_sep = "\r\n";
	const static std::string line_sep = ": ";
	const static std::string prefixpath = ""; // web根目录
	const static std::string homepage = "index.html";
	const static std::string httpversion = "HTTP/1.0";
	const static std::string spacesep = " ";
	const static std::string suffixsep = ".";
	const static std::string html_404 = "404.html";
	const static std::string arg_sep = "?";



	class HttpRequest{
	public:
		typedef std::shared_ptr<HttpRequest> ptr;
		HttpRequest():m_blank_line(base_sep),m_path(prefixpath){};
		std::string getLine(std::string& req_line);//拆分http的请求字符串
		void parseReqLine();//解析http请求行
		void parseReqHeader();//将原始请求头列表解析为键值对哈希表
		void deserialize(std::string &reqstr);
		void hprint();


		std::string getUrl()const{return m_url;}
		std::string getPath()const{return m_path;}
		std::string getMethod()const{return m_method;}
		std::string getSuffix()const{return m_suffix;}
		std::string getRequestBody()const{return m_body_text;}
		


	private:
		std::string m_req_line;//http请求行
		std::vector<std::string> m_req_headers;//未解析原始请求头列表
		std::string m_blank_line;//请求头与请求体之间的空行
		std::string m_body_text;//请求体内容
		
		std::string m_method;//http请求方法
		std::string m_url;//请求url路径
		std::string m_path;//资源在服务器本地的实际路径
		std::string m_suffix;//资源的后缀名
		std::string m_version;//解析后http协议版本
		std::unordered_map<std::string,std::string> m_headers_kv;//解析后的请求头键值对


	};
	class HttpResponse{
	public:
		typedef std::shared_ptr<HttpResponse> ptr;
		HttpResponse():m_version(httpversion),m_blank_line(base_sep){};
		void addCode(int code,const std::string& desc);//响应配置函数
		void addHeader(const std::string& k,const std::string& v);//响应头配置
		void addBodyText(const std::string& body);//响应体配置
		std::string serialize();//序列化
		

	private:
		std::string m_version;
		int m_status_code;
		std::string m_desc;
		std::unordered_map<std::string,std::string> m_headers_kv;

		std::string m_status_line;
		std::vector<std::string> m_resp_headers;
		std::string m_blank_line;
		std::string m_resp_body_text;

	};

	
	class HttpServer{
	public:
		typedef std::shared_ptr<HttpServer> ptr;
		typedef std::function<HttpResponse(HttpRequest& req)> func_t;
		std::string getFileContent(const std::string &path);
		HttpServer();
		std::string handlerHttpRequest(std::string& reqstr);
		void insertService(const std::string& serviceNmae,func_t f);
		bool isServiceExist(const std::string& serviceNmae);

	private:
		std::unordered_map<std::string,std::string> m_mime_type;
		std::unordered_map<int,std::string> m_code_to_desc;
		std::unordered_map<std::string,func_t> m_service_list;
	};	



}


#endif
