/* *********************************************************************************************** */
/* Copyright (C) 1999-2015 by Sequiter, Inc., 9644-54 Ave, NW, Suite 209, Edmonton, Alberta Canada.*/
/* This program is free software: you can redistribute it and/or modify it under the terms of      */
/* the GNU Lesser General Public License as published by the Free Software Foundation, version     */
/* 3 of the License.                                                                               */
/*                                                                                                 */
/* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;       */
/* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.       */
/* See the GNU Lesser General Public License for more details.                                     */
/*                                                                                                 */
/* You should have received a copy of the GNU Lesser General Public License along with this        */
/* program. If not, see <https://www.gnu.org/licenses/>.                                           */
/* *********************************************************************************************** */

/* revisited by altomaltes@gmail.com
 */

#include "d4all.h"

#ifndef S4STAND_ALONE

/* AS 03/19/98
   Change to design and implementation.
   CODE4.errorCode does not cause communications failures any more.
   the CONNECT4BUFFER contains a member communicationError which causes
   the communications to fail if < 0.

   The reason is that it is legal for the server to send communications to
   the client when the CODE4 is in an error state.  Usually this means a
   client error, and not a real problem.

   When we get a failure from the low level communications, both the code4
   and the communicationError are set to error states.

   The exception to this rule are the various set-up connections functions,
   which can not set up connections when in an error-state.
*/

#ifndef S4OFF_BLAST
#ifdef S4CLIENT
CONNECT4BUFFER *connect4bufferAuxConnection( CONNECT4 *connect, int numAdvanceReadBuffers, int bufSize, int maxOutstandingWrites )
{
/* DESCRIPTION

   THis function creates a new connection between an already existing client/
     server pair. This connection can then be used for blast functions.

   PARAMATERS

   connect is a CONNECT4 that already has a write connection
   The remaining parameters are a the qualifications neccesary for
     this connection. NOTE: This can be either a read-only or write-only
     connection, but not both.

   ERRORS

   If error4code( CODE4 ) is < 0 this function should just return NULL
   Any failure should cause an error4( ) to be generated and the function to
     return NULL.  Any initialization which was performed must be undone
     before returning NULL.

   RETURNS

   NULL - a failure occurred.  Check error4code( CODE4 ) to discover which
     error
   NON-NULL - a valid connected CONNECT4 is returned to the caller
*/

   CODE4 *c4 ;
   CONNECT4BUFFER *connectBuffer ;
   int rc ;

   #ifdef E4PARM_LOW
      if ( !connect )
      {
         error4( NULL, e4parmNull, E96914 ) ;
         return NULL ;
      }
      if ( ( ( numAdvanceReadBuffers > 0 ) && ( maxOutstandingWrites > 0 ) ) || ( ( numAdvanceReadBuffers == 0 ) && ( maxOutstandingWrites == 0 ) ) )
      {
         error4( connect->cb, e4parmNull, E96914 ) ;
         return NULL ;
      }
   #endif

   c4 = connect->cb ;

   if ( error4code( c4 ) )
      return NULL ;

   if ( c4->connectBufferMemory == NULL )
      c4->connectBufferMemory = mem4create( c4, MEMORY4START_CONNECT_BUFFER, sizeof( CONNECT4BUFFER ), MEMORY4EXPAND_CONNECT_BUFFER, 0 ) ;

   connectBuffer = ( CONNECT4BUFFER * )mem4alloc( c4->connectBufferMemory ) ;
   if ( connectBuffer == 0 )
   {
      error4( c4, e4memory, E96914 ) ;
      return NULL ;
   }
   rc = connect4bufferInit( connectBuffer, connect, NULL, numAdvanceReadBuffers, bufSize, maxOutstandingWrites ) ;
   if ( rc < 0 )
   {
      connect4bufferInitUndo( connectBuffer ) ;
      mem4free( c4->connectBufferMemory, connectBuffer );
      error4( c4, rc, E86902 ) ;
      return NULL ;
   }
   connect4lowBlast( connectBuffer->connectLow ) ;
   return connectBuffer ;
}

CONNECT4BUFFER *connect4bufferAuxConnectionGet( CONNECT4 *connect, int numAdvanceReadBuffers, int bufSize, int maxOutstandingWrites )
{
/* DESCRIPTION

   This function  returns a connection between the Client and Server. It
     does this by first checking if there is an unused connection it can
     use, and if not, creates a new one.

   PARAMATERS

   connect is a CONNECT4 that already has a write connection
   The remaining parameters are a the qualifications neccesary for
     this connection. NOTE: This can be either a read-only or write-only
     connection, but not both.

   ERRORS

   If error4code( CODE4 ) is < 0 this function should just return NULL
   Any failure should cause an error4( ) to be generated and the function to
     return NULL.  Any initialization which was performed must be undone
     before returning NULL.

   RETURNS

   NULL - a failure occurred.  Check error4code( CODE4 ) to discover which
     error
   NON-NULL - a valid connected CONNECT4 is returned to the caller
*/
   CONNECT4BUFFER *connectBuffer = NULL;

   #ifdef E4PARM_LOW
      if ( !connect )
      {
         error4( NULL, e4parmNull, E96915 ) ;
         return NULL ;
      }
      if ( ( ( numAdvanceReadBuffers > 0 ) && ( maxOutstandingWrites > 0 ) ) || ( ( numAdvanceReadBuffers == 0 ) && ( maxOutstandingWrites == 0 ) ) )
      {
         error4( connect->cb, e4parmNull, E96915 ) ;
         return NULL ;
      }
   #endif

   if ( error4code( connect->cb ) )
      return NULL ;

   connectBuffer = ( CONNECT4BUFFER * )l4first( &connect->blastList ) ;
   while ( connectBuffer )
   {
      if ( connectBuffer->type > 0 )
      {
         if ( maxOutstandingWrites > 0 )
         {
            l4remove( &connect->blastList, connectBuffer ) ;
            connectBuffer->maxWritesOutstanding = maxOutstandingWrites ;
            return connectBuffer ;
         }
      }
      else        /* connectBuffer->type < 0 */
      {
         if ( ( connectBuffer->advanceReads == numAdvanceReadBuffers ) &&
           ( connectBuffer->readSize == bufSize ) )
         {
            l4remove( &connect->blastList, connectBuffer ) ;
            return connectBuffer ;
         }
      }
      connectBuffer = ( CONNECT4BUFFER * )l4next( &connect->blastList, ( LINK4 * )connectBuffer );
   }
   return connect4bufferAuxConnection( connect, numAdvanceReadBuffers, bufSize, maxOutstandingWrites ) ;
}
#endif /*S4CLIENT */

