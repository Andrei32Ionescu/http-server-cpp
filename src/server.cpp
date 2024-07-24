#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unordered_map>
#include "server.h"
#include <thread>
#include <vector>
#include <fstream>
#include <set>
#include <sstream>
#include <zlib.h>

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }
  
  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }
  
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }
  
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  
  std::cout << "Waiting for a client to connect...\n";
  std::vector<std::thread> threads;
  int i = 0;
  while(true){
    int client_connection = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    std::cout << "A client has connected - connection id: " + std::to_string(client_connection) + "\n";
    threads.emplace_back(respond_to_request, i, client_connection, argc, argv);
    i++;
  }

  close(server_fd);

  return 0;
}
// TODO: refactor this method into multiple smaller ones 
void respond_to_request(int id, int client_connection, int argc, char **argv)
{
  char msg[65535] = {}; // TCP message max length

  if (recv(client_connection, msg, sizeof(msg) - 1, 0) < 0) {

    std::cerr << "listen failed\n";

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

std::string compress(const std::string &data) {

    z_stream zs;

    memset(&zs, 0, sizeof(zs));

    if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {

        throw std::runtime_error("deflateInit2 failed while compressing.");

    }

    zs.next_in = (Bytef *)data.data();

    zs.avail_in = data.size();

    int ret;

    char outbuffer[32768];

    std::string outstring;

    do {

        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);

        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {

            outstring.append(outbuffer, zs.total_out - outstring.size());

        }

    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {

        throw std::runtime_error("Exception during zlib compression: (" + std::to_string(ret) + ") " + zs.msg);

    }

    return outstring;

}