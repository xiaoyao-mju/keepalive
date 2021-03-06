/*
//	@(#) libkeepalive.c
//
//	License: Creative Commons CC0 
//		http://creativecommons.org/publicdomain/zero/1.0/legalcode
//	Author:	James Sainsbury
//		toves@sdf.lonestar.org
//
*/
# define	_GNU_SOURCE
# include	<dlfcn.h>

# include	<errno.h>
# include	<stdlib.h>
# include	<strings.h>
# include	<unistd.h>
# include	<sys/types.h>
# include	<sys/socket.h>
# include	<netinet/in.h>
# include	<netinet/tcp.h>
# include       <arpa/inet.h>

# include	"constants.h"
# include	"keepalivecfg.h"

static	CFG_KL*	cf		= 0;
static	int	cf_status	= ok;

typedef	int	(*CONNECT_FN)(int, __CONST_SOCKADDR_ARG, socklen_t);
typedef	int	(*ACCEPT_FN)(int, __SOCKADDR_ARG, socklen_t*);

static	CONNECT_FN	libc_connect	= 0;
static	ACCEPT_FN	libc_accept	= 0;

void __attribute__((constructor, visibility("hidden")))	keepalive_init(void) {
	libc_connect	= dlsym (RTLD_NEXT, "connect");
	if (dlerror()) {
		_exit(EXIT_FAILURE);
	}
	libc_accept	= dlsym (RTLD_NEXT, "accept");
	if (dlerror()) {
		_exit(EXIT_FAILURE);
	}
	cf_status	= cfg_init (&cf, LIBKEEPALIVE_CFG);
}

static	int	set_keepalive (int sd,
			 int keepidle, int keepintvl, int keepcnt){
	int	val	= true;
	int	rv	= setsockopt (sd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
# if	defined( TCP_KEEPCNT)
	val	= keepcnt;
	setsockopt (sd, SOL_TCP, TCP_KEEPCNT, &val, sizeof(val));
#endif
#ifdef TCP_KEEPIDLE
	val	= keepidle;
       	setsockopt (sd, SOL_TCP, TCP_KEEPIDLE, &val, sizeof(val));
#endif
#ifdef TCP_KEEPINTVL
	val	= keepintvl;
       	setsockopt (sd, SOL_TCP, TCP_KEEPINTVL, &val, sizeof(val));
#endif
}

static	int	socket_type (int sd) {
	int	stype	= 0;
	socklen_t	stlen   = sizeof(stype);
	int	result  = err;
	if (getsockopt(sd, SOL_SOCKET, SO_TYPE, &stype, &stlen) == 0)
		result  = stype;
	return  result;
}

static	int	socket_family(__CONST_SOCKADDR_ARG sock) {
	struct	sockaddr*	s	= *(struct sockaddr**)&sock;
	return	s->sa_family;
}

int	__attribute__((visibility("default"))) connect (int sd,  __CONST_SOCKADDR_ARG sock, socklen_t len) {
	int	stype	= socket_type (sd);
	int	family	= socket_family (sock);
	int	result	= libc_connect (sd, sock, len);
	int	found	= false;
	int	so_keepalive	= 0;
	int	tcp_idle	= 0;
	int	tcp_intvl	= 0;
	int	tcp_cnt		= 0;
	if (result == ok && family == AF_INET && stype == SOCK_STREAM) {
		if (cf_status==ok) 
			found	= cfg_parameters (cf, sd, 'C', sock, len, &so_keepalive, &tcp_idle, &tcp_intvl, &tcp_cnt );
		if (found && so_keepalive)
			set_keepalive (sd, tcp_idle, tcp_intvl, tcp_cnt);
	}
	return	result;
}

int	__attribute__((visibility("default"))) 	accept (int sd,  __SOCKADDR_ARG sock, socklen_t* lenp) {
/*
//	We cast this to a constant structure once to save the same gymnastics
//	in two places. Since the sockaddr became a transparent union the
//	compiler chokes otherwise.
*/
	__CONST_SOCKADDR_ARG	csock	= *(__CONST_SOCKADDR_ARG*)(&sock);
	int	stype	= socket_type (sd);
	int	family	= socket_family (csock);
	int	result	= libc_accept (sd, sock, lenp);
	int	found	= false;
	int	so_keepalive	= 0;
	int	tcp_idle	= 0;
	int	tcp_intvl	= 0;
	int	tcp_cnt		= 0;
	if (result != err && family == AF_INET && stype == SOCK_STREAM) {
		if (cf_status==ok) 
			found	= cfg_parameters (cf, sd, 'A', csock, *lenp,
				&so_keepalive, &tcp_idle, &tcp_intvl, &tcp_cnt );
		if (found && so_keepalive)
			set_keepalive (result, tcp_idle, tcp_intvl, tcp_cnt);
	}
	return	result;
}