void connect4bufferAuxConnectionPut( CONNECT4BUFFER *connectBuffer, CONNECT4 *connect )
{
/* DESCRIPTION

   This function is intended to be called when a CONNECT4BUFFER is no
     longer needed for blast functions. It is then returned to the list
     and can be picked out of when a new connection is needed */

   l4add( &connect->blastList, connectBuffer ) ;
}

#ifdef S4SERVER
CONNECT4BUFFER *connect4bufferAuxConnectionSpecific( CONNECT4 *connect, short id )
{
/*
   DESCRIPTION

   This function is used by the server. When a client requests that a
     connection is to be reused, it passes the id number of that connection.
     This function takes that id and uses it to find the correct connection.
     It then takes it off the list, and returns it
*/

   CONNECT4BUFFER *connectBuffer = NULL ;

   #ifdef E4PARM_LOW
      if ( !connect )
      {
         error4( NULL, e4parmNull, E96916 ) ;
         return NULL ;
      }
      if ( !id )
      {
         error4( connect->cb, e4parmNull, E96916 ) ;
         return NULL ;
      }
   #endif

   if ( connectBuffer->communicationError < 0 )
      return 0 ;

   do
   {
      connectBuffer = ( CONNECT4BUFFER * )l4next( &connect->blastList, connectBuffer ) ;
      if ( connectBuffer == NULL )
         return NULL ;
   } while ( connectBuffer->id != id ) ;

   l4remove( &connect->blastList, connectBuffer ) ;
   return connectBuffer ;
}
#endif /* S4SERVER */
#endif /* !S4OFF_BLAST */

#ifdef S4CLIENT
int connect4bufferConnect( CONNECT4BUFFER *connectBuffer, CONNECT4 *connect )
{
/* DESCRIPTION

   THis function is used to create the complementary socket on a typical
     connection. It can also be used to create the single socket
     connection that is used by the blast functions.

   PARAMATERS

   connectBuffer is the structure we want the connection to be in. If its
     type indicates its going to be a one-way connection, a connect4low is
     created for it. Otherwise, it uses the existing connect4low.
   connect is a CONNECT4 that already has a write connection so that
     messages can be sent along it to get the server to try to connect
     to the client.

   ERRORS

      If an error occurs, just return the error code.

   RETURNS

   <0 - a failure occurred.  Check error4code( CODE4 ) to discover which
     error
   r4success - The connection was successfu;
*/

   CONNECT4LOW listenSocket, *connectLow ;
   C4NETID netID ;
   CODE4 *c4 ;
   int rc ;
   short port ;
   short addrLen, temp ;
   short messageType ;
   short reconnection = 0 ;
   short conn ;

   #ifdef E4PARM_LOW
      if ( ( !connect ) || ( !connectBuffer ) )
         return e4parmNull ;
   #endif

   c4 = connect->cb ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   #ifndef S4OFF_BLAST
      conn = connectBuffer->type ;
      if ( conn == 0 )
         connectLow = connectBuffer->connectLow ;
      else
      {
         connectBuffer->id = ++connect->numBlasts ;
         reconnection = htons( ( short )( ( -conn ) * ( connectBuffer->id ) ) ) ;
         connectLow = ( CONNECT4LOW * )mem4alloc( c4->connectLowMemory ) ;
         if ( connectLow == 0 )
         {
            connectBuffer->communicationError = e4connection ;
            return error4( c4, e4memory, E96917 ) ;
         }
         connectBuffer->connectLow = connectLow ;
      }
   #else
      connectLow = connectBuffer->connectLow ;
   #endif
   connectBuffer->connectLow = connectLow ;
   #ifdef S4COM_PRINT
      #ifdef S4CLIENT
         #ifdef __unix__
            printf( "Sending Message:  STREAM4RECONNECT\n" ) ;
         #endif
      #endif
   #endif

   messageType = htons( STREAM4RECONNECT ) ;
   port = -1 ;
   rc = connect4lowListen( &listenSocket, c4, &port, 1 ) ;
   if ( rc < 0 )
   {
      connectBuffer->communicationError = e4connection ;
      return error4( c4, e4result, E96908 ) ;
   }

   rc = address4getLocalNetID( &netID, &addrLen, connect->connectBuffer.connectLow ) ;
   if ( rc < 0 )
   {
      connectBuffer->communicationError = e4connection ;
      return error4( c4, rc, E96918 ) ;
   }

   connect4send( connect, &messageType, sizeof( short ) ) ;
   connect4send( connect, &reconnection, sizeof( short ) ) ;
   temp = htons( sizeof( short ) ) ;
   connect4send( connect, &temp, sizeof( short ) ) ;
   connect4send( connect, &port, sizeof( short ) ) ;
   temp = htons( addrLen ) ;
   connect4send( connect, &temp, sizeof( short ) ) ;
   connect4send( connect, &netID, addrLen );
   connect4sendFlush( connect ) ;
   if ( connect4lowAcceptWait( &listenSocket, connectLow, code4timeout( c4 ) ) < 0 )
   {
      connect4lowClose( &listenSocket ) ;
      connect4lowClose( connectLow ) ;
      mem4free( c4->connectLowMemory, connectLow ) ;
      return e4result ;
   }
   connect4lowClose( &listenSocket ) ;
   connectBuffer->connected++ ;
   return connectBuffer->communicationError ;
}
#endif  /*S4CLIENT*/

