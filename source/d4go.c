/* d4go.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifdef S4CLIENT
/* d4go, d4skip, d4seek, after complete, perform the record information transferral... */
int d4goVirtual( DATA4 *data, const long recNo, const int rc, const void *outVoid, void *connectionVoid )
{
   const CONNECTION4GO_INFO_OUT *out ;
   CONNECTION4 *connection ;
   CODE4 *c4 ;

   out = (CONNECTION4GO_INFO_OUT *)outVoid ;
   connection = (CONNECTION4 *)connectionVoid ;
   c4  = data->codeBase ;

   if ( rc == r4entry )   /* invalid record (beyond eof) */
   {
      data->recNum = -1 ;
      d4blankLow( data, data->record ) ;
      if ( c4->errGo )
         return error4describe( c4, e4read, E93101, d4alias( data ), 0, 0 ) ;
   }

   if ( rc > 0 )  /* eg. r4entry */
      return rc ;

   if ( rc < 0 )
   {
      data->recNum = -1 ;
      if ( c4->errGo )
         connection4errorDescribe( connection, c4, rc, E93101, d4alias( data ), 0, 0 ) ;
      return rc ;
   }

   if ( (long)ntohl(out->recNo) != recNo )
      return error4( c4, e4info, E83101 ) ;
   /* now copy the data into the record */
   memcpy( data->record, ((char *)out) + sizeof( CONNECTION4GO_INFO_OUT ), dfile4recWidth( data->dataFile ) ) ;

   data->recNum = recNo ;
   data->bofFlag = data->eofFlag = 0 ;

   if ( out->recordLocked )
   {
      d4localLockSet( data, recNo ) ;
      memcpy( data->recordOld, data->record, dfile4recWidth( data->dataFile ) ) ;
      data->recNumOld = recNo ;
      #ifndef S4OFF_MULTI
         #ifndef S4OFF_MEMO
            data->memoValidated = 1 ;
         #endif
      #endif
   }

   return 0 ;
}

int S4FUNCTION d4go( DATA4 *data, const long recNo )
{
   int rc ;
   CONNECTION4 *connection ;
   CONNECTION4GO_INFO_IN *info ;
   CODE4 *c4 ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E93101 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E93101 ) ;
   #endif

   c4 = data->codeBase ;

   #ifdef S4OFF_WRITE
      if ( error4code( c4 ) < 0 )
         return e4codeBase ;
   #else
      rc = d4updateRecord( data, 0 ) ;   /* returns -1 if error4code( codeBase ) < 0 */
      if ( rc )
         return rc ;
   #endif

   connection = data->dataFile->connection ;
   if ( connection == 0 )
      return e4connection ;

   connection4assign( connection, CON4GO, data4clientId( data ), data4serverId( data ) ) ;
   connection4addData( connection, NULL, sizeof( CONNECTION4GO_INFO_IN ), (void **)&info ) ;
   info->recNo = htonl(recNo) ;
   rc = connection4repeat( connection ) ;
   /* AS 06/22/98 rc == r4entry gets caucht in d4goVirtual - ex9.c test program
      in client/server, or see d4goVirtual() */
/*   if ( rc == r4locked || rc == r4entry ) */
   if ( rc == r4locked )
      return rc ;

   return d4goVirtual( data, recNo, rc, connection4data( connection ), connection ) ;
}

#else

