/* net.c -- 
 * Created: Fri Feb 21 20:58:10 1997 by faith@cs.unc.edu
 * Revised: Mon Mar 10 10:51:31 1997 by faith@cs.unc.edu
 * Copyright 1997 Rickard E. Faith (faith@cs.unc.edu)
 * This program comes with ABSOLUTELY NO WARRANTY.
 * 
 * $Id: net.c,v 1.4 1997/03/10 21:46:59 faith Exp $
 * 
 */


#include "dictd.h"
#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

static char netHostname[MAXHOSTNAMELEN];

const char *net_hostname( void )
{
   if (!netHostname[0]) {
      memset( netHostname, 0, sizeof(netHostname) );
      gethostname( netHostname, sizeof(netHostname)-1 );
   }
   return netHostname;
}

int net_open_tcp( const char *service, int queueLength )
{
   struct servent     *serviceEntry;
   struct protoent    *protocolEntry;
   struct sockaddr_in ssin;
   int                s;

   memset( &ssin, 0, sizeof(ssin) );
   ssin.sin_family      = AF_INET;
   ssin.sin_addr.s_addr = INADDR_ANY;

   if ((serviceEntry = getservbyname(service, "tcp"))) {
      ssin.sin_port = serviceEntry->s_port;
   } else if (!(ssin.sin_port = htons(atoi(service))))
      err_fatal( __FUNCTION__, "Can't get \"%s\" service entry\n", service );

   if (!(protocolEntry = getprotobyname("tcp")))
      err_fatal( __FUNCTION__, "Can't get \"tcp\" service entry\n" );
   
   if ((s = socket(PF_INET, SOCK_STREAM, protocolEntry->p_proto)) < 0)
      err_fatal_errno( __FUNCTION__, "Can't open socket on port %d\n",
		       ntohs(ssin.sin_port) );

   if (bind(s, (struct sockaddr *)&ssin, sizeof(ssin)) < 0)
      err_fatal_errno( __FUNCTION__, "Can't bind %s/tcp to port %d\n",
		       service, ntohs(ssin.sin_port) );

   if (listen( s, queueLength ) < 0)
      err_fatal_errno( __FUNCTION__, "Can't listen to %s/tcp on port %d\n",
		       service, ntohs(ssin.sin_port) );
   
   return s;
}
