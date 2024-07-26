#pragma once
#include <set>
#include <unordered_map>

struct Http_request {
    std::string http_method;
    std::string request_target;
    std::string http_version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

struct Http_request parse_message(std::string msg);
std::set<std::string> parse_encodings(std::string encoding_list);