/*
//	@(#) proxycfg.h
//
//	License: Creative Commons CC0 
//		http://creativecommons.org/publicdomain/zero/1.0/legalcode
//	Author:	James Sainsbury
//		toves@sdf.lonestar.org
//
*/
# if	!defined(PROXYCFG_H)
# define	PROXYCFG_H

/* If we are building a shared library */
# if	!defined( SOLIB)
# define	SOLIB	
# endif

typedef	struct	cfg_kl	CFG_KL;
typedef      struct  sockaddr_in     SOCKADDR;
typedef	in_addr_t	h_addr_t;	

SOLIB	int	cfg_init (CFG_KL** cfp, char* cfgfile);
SOLIB	int	cfg_parameters (CFG_KL* cf, int type, __CONST_SOCKADDR_ARG sock, socklen_t len, int*, int*, int*, int*);
# endif
