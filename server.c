#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>

int main() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in address = {
    .sin_family      = AF_INET,     // IPv4
    .sin_port        = htons(PORT),
    .sin_addr.s_addr = INADDR_ANY
  };

  bind(sockfd, (struct sockaddr*)&address, sizeof(address));

  listen(sockfd, 10);

  int clientfd = accept(sockfd, 0, 0);

  struct pollfd fds[2] = {
    {0, POLLIN, 0},      // stdin
    {clientfd, POLLIN, 0} // client socket
  };

  for (;;) {
    char buffer[256] = {0};

    poll(fds, 2, 50000);

    if (fds[0].revents & POLLIN) {
      read(0, buffer, 255);
      send(clientfd, buffer, 255, 0);
    } else if (fds[1].revents & POLLIN) {
      if (recv(clientfd, buffer, 255, 0) == 0) {
        return 0;
      }
      printf("received: %s\n", buffer);
    }
  }

  return 0;
}
