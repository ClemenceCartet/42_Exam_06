#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

typedef struct	t_message {
	char				*content;
	int					offset;
	struct t_message	*next;
}				message;

typedef struct	t_client {
	int					fd;
	int					id;
	char				buffer[200000];
	struct t_message	*msg;
	struct t_client		*next;
}				client;

void	clear_client(client *cli) {

	close(cli->fd);
	cli->fd = -1;
	if (cli->msg) {
		message	*tmp = NULL;

		while (cli->msg) {
			tmp = cli->msg;
			cli->msg = cli->msg->next;
			free(tmp->content);
			tmp->content = NULL;
			tmp->next = NULL;
			free(tmp);
		}
	}
	cli->next = NULL;
	free(cli);
}

void	error_exit(char *msg, client *cli, int fd) {

	if (fd > 0)
		close(fd);
	if (cli) {
		client	*tmp = NULL;

		while (cli) {
			tmp = cli;
			cli = cli->next;
			clear_client(tmp);
		}
	}
	write(2, msg, strlen(msg));
	exit(1);
}

int	set_msg(char *msg, client *clients, int fd) {

	client	*cli = clients;
	while (cli) {

		if (cli->fd != fd) {
			
			message	*newMsg = malloc(sizeof(message));
			if (!newMsg)
				return 1;
			newMsg->content = malloc(sizeof(char) * (strlen(msg) + 1));
			if (!newMsg->content) {
				free(newMsg);
				return 1;
			}
			strcpy(newMsg->content, msg);
			newMsg->content[strlen(msg)] = '\0';
			newMsg->offset = 0;
			newMsg->next = NULL;

			if (!cli->msg)
				cli->msg = newMsg;
			else {
				message	*list = cli->msg;
				while (list->next)
					list = list->next;
				list->next = newMsg;
			}
		}
		cli = cli->next;
	}
	return (0);
}

int main(int argc, char **argv) {

	int socketServ = 0;

	if (argv != 2) {
		error_exit("Wrong number of arguments\n", NULL, 0);
	}

	socketServ = socket(AF_INET, SOCK_STREAM, 0); 
	if (socketServ == -1)
		error_exit("Fatal error\n", NULL, 0);

	struct sockaddr_in servaddr, cliaddr; 
	bzero(&servaddr, sizeof(servaddr));
	bzero(&cliaddr, sizeof(cliaddr));

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(argv[1])); 
  
	if ((bind(socketServ, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		error_exit("Fatal error\n", NULL, socketServ);

	if (listen(socketServ, 4096) != 0)
		error_exit("Fatal error\n", NULL, socketServ);

	int len = sizeof(cliaddr);
	int ret = 0;
	int max = socketServ;
	int nbClients = 0;
	fd_set allfds;
	fd_set rfds;
	fd_set wfds;
	FD_ZERO(&allfds);
	FD_SET(socketServ, &allfds);
	client *clients = NULL;

	while (1) {

		rfds = wfds = allfds;

		if (select(max + 1, &rffds, &wfds, NULL, 0) < 0) 
			continue;

		if (FD_ISSET(socketServ, &rfds)) {

			int newFd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
			if (newFd < 0)
				error_exit("Fatal error\n", clients, socketServ);	
			client *newCli = malloc(sizeof(client));
			if (!newCli)
				error_exit("Fatal error\n", clients, socketServ);
			newCli->fd = newFd;
			FD_SET(newFd, &allfds);
			newCli->id = nbClients;
			nbClients++;
			memset(cli->buffer, 0, sizeof(cli->buffer));
			newCli->msg = NULL;
			newCli->next = NULL;

			if (!clients) 
				clients = newCli;
			else {
				client *list = clients;
				while (list->next)
					list = list->next;
				list->next = newCli;
			}

			if (max < newFd)
				maw = newFd;
			
			char buf[33];
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "server: client %d just arrived\n", newCli->id);
			if (set_msg(buf, clients, newFd))
				error_exit("Fatal error\n", clients, socketServ);
		}

		client *prev = NULL;
		client *cli = clients;
		while (cli) {

			if (FD_ISSET(cli->fd, &rfds)) {

				ret = recv(cli->fd, cli->buffer + strlen(cli->buffer), 1, 0);
				if (ret <=0) {
					char buf[30];
					memset(buf, 0, sizeof(buf));
					sprintf(buf, "server: client %d just left\n", cli->id);
					if (set_msg(buf, clients, cli->fd))
						error_exit("Fatal error\n", clients, socketServ);
					FD_CLR(cli->fd, &allfds);
					client *next = cli->next;
					clear_client(cli);
					if (prev) {
						prev->next = next;
						cli = prev->next;
					}
					else {
						cli = NULL;
						clients = next;
					}
					continue;
				}
				else if (cli->buffer[strlen(cli->buffer) - 1] == '\n') {
					char buf[200020];
					memset(buf, 0, sizeof(buf));
					sprintf(buf, "client %d: %s", cli->id, cli->buffer);
					memset(cli->buffer, 0, sizeof(char) * 200000);
					if (set_msg(buf, clients, cli->fd))
						error_exit("Fatal error\n", clients, socketServ);
				}
			}

			if (FD_ISSET(cli->fd, &wfds) && cli->msg) {
				message	*tmp = cli_>msg;
				int blop = send(cli->fd, tmp->content + tmp->offset, strlen(tmp->content) - tmp->offset, 0);
				if (blop > 0) {
					if (tmp->offset + blop < (int)strlen(tmp->content))
						tmp->offset += blop;
					else {
						message *next = tmp->next;
						free(tmp->content);
						tmp->content = NULL;
						tmp->next = NULL;
						free(tmp);
						cli->msg = next;
					}
				}
			}

			prev = cli;
			cli = cli->next;
		}


	}
	return (0);

}