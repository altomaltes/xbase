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

/* d4append.c  */

#include "d4all.h"

#ifndef S4OFF_WRITE

#ifndef S4OFF_MULTI

int S4FUNCTION d4lockAppendRecord( DATA4 *data )
{
   int rc, oldUnlock ;

   E4PARMLOW( data, E91101 ) ;

   if ( d4lockTestFile( data ) == 1 )
      return 0 ;

   rc = 0 ;

   /* AS 12/19/97 ole-db t5row6.cpp fails if we unlock before checking lock */
   if ( d4lockTestAppend( data ) == 0 || d4lockTest( data, d4recCount( data ) + 1 ) == 0 )
   {
      switch( code4unlockAuto( data->codeBase ) )
      {
         case LOCK4ALL :
            code4unlock( data->codeBase ) ;
         break ;

         case LOCK4DATA :
            rc = d4unlockLow( data, data4clientId( data ), 0 ) ;
         break ;

         default:
         break ;
      }

      oldUnlock = code4unlockAuto( data->codeBase ) ;
      if ( d4lockTestAppend( data ) == 0 )
      {
         rc = d4lockAppend( data ) ;

         if ( rc == 0 )
         {
            data->dataFile->numRecs = -1 ;
            code4unlockAutoSet( data->codeBase, 0 ) ;
            #ifndef S4OPTIMIZE_OFF
               data->codeBase->opt.forceCurrent = 1 ;  /* force the recCount to be current */
            #endif
               rc = d4lock( data, d4recCount( data ) + 1 ) ;

            #ifndef S4OPTIMIZE_OFF
               data->codeBase->opt.forceCurrent = 0 ;
            #endif
         }
      }
      else
      {
         code4unlockAutoSet( data->codeBase, 0 ) ;
            rc = d4lock( data, d4recCount( data ) + 1 ) ;
      }
      if ( rc )
            d4unlockData( data ) ;

      code4unlockAutoSet( data->codeBase, oldUnlock ) ;
   }

   return rc ;
}

#endif  /* S4OFF_MULTI */

static int dfile4appendData( DATA4FILE *data, const void *record, S4LONG *recNum )
{
   long count ;
   FILE4LONG pos ;
   int  rc ;

   E4PARMLOW( data && record && recNum, E91102 ) ;

   #ifdef S4OFF_MULTI
      count = dfile4recCount( data, 0L ) ;  /* returns -1 if error4code( codeBase ) < 0 */
   #else
      count = dfile4recCount( data, -2L ) ;  /* returns -1 if error4code( codeBase ) < 0 */
   #endif
   if ( count < 0L )
      return -1 ;
   data->fileChanged = 1 ;
   pos = dfile4recordPosition( data, count + 1L ) ;

   #ifdef S4FOX
      if ( data->version == 0x30 )   /* no eof marker */
         rc = file4writeInternal( &data->file, pos, record, ( data->recWidth ) ) ;
      else
         rc = file4writeInternal( &data->file, pos, record, ( data->recWidth + 1 ) ) ;
   #else
      rc = file4writeInternal( &data->file, pos, record, ( data->recWidth + 1 ) ) ;
   #endif

   if ( rc == 0 )
   {
      data->numRecs = count + 1L ;
      *recNum = data->numRecs ;
      #ifndef S4OFF_TRAN
         /* 04/24/96 AS --> 2 lines below added so that server datafile has correct
            min count when not a transaction.  t4commit.c */
         if ( code4tranStatus( data->c4 ) != r4active )
            data->minCount = data->numRecs ;
      #endif
   }

   return rc ;
}

