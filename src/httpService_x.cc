#include "../include/httpService_x.h"




namespace flz {

	std::string HttpRequest::getLine(std::string& req_line){
		auto pos = req_line.find(base_sep);
		if(pos == std::string::npos){
			return std::string();
		}
		std::string line = req_line.substr(0,pos);
		size_t erase_len = line.size() + base_sep.size();
		if (erase_len <= req_line.size()) {
			req_line.erase(0, erase_len);
		} else {
			req_line.clear(); // 若长度不足，直接清空
		}
		return line.empty()?base_sep:line;
	}
	
	void HttpRequest::parseReqLine(){
		std::stringstream ss(m_req_line);
		ss>>m_method>>m_url>>m_version;
		if(strcasecmp(m_method.c_str(),"GET")==0){
			auto pos = m_url.find(arg_sep);
			if(pos != std::string::npos){
				m_body_text = m_url.substr(pos + arg_sep.size());
				m_url.resize(pos);
			}
		}
		m_path = m_url;
		if(!m_path.empty() && m_path[m_path.size()-1]=='/'){
			m_path += homepage;
		}
		auto pos = m_path.rfind(suffixsep);
		if(pos != std::string::npos){
			if (pos <= m_path.size()) {
				m_suffix = m_path.substr(pos);
			} else {
				m_suffix = ".default";
			}
		}else{
			m_suffix = ".default";
		}
		
	}

	void HttpRequest::parseReqHeader(){
		for(auto &header:m_req_headers){
			auto pos = header.find(line_sep);
			if(pos != std::string::npos){
				std::string key = header.substr(0,pos);
				std::string value = header.substr(pos + line_sep.size());
				size_t start_pos = pos + line_sep.size();
				if (start_pos <= header.size()) {
					value = header.substr(start_pos);
				} else {
					value = ""; // 非法场景兜底为空字符串
				}
				if(key.empty() || value.empty())continue;
				m_headers_kv[key] = value;
			}
		}
	}

	void HttpRequest::deserialize(std::string& reqstr){
		m_req_line = getLine(reqstr);
		std::string header;
		do{
			header = getLine(reqstr);
			if(header.empty())break;
			else if(header == base_sep)break;
			m_req_headers.push_back(header);
		}while(true);
		if(!reqstr.empty()){
			m_body_text = reqstr;
		}
		parseReqLine();
		parseReqHeader();
	}


	void HttpRequest::hprint(){
		std::cout << "------------\n";
		std::cout<<"###"<<m_req_line<<"\n";
		for(auto &header:m_req_headers){
			std::cout<<"@@@"<<header<<"\n";
		}
		std::cout<<"***"<<m_blank_line;
		std::cout<<">>>"<<m_body_text<<"\n";
		std::cout<<"Method: "<<m_method<<"\n";
		std::cout<<"Url: "<<m_url<<"\n";
		std::cout<<"Version"<<m_version<<"\n";
		for(auto &header_kv:m_headers_kv){
			std::cout<<")))"<<header_kv.first<<"->"<<header_kv.second<<"\n";
		}
	}
	void HttpResponse::addCode(int code, const std::string &desc)
    {
        m_status_code = code;
        m_desc = desc;
    }
    void HttpResponse::addHeader(const std::string &k, const std::string &v)
    {
        m_headers_kv[k] = v;
    }
    void HttpResponse::addBodyText(const std::string &body_text)
    {
        m_resp_body_text = body_text;
    }
    std::string HttpResponse::serialize()
    {
        // 1.构建状态行
        m_status_line = m_version + spacesep + std::to_string(m_status_code) + spacesep + m_desc + base_sep;
 
        // 2.构建应答报头
        for (auto &header : m_headers_kv)
        {
            std::string header_line = header.first + line_sep + header.second + base_sep;
            m_resp_headers.push_back(header_line);
        }
 
        // 3.空行和正文
 
        // 4.正式序列化
        std::string responsestr = m_status_line;
        for (auto &line : m_resp_headers)
        {
            responsestr += line;
        }
        responsestr += m_blank_line;
        responsestr += m_resp_body_text;
 
        return responsestr;
    }
	
	HttpServer::HttpServer(){
		m_mime_type.insert(std::make_pair(".html","text/html"));
		m_mime_type.insert(std::make_pair(".jpg","image/jpeg"));
		m_mime_type.insert(std::make_pair(".png","image/png"));
		m_mime_type.insert(std::make_pair(".default","text/html"));

		m_code_to_desc.insert(std::make_pair(100,"Continue"));
		m_code_to_desc.insert(std::make_pair(200,"Ok"));
		m_code_to_desc.insert(std::make_pair(201,"Created"));
		m_code_to_desc.insert(std::make_pair(301,"Moved Permanently"));
		m_code_to_desc.insert(std::make_pair(302,"Found"));
		m_code_to_desc.insert(std::make_pair(404,"Not Found"));
	}

	std::string HttpServer::getFileContent(const std::string& path){
		return flz::File::getContent(path);
	}

	std::string HttpServer::handlerHttpRequest(std::string& reqstr){
#ifdef TEST
		std::cout<<"---------------\n";
		std::cout<<reqstr;

		std::string responseStr = "HTTP/1.1 200 OK\r\n";
		responseStr += "Content-Type: text/html\r\n";
		responseStr += "<html><h1>Hello Linux</h1><html>";
		return responseStr;
#else	
		std::cout<<"---------------\n";
		std::cout<<reqstr;
		std::cout<<"---------------\n";
		HttpRequest request;
		HttpResponse response;
		request.deserialize(reqstr);
		if(request.getPath() == "wwwroot/redir"){
			std::string redir_path = "https://www.baidu.com";
			response.addCode(301,m_code_to_desc[301]);
			response.addHeader("Location",redir_path);
		}else if(!request.getRequestBody().empty()){
			if(isServiceExist(request.getPath())){
				response = m_service_list[request.getPath()](request);
			}
		}else{
			std::string content = getFileContent(request.getPath());
			if(content.empty()){
				content = getFileContent("wwwroot/404.html");
				response.addCode(404,m_code_to_desc[404]);
				response.addHeader("Content-Length",std::to_string(content.size()));
				response.addHeader("Content-Type",m_mime_type[".html"]);
				response.addBodyText(content);
			}else{
				response.addCode(200,m_code_to_desc[200]);
				response.addHeader("Content-Length",std::to_string(content.size()));
				response.addHeader("Content-Type",m_mime_type[request.getSuffix()]);
				response.addHeader("Set-Cookie","username=admin");
				response.addBodyText(content);
			}
		}
		return response.serialize();
#endif
	}

	void HttpServer::insertService(const std::string& serviceName,func_t f){
		m_service_list[serviceName] = f;
	}
	
	bool HttpServer::isServiceExist(const std::string& serviceName){
		auto it = m_service_list.find(serviceName);
		if(it == m_service_list.end())return false;
		return true;
	}

}
