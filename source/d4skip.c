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

/* d4skip.c   (c)Copyright Sequiter Software Inc., 1988-2001.  All rights reserved. */

#include "d4all.h"

#ifdef S4SERVER
   long d4skipRecno( DATA4 *data, long n )
   {
      /* -2 means eof, -3 means bof */
      #ifndef S4OFF_INDEX
         TAG4 *tag ;
         TAG4FILE *tagFile ;
      #endif
      long startRec, newRec ;
      #ifndef S4OFF_INDEX
         unsigned char *keyValue ;
         int rc ;
         long recno, nSkipped ;
      #endif
      CODE4 *c4 ;

      #ifdef E4VBASIC
         if ( c4parm_check( data, 2, E94801 ) )
            return -1 ;
      #endif

      #ifdef E4PARM_LOW
         if ( data == 0 )
            return error4( 0, e4parm_null, E94801 ) ;
      #endif

      c4 = data->codeBase ;
      if ( c4 == 0 )
         return e4info ;
      if ( error4code( c4 ) < 0 )
         return error4code( c4 ) ;

      if ( data->recNum < 1L )
      {
         #ifndef S4SERVER
            if ( c4->errSkip )
               return error4( c4, e4info, E84801 ) ;
         #endif
         return e4info ;
      }

      #ifndef S4OFF_INDEX
         tag = data->tagSelected ;  /* avoid function call */
         if ( tag == 0 )
         {
      #endif
         if ( n != 0L )
            if ( d4recCountLessEq( data, 1L ) == 0 )  /* count == 0 */
               return -4L ;

         data->bofFlag = 0 ;
         startRec = data->recNum ;
         newRec = startRec + n ;

         if ( newRec > 0L )
         {
            if ( d4recCountLessEq( data, newRec ) == 0 )
               return -2L ;
            return newRec ;
         }
         else
            return -3L ;
      #ifndef S4OFF_INDEX
         }
         else
         {
            tagFile = tag->tagFile ;
            if ( data->eofFlag )
            {
               if ( n >= 0L )
                  return -2L ;

               rc = d4bottom( data ) ;
               if ( rc && rc != r4eof )
                  return rc ;
               if ( rc == r4eof )
                  return -2L ;
               n++ ;
               data->recNum = (long)tfile4recNo( tagFile ) ;
            }

            data->bofFlag = 0 ;

            #ifndef S4OFF_WRITE
               if ( data->recordChanged )
               {
                  /* AS 04/22/97 causes problems with t4seek, unlock reset to 0 */
                  rc = d4updateRecord( data, 0, 1 ) ;
                  if ( rc < 0 )
                     return rc ;
               }
            #endif

            t4versionCheck( tag, 1, 0 ) ;

            if ( n == 0 )
               return data->recNum ;

            if ( (long)tfile4recNo( tagFile ) != data->recNum )
            {
               if ( d4lockTest( data, data->recNum, lock4any ) != 1 )  /* ensure latest from disk */
               {
                  rc = d4go( data, data->recNum ) ;
                  if ( rc < 0 )
                     return rc ;
               }

               expr4context( tagFile->expr, data ) ;
               tfile4exprKey( tagFile, &keyValue ) ;

               rc = tfile4go( tagFile, keyValue, data->recNum, 0 ) ;
               if ( rc < 0 )
                  return rc ;

               if ( tfile4empty( tagFile ) )
                  return -4L ;

               if ( tfile4eof( tagFile ) && n >= 0L )
                  return -2L ;

               #ifdef S4HAS_DESCENDING
                  if ( tagFile->header.descending )
                  {
                     if ( (rc > 0) && (n < 0) )
                        n-- ;
                  }
                  else
                     if ( (rc > 0) && (n > 0) )
                        n-- ;
               #else
                  if ( (rc > 0) && (n > 0) )
                     n-- ;
               #endif
            }
            else
            {
               if ( tfile4eof( tagFile ) )
                  return -2L ;
            }

            nSkipped = tfile4dskip( tagFile, n ) ;

            if ( n > 0 && nSkipped != n )
               return -2L ;
            if ( n < 0 && nSkipped != n )
               return -3L ;

            if ( tfile4eof( tagFile ) )
               return -2L ;

            recno = (long)tfile4recNo( tagFile ) ;
            if ( recno < 0 )
               return recno ;
            return recno ;
         }
      #endif
   }
