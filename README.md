[![progress-banner](https://backend.codecrafters.io/progress/http-server/354e5fe9-8306-40d1-a121-caf4ba5cced1)](https://app.codecrafters.io/users/codecrafters-bot?r=2qF)

This my C++ solution to the Codecrafters ["Build Your Own HTTP server" Challenge](https://app.codecrafters.io/courses/http-server/overview). I implemented a basic HTTP server from scratch and passed the tests provided by the challenge.
# Topics covered in this project
- [HTTP (Hypertext Transfer
Protocol)](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol)
- [HTTP request syntax](https://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html)
- Socket Programming
- Threads/Thread Pools
- Compression using `gzip`
- Modern C++ Types and Syntax
- CMake

# Overview
This project is a basic HTTP server implemented in C++, capable of handling standard HTTP requests such as GET and POST. It supports compression, multi-threading for concurrent request handling and provides error responses for unsupported methods and invalid paths.

## Features

1. **GET Requests:**

   - Serve the root directory (`/`).
   - Echo back a message (`/echo/{message}`).
   - Return the user-agent string (`/user-agent`).
   - Serve files from the `files` directory (`/files/{filename}`).

2. **POST Requests:**

   - Save files to the `files` directory (`/files/{filename}`).

3. **Multi-threaded Server**

   - The server handles multiple client connections concurrently using threads. Each incoming connection is handled in a separate thread, allowing the server to process multiple requests simultaneously.

4. **Error Handling:**
   - Responds with `404 Not Found` for invalid paths.
   - Responds with `405 Method Not Allowed` for unsupported HTTP methods.
   - Responds with `500 Internal Server Error` for caught exceptions.

5. **Compression:**
   - The server supports the `gzip` encoding scheme.

## How to Build and Run

1. **Clone the Repository**

   ```sh
   $ git clone https://github.com/Andrei32Ionescu/http-server-cpp.git
   $ cd http-server-cpp
   ```

2. **Build the Project** - Ensure you have `cmake` installed locally

   ```sh
   $ make
   ```

   This will compile the source files and produce an executable named `server`.

3. **Run the Server**

   ```sh
   $ ./server
   ```

   The server will start and listen for incoming connections on port `4221`.

## Usage

Once the server is running, you can test it using a web browser or tools like `curl`.

### GET Requests

- **Root Directory**

  ```sh
  $ curl http://localhost:4221/
  ```

- **Echo Message**

  ```sh
  $ curl http://localhost:4221/echo/hello
  ```

- **User-Agent**

  ```sh
  $ curl http://localhost:4221/user-agent
  ```

- **Serve File**

  ```sh
  $ echo -n 'Hello, World!' > /tmp/foo
  $ curl -i http://localhost:4221/files/foo
  ```

### POST Requests

- **Save File**

  ```sh
  $ curl -v --data "12345" -H "Content-Type: application/octet-stream" http://localhost:4221/files/file_123
  ```

## Error Handling

- **404 Not Found**

  Accessing a non-existent path:

  ```sh
  $ curl http://localhost:4221/nonexistent
  ```

- **405 Method Not Allowed**

  Using an unsupported HTTP method:

  ```sh
  $ curl -X PUT http://localhost:4221/
  ```


## Compression

Compressing using `gzip`:
```sh
$ curl -v -H "Accept-Encoding: gzip" http://localhost:4221/echo/abc | hexdump -C
```

## Multiple Concurrent Connections
The server's ability to handle multiple concurrent connections can be tested using the following commands:
```sh
$ (sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 4221 &
$ (sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 4221 &
$ (sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 4221 &
```