static int d4doAppend( DATA4 *data )
{
   int rc ;
   CODE4 *c4 ;
   DATA4FILE *d4file ;
      #ifndef S4OFF_MEMO
         long newId ;
      #endif

      #ifndef S4INDEX_OFF
         int saveError ;
         TAG4 *tagOn ;
         TAG4FILE *t4file ;
         #ifndef S4OFF_MULTI
            int indexLocked ;
         #endif
      #endif

   #ifndef S4OFF_MEMO
      F4MEMO *mfield ;
      short i ;
   #endif /* S4OFF_MEMO */

   #ifndef S4OFF_MULTI
         long serverId ;
   #endif

   #ifdef S4DEMO
      if ( d4recCount( data ) >= 200L)
      {
         d4close( data ) ;
         return error4( data->codeBase, e4demo, 0 ) ;
      }
   #endif

   C4PARMDATA( data, E91103, -1 );

   #ifdef E4MISC
      if ( data->record[0] != ' ' && data->record[0] != '*' )
         return error4( data->codeBase, e4append, E83301 ) ;
   #endif

   c4 = data->codeBase ;
   d4file = data->dataFile ;

   #ifndef S4OFF_MULTI
      serverId = data4serverId( data ) ;
   #endif

   #ifdef E4ANALYZE
      if ( c4 == 0 )
         return error4( 0, e4struct, E91103 ) ;
   #endif

   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   if ( data->readOnly == 1 )
      return error4describe( c4, e4write, E80606, d4alias( data ), 0, 0 ) ;

      /* 1. Update index file
         2. Update memo File
         3. Update data file */

      #ifndef S4INDEX_OFF
         #ifdef N4OTHER
            if ( l4numNodes( &data->indexes ) != 0 )   /* there are indexes, so do them */
         #else
            if ( l4numNodes( &d4file->indexes ) != 0 )   /* there are indexes, so do them */
         #endif
         {
            #ifndef S4OFF_MULTI
                  indexLocked = d4lockTestIndex( data ) ;
               if ( !indexLocked )
               {
                     rc = d4lockIndex( data ) ;
                  if ( rc )
                  {
                        d4unlockAppend( data ) ;
                     return rc ;
                  }
               }
            #endif  /* not S4OFF_MULTI */

            for( tagOn = 0 ;; )
            {
               tagOn = d4tagNext( data, tagOn ) ;
               if ( !tagOn )
                  break ;
               t4file = tagOn->tagFile ;

               rc = expr4context( t4file->expr, data ) ;
               if ( rc == 0 )
                  if ( t4file->filter != 0 )
                     rc = expr4context( t4file->filter, data ) ;

               if ( rc == 0 )
                  rc = t4addCalc( tagOn, data->recNum ) ;
               if ( rc < 0 || rc == r4unique )
               {
                  saveError = error4set( c4, 0 ) ;

                  /* Remove the keys which were just added */
                  for(;;)
                  {
                     tagOn = d4tagPrev( data, tagOn ) ;
                     if ( !tagOn )
                        break ;
                     tfile4removeCalc( tagOn->tagFile, data->recNum ) ;
                  }

                  error4set( c4, (short)saveError ) ;
                  data->recNum = 0 ;
                  #ifndef S4OFF_MULTI
                        d4unlockAppend( data ) ;
                     if ( !indexLocked )
                        dfile4unlockIndex( d4file, serverId ) ;
                  #endif
                  return rc ;
               }
            }

            #ifndef S4OFF_MULTI
               if ( !indexLocked )
                  dfile4unlockIndex( d4file, serverId ) ;
            #endif  /* not S4OFF_MULTI */
         }
      #endif  /* not S4INDEX_OFF */


      #ifndef S4OFF_MEMO
         for ( i = 0 ; i < d4file->nFieldsMemo ; i++ )
         {
            mfield = data->fieldsMemo + i ;
            mfield->isChanged = 0 ;
            if ( mfield->len > 0 )
            {
               newId = 0L ;
               if ( memo4fileWrite( &d4file->memoFile, &newId, mfield->contents, mfield->len ) )  /* positive value means fail, so don't update field, but append record */
                  break ;
               f4assignLong( mfield->field, newId ) ;
            }
            else
            {
               #ifdef S4FOX
                  if ( d4version ( data ) == 0x30 )
                  {
                     if ( !f4null( mfield->field ) )  /* if null, then don't assign since that will replace the null flag */
                        f4assignLong( mfield->field, 0 ) ;
                  }
                  else
                     f4assign( mfield->field, " " ) ;
               #else
                  f4assign( mfield->field, " " ) ;
               #endif
            }
         }
      #endif  /* S4OFF_MEMO */

      data->record[dfile4recWidth( d4file )] = 0x1A ;
      rc = dfile4appendData( d4file, data->record, (S4LONG *)&data->recNum ) ;
      if ( rc == 0 )
      {
         data->recordChanged = 0 ;
         data->record[dfile4recWidth( d4file )] = 0 ;
      }

      #ifndef S4OFF_INDEX
         if ( rc != 0 )  /* must remove tag entries since the data append failed */
         {
            for( tagOn = 0 ;; )
            {
               tagOn = d4tagNext( data, tagOn ) ;
               if ( !tagOn )
                  break ;

               tfile4removeCalc( tagOn->tagFile, data->recNum ) ;
            }
         }
      #endif

      #ifndef S4OFF_MULTI
         if ( rc == 0 )
            if ( d4file->file.lowAccessMode != OPEN4DENY_RW )
               rc = dfile4updateHeader( d4file, 1, 1 ) ;

            d4unlockAppend( data ) ;
      #endif  /* S4OFF_MULTI */

      return rc ;
}

