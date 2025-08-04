#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
  char client_name[32];

  if (argc > 1) {
    strncpy(client_name, argv[1], sizeof(client_name) - 1);
    client_name[sizeof(client_name) - 1] = '\0';
  } else {
    printf("Enter your name: ");
    fflush(stdout);
    if (fgets(client_name, sizeof(client_name), stdin) == NULL) {
      return 1;
    }
    // removing new line
    char *newline = strchr(client_name, '\n');
    if (newline) {
      *newline = '\0';
    }
  }

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("Error creating socket");
    return 1;
  }

  struct sockaddr_in address = {
    .sin_family      = AF_INET,  // IPv4
    .sin_port        = htons(PORT),
    .sin_addr.s_addr = INADDR_ANY
  };

  if (connect(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    perror("Error connecting socket");
    return 1;
  }

  struct pollfd fds[2] = {
    {0, POLLIN, 0},    // stdin
    {sockfd, POLLIN, 0} // server socket
  };

  for (;;) {
    char buffer[256] = {0};

    poll(fds, 2, 50000);

    if (fds[0].revents & POLLIN) {
      read(0, buffer, 255);
      send(sockfd, buffer, 255, 0);
    } else if (fds[1].revents & POLLIN) {
      if (recv(sockfd, buffer, 255, 0) == 0) {
        return 0;
      }
      printf("received: %s\n", buffer);
    }
  }

  return 0;
}
