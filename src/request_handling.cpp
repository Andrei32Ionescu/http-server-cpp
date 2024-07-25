#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <unordered_map>
#include <set>
#include "compression.hpp"

void respond_to_request(int id, int client_connection, int argc, char **argv) {
  char msg[65535] = {}; // TCP message max byte size
  if (recv(client_connection, msg, sizeof(msg) - 1, 0) < 0) {
    std::cerr << "Failed to receive the message for connection id: " + std::to_string(client_connection) + "!\n";
  }

  std::string encoded_text_message = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Encoding: gzip\r\nContent-Length: ";
  std::string get_ok_message = "HTTP/1.1 200 OK\r\n\r\n";
  std::string post_ok_message = "HTTP/1.1 201 Created\r\n\r\n";
  std::string bad_request_message = "HTTP/1.1 404 Not Found\r\n\r\n";
  std::string text_message = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";
  std::string file_message = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: ";

  std::string msg_str = (std::string) msg;
  std::string http_method = msg_str.substr(0, msg_str.find(' '));
  msg_str = msg_str.substr(msg_str.find(' ') + 1);
  std::string request_target = msg_str.substr(0, msg_str.find(' '));
  msg_str = msg_str.substr(msg_str.find(' ') + 1);
  std::string http_version = msg_str.substr(0, msg_str.find("\r\n"));
  msg_str = msg_str.substr(msg_str.find("\r\n") + 2);

  std::unordered_map<std::string, std::string> headers;
  while(msg_str.find("\r\n") != -1) {
    std::string header = msg_str.substr(0, msg_str.find("\r\n"));
    if(header.size() == 0) {
      msg_str = msg_str.substr(msg_str.find("\r\n") + 2);
      break;
    }
    std::string header_name = header.substr(0, msg_str.find(": "));
    std::string header_value = header.substr(msg_str.find(": ") + 2);
    for(int i = 0; i < header_name.size(); i++){
      header_name[i] = std::tolower(header_name[i]);
    }
    headers.insert({header_name, header_value});
    msg_str = msg_str.substr(msg_str.find("\r\n") + 2);
  }

  std::string body = msg_str;

  if(http_method == "GET") {
    if(request_target == "/") {
      send(client_connection, get_ok_message.c_str(), get_ok_message.length(), 0);
    } 
    else if(request_target.substr(0,6) == "/echo/") {
      if(headers.find("accept-encoding") == headers.end()) {
        std::string text = request_target.substr(6);
        std::string send_echo = text_message + std::to_string(text.size()) + "\r\n\r\n" + text;
        send(client_connection, send_echo.c_str(), send_echo.length(), 0);
      }
      else {
        std::string encoding_list = headers["accept-encoding"];
        std::set<std::string> accepted_encodings;
        while(encoding_list.find(',') != -1) {
          std::string encoding = encoding_list.substr(0, encoding_list.find(", "));
          accepted_encodings.insert(encoding);
          encoding_list = encoding_list.substr(encoding_list.find(", ") + 2);
        }
        if(encoding_list.size() > 0) {
          accepted_encodings.insert(encoding_list);
        }

        if(accepted_encodings.find("gzip") != accepted_encodings.end()) {
          std::string text = compress(request_target.substr(6));
          std::string send_echo = encoded_text_message + std::to_string(text.size()) + "\r\n\r\n" + text;
          send(client_connection, send_echo.c_str(), send_echo.length(), 0);
        }
        else {
          std::string text = request_target.substr(6);
          std::string send_echo = text_message + std::to_string(text.size()) + "\r\n\r\n" + text;
          send(client_connection, send_echo.c_str(), send_echo.length(), 0);
        }
      }
      
    } 
    else if(request_target == "/user-agent") {
      std::string text = headers["user-agent"];
      std::string send_user_agent = text_message + std::to_string(text.size()) + "\r\n\r\n" + text;
      send(client_connection, send_user_agent.c_str(), send_user_agent.length(), 0);  
    }
    else if(request_target.substr(0,7) == "/files/") {
      std::string directory;
      for (int i = 1; i < argc; i++) {  
        if (i + 1 != argc) {
          if (strcmp(argv[i], "--directory") == 0) {                
              directory = argv[i + 1];
              i++;
          }
        }
      }
      std::string file_location = directory + request_target.substr(7);
      std::ifstream file(file_location.c_str(), std::ios::binary);

      if (!file) {
        send(client_connection, bad_request_message.c_str(), bad_request_message.length(), 0);
      }
      else {
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0);
        std::vector<char> buffer(file_size);
        file.read(buffer.data(), file_size);
        std::string send_file = file_message + std::to_string(file_size) + "\r\n\r\n" + std::string(buffer.begin(), buffer.end());
        send(client_connection, send_file.c_str(), send_file.length(), 0);
      }
    }
    else {
      send(client_connection, bad_request_message.c_str(), bad_request_message.length(), 0);
    }
  }
  else if(http_method == "POST") {
    if(request_target.substr(0,7) == "/files/") {
      std::string directory;
      for (int i = 1; i < argc; i++) {  
        if (i + 1 != argc) {
          if (strcmp(argv[i], "--directory") == 0) {                
              directory = argv[i + 1];
              i++;
          }
        }
      }
      std::string file_location = directory + request_target.substr(7);
      std::ofstream file(file_location.c_str(), std::ios::binary);
      file<<body;
      send(client_connection, post_ok_message.c_str(), post_ok_message.length(), 0);

      
    }
  }
  return;
}