int S4FUNCTION d4go( DATA4 *data, const long recNo )
{
   int rc, fromDisk ;
   CODE4 *c4 ;
   #ifndef S4OFF_MULTI
      int didLock ;
   #endif
   #ifndef S4OFF_MEMO
      int i ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E93101 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E93101 ) ;
   #endif

   c4 = data->codeBase ;
   rc = 0 ;

   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   #ifndef S4OFF_WRITE
      if ( data->recordChanged )
      {
         rc = d4updateRecord( data, 0 ) ;
         if ( rc )
            return rc ;
      }
      #ifndef S4OFF_MEMO
         else
      #endif
   #endif
      #ifndef S4OFF_MEMO
         if ( data->fieldsMemo != 0 )
            for ( i = 0; i < data->dataFile->nFieldsMemo; i++ )
               f4memoReset( data->fieldsMemo[i].field ) ;
      #endif

   if ( d4recCountLessEq( data, recNo ) == 0 )  /* past eof */
   {
      data->recNum = -1 ;  /* at an invalid position */
      d4blankLow( data, data->record ) ;
      #ifndef S4SERVER
         if ( c4->errGo )
            return error4describe( c4, e4read, E91102, d4alias( data ), 0, 0 ) ;
      #endif
      return r4entry ;
   }

   fromDisk = 0 ;
   #ifndef S4OFF_MULTI
      didLock = 0 ;

      if ( c4getReadLock( c4 ) )
      {
         #ifdef S4SERVER
            if ( !dfile4lockTest( data->dataFile, data4clientId( data ), data4serverId(data ), recNo ) )  /* record not already locked */
            {
               switch( code4unlockAuto( c4 ) )
               {
                  case LOCK4ALL :
                     rc = code4unlockDo( tran4dataList( data->trans ) ) ;
                     break ;
                  case LOCK4DATA :
                     rc = d4unlockLow( data, data4clientId( data ), 0 ) ;
                     break ;
                  default:
                     break ;
               }
               if ( rc < 0 )
                  return error4stack( c4, rc, E93101 ) ;

               rc = dfile4lock( data->dataFile, data4clientId( data ), data4serverId( data ), recNo ) ;
               if ( rc )
                  return rc ;
               didLock = 1 ;
            }
         #else
            if ( !d4lockTest( data, recNo ) )  /* record not already locked */
            {
               rc = d4lock( data, recNo ) ;
               if ( rc )
                  return rc ;
               didLock = 1 ;
            }
         #endif
      }

      #ifndef S4OFF_MEMO
         if ( !data->memoValidated )
            #ifdef S4SERVER
               if ( dfile4lockTest( data->dataFile, data4clientId( data ), data4serverId( data ), recNo ) )  /* record not already locked */
            #else
               #ifndef S4OFF_MULTI
                  if ( d4lockTest( data, recNo ) )  /* record not already locked */
               #endif
            #endif
               fromDisk = 1 ;
      #endif  /* S4OFF_MEMO */
   #endif

   rc = dfile4goData( data->dataFile, recNo, data->record, fromDisk ) ;

   if ( rc )
   {
      data->recNum = -1 ;  /* at an invalid position */
      #ifndef S4OFF_MULTI
         if ( didLock == 1 )
            #ifdef S4SERVER
               dfile4unlockRecord( data->dataFile, data4clientId( data ), data4serverId( data ), recNo ) ;
            #else
               d4unlockRecord( data, recNo ) ;
            #endif
      #endif
      return rc ;
   }
   else
      data->recNum = recNo ;

   data->bofFlag = data->eofFlag = 0 ;

   #ifndef S4OFF_MULTI
      #ifdef S4SERVER
         if ( dfile4lockTest( data->dataFile, data4clientId( data ), data4serverId( data ), recNo ) )  /* record not already locked */
      #else
         if ( d4lockTest( data, recNo ) )  /* record not already locked */
      #endif
      {
   #endif
      memcpy( data->recordOld, data->record, dfile4recWidth( data->dataFile ) ) ;
      data->recNumOld = data->recNum ;
   #ifndef S4OFF_MULTI
      #ifndef S4OFF_MEMO
         data->memoValidated = 1 ;
      #endif
      }
      #ifndef S4OFF_MEMO
         else
            data->memoValidated = 0 ;
      #endif
   #endif

   return 0 ;
}

/* fromDisk is set to true if a disk read is desired from the go */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int dfile4goData( DATA4FILE *data, long rec, void *record, int fromDisk )
{
   unsigned len ;
   CODE4 *cb ;

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E91102 ) ;
   #endif

   cb = data->c4 ;

   if( error4code( cb ) < 0 )
      return e4codeBase ;

   if ( rec <= 0 )
      len = 0 ;
   else
   {
      #ifndef S4OFF_MULTI
         #ifndef S4OPTIMIZE_OFF
            if ( fromDisk == 1 ) /* if bufferred old, and only record is locked, read from disk */
               if ( data->file.doBuffer && data->fileServerLock != 0 )
                  cb->opt.forceCurrent = 1 ;
         #endif
      #endif

      len = file4readInternal( &data->file, dfile4recordPosition( data, rec ), record, data->recWidth ) ;

      #ifndef S4OFF_MULTI
         #ifndef S4OPTIMIZE_OFF
            if ( fromDisk == 1 )
               cb->opt.forceCurrent = 0 ;
         #endif
      #endif

      if( error4code( cb ) < 0 )
         return -1 ;
   }

   if ( len != data->recWidth )
   {
      memset( record, ' ', data->recWidth ) ;  /* clear the partially read record to avoid corruption */
      #ifndef S4SERVER
         if ( cb->errGo )
            return error4describe( cb, e4read, E91102, data->file.name, 0, 0 ) ;
      #endif
      return r4entry ;
   }
   return 0 ;
}

int d4goData( DATA4 *data, long rec )
{
   int rc ;
   rc = dfile4goData( data->dataFile, rec, data->record, 0 ) ;
   if ( rc != 0 )
      data->recNum = -1 ;  /* at an invalid position */
   else
      data->recNum = rec ;
   return rc ;
}
#endif  /* S4CLIENT */

int  S4FUNCTION d4goEof( DATA4 *data )
{
   long count ;
   #ifndef S4OFF_WRITE
      int rc ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E93103 ) ;
   #endif

   if ( error4code( data->codeBase ) < 0 )
      return e4codeBase ;

   #ifndef S4OFF_WRITE
      /* AS 04/22/97 causes problems with t4seek, unlock reset to 0 */
      rc = d4updateRecord( data, 0 ) ;
      if ( rc )
         return rc ;
   #endif

   count = d4recCount( data ) ;
   if ( count < 0 )
      return -1 ;
   data->recNum = count + 1L ;
   data->eofFlag = 1 ;
   if ( data->recNum == 1 )
      data->bofFlag = 1 ;
   d4blankLow( data, data->record ) ;
   return r4eof ;
}
