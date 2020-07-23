#ifndef EVENT_HANDLE_H
#define EVENT_HANDLE_H

extern int sig_pipe_fd[2];
int set_nonblocking(int fd);
void add_fd(int epoll_fd,int fd);
void remove_fd(int epoll_fd,int fd);
void sig_handler(int sig);
void add_sig(int sig,void (*handler)(int),bool restart=true);

#endif