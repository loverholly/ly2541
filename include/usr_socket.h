#ifndef __USER_SOCKET_H__
#define __USER_SOCKET_H__

#ifdef __cplusplus
extern "C"
{
#endif

int usr_create_socket(int port);

int usr_accept_socket(int listen_st);

int usr_send_to_socket(int accept_fd, char *buf, int size);

int usr_recv_from_socket(int accept_fd, char *buf, int size);

int usr_close_socket(int fd);

#ifdef __cplusplus
}
#endif

#endif	/* __USER_SOCKET_H__ */
