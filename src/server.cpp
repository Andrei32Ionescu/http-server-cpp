#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <netdb.h>
#include <thread>
#include <vector>
#include "server_init.hpp"
#include "request_handling.hpp"

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::string file_directory;
  for (int i = 1; i < argc; i++) {  
    if (i + 1 != argc) {
      if (strcmp(argv[i], "--directory") == 0) {                
          file_directory = argv[i + 1];
          break;
      }
    }
  }

  std::pair<int, int> server_status = create_server(4221);
  if(server_status.first != 0) {
    std::cerr<< "Server creation failed!\n";
  }
  int server_fd = server_status.second;

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  std::cout << "Waiting for a client to connect...\n";

  std::vector<std::thread> threads;
  int i = 0;
  while(true){
    int client_connection = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    std::cout << "A client has connected - connection id: " + std::to_string(client_connection) + "\n";
    threads.emplace_back(respond_to_request, i, client_connection, file_directory);
    i++;
  }

  return 0;
}