void connect4bufferDisconnect( CONNECT4BUFFER *connectBuffer )
{
/* ERRORS

   Ignore any errors and continue disconnecting.  Ignore error4code( CODE4 ).

   NOTES

   This function is used to disconnect a CONNECT4BUFFER.

   It calls connect4completeRequest( ) to request the communications thread
     to stop servicing the connect.
*/

   #ifndef S4OFF_THREAD
      NET4MESSAGE *message ;
   #endif
   #ifdef E4PARM_LOW
      if ( connectBuffer == NULL )
      {
         error4( NULL, e4parmNull, E96919 ) ;
         return;
      }
   #endif

   if ( !( connectBuffer->connected ) )
      return ;
   #ifndef S4OFF_THREAD
      connect4threadCompleteRequest( &connectBuffer->connectThread ) ;

      message = connect4bufferWriteRetrieve( connectBuffer, 0, 0 ) ;
      while ( message != NULL )
      {
         list4mutexAdd( &connectBuffer->cb->writeBuffersAvail, message ) ;
         message = connect4bufferWriteRetrieve( connectBuffer, 0, 0 ) ;
      }
   #endif
   connectBuffer->connected = 0 ;
}

#ifndef S4OFF_THREAD
NET4MESSAGE *connect4bufferGetWriteBuffer( CONNECT4BUFFER *connectBuffer )
{
/* DESCRIPTION

   This function does some searching to try and produce a write buffer for
     use by the caller. It also checks to make sure we don't try pending
     too many reads at once.

   ERRORS

   If error4code( CODE4 ) is < 0 this function should just return NULL
   Any failure should cause an error4( ) to be generated and the function to
     return NULL.  Any initialization which was performed must be undone
     before returning NULL.

   NOTES

   This function is used to obtain a NET4MESSAGE buffer for writing.

   The following is done:

   Call connect4threadWriteRetrieve( ) with doWait false.  If a write buffer is
     available that we previously sent, this will return it to us, and we
     just return it back to the caller. If there are already too many writes being
     attempted, we just stop until a previous one completes.

   If null is returned, check CODE4.writeBuffersAVail, which is a list of
     available write NET4MESSAGE buffers.  If a buffer is available there,
     just return it back to the caller.

   If asynchronous writing is being done and no buffer is available,
     skip through the list of CONNECT4s and call
     connect4threadWriteRetrieve( ) with each of them, to try to obtain a write
     buffer.

   If no buffer is still available, a new one is allocated with
     mem4alloc( CODE4.writeMemory ), and returned.
*/

   NET4MESSAGE *mess ;
   CONNECT4BUFFER *conBuf ;
   LIST4 *list ;
   int total ;
   CODE4 *c4 ;

   #ifdef E4PARM_LOW
      if ( connect == NULL )
      {
         error4( NULL, e4parmNull, E96920 ) ;
         return NULL ;
      }
   #endif

   if ( connectBuffer->communicationError < 0 )
      return NULL ;

   c4 = connectBuffer->cb ;

   mess = connect4bufferWriteRetrieve( connectBuffer, 0, 0 ) ;
   if ( mess != NULL )
      return mess ;
   total = l4numNodes( &connectBuffer->connectThread.writingSignalsList ) +
           l4numNodes( &connectBuffer->connectThread.writeMessageOutstandingList ) ;
   if ( total >= connectBuffer->maxWritesOutstanding )
   {
      mess = connect4bufferWriteRetrieve( connectBuffer, 0, -1 ) ;
      return mess ;
   }

   mess = ( NET4MESSAGE * )list4mutexRemove( &c4->writeBuffersAvail ) ;
   if ( mess != NULL )
   {
      mess->connectThread = &connectBuffer->connectThread ;
      return mess ;
   }

   if ( c4->ver4 == ver4NT )
   {
      list4mutexWait( &c4->connectBufferListMutex ) ;
      list = &c4->connectBufferListMutex.list ;
      conBuf = ( CONNECT4BUFFER * )l4first( list ) ;
      while( conBuf != NULL )
      {
         mess = connect4bufferWriteRetrieve( conBuf, 1, 0 ) ;
         if ( mess != NULL )
         {
            mess->connectThread = &connectBuffer->connectThread ;
            list4mutexRelease( &c4->connectBufferListMutex );
            return mess ;
         }
         conBuf = ( CONNECT4BUFFER * )l4next( list, conBuf ) ;
      }
      list4mutexRelease( &c4->connectBufferListMutex );
   }

   if ( c4->writeMemory == NULL )
      c4->writeMemory = mem4create( c4, MEMORY4START_WRITE_MEMORY, ( sizeof(NET4MESSAGE) +
         c4->writeMessageBufferLen ), MEMORY4EXPAND_WRITE_MEMORY, 0 ) ;

   mess = ( NET4MESSAGE * )mem4alloc( c4->writeMemory ) ;
   if ( mess != NULL )
   {
      mess->connectThread = &connectBuffer->connectThread ;
      mess->messageBufferLen = c4->writeMessageBufferLen ;
      return mess ;
   }
   connectBuffer->communicationError = e4connection ;
   error4( c4, e4memory, E96920 ) ;
   return NULL ;
}
#endif /*!S4OFF_THREAD */

