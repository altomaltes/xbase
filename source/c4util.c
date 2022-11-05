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

/* c4util.c  */

/* contains security utility routines */

#include "d4all.h"

#ifdef __TURBOC__
   #pragma hdrstop
#endif  /* __TUROBC__ */

DATA4 *S4FUNCTION code4directory( CODE4 *c4, char *directory )
{
   #ifndef S4CLIENT
      error4( c4, e4notSupported, 0 ) ;
      return 0 ;
   #else
      short rc ;
      char *name ;
      DATA4 *data ;
      int oldAccessMode ;
      CONNECT4 *connect = &c4->clientConnect ;

      connect4sendShort( connect, STREAM4DIRECTORY ) ;
      connect4sendString( connect, directory ) ;
      connect4sendFlush( connect ) ;

      rc = connect4receiveShort( connect ) ;
      if ( rc < 0 )
      {
         error4( c4, rc, E96701 ) ;
         return NULL ;
      }
      name = connect4receiveString( connect ) ;

      oldAccessMode = c4->accessMode ;
      c4->accessMode = OPEN4DENY_RW ;
      c4->openForCreate = 1 ;
      data = d4open( c4, name ) ;
      c4->openForCreate = 0 ;
      c4->accessMode = oldAccessMode ;

      u4free( name ) ;

      return data ;
   #endif
}

#ifdef S4CLIENT
/*
int S4FUNCTION code4catalogSet( CODE4 *c4, short catalogAdd, int catalogStatus )
{
   CONNECTION4 *connection ;
   CONNECTION4CATALOG_SET_INFO_IN *infoIn ;
   int rc ;

   if ( c4->defaultServer.connected )
   {
      connection = &c4->defaultServer ;
      connection4assign( connection, CON4CATALOG, 0L, 0L ) ;
      connection4addData( connection, NULL, sizeof(CONNECTION4CATALOG_SET_INFO_IN), (void **)&infoIn ) ;
      infoIn->catalogAdd = htons(catalogAdd) ;
      infoIn->catalogStatus = htons(catalogStatus) ;
      connection4sendMessage( connection ) ;
      rc = connection4receiveMessage( connection ) ;
      if ( rc < 0 )
         return rc ;

      rc = connection4status( connection ) ;
      if ( rc < 0 )
         connection4error( connection, c4, rc, 0 ) ;

      return rc ;
   }

   return 0 ;
}
*/

char * S4FUNCTION code4serverCurrentDirectory( CODE4 *c4 )
{
   short len ;
   char *name ;

   connect4sendShort( &c4->clientConnect, STREAM4CURRENT_DIRECTORY ) ;
   connect4sendFlush( &c4->clientConnect ) ;

   len = connect4receiveShort( &c4->clientConnect ) ;
   name = (char *)u4allocFree( c4, len+1 ) ;
   connect4receive( &c4->clientConnect, name, len, code4timeout( c4 ) ) ;
   name[len] = 0 ;

   return name ;
}

int S4FUNCTION code4serverCloseFiles( CODE4 *c4 )
{
   CONNECTION4 *connection ;
   int rc ;

   connection = &c4->defaultServer ;
   if ( connection == 0 )
      return error4( c4, e4connection, E81102 ) ;
   connection4assign( connection, CON4CLOSE_FILES, 0L, 0L ) ;
   connection4sendMessage( connection ) ;
   rc = connection4receiveMessage( connection ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E96701 ) ;

   rc = connection4status( connection ) ;
   if ( rc < 0 )
      return connection4error( connection, c4, rc, E96701 ) ;

   return rc ;
}

int S4FUNCTION code4serverRestart( CODE4 *c4 )
{
   CONNECTION4 *connection ;
   int rc ;

   connection = &c4->defaultServer ;
   if ( connection == 0 )
      return error4( c4, e4connection, E81102 ) ;
   connection4assign( connection, CON4RESTART, 0L, 0L ) ;
   connection4sendMessage( connection ) ;
   rc = connection4receiveMessage( connection ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E96701 ) ;

   rc = connection4status( connection ) ;
   if ( rc < 0 )
      return connection4error( connection, c4, rc, E96701 ) ;

   return rc ;
}