int S4FUNCTION d4append( DATA4 *data )
{
   #ifndef S4OFF_TRAN
      TRAN4 *trans = 0 ;
      int hasTran, saveRc ;
      long connectionId = 0L ;
      #ifndef S4OFF_MEMO
         unsigned long len ;
         long tempLong ;
         int i ;
      #endif
      CODE4 *c4 ;
   #endif
   #ifndef S4OFF_MULTI
      long count ;
   #endif
   int rc ;

   E4PARHIGH( data, E91103 ) ;

   #ifndef S4OFF_TRAN
      c4 = data->codeBase ;

      #ifdef E4ANALYZE
         if ( c4 == 0 )
            return error4( 0, e4struct, E91103 ) ;
      #endif

      if ( error4code( c4 ) < 0 )
         return e4codeBase ;

      #ifdef E4MISC
         if ( data->recNum )
            return error4( c4, e4result, E81103 ) ;
      #endif

      hasTran = 0 ;
      if ( data->logVal != LOG4TRANS )
         if ( code4transEnabled( c4 ) )
            if ( ( code4tranStatus( c4 ) == r4inactive ) )  /* start a mini-transaction */
            {
               rc = code4tranStartSingle( c4 ) ;
               if ( rc != 0 )
                  return rc ;
               hasTran = 1 ;
            }
   #endif

   #ifdef S4OFF_MULTI
      rc = 0 ;
   #else
      /* 09/27/95 AS - removed.  unlockAuto also applies to append
         bytes (t4lock.c, tran-dep) - see d4append locking description */
      /*
         appendLocked = dfile4lockTestAppend( data->dataFile, data4clientId( data), data4serverId( data ), data ) ;
      */
      rc = d4lockAppendRecord( data ) ;
   #endif  /* S4OFF_MULTI */

   if ( rc == 0 )
   {
      data->bofFlag = data->eofFlag = 0 ;
      data->recordChanged = 0 ;
      #ifdef S4OFF_MULTI
         data->recNum = d4recCount( data ) + 1 ;
      #else
         count = d4recCount( data ) + 1 ;
         data->recNum = count ;
      #endif

      #ifndef S4OFF_TRAN
         if ( code4transEnabled( c4 ) )
            if ( code4tranStatus( c4 ) == r4active )
            {
               trans = code4trans( c4 ) ;
                  connectionId = 0L ;
               rc = tran4set( trans, trans->currentTranStatus, -1L, connectionId, TRAN4APPEND,
                             sizeof( data->recNum ) + (unsigned int)dfile4recWidth( data->dataFile ), data4clientId( data ), data4serverId( data ) ) ;
               if ( rc < 0 )
               {
                  if ( hasTran )
                     code4tranRollbackSingle( c4 ) ;
                  return rc ;
               }
               if ( tran4putData( trans, &data->recNum, sizeof( data->recNum ) ) == e4memory )
               {
                  if ( hasTran )
                     code4tranRollbackSingle( c4 ) ;
                  return e4memory ;
               }
               if ( tran4putData( trans, d4record( data ), (unsigned int)dfile4recWidth( data->dataFile ) ) == e4memory )
               {
                  if ( hasTran )
                     code4tranRollbackSingle( c4 ) ;
                  return e4memory ;
               }
               #ifndef S4OFF_MEMO
                  len = trans->header.dataLen ;
                  for ( i = 0; i < data->dataFile->nFieldsMemo; i++ )
                  {
                     len += ( data->fieldsMemo[i].len + sizeof( unsigned long ) ) ;
                     tempLong = data->fieldsMemo[i].len;
                     if ( tran4putData( trans, &tempLong, sizeof( unsigned long ) ) == e4memory )
                     {
                        if ( hasTran )
                           code4tranRollbackSingle( c4 ) ;
                        return e4memory ;
                     }
                     if ( data->fieldsMemo[i].len != 0 )
                        if ( tran4putData( trans, data->fieldsMemo[i].contents, data->fieldsMemo[i].len ) == e4memory )
                        {
                           if ( hasTran )
                              code4tranRollbackSingle( c4 ) ;
                           return e4memory ;
                        }
                  }
                  trans->header.dataLen = len ;
               #endif
               #ifdef S4OFF_OPTIMIZE
                  if ( tran4lowAppend( trans, 0, 1 ) != 0 )
               #else
                  if ( tran4lowAppend( trans, 0, ( ( data->dataFile->file.bufferWrites == 1 && data->dataFile->file.doBuffer == 1) ? 0 : 1 ) ) != 0 )
               #endif
               {
                  if ( hasTran )
                     code4tranRollbackSingle( c4 ) ;
                  return e4transAppend ;
               }
         }
      #endif

      rc = d4doAppend( data ) ;

      #ifndef S4OFF_TRAN
         if ( rc < 0 || rc == r4unique )
         {
            if ( code4transEnabled( c4 ) )
               if ( code4tranStatus( c4 ) == r4active )
               {
                  saveRc = tran4set( trans, trans->currentTranStatus, -1L, connectionId, TRAN4VOID, 0, data4clientId( data ), data4serverId( data ) ) ;
                  if ( saveRc < 0 )
                  {
                     if ( hasTran )
                        code4tranRollbackSingle( c4 ) ;
                     return saveRc ;
                  }
                  #ifdef S4OFF_OPTIMIZE
                     if ( tran4lowAppend( trans, "\0", 1 ) != 0 )
                  #else
                     if ( tran4lowAppend( trans, "\0", ( ( data->dataFile->file.bufferWrites == 1 && data->dataFile->file.doBuffer == 1) ? 0 : 1 ) ) != 0 )
                  #endif
                  {
                     if ( hasTran )
                        code4tranRollbackSingle( c4 ) ;
                     return e4transAppend ;
                  }
               }
         }
      #endif
   }
   #ifndef S4OFF_MULTI
      else
         count = -1 ;
   #endif

   #ifndef S4OFF_TRAN
      if ( hasTran )
      {
         code4tranCommitSingle( c4 ) ;
         #ifndef S4OFF_MULTI
            if ( code4unlockAuto( c4 ) != LOCK4OFF )
            {
                  saveRc = d4unlockAppend( data ) ;
               #ifndef S4OFF_INDEX
                  if ( saveRc == 0 )
                     saveRc = dfile4unlockIndex( data->dataFile, data4serverId( data ) ) ;
               #endif
               if ( saveRc < 0 )
                  return saveRc ;
            }
         #endif /* S4OFF_MULTI */
      }
   #endif

   #ifndef S4OFF_MULTI
      if ( count != -1 && ( rc < 0 || rc == r4unique ) )   /* append failed, so unlock record */
         dfile4unlockRecord( data->dataFile, data4clientId( data ), data4serverId( data ), count ) ;
   #endif

   return rc ;
}