#endif /* S4SERVER */



/* flush the current record to disk, and update the data pointer to point
   to a valid tag location (in case the current record has been filtered out */
/* direction is -1 if line up backwards, or 1 if line up forwards */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4tagSync( DATA4 *data, TAG4 * const tag )
{
   #ifdef S4OFF_INDEX
      return 0 ;
   #else
      CODE4 *c4 ;

      #ifdef E4VBASIC
         if ( c4parm_check( data, 2, E94803 ) )
            return -1 ;
      #endif

      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm_null, E94803 ) ;
      #endif

      if ( tag == 0 )
         return 0 ;

      c4 = data->codeBase ;
      if ( error4code( c4 ) < 0 )
         return error4code( c4 ) ;

         return d4tagSyncDo( data, tag, 1 ) ;
   #endif /* S4OFF_INDEX */
}

#if !defined(S4OFF_INDEX)
   #ifdef P4ARGS_USED
      #pragma argsused
   #endif
   int d4tagSyncDo( DATA4 *data, TAG4 * const tag, int direction )
   {
      int rc, saveRc = 0 ;
      unsigned char *keyValue ;
      #ifndef S4OFF_MULTI
         int done, verifyReccount ;
         #ifndef S4OFF_TRAN
            int nSkipped ;
            CODE4 *c4 ;
         #endif
      #endif
      TAG4 *oldSelected ;
      TAG4FILE *tagFile ;
      long recno ;

      #if !defined(S4OFF_MULTI) && !defined(S4OFF_TRAN)
         c4 = data->codeBase ;
      #endif

      // AS Sep 8/08 - update the record prior to positioning to avoid being in an invalid position
      #ifndef S4OFF_WRITE
         if ( data->recordChanged )
         {
            rc = d4updateRecord( data, 0, 1 ) ;
            if ( rc )
            {
               return rc ;
            }
         }
      #endif

      tagFile = tag->tagFile ;
      expr4context( tagFile->expr, data ) ;
      tfile4exprKey( tagFile, &keyValue ) ;
      recno = (long)tfile4recNo( tagFile ) ;
      if ( recno != data->recNum )
      {
         rc = tfile4go( tagFile, keyValue, data->recNum, 0 ) ;
         if ( rc < 0 )
            return rc ;
         if ( rc == r4after )
            saveRc = r4after ;
      }

      oldSelected = data->tagSelected ;
      d4tagSelect( data, tag ) ;

      if ( data->eofFlag )
      {
         d4tagSelect( data, oldSelected ) ;

         /* if forward skip not true, not eof, here is */
         return d4goEof( data ) ;
      }
      else
      {
         t4versionCheck( tag, 1, 0 ) ;

         if ( tfile4eof( tagFile ) )
            return d4goEof( data ) ;

         recno = (long)tfile4recNo( tagFile ) ;
         if ( recno != data->recNum )
         {
            rc = d4go( data, recno ) ;
            saveRc = r4after ;
         }
         else
            rc = 0 ;
      }

      #ifndef S4OFF_MULTI
         #ifdef S4SERVER
            // AS Apr 15/03 - support for new lockId for shared clone locking
            if ( dfile4lockTestFile( data->dataFile, data4lockId( data ), data4serverId( data ), lock4write ) != 1 )
            if ( dfile4lockTestFile( data->dataFile, data4lockId( data ), data4serverId( data ), lock4read ) != 1 )
         #else
            if ( d4lockTestFile( data ) != 1 )
         #endif
            for( verifyReccount = 1, done = 0 ; done == 0 ; )
            {
               #ifndef S4OFF_TRAN
                  /* if transactions are enabled, we do not remove keys from an index file when
                     records are changed to avoid unique problems caused by another user.
                     The result of this is we may need to re-sync ourselves in that case - namely,
                     we do not want to position to a record indicated by a 'removed' entry in
                     a tag.
                  */
                  // AS Apr 29/03 - transcations are run-time in odbc now
                  // AS Jun 20/03 - was checking wrong flag
                  #if defined( S4SERVER ) && defined( S4ODBC_BUILD )
                     if ( c4->server->odbcTrans == 1 )  // odbc build, no trans available
                  #endif
                        if ( code4transEnabled( c4 ) )
                        {
                           if ( t4unique( tag ) != 0 || tagFile->filter != 0 )
                              if ( code4tranStatus( c4 ) == r4active && d4lockTest( data, recno, lock4write ) == 1 )
                              {
                                 /* if within a transaction, and record is locked, it may mean
                                    that the record contents have changed.  If the current tag key
                                    does not match, it must be an old entry (if a unique tag),
                                    and thus skip over
                                 */
                                 for ( ;; )
                                 {
                                    if ( recno > dfile4getMinCount( data->dataFile ) && verifyReccount == 1 )   /* ensure correct sequencing for multi-user */
                                       break ;  /* just let the code below take care of this case */

                                    Bool5 passedFilterTest = 1 ;
                                    Bool5 passedUniqueTest = 1 ;
                                    /* AS 03/25/99 --> also must consider instance where a record now becomes
                                       filtered from the record set...*/
                                    if ( tagFile->filter != 0 )
                                    {
                                       expr4context( tagFile->filter, data ) ;
                                       if ( recno != d4recNo( data ) )  /* need to reposition data4 first */
                                           d4go( data, recno ) ;
                                       if ( expr4true( tagFile->filter ) == 0 ) // should be filtered
                                       {
                                          nSkipped = (int)tfile4skip( tagFile, direction ) ;
                                          passedFilterTest = 0 ;
                                       }
                                    }

                                    if ( t4unique( tag ) != 0 && passedFilterTest == 1 )
                                    {
                                       expr4context( tagFile->expr, data ) ;
                                       if ( recno != d4recNo( data ) )  /* need to reposition data4 first */
                                           d4go( data, recno ) ;
                                       tfile4exprKey( tagFile, &keyValue ) ;
                                       if ( memcmp( tfile4key( tagFile ), keyValue, tagFile->header.keyLen ) != 0 )  /* different, so skip */
                                       {
                                          nSkipped = (int)tfile4skip( tagFile, direction ) ;
                                          passedUniqueTest = 0 ;
                                       }
                                    }

                                    if ( passedUniqueTest == 1 && passedFilterTest == 1 )  // done
                                       break ;

                                    verifyReccount = 1 ;  /* don't re-check reccount if all is ok... */
                                    if ( direction > 0  &&  nSkipped != 1 )
                                    {
                                       rc = d4goEof( data ) ;
                                       done = 1 ;
                                       break ;
                                    }

                                    if ( tfile4eof( tagFile ) )
                                    {
                                       data->bofFlag = 1 ;
                                       rc = d4goEof( data ) ;
                                       done = 1 ;
                                       break ;
                                    }
                                    if ( (long)tfile4recNo( tagFile ) != recno ) /* ok, may continue */
                                    {
                                       recno = (long)tfile4recNo( tagFile ) ;
                                       d4go( data, recno ) ;
                                       // AS Jan 13/09 - we may not be done yet....example case is that this record also doesn't belong to the tag in which case we need to still keep going forward
                                       continue ;
                                    }
                                 }
                              }
                        }
               #endif /* S4OFF_TRAN */

               if ( done == 1 )
                  break ;

               if ( verifyReccount == 1 )
               {
                  verifyReccount = 0 ;
                  if ( recno > dfile4getMinCount( data->dataFile ) )   /* ensure correct sequencing for multi-user */
                  {
                     while ( d4recCountLessEq( data, recno ) == 0 && done == 0 && rc == 0 )
                     {
                        // AS July 16/02 - incrrectly returning -1 in this case.
                        rc = (int)tfile4skip( tagFile, direction ) ;
                        if ( rc == -direction )
                           done = 1 ;
                        else
                        {
                           if ( rc == 0 )
                           {
                              data->bofFlag = 1 ;
                              done = 1 ;
                              rc = d4goEof( data ) ;
                           }
                           else
                           {
                              recno = (long)tfile4recNo( tagFile ) ;
                              if ( recno <= 0 )
                              {
                                 done = 1 ;
                                 rc = -1 ;
                              }
                              if ( recno > dfile4getMinCount( data->dataFile ) )  // skip to next one
                                 continue ;
                              if ( (long)tfile4recNo( tagFile ) != recno ) /* ok, may continue */
                              {
                                 recno = (long)tfile4recNo( tagFile ) ;
                                 d4go( data, recno ) ;
                                 break ;
                              }
                           }
                        }
                     }
                  }
                  else
                     done = 1 ;
               }
               else
                  done = 1 ;
            }
      #endif /* S4OFF_MULTI */

      d4tagSelect( data, oldSelected ) ;

      if ( saveRc == 0 )
         return rc ;
      else
         return saveRc ;
   }
