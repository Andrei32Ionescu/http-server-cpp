#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

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
  
  int client_connection = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  
  char msg[500] = {};

  if (recv(client_connection, msg, sizeof(msg)-1, 0) < 0){

    std::cerr << "listen failed\n";

    return 1;

  }

  std::string ok_message = "HTTP/1.1 200 OK\r\n\r\n";
  std::string bad_request_message = "HTTP/1.1 404 Not Found\r\n\r\n";

  for(int i = 0; i < sizeof(msg); i++){
    if(msg[i] == '/'){
      if(msg[i+1] == ' '){
        send(client_connection, ok_message.c_str(), ok_message.length(), 0);
      }
      else send(client_connection, bad_request_message.c_str(), bad_request_message.length(), 0);
      break;
    }
  }

  std::cout << "Client has connected\n";
  
  close(server_fd);

  return 0;
}
