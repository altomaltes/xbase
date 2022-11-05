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

/* d4go.c   (c)Copyright Sequiter Software Inc., 1988-2001.  All rights reserved. */

#include "d4all.h"


int S4FUNCTION d4goLow( DATA4 *data, const long recNo, short goForWrite )
{
   // d4goLow() is exported, cannot change it, but call new d4goLow2 function...
   return d4goLow2( data, recNo, goForWrite, 0 ) ;
}



   int S4FUNCTION d4goLow2( DATA4 *data, const long recNo, short goForWrite, Bool5 assumeLocked )
   {
      /* if goForWrite is false, then the purpose of the d4go is for reading only
         In general in CodeBase we go for both purposes (i.e. the current record
         can be used for writing at any time).  However, on the server and OLE-DB,
         we are d4go'ing for purposes of reading only

         Basically either D4goRead() or D4goWrite() should be called

         AS 09/17/99 assumeLocked added to avoid lock-testing to improve performance in OLE-DB...
      */
      #ifdef E4VBASIC
         if ( c4parm_check( data, 2, E93101 ) )
            return -1 ;
      #endif

      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm_null, E93101 ) ;
         // AS Jan 23/06 - allow the server to go to a -1 position (used to synch up with client)
         #ifndef S4SERVER
            if ( recNo < 0 )
               return error4( data->codeBase, e4parm, E93101 ) ;  // AS Mar 10/06 - use the CODE4 in the data
         #endif
      #endif

      CODE4 *c4 = data->codeBase ;

      if ( error4code( c4 ) < 0 )
         return e4codeBase ;

      int rc = 0 ;

      #ifndef S4OFF_WRITE
         if ( data->recordChanged )
         {
            rc = d4updateRecord( data, 0, 1 ) ;
            if ( rc )
               return rc ;
         }
         #ifndef S4OFF_MEMO
            else
         #endif
      #endif
         #ifndef S4OFF_MEMO
            if ( data->fieldsMemo != 0 )
            {
               for ( int memoFieldIndex = 0 ; memoFieldIndex < data->dataFile->nFieldsMemo ; memoFieldIndex++ )
               {
                  f4memoReset( data->fieldsMemo[memoFieldIndex].field ) ;
               }
            }
         #endif

      if ( recNo < 0 || d4recCountLessEq( data, recNo ) == 0 )  /* past eof */  // AS Jan 23/06 - if recNo < 0 this also applies (server case)
      {
         data->recNum = -1 ;  /* at an invalid position */
         d4blankLow( data, data->record ) ;
         #ifndef S4SERVER
            if ( c4->errGo )
               return error4describe( c4, e4read, E91102, d4alias( data ), 0, 0 ) ;
         #endif
         return r4entry ;
      }

      int fromDisk = 0 ;
      #ifndef S4OFF_MULTI
         int weHaveRecordLocked ;
         if ( assumeLocked )
            weHaveRecordLocked = 1 ;
         else
            weHaveRecordLocked = -1 ;
         int didLock = 0 ;

         if ( c4getReadLock( c4 ) )
         {
            #ifdef S4SERVER
               int rc2 = d4lockTest( data, recNo, lock4read ) ;
               if ( rc2 != 1 )
                  rc2 = d4lockTest( data, recNo, lock4write ) ;
               if ( rc2 != 1 )  /* record not already locked - if r4locked, still do unlock to avoid deadlock */
               {
                  switch( code4unlockAuto( c4 ) )
                  {
                     case LOCK4ALL :
                        rc = code4unlockDo( tran4dataList( data->trans ) ) ;
                        break ;
                     case LOCK4DATA :
                        // AS Apr 15/03 - support for new lockId for shared clone locking
                        rc = d4unlockLow( data, data4lockId( data ), 0 ) ;
                        break ;
                     default:
                        break ;
                  }
                  if ( rc < 0 )
                     return rc ;

                  if ( rc2 != 0 )  /* r4locked - another user has locked */
                     return rc2 ;
                  rc = d4lockInternal( data, recNo, 0 ) ;
                  if ( rc )
                     return rc ;
                  didLock = 1 ;
               }
            #else
               rc = d4lockInternal( data, recNo, 1, lock4any  ) ;
               if ( rc )
                  return rc ;
               didLock = 1 ;
            #endif
         }

         #ifndef S4OFF_MEMO
            if ( !data->memoValidated )
            {
               if ( weHaveRecordLocked == -1 )  /* haven't checked yet */
                  weHaveRecordLocked = d4lockTest( data, recNo, lock4write ) ;
               if ( weHaveRecordLocked == 1 )  /* record not already locked */
                  fromDisk = 1 ;
            }
         #endif  /* S4OFF_MEMO */
      #endif

      #if defined( S4TESTING ) || defined( E4ANALYZE )
         // AS Jun 24/02 - function to perform data movement counting for testing
         #ifdef S4SERVER
            c4->currentClient->testCount++ ;
         #else
            c4->testCount++ ;
         #endif
      #endif

      rc = dfile4goData( data->dataFile, recNo, data->record, fromDisk ) ;

      if ( rc != 0 )
      {
         data->recNum = -1 ;  /* at an invalid position */
         #ifndef S4OFF_MULTI
            if ( didLock == 1 )
               d4unlockRecord( data, recNo ) ;
         #endif
         return rc ;
      }
      else
         data->recNum = recNo ;

      data->bofFlag = data->eofFlag = 0 ;

      if ( goForWrite == 1 )
      {
         #ifndef S4OFF_MULTI
            if ( weHaveRecordLocked == -1 )  /* haven't checked yet */
               weHaveRecordLocked = d4lockTest( data, recNo, lock4write ) ;
            if ( weHaveRecordLocked == 1 )  /* record not already locked */
            {
         #endif
               c4memcpy( data->recordOld, data->record, dfile4recWidth( data->dataFile ) ) ;
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
      }
      else
         // AS 06/09/00 was not compiling in S4OFF_MULTI
         #if !defined( S4OFF_MEMO ) && !defined( S4OFF_MULTI )
            data->memoValidated = 0 ;
         #else
            ;
         #endif

      return 0 ;
   }



   #ifdef P4ARGS_USED
      #pragma argsused
   #endif
   int dfile4goData( DATA4FILE *data, long rec, void *record, int fromDisk )
   {
      /* fromDisk is set to true if a disk read is desired from the go */
      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm_null, E91102 ) ;
      #endif

      CODE4 *cb = data->c4 ;

      if( error4code( cb ) < 0 )
         return e4codeBase ;

      unsigned len ;

      if ( rec <= 0 )
         len = 0 ;
      else
      {
         #ifndef S4OFF_MULTI
            #ifndef S4OFF_OPTIMIZE
               // AS 05/20/99 --> looks incorrect - was forcing current if was bufferred and file was locked or read-locks existed.
               // instead, those conditions mean ok (i.e. we have it locked or file is locked - don't need to update)... - t5transmm was failing
               // if ( fromDisk == 1 ) /* if bufferred old, and only record is locked, read from disk */
               //    if ( data->file.doBuffer && ( data->fileServerWriteLock != 0 || data->fileReadLocks.initIterate() != 0 ) )
               //       cb->opt.forceCurrent = 1 ;
               if ( fromDisk == 1 ) /* if bufferred old, and only record is locked, read from disk */
                  if ( data->file.doBuffer && ( data->fileServerWriteLock == 0 || data->fileReadLocks.initIterate() == 0 ) )
                     cb->opt.forceCurrent = 1 ;
            #endif
         #endif

         len = file4readInternal( &data->file, dfile4recordPosition( data, rec ), record, data->recWidth ) ;

         #ifndef S4OFF_MULTI
            #ifndef S4OFF_OPTIMIZE
               if ( fromDisk == 1 )
                  cb->opt.forceCurrent = 0 ;
            #endif
         #endif

         if( error4code( cb ) < 0 )
            return -1 ;
      }

      if ( len != data->recWidth )
      {
         c4memset( record, ' ', data->recWidth ) ;  /* clear the partially read record to avoid corruption */
         #ifndef S4SERVER
            if ( cb->errGo )
            {
               FILE4LONG pos = dfile4recordPosition( data, rec ) ;
               long posLong = file4longGetLo( pos ) ;
               char nBuf[80] ;

               // AS Dec 13/05 - under Windows sprintf is becoming deprecated...
               #ifdef S4WINDOWS_VS5_PLUS
                  sprintf_s( nBuf, "Pos: %ld, Len: %ld", posLong, data->recWidth ) ;
               #else
                  sprintf( nBuf, "Pos: %ld, Len: %ld", posLong, data->recWidth ) ;
               #endif

               return error4describe( cb, e4read, E91102, data->file.name, nBuf, 0 ) ;
            }
         #endif
         return r4entry ;
      }
      return 0 ;
   }



   int d4goData( DATA4 *data, long rec )
   {
      int rc = dfile4goData( data->dataFile, rec, data->record, 0 ) ;
      if ( rc != 0 )
         data->recNum = -1 ;  /* at an invalid position */
      else
         data->recNum = rec ;
      return rc ;
   }



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
      rc = d4updateRecord( data, 0, 1 ) ;
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

   int S4FUNCTION d4goDirectRead( DATA4 *data, const long recNo, char *buffer, int fromDisk )
   {
      // this function is currently used by the ODBC driver only for reading a record (not for write record retrieval)
      // the purpose of this function is to do a direct record retrieval from disk into the location
      // pointed to by buffer.
      // there is no locking or updating of currect record, etc.
      // this function assumes that any server-side security verficication has been done already (i.e. assumes
      // the caller has security clearance to call this function).
      //
      // caller indicates 'fromDisk' input whether to read from disk or whether a bufferred copy would be ok
      //  (usually this depends on isolation levels)
      //
      // returns eof/bof if recno at an invalid spot, else returns 0/error (no r4locked since lock checking not done)
      //

      #ifdef E4PARM_LOW
         if ( data == 0 || recNo <= 0 || buffer == 0 || fromDisk > 1 || fromDisk < 0 )
            return error4( 0, e4parm_null, E93101 ) ;
      #endif

      if ( error4code( data->codeBase ) < 0 )
         return e4codeBase ;

      if ( d4recCountLessEq( data, recNo ) == 0 )  /* past eof */
         return r4eof ;

      return dfile4goData( data->dataFile, recNo, buffer, fromDisk ) ;
   }