int S4FUNCTION d4appendBlank( DATA4 *data )
{
   int rc ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E91104 ) )
         return -1 ;
   #endif  /* S4VBASIC */

   E4PARHIGH( data, E91104 ) ;

   #ifdef S4DEMO
      if ( d4recCount( data ) >= 200L)
      {
         d4close( data ) ;
         return error4( data->codeBase, e4demo, 0 ) ;
      }
   #endif

   rc = d4appendStart( data, 0 ) ;  /* updates the record, returns -1 if error4code( codeBase ) < 0 */
   if ( rc )
      return rc ;

   d4blank( data ) ;   /* make sure goes through f4blank() for non-character field types */
   return d4append( data ) ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4appendStart( DATA4 *data, int useMemoEntries )
{
   CODE4 *c4 ;
      int rc ;
      #ifndef S4OFF_MEMO
         char *savePtr ;
         int i, oldLockEnforce ;
      #endif

   C4PARMDATA( data, E91107, -1 ); 

   c4 = data->codeBase ;

   #ifdef E4ANALYZE
      if ( c4 == 0 )
         return error4( 0, e4struct, E91107 ) ;
   #endif

   #ifdef S4DEMO
      if ( d4recCount( data ) >= 200L)
      {
         d4close( data ) ;
         return error4( c4, e4demo, 0 ) ;
      }
   #endif

      rc = d4updateRecord( data, 1 ) ;   /* returns -1 if error4code( c4 ) < 0 */
      if ( rc )
         return rc ;

      #ifndef S4OFF_MEMO
         if ( data->dataFile->nFieldsMemo != 0 )
         {
            for ( i = 0; i < data->dataFile->nFieldsMemo; i++ )
               f4memoReset( data->fieldsMemo[i].field ) ;

            if ( useMemoEntries == 1 )
            {
               if ( data->recNum > 0 && !d4eof( data ) && !d4bof( data ) )
               {
                     #ifdef E4ANALYZE
                        if ( !file4openTest( &data->dataFile->memoFile.file ) )
                           return error4( c4, e4data, E81101 ) ;
                     #endif

                  /* Read in the current memo entries of the current record */
                  #ifndef S4OFF_MULTI
                     rc = d4lock( data, data->recNum ) ;
                     if ( rc )
                        return rc ;
                  #endif  /* S4OFF_MULTI */

                  savePtr = data->record ;
                  data->record = data->recordOld ;

                     d4goData( data, data->recNum ) ;

                  for ( i = 0 ; i < data->dataFile->nFieldsMemo ; i++ )
                  {
                        f4memoReadLow( data->fieldsMemo[i].field ) ;
                     data->fieldsMemo[i].status = 0 ;
                  }

                  data->record = savePtr ;

                  if ( error4code( c4 ) < 0 )
                     return error4stack( c4, error4code( c4 ), E91107 ) ;
               }
            }
         }

         oldLockEnforce = c4->lockEnforce ;
         c4->lockEnforce = 0 ;
         /* no matter what, must clear the memo entries from the data file */
         for ( i = 0 ; i < data->dataFile->nFieldsMemo ; i++ )
            f4assignLong( data->fieldsMemo[i].field, 0 ) ;
         c4->lockEnforce = oldLockEnforce ;
      #endif  /* not S4OFF_MEMO */

      data->recNum = 0 ;

      #ifndef S4OFF_MULTI
               if ( d4lockTestFile( data ) != 1 )
                  rc = d4unlockData( data ) ;
            if ( rc )
               return rc ;
         #endif

   return 0 ;
}