int connect4bufferInit( CONNECT4BUFFER *connectBuffer, CONNECT4 *connect, CONNECT4LOW *connectLow, int numAdvanceReadBuffers, int bufSize, int maxOutstandingWrites )
{
/* PARAMATERS

   connectBuffer is the CONNECT4BUFFER to initialize

   ERRORS

   If error4code( CODE4 ) is < 0 this function should just return error
   Any failure should cause an error4( ) to be generated and the function to
     return.  Any initialization which was performed must be undone
     before returning NULL.

   NOTES

   Initializes the CONNECT4BUFFER structure:
     Initialized CONNECT4BUFFER.connectThread
     Create the connection ( could be secondary or blast )
     Perform advance reads if needed
*/

   int rc ;
   CODE4 *c4 ;

   #ifdef E4PARM_LOW
      if ( ( connectBuffer == NULL ) || ( connect == NULL ) || ( connect->cb == NULL ) )
         return ( error4( NULL, e4parmNull, E96921 ) ) ;
   #endif

   c4 = connect->cb ;

   if ( error4code( c4 ) < 0 )
   {
      connectBuffer->communicationError = e4connection ;
      return e4codeBase ;
   }

   connectBuffer->connect = connect ;
   connectBuffer->cb = c4 ;
   #ifndef S4OFF_THREAD
      connectBuffer->maxWritesOutstanding = maxOutstandingWrites ;
      connectBuffer->advanceReads = numAdvanceReadBuffers ;
      connectBuffer->readSize = bufSize;
   #endif
   connectBuffer->connectLow = connectLow ;

   #ifndef S4OFF_THREAD
      rc = connect4threadInit( &connectBuffer->connectThread, c4, connectBuffer );
      if ( rc<0 )
      {
         connect4bufferInitUndo( connectBuffer ) ;
         connectBuffer->communicationError = e4connection ;
         return error4( c4, rc, E96925 ) ;
      }
      if ( maxOutstandingWrites > 0 )
         if ( numAdvanceReadBuffers > 0 )
            connectBuffer->type = 0 ;  /* Main connection */
         else
            connectBuffer->type = 1 ;  /* Write only connection */
      else
         connectBuffer->type = -1 ;     /* Read only connection */
   #else
      connectBuffer->workingWriteLen = c4->writeMessageBufferLen ;
      connectBuffer->workingWriteBuffer = (char *)u4alloc( connectBuffer->workingWriteLen ) ;
      if ( connectBuffer->workingWriteBuffer == NULL )
      {
         connect4bufferInitUndo( connectBuffer ) ;
         connectBuffer->communicationError = e4connection ;
         return error4( c4, e4memory, E96921 ) ;
      }
   #endif

   #ifndef S4OFF_THREAD
      if ( connectBuffer->type )/* != 0*/
         connectBuffer->connected = 0 ;
      else
   #endif
         connectBuffer->connected = 1 ;

   #ifdef S4CLIENT
      rc = connect4bufferConnect( connectBuffer, connect ) ;
      if ( rc<0 )
      {
         connect4bufferInitUndo( connectBuffer ) ;
         connectBuffer->communicationError = e4connection ;
         return error4( c4, rc, E86903 ) ;
      }
   #endif
   #ifndef S4OFF_BLAST
      if ( !connectBuffer->type )
         list4mutexAdd( &c4->connectBufferListMutex, connectBuffer ) ;
   #endif
   #ifndef S4OFF_THREAD
   if ( numAdvanceReadBuffers > 0 )
   {
      rc = connect4bufferInitRead( connectBuffer, numAdvanceReadBuffers, bufSize ) ;
      if ( rc<0 )
      {
         connect4bufferInitUndo( &connect->connectBuffer ) ;
         connectBuffer->communicationError = e4connection ;
         return error4( c4, rc, E96921 ) ;
      }
   }
   #endif
   return r4success ;
}

void connect4bufferInitUndo( CONNECT4BUFFER *connectBuffer )
{
/* ERRORS

   Ignore any errors and continue uninitializing.  Do not set
     error4code( CODE4 ).  Errors may occur if the other side forcefully
     disconnected.  Ignore error4code( CODE4 )

   NOTES


   Disconnects the connection.
   Cleans up the CONNECT4BUFFER structure.
*/

   #ifdef E4PARM_LOW
      if ( connectBuffer == NULL )
      {
         error4( NULL, e4parmNull, E96922 ) ;
         return ;
      }
   #endif

   connect4bufferDisconnect( connectBuffer ) ;
   #ifndef S4OFF_THREAD
      connect4threadInitUndo( &connectBuffer->connectThread ) ;
      if ( !connectBuffer->type )
         if ( ( ( LINK4 * )connectBuffer )->n )
            list4mutexRemoveLink( &connectBuffer->cb->connectBufferListMutex, ( LINK4 * )connectBuffer ) ;
      u4free( connectBuffer->messageReadArray ) ;
   #else
      u4free( connectBuffer->workingWriteBuffer ) ;
   #endif
   memset( connectBuffer, 0, sizeof( CONNECT4BUFFER ) ) ;
}