/* for testing only, causes a server crash */
int S4FUNCTION code4serverCrash( CODE4 *c4 )
{
   CONNECTION4 *connection ;
   int rc ;

   connection = &c4->defaultServer ;
   if ( connection == 0 )
      return error4( c4, e4connection, E81102 ) ;
   connection4assign( connection, CON4CRASH, 0L, 0L ) ;
   connection4sendMessage( connection ) ;
   rc = connection4receiveMessage( connection ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E96701 ) ;

   rc = connection4status( connection ) ;
   if ( rc < 0 )
      return connection4error( connection, c4, rc, E96701 ) ;

   return rc ;
}

/* input needs to be short for visual basic compatability */
int S4FUNCTION code4connectAcceptNew( CODE4 *c4, short settingIn )
{
   CONNECTION4 *connection ;
   int rc ;
   unsigned char setting = (settingIn == 0 ) ? 0 : 1 ;

   connection = &c4->defaultServer ;
   if ( connection == 0 )
      return error4( c4, e4connection, E81102 ) ;
   connection4assign( connection, CON4CONNECT_ACCEPT_NEW, 0L, 0L ) ;
   connection4addData( connection, NULL, sizeof( unsigned char ), (void **)&setting ) ;
   connection4sendMessage( connection ) ;
   rc = connection4receiveMessage( connection ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E96701 ) ;

   rc = connection4status( connection ) ;
   if ( rc < 0 )
      return connection4error( connection, c4, rc, E96701 ) ;

   return rc ;
}

int S4FUNCTION code4connectCutAll( CODE4 *c4 )
{
   CONNECTION4 *connection ;
   int rc ;

   connection = &c4->defaultServer ;
   if ( connection == 0 )
      return error4( c4, e4connection, E81102 ) ;
   connection4assign( connection, CON4CONNECT_CUT_ALL, 0L, 0L ) ;
   connection4sendMessage( connection ) ;
   rc = connection4receiveMessage( connection ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E96701 ) ;

   rc = connection4status( connection ) ;
   if ( rc < 0 )
      return connection4error( connection, c4, rc, E96701 ) ;

   return rc ;
}

int S4FUNCTION code4connectCut( CODE4 *c4, long connId )
{
   CONNECTION4 *connection ;
   int rc ;

   connection = &c4->defaultServer ;
   if ( connection == 0 )
      return error4( c4, e4connection, E81102 ) ;
   connection4assign( connection, CON4CONNECT_CUT, 0L, 0L ) ;
   connection4addData( connection, &connId, sizeof( long ), NULL ) ;
   connection4sendMessage( connection ) ;
   rc = connection4receiveMessage( connection ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E96701 ) ;

   rc = connection4status( connection ) ;
   if ( rc < 0 )
      return connection4error( connection, c4, rc, E96701 ) ;

   return rc ;
}

int S4FUNCTION code4serverShutdown( CODE4 *c4 )
{
   CONNECTION4 *connection ;
   int rc ;

   code4close( c4 ) ;  /* this helps code4initUndo() later to not have outstanding DATA4s. */

   connection = &c4->defaultServer ;
   if ( connection == 0 )
      return error4( c4, e4connection, E81102 ) ;
   connection4assign( connection, CON4SHUTDOWN, 0L, 0L ) ;
   connection4sendMessage( connection ) ;

   /* Ignore any connection die messages, since that is ok (it just means
      that connection went down before we got our message back).  Note
      that we must wait for a reply though, to ensure the message got through. */

   c4->errOff = 1 ;
   rc = connection4receiveMessage( connection ) ;
   if ( rc < 0 )
   {
      error4set( c4, 0 ) ;
      return rc ;
   }

   rc = connection4status( connection ) ;
   if ( rc < 0 )
   {
      error4set( c4, 0 ) ;
      return rc ;
   }
   connection4initUndo( connection ) ;
   c4->defaultServer.connected = 0 ;

   return 0 ;
}

/*
int d4getTables( DATA4 *data, const char *path )
{
   DESCRIPTION

   Fills in the input DATA4 with all available tables in the input path directory.

   PARAMATERS

   data is a DATA4 with fields as defined by the schema table requirements
     (tables schema)

   NOTES

   For simplicity, return '.dbf' extension files.

   This functionality is used by code4tables() and Schema5tables::addDirectory.

}
*/

