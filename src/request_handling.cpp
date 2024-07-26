#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include<set>
#include <unordered_map>
#include <stdexcept>
#include "request_handling.hpp"
#include "compression.hpp"

const int MAX_MSG_SIZE = 65535;
const std::string ROOT_PATH = "/";
const std::string ECHO_PATH = "/echo/";
const std::string USER_AGENT_PATH = "/user-agent";
const std::string FILES_PATH = "/files/";

void respond_to_request(int id, int client_connection, const std::string& file_directory) {
    try {
        char msg[MAX_MSG_SIZE] = {};
        if (recv(client_connection, msg, sizeof(msg) - 1, 0) < 0) {
            throw std::runtime_error("Failed to receive the message for connection id: " + std::to_string(client_connection) + "!");
        }

        Http_request request = parse_message(std::string(msg));
        std::string response;

        if (request.http_method == "GET") {
            if (request.request_target == ROOT_PATH) {
                response = handle_root_request();
            } else if (request.request_target.substr(0, ECHO_PATH.length()) == ECHO_PATH) {
                response = handle_echo_request(request);
            } else if (request.request_target == USER_AGENT_PATH) {
                response = handle_user_agent_request(request);
            } else if (request.request_target.substr(0, FILES_PATH.length()) == FILES_PATH) {
                response = handle_file_request(file_directory, request, false);
            } else {
                response = create_response_message("HTTP/1.1", 404, "", "", 0, "");
            }
        } else if (request.http_method == "POST") {
            if (request.request_target.substr(0, FILES_PATH.length()) == FILES_PATH) {
                response = handle_file_request(file_directory, request, true);
            } else {
                response = create_response_message("HTTP/1.1", 404, "", "", 0, "");
            }
        } else {
            response = create_response_message("HTTP/1.1", 405, "", "", 0, "");
        }

        send_response(client_connection, response);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::string error_response = create_response_message("HTTP/1.1", 500, "", "", 0, "");
        send_response(client_connection, error_response);
    }
}

void send_response(int client_connection, const std::string& response) {
    if (send(client_connection, response.c_str(), response.length(), 0) < 0) {
        throw std::runtime_error("Failed to send response");
    }
}

std::string handle_root_request() {
    return create_response_message("HTTP/1.1", 200, "", "", 0, "");
}

std::string handle_echo_request(const Http_request& request) {
    std::string body = request.request_target.substr(ECHO_PATH.length());
    std::string content_type = "text/plain";
    std::string encoding;

    if (request.headers.find("accept-encoding") != request.headers.end()) {
        std::set<std::string> accepted_encodings = parse_encodings(request.headers.at("accept-encoding"));
        if (accepted_encodings.find("gzip") != accepted_encodings.end()) {
            body = compress(body);
            encoding = "gzip";
        }
    }

    return create_response_message("HTTP/1.1", 200, content_type, encoding, body.size(), body);
}

std::string handle_user_agent_request(const Http_request& request) {
    std::string body = request.headers.at("user-agent");
    return create_response_message("HTTP/1.1", 200, "text/plain", "", body.size(), body);
}

std::string handle_file_request(const std::string& file_directory, const Http_request& request, bool is_post = false) {
    std::string file_location = file_directory + request.request_target.substr(FILES_PATH.length());
    
    if (is_post) {
        // Handle POST request
        std::ofstream file(file_location, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to create file!");
        }
        file << request.body;
        return create_response_message("HTTP/1.1", 201, "", "", 0, "");
    } else {
        // Handle GET request
        std::ifstream file(file_location, std::ios::binary);
        if (!file) {
            return create_response_message("HTTP/1.1", 404, "", "", 0, "");
        }
        std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return create_response_message("HTTP/1.1", 200, "application/octet-stream", "", buffer.size(), std::string(buffer.begin(), buffer.end()));
    }
}

std::string create_response_message(std::string http_version, int status_code, std::string content_type, 
    std::string content_encoding, int content_length, std::string body) {
  std::string reason_phrase;
  switch (status_code) {
  case 200:
    reason_phrase = "OK";
    break;
  case 201:
    reason_phrase = "Created";
    break;
  case 404:
    reason_phrase = "Not Found";
    break;
  case 405:
    reason_phrase = "Method Not Allowed";
    break;
  case 500:
    reason_phrase = "Internal Server Error";
    break;
  default:
    reason_phrase = "Not Found";
    break;
  }

  std::string response_message = http_version + " " + std::to_string(status_code) + " " + reason_phrase;
  
  if (content_type != std::string()) {
    response_message += "\r\nContent-Type: " + content_type;

    if(content_encoding != std::string()) {
        response_message += "\r\nContent-Encoding: " + content_encoding;
    } 

    response_message +=  "\r\nContent-Length: " + std::to_string(content_length);
  }

  response_message += "\r\n\r\n";
  response_message += body;

  return response_message;
}