#endif   && !S4OFF_INDEX



// AS Feb 24/09 - support for d4skipCache()
// CS Mar 3/09 Define in CS and SA
// AS Jul 17/09 - modus functionality added
// S4CLIENT
int S4FUNCTION d4skipCache( DATA4 *data, const long nCache, long modus )
{
   // this uses the cache for skipping records from the server
   // functions that reposition the current DATA4 pointer effectively reset the cache
   // a cross between d4skipFetchMultiple and tfile4skipCache
   // input is the # of rows to store in the cache
   // skips 1 record forward from the current position and retrieves the record
   // it will retrieve from the cache if it can
   // otherwise it will read from the server and cache the next nCache records
   // modus...if > 0, then nCache is the number of rows to cache and modus is the number of that numRowsin is doubled when caching continues.
   // modus<= -1 then nCache is the maximum number of rows to cache and abs(modus) is the number of characters that have to match at the beginning of the key.

      return e4notSupported;
}



int S4FUNCTION d4skip( DATA4 *data, const long nSkip )
{
   #ifdef E4VBASIC
      if ( c4parm_check( data, 2, E94802 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E94802 ) ;
   #endif

   CODE4 *c4 = data->codeBase ;
   if ( error4code( c4 ) < 0 )
      return error4code( c4 ) ;

   if ( data->recNum < 1L )
   {
      #ifndef S4SERVER
         if ( c4->errSkip )
            error4( c4, e4info, E84801 ) ;
      #endif
      return e4info ;
   }

   int rc = 0 ;

      #ifndef S4SERVER
         int saveFlag ;
      #endif
      long startRec, newRec ;
      int oldEofFlag, c1 ;
      long n = nSkip ;

      #ifndef S4OFF_INDEX
         TAG4 *tag = data->tagSelected ;
         if ( tag == 0 )
         {
      #endif
         data->bofFlag = 0 ;
         startRec = data->recNum ;
         newRec = startRec + n ;
         if ( newRec > 0L )
         {
            #ifndef S4SERVER
               saveFlag = c4->errGo ;
               c4->errGo = 0 ;
            #endif
            #ifndef S4OFF_OPTIMIZE
               data->dataFile->hiPrio = -1 ;  /* indicate d4skip - data level */
            #endif
            rc = d4go( data, newRec ) ;
            #ifndef S4OFF_OPTIMIZE
               data->dataFile->hiPrio = 0 ;
            #endif
            #ifndef S4SERVER
               c4->errGo = saveFlag ;
            #endif
            if ( rc >= 0 && rc != r4entry )
               return rc ;
         }

         c1 = d4recCountLessEq( data, 1L ) ;
         if ( c1 < 0 )
            return c1 ;
         if ( ( c1 == 0 ) || ( d4recCountLessEq( data, newRec ) == 0 ) )
         {
            if ( c1 == 0L )
            {
               rc = d4goEof( data ) ;
               if ( rc != r4eof )
                  return rc ;
               data->bofFlag = 1 ;
            }
            if ( n < 0 )
            {
               data->bofFlag = 1 ;
               return r4bof ;
            }
            else
               return d4goEof( data ) ;
         }

         if ( newRec < 1L )
         {
            oldEofFlag = data->eofFlag ;
            rc = d4go( data, 1L ) ;
            if ( rc )
               return rc ;
            data->bofFlag = 1 ;
            data->eofFlag = oldEofFlag ;
            return r4bof ;
         }

         return d4go( data, newRec ) ;
      #ifndef S4OFF_INDEX
         }
         else
         {
            TAG4FILE *tagFile = tag->tagFile ;
            if ( data->eofFlag )
            {
               if ( n >= 0 )
                  return d4goEof( data ) ;

               rc = d4bottom( data ) ;
               if ( rc && rc != r4eof )
                  return rc ;
               if ( rc == r4eof )
               {
                  rc = d4goEof( data ) ;
                  if ( rc != r4eof )
                     return rc ;
                  return r4bof ;
               }
               n++ ;
               data->recNum = (long)tfile4recNo( tagFile ) ;
            }

            data->bofFlag = 0 ;

            #ifndef S4OFF_WRITE
               if ( data->recordChanged )
               {
                  /* AS 04/22/97 causes problems with t4seek, unlock reset to 0 */
                  rc = d4updateRecord( data, 0, 1 ) ;
                  if ( rc < 0 )
                     return rc ;
               }
            #endif

            t4versionCheck( tag, 1, 0 ) ;

            if ( n == 0 )
               return 0 ;

            if ( (long)tfile4recNo( tagFile ) != data->recNum )
            {
               rc = d4go( data, data->recNum ) ;
               if ( rc )
                  return rc ;

               expr4context( tagFile->expr, data ) ;
               unsigned char *keyValue ;
               tfile4exprKey( tagFile, &keyValue ) ;

               rc = tfile4go( tagFile, keyValue, data->recNum, 0 ) ;
               if ( rc < 0 )
                  return rc ;

               #ifdef S4HAS_DESCENDING
                  if ( tagFile->header.descending )
                  {
                     if ( (rc > 0) && (n < 0) )
                        n-- ;
                  }
                  else
                     if ( (rc > 0) && (n > 0) )
                        n-- ;
               #else
                  if ( (rc > 0) && (n > 0) )
                     n-- ;
               #endif
            }

            long nSkipped = tfile4dskip( tagFile, n ) ;
            if ( n > 0  &&  nSkipped != n )
               return d4goEof( data ) ;

            if ( tfile4eof( tagFile ) )
            {
               data->bofFlag = 1 ;
               return d4goEof( data ) ;
            }

            /* AS 03/15/99 -- it is possible that the key found may not be in the data file because of update conflicts.  For example
               added to tag but not added to datafile yet.  Or added by another user in transaction so shouldn't go to recor yet.
               modified to skip forward in that case.  */
            int skipDirection ;
            if ( n < 0 )
               skipDirection = -1 ;
            else
               skipDirection = 1 ;

            long recno ;
            for( ;; )
            {
               recno = (long)tfile4recNo( tagFile ) ;
               if ( recno < 0 )
                  return (int)recno ;
               if ( d4recCountLessEq( data, (long)tfile4recNo( tagFile ) ) != 0 )  /* valid spot */
                  break ;
               if ( tfile4dskip( tagFile, skipDirection ) == 0 ) // didn't skip
               {
                  data->bofFlag = 1 ;
                  return d4goEof( data ) ;
               }
            }
            rc = d4go( data, recno ) ;
            if ( rc )
               return rc ;

            #if !defined(S4OFF_MULTI) && !defined(S4OFF_TRAN)
               /* if transactions are enabled, we do not remove keys from an index file when
                  records are changed to avoid unique problems caused by another user.
                  The result of this is we may need to re-sync ourselves in that case - namely,
                  we do not want to position to a record indicated by a 'removed' entry in
                  a tag.
               */
               // AS Apr 29/03 - transcations are run-time in odbc now
               // AS Jun 20/03 - was checking wrong flag
               #if defined( S4SERVER ) && defined( S4ODBC_BUILD )
                  if ( c4->server->odbcTrans == 1 )  // odbc build, no trans available
               #endif
                     if ( code4transEnabled( c4 ) && t4unique( tag ) != 0 )
                     {
                        #ifdef S4SERVER
                           // AS Apr 15/03 - support for new lockId for shared clone locking
                           if ( dfile4lockTestFile( data->dataFile, data4lockId( data ), data4serverId( data ), lock4write ) != 1 )
                           if ( dfile4lockTestFile( data->dataFile, data4lockId( data ), data4serverId( data ), lock4read ) != 1 )
                        #else
                           if ( d4lockTestFile( data ) != 1 )
                        #endif
                        {
                           rc = d4tagSyncDo( data, tag, n > 0 ? 1 : -1 ) ;
                           if ( rc != 0 )
                              return rc ;
                        }
                     }
            #endif

            if ( n == nSkipped )
               return 0 ;

            #ifdef S4HAS_DESCENDING
               if ( ( n < 0 && !tagFile->header.descending ) ||  ( n < 0 && tagFile->header.descending ) )
            #else
               if ( n < 0 )
            #endif
            {
               data->bofFlag = 1 ;
               return r4bof ;
            }
         }
         return 0 ;
      #endif
}