#ifndef S4OFF_THREAD
int connect4bufferInitRead( CONNECT4BUFFER *connectBuffer, int numAdvance, int bufSize )
{
/* PARAMATERS

   connectBuffer is the CONNECT4BUFFER to initialize

   ERRORS

   If error4code( CODE4 ) is < 0 this function should just return error
   Any failure should cause an error4( ) to be generated and the function to
     return.  Any initialization which was performed must be undone
     before returning NULL.

   NOTES

    Allocate CODE4.readMessageNumBuffers NET4MESSAGE buffers with message
       sizes of CODE4.readMessageBufferLen ( in a single allocation )
     Add the NET4MESSAGE structures to CONNECT4BUFFER.buffersAvail
        -actually, this area is never used, so it was removed.
     Place advance reads

   Uses readMessageBufferLen and readMessageNumBuffers to allocate a
     pool of memory to use for NET4MESSAGE structures that are used when
     performing read operations on the connection.  In the future we may
     do some memory optimization by one or all of the following:
     - do not allocate the memory pool until it is actually needed ( i.e.
       the connection may only be used to perform writes )
     - keep a list of unused pools located centrally to reduce the number
       required
     - have some additional pools to increase the number of ReadFiles which
       can be bufferred.
*/

   int size,i ;
   NET4MESSAGE *mess ;
   char *ptr ;

   if ( connectBuffer->communicationError < 0 )
      return -1 ;

   size = numAdvance *( sizeof( NET4MESSAGE )+bufSize ) ;
   connectBuffer->messageReadArray = ( NET4MESSAGE * )u4alloc( size );
   ptr = ( char * )connectBuffer->messageReadArray ;
   if ( ptr == NULL )
   {
      connectBuffer->communicationError = e4connection ;
      return ( error4( connectBuffer->cb, e4memory, E96923 ) ) ;
   }
   for ( i = 0; i < numAdvance; i++ )
   {
      mess = ( NET4MESSAGE * )( ptr + ( i*( ( sizeof( NET4MESSAGE ) ) +bufSize ) ) ) ;
      mess->messageBufferLen = bufSize ;

      if ( connect4threadReadRequest( &connectBuffer->connectThread, mess ) < 0 )
      {
         connectBuffer->communicationError = e4connection ;
         return -1 ;
      }
   }
   return r4success ;
}

NET4MESSAGE *connect4bufferReadRetrieve( CONNECT4BUFFER *connectBuffer, int waitSecs )
{
/* DESCRIPTION

   This function is used to retrieve a read message, if one is available.

   PARAMATERS

   waitSecs is the number of seconds to wait if no message is available.
     if set to WAIT4EVER then do not return until a message is available.

   ERRORS

   If an error occurs return NULL.  set error4code( CODE4 ) to error value.
   Check NET4MESSAGE.messageLen after a successful receive.  If it is '-1',
     then need to shut down the connection.  Set error4code( CODE4 ) to e4connect,
     free up the NET4MESSAGE structure, and return NULL.

   RETURNS

   The message retrieved, or NULL if the timeout elapsed

   NOTES

   This function does the following:

   call sema4wait( ) on CONNECT4THREAD.readMessageCompletedListSemaphore,
     inputing the waitSecs.

   if the result of the wait is true, return the first element of the
     list4mutex readMessageCompletedList( ).

   if the result of the wait was false, return NULL
*/

   int rc ;
   NET4MESSAGE *mess ;
   CODE4 *c4 ;

   #ifdef E4PARM_LOW
      if ( connectBuffer == NULL )
      {
         error4( NULL, e4parmNull, E96924 ) ;
         return NULL ;
      }
   #endif

   if ( connectBuffer->communicationError < 0 )
      return NULL ;

   c4 = connectBuffer->cb ;

   rc = semaphore4wait( &connectBuffer->connectThread.readMessageCompletedListSemaphore, waitSecs ) ;
   if ( rc < 0 )
   {
      connectBuffer->communicationError = e4connection ;
      error4( c4, rc, E96966 );
      return NULL;
   }
   if ( rc == 0 )
      return NULL ;
   mess = ( NET4MESSAGE * )list4mutexRemove( &connectBuffer->connectThread.readMessageCompletedList ) ;
   if ( mess == NULL )
   {
      connectBuffer->communicationError = e4connection ;
      error4( c4, e4result, E96966 );
      return NULL ;
   }
   if ( mess->messageLen == -1 )
   {
      connect4lowClose( connectBuffer->connectLow ) ;
      connectBuffer->communicationError = e4connection ;
      error4( c4, e4connect, E96966 ) ;
      return NULL ;
   }
   return mess ;
}
#endif /*S4OFF_THREAD */

