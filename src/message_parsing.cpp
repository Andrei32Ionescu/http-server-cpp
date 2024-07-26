#include <iostream>
#include "message_parsing.hpp"

struct Http_request parse_message(std::string msg) {
  Http_request request;
  std::string request_line = msg.substr(0,msg.find("\r\n"));
  std::string headers_and_body = msg.substr(msg.find("\r\n") + 2);
 
  request.http_method = request_line.substr(0, request_line.find(" "));
  request_line = request_line.substr(request_line.find(" ") + 1);
  request.request_target = request_line.substr(0, request_line.find(" "));
  request_line = request_line.substr(request_line.find(" ") + 1);
  request.http_version = request_line.substr(0, request_line.find(" "));

  std::unordered_map<std::string, std::string> headers;
  while(headers_and_body.find("\r\n") != -1) {
    std::string header = headers_and_body.substr(0, headers_and_body.find("\r\n"));

    if(header.size() == 0) {
      headers_and_body = headers_and_body.substr(headers_and_body.find("\r\n") + 2);
      break;
    }

    std::string header_name = header.substr(0, headers_and_body.find(": "));
    std::string header_value = header.substr(headers_and_body.find(": ") + 2);

    for(int i = 0; i < header_name.size(); i++){
      header_name[i] = std::tolower(header_name[i]);
    }

    headers.insert({header_name, header_value});
    headers_and_body = headers_and_body.substr(headers_and_body.find("\r\n") + 2);
  }
  request.headers = headers;
  request.body = headers_and_body;

  return request;
}

std::set<std::string> parse_encodings(std::string encoding_list) {
    std::set<std::string> accepted_encodings;

    while(encoding_list.find(',') != -1) {
        std::string encoding = encoding_list.substr(0, encoding_list.find(", "));
        accepted_encodings.insert(encoding);
        encoding_list = encoding_list.substr(encoding_list.find(", ") + 2);
    }

    if(encoding_list.size() > 0) {
        accepted_encodings.insert(encoding_list);
    }
    return accepted_encodings;
}