char * S4FUNCTION code4tables( CODE4 *c4, const char *path )
{
/*
   DESCRIPTION

   Returns a char pointer containing a name of table for all available tables
     in the input path directory.

   RETURNS

   The returned name must be freed by the caller using u4free()

   NOTES

   For simplicity, return '.dbf' extension files.

   This functionality is used by the schema tables as well (for OLE-DB)
*/
   short len ;
   char *name ;

   if ( error4code( c4 ) < 0 )
      return 0 ;

   connect4sendShort( &c4->clientConnect, STREAM4TABLES ) ;
   len = strlen( path ) ;
   connect4sendShort( &c4->clientConnect, len ) ;
   connect4send( &c4->clientConnect, path, len ) ;
   connect4sendFlush( &c4->clientConnect ) ;

   len = connect4receiveShort( &c4->clientConnect ) ;
   name = (char *)u4allocFree( c4, len+1 ) ;
   connect4receive( &c4->clientConnect, name, len, code4timeout( c4 ) ) ;
   name[len] = 0 ;

   return name ;
}

DATA4 *S4FUNCTION code4connectionStatus( CODE4 *c4 )
{
   short len, rc ;
   char *name ;
   DATA4 *data ;
   int oldAccessMode ;

   if ( error4code( c4 ) < 0 )
      return 0 ;

   connect4sendShort( &c4->clientConnect, STREAM4STATUS ) ;
   connect4sendFlush( &c4->clientConnect ) ;

   rc = connect4receiveShort( &c4->clientConnect ) ;
   if ( rc < 0 )
   {
      error4( c4, rc, E96701 ) ;
      return NULL ;
   }
   len = connect4receiveShort( &c4->clientConnect ) ;
   /* retrieves the name for d4open() */
   name = (char *)u4alloc( len + 1 ) ;

   connect4receive( &c4->clientConnect, name, len, code4timeout( c4 ) ) ;
   name[len] = 0 ;

   oldAccessMode = c4->accessMode ;
   c4->accessMode = OPEN4DENY_RW ;
   c4->openForCreate = 1 ;
   data = d4open( c4, name ) ;
   c4->openForCreate = 0 ;
   c4->accessMode = oldAccessMode ;

   u4free( name ) ;

   return data ;
}
#endif /* S4CLIENT */