int connect4bufferReceive( CONNECT4BUFFER *connectBuffer, void *data, long dataLen, int waitSecs )
{
/* PARAMATERS

   connectBuffer is the connection on which read data is being requested.
   data is a pointer to the address of where the retrieved data should be
     placed.
   dataLen is the quantity of data to retrieve.
   waitSecs is the number of seconds to wait before returing.  If set to
     WAIT4EVER, then do not return until completed.

   ERRORS

   If error4code( CODE4 ) is < 0 this function should just return error
   Any other failure should cause an error4( ) to be generated and the function
     to return.

   RETURNS

   r4success - data has been retrieved.
   r4timeout - timed out
   < 0 - error

   NOTES:

   This function usually does not return until all the required data has been
     obtained from the socket.  It would make multiple calls to
     connect4threadReadRetrieve( ) and copy the data until all of it was
     retrieved.

   If a call to connect4threadReadRetrieve( ) times out, return r4timeout;
     after call to connect4threadReadRetrieve( ), if return was NULL, check
     code4errorCode( ); if zero, then return r4timeout.  If e4connect, the
     the connection has failed.  In that case, call connect4disconnect( )
     and return e4connect.

   This function must verify that the connection is valid ( i.e. that
     CONNECT4.connectThread is non-null ).  If it is null, then return
     e4connect ( we have no connection )

   The only problem arises when too much data is retrieved from
     connect4threadReadRetrieve( ).
     To handle this contingincy a pointer to extra data must be kept and used
     as required.  Two members are added to CONNECT4 for this use:
      NET4MESSAGE *workingReadMessage ; <-- pointer to partially used message
      long workingReadMessagePos ;      <-- current position in partial message

   When all of a given NET4MESSAGE structure is finished with,  call
     connect4threadReadRequest( CONNECT4THREAD, NET4MESSAGE )
*/
   #ifdef S4OFF_THREAD
      int rc ;
   #endif
   long left, read ;
   char *ptr ;
   #ifndef S4OFF_THREAD
      NET4MESSAGE *mess ;
   #endif
   #ifdef E4PARM_LOW
      if ( ( connectBuffer == NULL ) || ( data == NULL && dataLen != 0 ) || dataLen < 0 )
         return ( error4( NULL, e4parmNull, E96967 ) ) ;
   #endif

   if ( dataLen == 0 )
      return r4success ;

   if ( connectBuffer->communicationError < 0 )
      return -1 ;

   if ( connectBuffer->connectLow == NULL )
   {
      connectBuffer->communicationError = e4connection ;
      return error4( connectBuffer->cb, e4connection, E96967 ) ;
   }

   #ifdef S4OFF_THREAD
      rc = connect4lowRead( connectBuffer->connectLow, (char *)data, dataLen ) ;
      if ( rc < 0 )
      {
         connect4bufferDisconnect( connectBuffer ) ;
         connectBuffer->communicationError = e4connection ;
         return error4( connectBuffer->cb, e4net, E96967 ) ;
      }
   #else
      left = dataLen ;               /* amount that still needs to be read */
      ptr = ( char * )data ;                   /* Where we should write to */
      if ( connectBuffer->workingReadMessage != NULL )
      {
         if ( left >= connectBuffer->workingReadMessage->messageLen - connectBuffer->workingReadMessagePos )
         {
            read = connectBuffer->workingReadMessage->messageLen - connectBuffer->workingReadMessagePos ;
            memcpy( ptr, connectBuffer->workingReadMessage->message + connectBuffer->workingReadMessagePos, read ) ;
            connect4threadReadRequest( &connectBuffer->connectThread, connectBuffer->workingReadMessage ) ;
            connectBuffer->workingReadMessage = NULL ;
            ptr += read ;
            left -= read ;
         }
         else
         {
            memcpy( ptr, connectBuffer->workingReadMessage->message+connectBuffer->workingReadMessagePos, left ) ;
            connectBuffer->workingReadMessagePos += left ;
            return r4success ;
         }
      }

      while ( left > 0 )
      {
         mess = connect4bufferReadRetrieve( connectBuffer, waitSecs ) ;
         if ( mess == NULL )
         {
            #ifdef S4SERVER
               /* EA/AS 08/29/97 fix for failures during partial connection */
               /*  for server, we don't care whether or not a timeout, just disconnect anyway
                   ( i.e. timeout equivalent to connection error for server ) */
               connectBuffer->communicationError = e4connection ;
               return e4net ;
            #else
               if ( error4code( connectBuffer->cb ) == 0 )
                  return r4timeout ;
               else
               {
                  connect4bufferDisconnect( connectBuffer ) ;
                  connectBuffer->communicationError = e4connection ;
                  return error4( connectBuffer->cb, e4net, E96967 ) ;
               }
            #endif
         }
         read = mess->messageLen ;
         if ( read > left )
         {
            memcpy( ptr, mess->message, left ) ;
            connectBuffer->workingReadMessage = mess ;
            connectBuffer->workingReadMessagePos = left ;
            return r4success ;
         }
         memcpy( ptr, mess->message, mess->messageLen ) ;
         connect4threadReadRequest( &connectBuffer->connectThread, mess ) ;
         left -= read ;
         ptr += read ;
      }
#endif /*!/S4OFF_THREAD*/
   return r4success ;
}

