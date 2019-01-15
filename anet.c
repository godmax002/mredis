#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stddef.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "anet.h"

# define ANET_CONNECT_NONE 0
# define ANET_CONNECT_NONBLOCK 1


static void anetSetError(char *err, const char *fmt, ...){
    va_list ap;
    if(!err) return;
    va_start(ap, fmt);
    vsnprintf(err, ANET_ERR_LEN, fmt, ap);
    va_end(ap);
}

static int anetTcpGenericConnect(char *err, char *addr, int port, int flags){
    int s, on_opt = 1;

    if((s = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        anetSetError(err, "creating socket:%s\n", strerror(errno));
        return ANET_ERR;
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on_opt, sizeof(on_opt));

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = port;
    anetResolve(err, addr, &sa.sin_addr);

    if(flags & ANET_CONNECT_NONBLOCK){
        if(anetNonBlock(err, s)!= ANET_OK){
            return ANET_ERR;
        }
    }

    if(connect(s, (struct sockaddr*)&sa, sizeof(sa)) == -1){
        if(errno == EINPROGRESS && 
                flags & ANET_CONNECT_NONBLOCK)
            return s;
        
        anetSetError(err, "connect: %s\n", strerror(errno));
        close(s);
        return ANET_ERR;
    }

    return s;
}


int anetTcpConnect(char *err, char *addr, int port){
    return anetTcpGenericConnect(err, addr, port, ANET_CONNECT_NONE);
}

int anetTcpNonBlockConnect(char *err, char *addr, int port){
    return anetTcpGenericConnect(err, addr, port, ANET_CONNECT_NONBLOCK);
}

int anetRead(int fd, char *buf, int count){
    int tot_len, nread = 0 ;
    while(1){
        nread = read(fd, buf, count - tot_len);
        if(nread == -1) return -1;
        if(nread == 0) return tot_len;
        tot_len += nread;
        buf += nread;
    }
    return tot_len;
}

int anetWrite(int fd, void *buf, int count){
    int tot_len, nwrite = 0;
    while(1){
        nwrite = write(fd, buf, count - tot_len);
        if (nwrite == 0) return tot_len;
        if(nwrite == -1) return -1;
        tot_len += nwrite;
        buf += nwrite;
    }
    return tot_len;
}

int anetResolve(char *err, char *host, char *ipbuf){
    struct sockaddr_in sa;

    if(inet_aton(host, &sa.sin_addr) == 0){
        // by hostname
        struct hostent *he;
        he = gethostbyname(host);
        if(he == NULL){
            anetSetError(err, "can't resolve: %s\n", host);
            return ANET_ERR;
        }
        memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
    }
    strcpy(ipbuf, inet_ntoa(sa.sin_addr));
    return ANET_OK;
}

int anetTcpServer(char *err, int port, char *bindaddr){
    int s, on=1;
    struct sockaddr_in sa;

    if((s = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        anetSetError(err, "creating socket:%s\n", strerror(errno));
        return ANET_ERR;
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bindaddr){
        if(inet_aton(bindaddr, &sa.sin_addr) == 0){
            anetSetError(err, "Invalid bind address \n");
            close(s);
            return ANET_ERR;
        }
    }

    if(bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1){
        anetSetError(err, "bind:%s\n", strerror(errno));
        close(s);
        return ANET_ERR;
    }

    if(listen(s, 32) == -1){
        anetSetError(err, "listen:%s\n", strerror(errno));
        close(s);
        return ANET_ERR;
    }

    return s;
}

int anetAccept(char *err, int serversock, char *ip, int *port){
    struct sockaddr_in sa;
    int fd;
    unsigned int saLen;

    while(1){
        fd = accept(serversock, (struct sockaddr *)&sa, &saLen);
        if(fd == -1){
            if(errno == EINTR)
                continue;
            else{
            }
        }
    }
}



int anetNonBlock(char *err, int fd){
    int flags;
    if((flags = fcntl(fd, F_GETFL)) == -1){
        anetSetError(err, "fcnctl(F_GETFL): %s\n", strerror(errno));
        return ANET_ERR;
    }
    if((flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK)) == -1){
        anetSetError(err, "fcntl(F_SETFL): %s\n", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int anetTcpNoDelay(char *err, int fd){
    int yes = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1){
        anetSetError(err, "setsockopt TCP_NODELAY: %s\n", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int anetTcpKeepAlive(char *err, int fd){
    int yes = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) == -1){
        anetSetError(err, "sotsockopt SO_KEEPALIVE: %s\n", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}


    

