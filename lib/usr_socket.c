#include "common.h"
#include "usr_socket.h"

int usr_create_socket(int port)
{
	int listen_fd;
	struct sockaddr_in sockaddr;
	int on = 1;
	int keepalive = 1;
	int keepidle = 10;     /* if NO data interaction within 30s, start detect */
	int keepcount = 3;     /* detect at most 3 times */
	int keepinterval = 2;  /* set detect interval as 2s */

	listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (listen_fd == -1) {
		printf("%s create socket failed!\n", __func__);
		return -1;
	}

	setsockopt(listen_fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive));
	setsockopt(listen_fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle , sizeof(keepidle));
	setsockopt(listen_fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval , sizeof(keepinterval));
	setsockopt(listen_fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount , sizeof(keepcount));

	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
		printf("%s setsockopt error:%s\n", __func__, strerror(errno));
		return -1;
	}

	sockaddr.sin_port = htons(port);
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listen_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1) {
		printf("%s bind error:%s\n", __func__, strerror(errno));
		return -1;
	}

	if (listen(listen_fd, 5) == -1) {
		printf("%s listen error:%s\n", __func__, strerror(errno));
		return -1;
	}

	return listen_fd;
}

int usr_accept_socket(int listen_fd)
{
	int accept_fd;
	struct sockaddr_in accept_sockaddr;
	socklen_t addrlen = sizeof(accept_sockaddr);
	if (listen_fd == -1) {
		printf("%s accept listen is invalid\n", __func__);
		return -1;
	}

	memset((void *)&accept_sockaddr, 0, addrlen);
	accept_fd = accept(listen_fd, (struct sockaddr *)&accept_sockaddr, &addrlen);
	if (accept_fd == -1) {
		printf("%s accept socket failed! %s\n", __func__, strerror(errno));
		return -1;
	}

	printf("accept ip:%s\n", inet_ntoa(accept_sockaddr.sin_addr));
	return  accept_fd;
}

int usr_send_to_socket(int accept_fd, char *buf, int size)
{
	if (accept_fd == -1)
		return -1;

	return send(accept_fd, buf, size, MSG_NOSIGNAL);
}

int usr_recv_from_socket(int accept_fd, char *buf, int size)
{
	if (accept_fd == -1)
		return -1;

	return recv(accept_fd, buf, size, MSG_NOSIGNAL);
}

int usr_close_socket(int fd)
{
	if (fd == -1)
		return -1;

	close(fd);
	return 0;
}
