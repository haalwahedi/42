#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

// try 20000000 or 30000000
#define BUF_SIZE 10000000

int clients = 0, clientId[BUF_SIZE], max_fds = 0, error_flag = 0, readlen = 0;
char *msg = NULL, buf[BUF_SIZE];
fd_set  fds, rfds, wfds;

int sockfd, connfd;
socklen_t len;
struct sockaddr_in servaddr, cli;

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

void  ft_error() {
  char str[50] = "Wrong number of arguments\n";
  if (!error_flag)
    write(2, str, strlen(str));
  else {
    strcpy(str, "Fatal error\n");
    write(2, str, strlen(str));
  }
	exit(1);
}

void  logger(int senderFd, int flag, char **msg) {
  	if (!flag)
		sprintf(buf, "server: client %d just arrived\n", clientId[senderFd]);
	else if (flag == 1)
		sprintf(buf, "server: client %d just left\n", clientId[senderFd]);
	else
		sprintf(buf, "client %d: %s", clientId[senderFd], *msg);
  
  int fd = 0;
  while (fd <= max_fds) {
    if (FD_ISSET(fd, &wfds) && fd != senderFd)
      send(fd, buf, strlen(buf), 0);
    fd++;
  }
  memset(buf, 0, BUF_SIZE);
}

void  setup(int ac, char **av) {
  if (ac != 2)
    ft_error();
  error_flag = 1;

  // socket create and verification 
  FD_ZERO(&fds);
  sockfd = max_fds = socket(AF_INET, SOCK_STREAM, 0);
	if (max_fds == -1)
    ft_error();
  FD_SET(max_fds, &fds);

	bzero(&servaddr, sizeof(servaddr)); 

  int port = atoi(av[1]);
	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(port);
  
	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)))
    || listen(sockfd, 10))
      ft_error();
}

void  addClient(int fd) {
  if (max_fds < fd)
    max_fds = fd;
  clientId[fd] = clients++;
  FD_SET(fd, &fds);
  logger(fd, 0, NULL);
}

void  rmClient(int fd) {
  logger(fd, 1, NULL);
  free (msg);
  msg = NULL;
  FD_CLR(fd, &fds);
  close(fd);
}

void  send_all(int fd) {
  // buf[readlen] = '\0';
  msg = str_join(msg, buf);
  char *str;
  while (extract_message(&msg, &str)) {
    logger(fd, 2, &str);
    free(str);
    str = NULL;
  }
  free(str);
}

int  handleCommunication(int fd) {
  if (fd == sockfd) {
    connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
    if (connfd >= 0)
      return (addClient(connfd), 1);
  }
  else {
    readlen = recv(fd, buf, BUF_SIZE, 0);
    if (readlen <= 0)
      return (rmClient(fd), 1);
    else
      send_all(fd);
  }
  return 0;
}

void  run() {
  len = sizeof(cli);
  while (1) {
    rfds = fds;
    wfds = fds;
    if (select(max_fds + 1, &rfds, &wfds, NULL, NULL) < 0)
      ft_error();
    int fd = 0;
    while (fd <= max_fds) {
      if (FD_ISSET(fd, &rfds))
        if (handleCommunication(fd))
          break ;
      fd++;
    }
  }
}

int main(int ac, char **av) {
  setup(ac, av);
	run();
}