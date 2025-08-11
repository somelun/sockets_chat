#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

#define MAX_CLIENTS 10
#define NAME_SIZE 25

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("Error creating socket");
    return 1;
  }

  struct sockaddr_in address = {
    .sin_family      = AF_INET,     // IPv4
    .sin_port        = htons(PORT),
    .sin_addr.s_addr = INADDR_ANY
  };

  // allowing port reuse
  int opt = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (bind(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    perror("Error binding socket");
    return 1;
  }

  if (listen(sockfd, MAX_CLIENTS) < 0) {
    perror("Error listening");
    return 1;
  }

  // array of pollfd structures: [0] = server socket, [1+] = clients
  struct pollfd fds[MAX_CLIENTS + 1];
  fds[0].fd = sockfd;      // server socket
  fds[0].events = POLLIN;

  char client_names[MAX_CLIENTS][NAME_SIZE];

  for (int i = 1; i < MAX_CLIENTS + 1; ++i) {
    fds[i].fd = -1; // -1 means slot is empty
  }

  // empty client names
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    client_names[i][0] = '\0';
  }

  // how many file descriptors do we have, 1 means we only have one from the server
  int nfds = 1;

  for (;;) {
    if (poll(fds, nfds, 50000) < 0) {
      perror("Error polling");
      break;
    }

    // checking if we have new connections
    if (fds[1].revents & POLLIN) {
      int clientfd = accept(sockfd, NULL, NULL);
      if (clientfd >= 0) {

        // we find empty stop for the client, I know this can ne optimizied,
        // but this is for TODO
        int slot = -1;
        for (int i = 1; i < MAX_CLIENTS + 1; ++i) {
          if (fds[i].fd == -1) {
            slot = i;
            break;
          }
        }

        if (slot != -1) {
          fds[slot].fd = clientfd;
          fds[slot].events = POLLIN;

          if (slot >= nfds) {
            nfds = slot + 1;
          }

          // receiving client name
          int name_len = recv(clientfd, client_names[slot - 1], NAME_SIZE - 1, 0);
          if (name_len > 0) {
            client_names[slot - 1][name_len] = '\0';
            printf("[SERVER] %s joined the chat\n", client_names[slot - 1]);

            // notify other clients
            char join_msg[100];
            snprintf(join_msg, sizeof(join_msg), "%s joined the chat\n", client_names[slot - 1]);
            for (int i = 1; i < MAX_CLIENTS + 1; i++) {
              if (i != slot && fds[i].fd != -1) {
                send(fds[i].fd, join_msg, strlen(join_msg), 0);
              }
            }
          } else {
            // if we failed to get name we close the connection
            close(clientfd);
            fds[slot].fd = -1;
          }
        } else {
          printf("Too many clients, sorry\n");
          close(clientfd);
        }
      }
    }

    char buffer[256] = {0};

    // cheking messages from the clients here
    for (int i = 2; i < nfds; ++i) {
      if (fds[i].fd != -1 && (fds[i].revents & POLLIN)) {
        int len = recv(fds[i].fd, buffer, 255, 0);
        if (len <= 0) {
          //
        } else {
          buffer[len] = '\0';
          // remove newline if present
          char *newline = strchr(buffer, '\n');
          if (newline) {
            *newline = '\0';
          }

          // print message locally
          printf("%s: %s\n", client_names[i - 2], buffer);

          // format message with client name and send to all other clients
          char formatted_msg[350];
          snprintf(formatted_msg, sizeof(formatted_msg), "%s: %s\n", client_names[i - 2], buffer);

          for (int j = 2; j < MAX_CLIENTS + 2; j++) {
            if (j != i && fds[j].fd != -1) {
              send(fds[j].fd, formatted_msg, strlen(formatted_msg), 0);
            }
          }
        }
      }
    }
  }

  return 0;
}