#ifndef S4OFF_TRAN
int S4FUNCTION d4unappend( DATA4 *data )
{
   int rc, saveError ;
   CODE4 *c4 ;
   #ifndef S4INDEX_OFF
      TAG4 *tagOn ;
      #ifndef S4OFF_MULTI
         int indexLocked ;
      #endif
   #endif
   #ifndef S4OFF_MEMO
      int i ;
   #endif

   C4PARMDATA( data, E91108, -1 );

   c4 = data->codeBase ;
   rc = 0 ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   /* 0. Lock record count bytes
      1. Update index file
      2. Update memo File
      3. Update data file */

   #ifndef S4OFF_MULTI
         d4lockAppend( data ) ;
         d4lock( data, data->dataFile->numRecs ) ;
   #endif  /* S4OFF_MULTI */

   data->bofFlag = data->eofFlag = 0 ;
   data->recordChanged = 1 ;

   data->recNum = d4recCount( data ) ;
   if ( data->recNum < 0 )
      return (int)data->recNum ;
   data->count = data->recNum - 1 ; /* reset the count approximator */
   data->dataFile->numRecs = data->count ;   /* put back in for t4comit3...double append/unappend requires */
   saveError = 0 ;

   #ifndef S4INDEX_OFF
      #ifndef S4OFF_MULTI
            indexLocked = d4lockTestIndex( data ) ;
            if ( !indexLocked )
            {
               rc = d4lockIndex( data ) ;
               if ( rc )
               {
                  d4unlockAppend( data ) ;
                  return rc ;
               }
            }
      #endif  /* not S4OFF_MULTI */

      for( tagOn = 0 ;; )
      {
         tagOn = d4tagNext( data, tagOn ) ;
         if ( !tagOn )
            break ;

         rc = expr4context( tagOn->tagFile->expr, data ) ;
         if ( rc == 0 )
            if ( tagOn->tagFile->filter != 0 )
               rc = expr4context( tagOn->tagFile->filter, data ) ;

         if ( rc == 0 )
            rc = tfile4removeCalc( tagOn->tagFile, data->recNum ) ;
         if ( rc < 0 )
            saveError = error4set( c4, 0 ) ;
      }

      #ifndef S4OFF_MULTI
         if ( !indexLocked )
            dfile4unlockIndex( data->dataFile, tran4serverDataId( data->trans ) ) ;
      #endif
   #endif  /* not S4INDEX_OFF */

   #ifndef S4OFF_MEMO
      if ( data->dataFile->nFieldsMemo > 0 )
      {
         #ifndef S4OFF_MULTI
            if ( ( rc = d4validateMemoIds( data ) ) != 0 )
            {
               data->recordChanged = 0 ;
               return rc ;
            }
         #endif

         /* Cycle through the fields to be flushed */
         for ( i = 0; i < data->dataFile->nFieldsMemo; i++ )
         {
            f4memoAssignN( data->fieldsMemo[i].field, "\0", 0 ) ;
            rc = f4memoUpdate( data->fieldsMemo[i].field ) ;
            if ( rc < 0 )
               saveError = error4set( c4, 0 ) ;
         }
      }
   #endif  /* S4OFF_MEMO */

   data->record[dfile4recWidth( data->dataFile )] = 0x1A ;

   #ifndef S4OFF_MULTI
         rc = dfile4unappendData( data->dataFile, data->dataFile->fileClientLock, data->dataFile->fileServerLock ) ;
   #endif

   #ifndef S4OFF_MULTI
      if ( rc == 0 )
         if ( data->dataFile->file.lowAccessMode != OPEN4DENY_RW )
            rc = dfile4updateHeader( data->dataFile, 1, 1 ) ;
   #endif

   #ifndef S4OFF_MULTI
         d4unlockAppend( data ) ;
   #endif  /* S4OFF_MULTI */

   error4set( c4, (short)saveError ) ;

   /* reset to an invalid position */
   data->recNum = data->recNumOld = -1 ;
   data->recordChanged = 0 ;
   data->bofFlag = data->eofFlag = 0 ;

   d4blankLow( data, data->record ) ;

   return rc ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int dfile4unappendData( DATA4FILE *data, const long clientId, const long serverId )
{
   long count ;
   FILE4LONG pos ;
   int  rc ;
   char endMark;

   E4PARMLOW( data, E91102 ) ;

   count = dfile4recCount( data, serverId ) ;
   if ( count < 0L )
      return error4stack( data->c4, (int)count, E91102 ) ;
   data->fileChanged = 1 ;
   pos = dfile4recordPosition( data, count + 1 ) ;

   rc = file4lenSetLow( &data->file, pos ) ;
   endMark = 0x1A;
   rc = file4writeInternal( &data->file, pos, &endMark, 1 ) ;

   if ( rc >= 0 )
   {
      #ifndef S4OFF_MULTI
         if ( dfile4lockTestAppend( data, clientId, serverId ) )
      #endif  /* S4OFF_MULTI */
      data->numRecs = count ;
   }

   return rc ;
}

#endif  /* S4OFF_TRAN */
#endif  /* S4OFF_WRITE */