#ifdef S4SERVER
DATA4 *S4FUNCTION code4connectionStatus( CODE4 *c4 )
{
   DATA4 *data, *dataOn ;
   DATA4FILE *dataFileOn = NULL ;
   SERVER4 *server ;
   SERVER4CLIENT *client ;
   int hasData ;
   LIST4 *list ;
   char buf[LEN4PATH], buf2[LEN4PATH+LEN4TABLE_NAME], buf3[LEN4TABLE_NAME] ;
   FIELD4INFO statusFields[] =
   {
      { "ACCOUNT_ID", r4str, LEN4ACCOUNT_ID, 0 },
      { "TCPADDRESS", r4str, 15, 0 },
      { "CONNECT_ID", r4num, 10, 0 },
      { "PATH",       r4str, LEN4PATH, 0 },
      { "TABLE",      r4str, LEN4TABLE_NAME, 0 },
      { "FILETYPE",   r4str, 1, 0 },   /* TYPE4PERMANENT, TYPE4TEMP, TYPE4SCHEMA */
      { "RECCOUNT",   r4num, 10, 0 },
      { "NUMFIELDS",  r4num, 4, 0 },
      { "REC_WIDTH",  r4num, 5, 0 },
      { "READ_ONLY",  r4log, 1, 0 },
      { "ACCESSMODE", r4num, 2, 0 },
      { "LOCKTYPE",   r4num, 2, 0 },   /* 0 means no locks for this client on this file */
      { "LOCKRECNO",  r4num, 10, 0 },  /* if locktype is LOCK4RECORD */
      { 0,0,0,0 },
   } ;

   TAG4INFO statusTags[] =
   {
         /* connections (account+address+connection id) */
      { "CONNECT",  "UPPER(ACCOUNT_ID) + TCPADDRESS + ASCEND(CONNECT_ID)", "LEFT(ACCOUNT_ID,1) <> ' '", r4uniqueContinue, 0 },
         /* connections with tables (account+address+connection id+path+table) */
      { "CONNECTT", "UPPER(ACCOUNT_ID) + TCPADDRESS + ASCEND(CONNECT_ID) + TRIM(LEFT(PATH,150)) + TABLE", "LEFT(ACCOUNT_ID,1) <> ' '", r4uniqueContinue, 0 },
         /* open tables (path+table) */
      { "TABLE",    "TRIM(LEFT(PATH,200)) + TABLE", "LEFT(ACCOUNT_ID,1) <> ' ' .AND. LEFT(TABLE,1) <> ' '", r4uniqueContinue, 0 },
         /* open tables and associated connections (path+table+account+address+connection id) */
      { "TABLE_C",  "TRIM(LEFT(PATH,135)) + TABLE + UPPER(ACCOUNT_ID) + TCPADDRESS + ASCEND(CONNECT_ID)", "LEFT(ACCOUNT_ID,1) <> ' '", r4uniqueContinue, 0 },
         /* lock information: table, connection info, lock info */
      { "LOCK_TCL", "TRIM(LEFT(PATH,135)) + TABLE + UPPER(ACCOUNT_ID) + TCPADDRESS + ASCEND(CONNECT_ID) + ASCEND(LOCKTYPE) + ASCEND(LOCKRECNO)", "LEFT(TABLE,1) <> ' ' .AND. LOCKTYPE <> 0 .AND. LEFT(ACCOUNT_ID,1) <> ' '",  0, 0 },
         /* table, lock info, connection info */
      { "LOCK_TLC", "TRIM(LEFT(PATH,135)) + TABLE + ASCEND(LOCKTYPE) + ASCEND(LOCKRECNO) + UPPER(ACCOUNT_ID) + TCPADDRESS + ASCEND(CONNECT_ID)", "LEFT(TABLE,1) <> ' ' .AND. LOCKTYPE <> 0 .AND. LEFT(ACCOUNT_ID,1) <> ' '",  0, 0 },
         /* connection info, table, lock info */
      { "LOCK_CTL", "UPPER(ACCOUNT_ID) + TCPADDRESS + ASCEND(CONNECT_ID) + TRIM(LEFT(PATH,135)) + TABLE + ASCEND(LOCKTYPE) + ASCEND(LOCKRECNO)", "LEFT(TABLE,1) <> ' ' .AND. LOCKTYPE <> 0 .AND. LEFT(ACCOUNT_ID,1) <> ' '",  0, 0 },
         /* connection info, lock info, table */
      { "LOCK_CLT", "UPPER(ACCOUNT_ID) + TCPADDRESS + ASCEND(CONNECT_ID) + ASCEND(LOCKTYPE) + ASCEND(LOCKRECNO) + TRIM(LEFT(PATH,135)) + TABLE", "LEFT(TABLE,1) <> ' ' .AND. LOCKTYPE <> 0 .AND. LEFT(ACCOUNT_ID,1) <> ' '",  0, 0 },
         /* lock info, table, connection info */
      { "LOCK_LTC", "ASCEND(LOCKTYPE) + ASCEND(LOCKRECNO) + TRIM(LEFT(PATH,135)) + TABLE + UPPER(ACCOUNT_ID) + TCPADDRESS + ASCEND(CONNECT_ID)", "LEFT(TABLE,1) <> ' ' .AND. LOCKTYPE <> 0 .AND. LEFT(ACCOUNT_ID,1) <> ' '",  0, 0 },
         /* lock info, connection info, table */
      { "LOCK_LCT", "ASCEND(LOCKTYPE) + ASCEND(LOCKRECNO) + UPPER(ACCOUNT_ID) + TCPADDRESS + ASCEND(CONNECT_ID) + TRIM(LEFT(PATH,135)) + TABLE", "LEFT(TABLE,1) <> ' ' .AND. LOCKTYPE <> 0 .AND. LEFT(ACCOUNT_ID,1) <> ' '",  0, 0 },
      { 0,0,0,0,0 }
   } ;

   FIELD4 *accountId, *tcpAddress, *connectId, *path, *table, *fileType,
          *recCount, *numFields, *recWidth, *readOnly, *accessMode, *lockType, *lockRecno ;
   char done, appendDone, recordDone ;
   LOCK4 *recordOn ;

   data = d4createTemp( c4, statusFields, statusTags ) ;
   if ( data == NULL )
      return NULL ;

   data->dataFile->singleClient = c4->currentClient ;
   data->trans = &c4->currentClient->trans ;

   accountId =  d4field( data, "ACCOUNT_ID" ) ;
   tcpAddress = d4field( data, "TCPADDRESS" ) ;
   connectId =  d4field( data, "CONNECT_ID" ) ;
   path =       d4field( data, "PATH" ) ;
   table =      d4field( data, "TABLE" ) ;
   fileType =   d4field( data, "FILETYPE" ) ;
   recCount =   d4field( data, "RECCOUNT" ) ;
   numFields =  d4field( data, "NUMFIELDS" ) ;
   recWidth =   d4field( data, "REC_WIDTH" ) ;
   readOnly =   d4field( data, "READ_ONLY" ) ;
   accessMode = d4field( data, "ACCESSMODE" ) ;
   lockType =   d4field( data, "LOCKTYPE" ) ;
   lockRecno =  d4field( data, "LOCKRECNO" ) ;

   server = c4->server ;

   list4mutexWait( &server->clients ) ;

   /* now go through all the clients and add all the info */
   for ( client = NULL ;; )
   {
      if ( client == c4->catalogClient )  /* done at the end */
         break ;

      client = (SERVER4CLIENT *)l4next( &server->clients.list, client ) ;
      if ( client == 0 )
         client = c4->catalogClient ;

      hasData = 0 ;

      list = tran4dataList( &client->trans ) ;
      if ( list != 0 )
      {
         int oldClientId, didEntry = 0 ;
         for ( dataOn = NULL ;; )
         {
            if ( dataOn != 0 )
               dataOn->clientId = oldClientId ;
            dataOn = (DATA4 *)l4next( list, dataOn ) ;
            if ( dataOn == 0 )
               break ;
            oldClientId = dataOn->clientId ;
            dataOn->clientId = 0 ;  // set to 0 to do a general query...
            hasData = 1 ;
            /* now add a record for each lock */
            recordOn = 0 ;
            for( done = 0, recordDone = 0, appendDone = 0 ; done != 1 ; )
            {
               /* lock types to consider: */
               /*    append lock          */
               /*    file lock            */
               /*    record locks         */
               /*    index lock           */

               d4appendStart( data, 0 ) ;
               f4blank( lockRecno ) ;  // clear out for file/append
               f4assignInt( lockType, 0L ) ;  // this gets set to 0 if no lock, init to that now

               if ( d4lockTestFile( dataOn ) )
               {
                  f4assignInt( lockType, LOCK4FILE ) ;
                  done = 1 ;   /* a file lock means all is locked, so done */
               }
               else
               {
                  if ( appendDone == 0 && d4lockTestAppend( dataOn ) )
                  {
                     f4assignInt( lockType, LOCK4APPEND ) ;
                     appendDone = 1 ;   /* done this lock test */
                  }
                  else
                  {
                     /*
                     AS 06/26/97 index locks unverifiable, and not settable, so don't include

                     if ( indexDone == 0 && d4lockTestIndex( dataOn ) ) /* try that
                     {
                        f4assignInt( lockType, LOCK4INDEX ) ;
                        indexDone = 1 ;   /* done this lock test
                     }
                     else
                     */
                     {
                        /* try the next record lock */
                        if ( d4lockTest( dataOn, -1L ) )
                        {
                           /* means there are record locks, get the next one */
                           for( ;; )
                           {
                              if ( recordOn ==  0 )  /* means just starting */
                              {
                                 recordOn = (LOCK4 *)single4initIterate( &(dataOn->dataFile->lockedRecords) ) ;
                                 if ( recordOn == 0 )
                                 {
                                    done = 1 ;
                                    break ;
                                 }
                              }
                              else
                                 recordOn = (LOCK4 *)single4next( &recordOn->link ) ;
                              if ( recordOn ==  0 )  /* means done */
                              {
                                 done = 1 ;
                                 break ;
                              }
                              if ( dataOn == tran4data( &client->trans, recordOn->id.serverId, recordOn->id.clientId) ) /* our data4 */
                                 break ;
                           }
                           if ( done == 1 )  // done, so quit loop
                           {
                              if ( didEntry == 1 )  /* if didEntry == 0, means we had no entry, add one now */
                                 break ;
                           }
                           else
                           {
                              recordDone = 1 ;
                              f4assignInt( lockType, LOCK4RECORD ) ;
                              f4assignLong( lockRecno, recordOn->id.recNum ) ;
                           }
                        }
                        else /* we are done, add a record if no locks recorded */
                        {
                           done = 1 ;
                           f4assignInt( lockType, 0L ) ;
                           if ( appendDone || recordDone )
                           {
                              if ( didEntry == 1 )  /* if didEntry == 0, means we had no entry, add one now */
                                 break ;   /* don't record this no-lock entry */
                           }
                        }
                     }
                  }
               }

               f4assign( accountId, client->account.accountId ) ;
               f4assign( tcpAddress, client->account.tcpAddress ) ;
               f4assignLong( connectId, client->id ) ;
               u4nameCurrent( buf2, sizeof( buf2 ), dataOn->dataFile->file.name ) ;
               u4namePath( buf, sizeof( buf ), buf2 ) ;
               f4assign( path, buf ) ;
               u4namePiece(buf3, sizeof( buf3 ), buf2, 0, 1 ) ;
               f4assign( table, buf3 ) ;
               if ( dataOn->dataFile->file.isTemp == 1 )
                  f4assignChar( fileType, TYPE4TEMP ) ;
               else
                  f4assignChar( fileType, TYPE4PERMANENT ) ;
               /* else if schema ... */
               f4assignLong( recCount, d4recCount( dataOn ) ) ;
               f4assignInt( numFields, d4numFields( dataOn ) ) ;
               f4assignInt( recWidth, d4recWidth( dataOn ) ) ;
               if ( dataOn->readOnly )
                  f4assignChar( readOnly, 'T' ) ;
               else
                  f4assignChar( readOnly, 'F' ) ;
               f4assignInt( accessMode, (char)dataOn->accessMode ) ;
               d4append( data ) ;
            }
         }
         didEntry = 1 ;
      }

      if ( hasData == 0 )  /* client has no datafiles -- add a record anyway */
      {
         d4appendStart( data, 0 ) ;
         d4blank( data ) ;
         f4assign( accountId, client->account.accountId ) ;
         f4assign( tcpAddress, client->account.tcpAddress ) ;
         f4assignLong( connectId, client->id ) ;
         f4assignInt( lockType, 0L ) ;  // this gets set to 0 if no lock, init to that now
         d4append( data ) ;
      }
   }

   /* now do all the data files with no clients (i.e. bufferred open) */

   for(;;)
   {
      dataFileOn = (DATA4FILE *)l4next( &c4->dataFileList, dataFileOn ) ;
      if ( !dataFileOn )
         break ;
      if ( dataFileOn->userCount == 0 )
      {
         d4appendStart( data, 0 ) ;
         d4blank( data ) ;
         f4assignInt( lockType, 0L ) ;  // this gets set to 0 if no lock, init to that now
         u4nameCurrent( buf2, sizeof( buf2 ), dataFileOn->file.name ) ;
         u4namePath( buf, sizeof( buf ), buf2 ) ;
         f4assign( path, buf ) ;
         u4namePiece(buf3, sizeof( buf3 ), buf2, 0, 1 ) ;
         f4assign( table, buf3 ) ;
         if ( dataFileOn->file.isTemp == 1 )
            f4assignChar( fileType, TYPE4TEMP ) ;
         else
            f4assignChar( fileType, TYPE4PERMANENT ) ;
         /* else if schema ... */
         f4assignLong( recCount, dfile4recCount( dataFileOn, 0L ) ) ;
         f4assignInt( numFields, dataFileOn->nFields ) ;
         f4assignInt( recWidth, dfile4recWidth( dataFileOn ) ) ;
         if ( dataFileOn->file.isReadOnly )
            f4assignChar( readOnly, 'T' ) ;
         else
            f4assignChar( readOnly, 'F' ) ;
         f4assignInt( accessMode, (char)dataFileOn->file.lowAccessMode ) ;
         if ( dfile4lockTestFile( dataFileOn, 0L, 0L ) )
            f4assignInt( lockType, LOCK4FILE ) ;
         else
         {
            if ( dfile4lockTestAppend( dataFileOn, 0L, 0L ) )
               f4assignInt( lockType, LOCK4APPEND ) ;
            else
            {
               if ( dfile4lockTest( dataFileOn, 0L, 0L, -1L ) )
                  f4assignInt( lockType, LOCK4RECORD ) ;
               else
                  f4assignInt( lockType, 0L ) ;
            }
         }
         d4append( data ) ;
      }
   }

   list4mutexRelease( &server->clients ) ;

   return data ;
}
#endif /* S4SERVER */
