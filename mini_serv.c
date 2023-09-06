#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <printf.h>

typedef struct s_client {
    int id;
    char msg[1024];
}   t_client;

t_client client[1024];
fd_set read_fd, write_fd, active_fd;
int fd_max = 0, next_id = 0;
char buf_read[120000], buf_write[120000];  

void ft_error (char *str) {
    if (str)
        write(2, str, strlen(str));
    else    
        write (2, "Fatal error\n", strlen("Fatal error\n"));
    exit (1);
}

void send_all (int nb) {
    for (int i = 0; i <= fd_max; i++) {
        if (FD_ISSET(i, &write_fd) && i != nb)
            send(i, buf_write, strlen(buf_write), 0);
    }
}

int main (int argc, char **argv) {
    if (argc != 2)
        ft_error("Wrong number of arguments\n");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0)
        ft_error(NULL);

    FD_ZERO(&active_fd);
    bzero(&client, sizeof(client));
    fd_max = sockfd;
    FD_SET(sockfd, &active_fd);
    
    struct sockaddr_in servaddr;
    socklen_t len;
    bzero(&servaddr, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(argv[1])); 

    if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
        ft_error(NULL);
    if (listen(sockfd, 10) < 0)
        ft_error(NULL);

    while (1) {
        read_fd = write_fd = active_fd;
        if (select(fd_max + 1, &read_fd, &write_fd, NULL, NULL) < 0)
            continue;
        for (int ifd = 0; ifd <= fd_max; ifd++) {
            if (FD_ISSET(ifd, &read_fd) && ifd == sockfd) {
                int connfd = accept(sockfd, (struct sockaddr *)&servaddr, &len);
                if (connfd < 0)
                    continue;
                fd_max = connfd > fd_max ? connfd : fd_max;
                client[connfd].id = next_id++;
                FD_SET(connfd, &active_fd);
                sprintf(buf_write, "server: client %d just arrived\n", client[connfd].id);
                send_all(connfd);
                break;
            }
            if (FD_ISSET(ifd, &read_fd) && ifd != sockfd) {
                int res = recv(ifd, buf_read, 65536, 0);
                if (res <= 0) {
                    sprintf(buf_write, "server: client %d just left\n", client[ifd].id);
                    send_all(ifd);
                    FD_CLR(ifd, &active_fd);
                    close(ifd);
                    break;
                }
                else {
                    for (int i = 0, j = strlen(client[ifd].msg); i < res; i++, j++) {
                        client[ifd].msg[j] = buf_read[i];
                        if (client[ifd].msg[j] == '\n') {
                            client[ifd].msg[j] = '\0';
                            sprintf(buf_write, "client %d: %s\n", client[ifd].id, client[ifd].msg);
                            send_all(ifd);
                            bzero(&client[ifd].msg, strlen(client[ifd].msg));
                            j = -1;
                        }
                    }
                    break;
                }
            }
        }
    }
    // return (0);
}