int connect4bufferSend( CONNECT4BUFFER *connectBuffer, const void *data, long dataLen )
{
/* ERRORS

   If error4code( CODE4 ) is < 0 this function should just return error
   Any other failure should cause an error4( ) to be generated and the function
     to return.  Any initialization must be undone before returning to caller.

   NOTES

   This function executes asynchronously.

   This function is used to add some data which will be sent on the
     connection.  The input data is copied into an internal buffer.  When
     the internal buffer is full, it is sent accross the connection.  To
     deliver any outstanding data, connect4sendFlush( ) must be called.

   This function uses the follwoing two CONNECT4 messages to keep track of the
     partially filled working send buffer:
      NET4MESSAGE *workingWriteMessage ; <-- pointer to partially used message
      long workingWriteMessagePos ;      <-- current position in partial message

   This function calls connect4sendFlush( ) when CONNECT4.workingWriteMessage
     is full and ready to send.

   To obtain a new NET4MESSAGE buffer ( i.e. if CONNECT4.workingWriteMessage is
     NULL or if we fill it up and still have more data to send ):
     call connectgetWriteBuffer( CONNECT4 )
*/

   long space, left ;
   const char *ptr ;
   int rc ;
   #ifndef S4OFF_THREAD
      NET4MESSAGE *mess ;
   #endif
   #ifdef E4PARM_LOW
      if ( ( connectBuffer == NULL ) || ( data == NULL && dataLen != 0 ) || dataLen < 0 )
         return error4( NULL, e4parmNull, E96968 ) ;
   #endif

   /* 03/19/98 AS - changed error handling - in case of CodeBase error, still ok to perform
      communications.  If the socket is in error, then don't perform.  */
   if ( connectBuffer->communicationError < 0 )
      return connectBuffer->communicationError ;

   if ( dataLen == 0 )
      return r4success ;

   left = dataLen ;
   ptr = ( const char * )data ;
   #ifdef S4OFF_THREAD
      if ( connectBuffer->workingWritePos > 0 )
      {
         space = connectBuffer->workingWriteLen - connectBuffer->workingWritePos;
         if ( space > dataLen )
         {
            memcpy( connectBuffer->workingWriteBuffer + connectBuffer->workingWritePos, data, left ) ;
            connectBuffer->workingWritePos += dataLen ;
            return r4success ;
         }
         else
         {
            memcpy( connectBuffer->workingWriteBuffer + connectBuffer->workingWritePos, data, space ) ;
            connectBuffer->workingWritePos = connectBuffer->workingWriteLen ;
            rc = connect4bufferSendFlush( connectBuffer ) ;
            if ( rc < 0 )
               return rc ;
            connectBuffer->workingWritePos = 0 ;
            left -= space ;
            ptr += space ;
         }
      }
      while ( left > 0 )
      {
         if ( connectBuffer->workingWriteLen > left )
         {
            memcpy( connectBuffer->workingWriteBuffer, ptr, left ) ;
            connectBuffer->workingWritePos = left ;
            return r4success ;
         }
         memcpy( connectBuffer->workingWriteBuffer, ptr, connectBuffer->workingWriteLen );
         ptr += connectBuffer->workingWriteLen ;
         left -= connectBuffer->workingWriteLen ;
         connectBuffer->workingWritePos = connectBuffer->workingWriteLen;  /* This line may not be needed */
         rc = connect4bufferSendFlush( connectBuffer ) ;
         if ( rc )
            return rc ;
      }
   #else
      if ( connectBuffer->workingWriteMessage != NULL )
      {
         space = connectBuffer->workingWriteMessage->messageBufferLen - connectBuffer->workingWriteMessage->messageLen ;
         if ( space > dataLen )
         {
            memcpy( connectBuffer->workingWriteMessage->message + connectBuffer->workingWriteMessage->messageLen, data, left ) ;
            connectBuffer->workingWriteMessage->messageLen += dataLen ;
            return r4success ;
         }
         else
         {
            memcpy( connectBuffer->workingWriteMessage->message + connectBuffer->workingWriteMessage->messageLen, data, space ) ;
            connectBuffer->workingWriteMessage->messageLen = connectBuffer->workingWriteMessage->messageBufferLen ;
            rc = connect4bufferSendFlush( connectBuffer ) ;
            if ( rc < 0 )
               return rc ;
            connectBuffer->workingWriteMessage = NULL ;
            left -= space ;
            ptr += space ;
         }
      }
      while ( left > 0 )
      {
         mess = connect4bufferGetWriteBuffer( connectBuffer ) ;
         connectBuffer->workingWriteMessage = mess ;
         if ( mess == NULL )
         {
            connectBuffer->communicationError = e4connection ;
            return error4( connectBuffer->cb, e4memory, E96968 ) ;
         }
         if ( mess->messageBufferLen > left )
         {
            memcpy( mess->message, ptr, left ) ;
            mess->messageLen = left ;
            return r4success ;
         }
         memcpy( mess->message, ptr, mess->messageBufferLen );
         ptr += mess->messageBufferLen ;
         left -= mess->messageBufferLen ;
         mess->messageLen = mess->messageBufferLen ; /* This line may not be needed */
         rc = connect4bufferSendFlush( connectBuffer ) ;
         if ( rc )
            return rc ;
      }
   #endif
   return r4success ;
}

int connect4bufferSendLong( CONNECT4BUFFER *connectBuffer, const long info )
{
/*
   int rc ;
   #ifndef S4OFF_THREAD
      NET4MESSAGE *mess ;
   #endif

   #ifdef E4PARM_LOW
      if ( ( connectBuffer == NULL ) )
         return error4( NULL, e4parmNull, E96969 ) ;
   #endif

   if ( error4code( connectBuffer->cb ) < 0 )
      return error4code( connectBuffer->cb ) ;
*/

   const long data = htonl( info );

   /* AS 01/05/98 - seems more maintainable to only send in one place, so changed */
   return connect4bufferSend( connectBuffer, &data, sizeof( long ) ) ;
}


char connect4bufferReceiveChar( CONNECT4BUFFER *connectBuffer, int waitSecs )
{
   char info ;
   connect4bufferReceive( connectBuffer, &info, sizeof( char ), waitSecs ) ;
   return info ;
}

short connect4bufferReceiveShort( CONNECT4BUFFER *connectBuffer, int waitSecs )
{
   short info ;
   connect4bufferReceive( connectBuffer, &info, sizeof( short ), waitSecs ) ;
   info = htons( info ) ;
   return info ;
}

long connect4bufferReceiveLong( CONNECT4BUFFER *connectBuffer, int waitSecs )
{
   long info ;
   connect4bufferReceive( connectBuffer, &info, sizeof( long ), waitSecs ) ;
   info = htonl( info ) ;
   return info ;
}

char *connect4bufferReceiveString( CONNECT4BUFFER *connectBuffer, int waitSecs )
{
   short len ;
   char *out ;

   len = connect4bufferReceiveShort( connectBuffer, waitSecs ) ;

   if ( len == 0 )
      return NULL ;
   else
   {
      out = ( char * )u4alloc( len+1 ) ;
      if ( out != NULL )
         connect4bufferReceive( connectBuffer, out, len, waitSecs ) ;
      out[len] = 0 ;
      return out ;
   }
}

int connect4bufferSendString( CONNECT4BUFFER *connectBuffer, const char *in )
{
   short len ;
   if ( in == NULL )
      len = 0 ;
   else
      len = strlen( in ) ;

   connect4bufferSendShort( connectBuffer, len ) ;
   return connect4bufferSend( connectBuffer, in, len ) ;
}

