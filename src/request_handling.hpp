#pragma once
#include "message_parsing.hpp"

void respond_to_request(int id, int client_connection, const std::string& file_directory);
void send_response(int client_connection, const std::string& response);
std::string create_response_message(std::string http_version, int status_code, std::string content_type, 
    std::string content_encoding, int content_length, std::string body);
std::string handle_root_request();
std::string handle_echo_request(const Http_request& request);
std::string handle_user_agent_request(const Http_request& request);
std::string handle_file_request(const std::string& file_directory, const Http_request& request, bool is_post);
