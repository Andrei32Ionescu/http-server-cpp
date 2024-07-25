#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "server_init.hpp"

std::pair<int, int> create_server(int port_number) {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket!\n";
   return {1, server_fd};
  }
  
  // Ensures that 'Address already in use' errors are not encountered during testing
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "Failed to modify the socket reuse option!\n";
    return {1, server_fd};
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port_number);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port "<<port_number<<"!\n";
    return {1, server_fd};
  }
  
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "Listen for a connections failed!\n";
    return {1, server_fd};
  }

  return {0, server_fd};
}