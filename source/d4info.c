/* d4info.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifdef S4CLIENT
/* must call u4free() on returned pointer */
char *S4FUNCTION code4serverConfigName( CODE4 *c4 )
{
   CONNECTION4 *connection ;
   int rc, len ;
   char *name, *ptr ;

   #ifdef E4PARM_LOW
      if ( c4 == 0 )
      {
         error4( 0, e4parm_null, E96101 ) ;
         return 0 ;
      }
   #endif

   if ( !c4->defaultServer.connected )
   {
      error4( c4, e4connection, E84302 ) ;
      return 0 ;
   }

   #ifdef E4ANALYZE
      if ( c4->defaultServer.connect == 0 )
      {
         error4( c4, e4parm, E96101 ) ;
         return 0 ;
      }
   #endif

   connection = &c4->defaultServer ;
   connection4assign( connection, CON4CONFIG_NAME, 0L, 0L ) ;
   connection4sendMessage( connection ) ;
   rc = connection4receiveMessage( connection ) ;
   if ( rc < 0 )
      return 0 ;

   rc = connection4status( connection ) ;
   if ( rc < 0 )
   {
      connection4error( connection, c4, rc, E96101 ) ;
      return 0 ;
   }

   ptr = (char *)connection4data( connection ) ;
   len = strlen( ptr ) ;

   #ifdef E4ANALYZE
      if ( len > LEN4PATH + 1 || len < 0 )
      {
         error4( c4, e4struct, E96101 ) ;
         return 0 ;
      }
   #endif

   name = (char *)u4alloc( len + 1 ) ;
   memcpy( name, ptr, len+1 ) ;

   return name ;
}

int S4FUNCTION code4infoRetrieve( CODE4 *c4, S4LONG *memAlloc, unsigned short *nClients, S4LONG *elapsedSeconds, int *nOpenFiles )
{
   CONNECTION4 *connection ;
   int rc ;
   CONNECTION4SERVER_INFO_OUT *out ;

   #ifdef E4PARM_LOW
      if ( c4 == 0 || memAlloc == 0 || nClients == 0 || elapsedSeconds == 0 || nOpenFiles == 0 )
         return error4( 0, e4parm_null, E96101 ) ;
   #endif

   if ( !c4->defaultServer.connected )
      return error4( c4, e4connection, E84302 ) ;

   #ifdef E4ANALYZE
      if ( c4->defaultServer.connect == 0 )
         return error4( c4, e4parm, E96101 ) ;
   #endif

   connection = &c4->defaultServer ;
   connection4assign( connection, CON4INFO, 0L, 0L ) ;
   connection4sendMessage( connection ) ;
   rc = connection4receiveMessage( connection ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E96101 ) ;

   rc = connection4status( connection ) ;
   if ( rc < 0 )
      return connection4error( connection, c4, rc, E96101 ) ;

   if ( connection4len( connection ) < sizeof( CONNECTION4SERVER_INFO_OUT ) )
      return error4( c4, e4packetLen, E96101 ) ;

   out = (CONNECTION4SERVER_INFO_OUT *)connection4data( connection ) ;

   *memAlloc = htonl( out->memAlloc ) ;
   *nClients = htons( out->numClients ) ;
   *elapsedSeconds = htonl( out->elapsedSeconds ) ;
   *nOpenFiles = htons( out->nOpenFiles ) ;

   return 0 ;
}

int S4FUNCTION code4info( CODE4 *c4 )
{
   CONNECTION4 *connection ;
   int rc ;
   #ifdef S4CONSOLE
      CONNECTION4SERVER_INFO_OUT *out ;
      CONNECTION4CLIENT_INFO *client ;
      unsigned short int i, numClients ;
   #endif

   #ifdef E4PARM_LOW
      if ( c4 == 0 )
         return error4( 0, e4parm_null, E96101 ) ;
   #endif

   if ( !c4->defaultServer.connected )
      return error4( c4, e4connection, E84302 ) ;

   #ifdef E4ANALYZE
      if ( c4->defaultServer.connect == 0 )
         return error4( c4, e4parm, E96101 ) ;
   #endif

   connection = &c4->defaultServer ;
   connection4assign( connection, CON4INFO, 0L, 0L ) ;
   connection4sendMessage( connection ) ;
   rc = connection4receiveMessage( connection ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E96101 ) ;

   rc = connection4status( connection ) ;
   if ( rc < 0 )
      return connection4error( connection, c4, rc, E96101 ) ;

   if ( connection4len( connection ) < sizeof( CONNECTION4SERVER_INFO_OUT ) )
      return error4( c4, e4packetLen, E96101 ) ;

   #ifdef S4CONSOLE
      out = (CONNECTION4SERVER_INFO_OUT *)connection4data( connection ) ;

      printf( "\nSERVER STATS\n------------\n" ) ;

      if ( ntohl(out->memMax) == -1 )
         printf( "memMax: unknown\n" ) ;
      else
         printf( "memMax: %ld\n", ntohl(out->memMax) ) ;

      if ( ntohl(out->memAlloc) == -1 )
         printf( "memAlloc: unknown\n" ) ;
      else
         printf( "memAlloc: %ld\n", ntohl(out->memAlloc) ) ;
      printf( "numRequests: %ld\n", ntohl(out->numRequests) ) ;
      numClients = ntohs(out->numClients) ;
      printf( "numClients: %d\n", numClients ) ;
      printf( "elapsedSeconds: %ld\n", (long)ntohl(out->elapsedSeconds) ) ;

      if ( connection4len( connection ) != (long)(sizeof( CONNECTION4SERVER_INFO_OUT ) + (long)(numClients * sizeof( CONNECTION4CLIENT_INFO )) ) )
         return error4( c4, e4packetLen, E96101 ) ;

      printf( "\nCLIENT STATS\n------------\n" ) ;

      for ( i = 0 ; i < numClients ; i++ )
      {
         client = (CONNECTION4CLIENT_INFO *)(((char *)out) + sizeof( CONNECTION4SERVER_INFO_OUT ) + i * sizeof( CONNECTION4CLIENT_INFO ) ) ;

      /* printf( Client Name... */
         printf( "numData: %d\n", ntohs(client->numData) ) ;
         printf( "numRelate: %d\n", ntohs(client->numRelate) ) ;
         printf( "numRequests: %ld\n", ntohl(client->numRequests) ) ;
         printf( "numTransactions: %d\n", ntohl(client->numTransactions) ) ;
         printf( "numCompletedTransactions: %d\n", ntohl(client->numCompletedTransactions) ) ;
         printf( "numRollbacks: %d\n", ntohl(client->numRollbacks) ) ;
         printf( "elapsedSeconds: %ld\n", (long)ntohl(client->elapsedSeconds) ) ;
         printf( "activeTransaction: %d\n", ntohs(client->activeTransaction) ) ;
      }

      printf( "-----------------------------------\n\n" ) ;
   #endif

   return 0 ;
}
#endif