int connect4bufferSendShort( CONNECT4BUFFER *connectBuffer, const short info )
{
/*
   short data ;
   int rc ;
   #ifndef S4OFF_THREAD
      NET4MESSAGE *mess ;
   #endif

   #ifdef E4PARM_LOW
      if ( connectBuffer == NULL )
         return error4( NULL, e4parmNull, E96970 ) ;
   #endif

   if ( error4code( connectBuffer->cb ) < 0 )
      return error4code( connectBuffer->cb ) ;
*/
   short data = htons( info ) ;

   /* AS 01/05/98 - seems more maintainable to only send in one place, so changed */
   return connect4bufferSend( connectBuffer, &data, sizeof( short ) ) ;
}

int connect4bufferSendChar( CONNECT4BUFFER *connectBuffer, const char info )
{
   /* AS 01/05/98 - seems more maintainable to only send in one place, so changed */
   return connect4bufferSend( connectBuffer, &info, sizeof( char ) ) ;
}

int connect4bufferSendFlush( CONNECT4BUFFER *connectBuffer )
{
/* ERRORS

   If error4code( CODE4 ) is < 0 this function should just return error
   Any other failure should cause an error4( ) to be generated and the function
     to return.

   NOTES

   This function executes asynchronously.

   This function is used to send the CONNECT4.workingWriteMessage via
     com4threadWriteRequired( ).
*/
   int rc ;
   CODE4 *c4 ;

   #ifdef E4PARM_LOW
      if ( connectBuffer == NULL )
         return error4( NULL, e4parmNull, E96971 ) ;
   #endif

   c4 = connectBuffer->cb ;

   #ifdef S4OFF_THREAD
      /* 03/19/98 AS - changed error handling - in case of CodeBase error, still ok to perform
         communications.  If the socket is in error, then don't perform.  */
      if ( connectBuffer->communicationError < 0 )
         return connectBuffer->communicationError ;

      rc = connect4lowWrite( connectBuffer->connectLow, connectBuffer->workingWriteBuffer, connectBuffer->workingWritePos ) ;
      if ( rc )
      {
         connect4bufferDisconnect( connectBuffer ) ;
         connectBuffer->communicationError = e4connection ;
         return error4( c4, e4connection, E96971 ) ;
      }
      connectBuffer->workingWritePos = 0 ;
   #else
      if ( connectBuffer->workingWriteMessage == NULL )
         return r4success ;

      /* 03/19/98 AS - changed error handling - in case of CodeBase error, still ok to perform
         communications.  If the socket is in error, then don't perform.  */
      if ( connectBuffer->communicationError < 0 )
         return connectBuffer->communicationError ;

      if ( c4->ver4 != ver4NT )
      {
         rc = connect4lowWriteSynchronous( connectBuffer->connectLow, connectBuffer->workingWriteMessage ) ;
         if ( rc )
         {
            connect4bufferDisconnect( connectBuffer ) ;
            connectBuffer->communicationError = e4connection ;
            return error4( c4, e4connection, E96971 ) ;
         }
         list4mutexAdd( &c4->writeBuffersAvail, connectBuffer->workingWriteMessage ) ;
      }
      else
         inter4writeRequested( connectBuffer->connectThread.inter, connectBuffer->workingWriteMessage ) ;

      connectBuffer->workingWriteMessage = NULL ;
   #endif
   return r4success ;
}

#ifndef S4OFF_THREAD
NET4MESSAGE *connect4bufferWriteRetrieve( CONNECT4BUFFER *connectBuffer, int obtainForOther, int waitSecs )
{
/* PARAMATERS

   connectThread is the CONNECT4THREAD requesting a completed write buffer
   waitSecs is the number of seconds to wait if no message is available.
     if set to WAIT4EVER then do not return until a message is available.

   ERRORS/RETURNS

   NON-NULL a valid NET4MESSAGE is returned to the caller
   NULL may be returned either if an error occurred or if no message was
     available and doWait was FALSE.  To check for error, examine
     CODE4.error, which gets set by error4( ) being called in the case of
     an error occurring in this function.
   Additional: remove error4( ) from the CODE4 check. It's causing
     problems with connect4disconnect( )

   NOTES

   This function is used to retrieve a NET4MESSAGE structure which was
     performing an asynchronous write.

   If waitSecs is <0, this function will not return until a NET4MESSAGE
     structure is available or the timeout elapses.

   This function does the following:
     Call semaphore4wait( CONNECT4THREAD.writeCompletedSemaphore, waitSecs )
     Remove the first element of CONNECT4THREAD.writeMessageCompletedList
     ( list4mutex ), and returns it to the caller.
*/

   int rc ;
   NET4MESSAGE *mess ;

   #ifdef E4PARM_LOW
      if ( connectBuffer == NULL )
      {
         error4( NULL, e4parmNull, E96972 ) ;
         return NULL ;
      }
   #endif

   if ( ( obtainForOther ) && ( !connectBuffer->canShareWriteBuffers ) )
      return NULL ;

   if ( connectBuffer->communicationError < 0 )
      return NULL ;

   rc = semaphore4wait( &connectBuffer->connectThread.writeMessageCompletedListSemaphore, waitSecs ) ;
   if ( rc < 0 )
   {
      error4( connectBuffer->cb, rc, E96972 ) ;
      connectBuffer->communicationError = e4connection ;
      return NULL ;
   }
   if ( !rc )
      return NULL ;
   mess = ( NET4MESSAGE * )list4mutexRemove( &connectBuffer->connectThread.writeMessageCompletedList ) ;
   return mess ;
}
#endif /*!S4OFF_THREAD */
#endif /*!S4STAND_ALONE */

