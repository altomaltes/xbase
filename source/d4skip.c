/* d4skip.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"


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

   C4PARMDATA( data, E94803, -1 );

      if ( tag == 0 )
         return 0 ;

      c4 = data->codeBase ;
      if ( error4code( c4 ) < 0 )
         return error4code( c4 ) ;

         return d4tagSyncDo( data, tag, 1 ) ;
   #endif /* S4OFF_INDEX */
}

#ifndef S4OFF_INDEX
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

   #ifndef S4OFF_MULTI
      #ifndef S4OFF_TRAN
         c4 = data->codeBase ;
      #endif
   #endif

   tagFile = tag->tagFile ;
   expr4context( tagFile->expr, data ) ;
   tfile4exprKey( tagFile, &keyValue ) ;
   recno = tfile4recNo( tagFile ) ;
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

   #ifndef S4OFF_WRITE
      if ( data->recordChanged )
      {
         rc = d4updateRecord( data, 0 ) ;
         if ( rc )
         {
            d4tagSelect( data, oldSelected ) ;
            return rc ;
         }
      }
   #endif

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

      recno = tfile4recNo( tagFile ) ;
      if ( recno != data->recNum )
      {
         rc = d4go( data, recno ) ;
         saveRc = r4after ;
      }
      else
         rc = 0 ;
   }

   #ifndef S4OFF_MULTI
         if ( !d4lockTestFile( data ) )
         for( verifyReccount = 1, done = 0 ; done == 0 ; )
         {
            #ifndef S4OFF_TRAN
               if ( code4transEnabled( c4 ) )
                  if ( t4unique( tag ) != 0 )
                     if ( code4tranStatus( c4 ) == r4active )
                        if ( d4lockTest( data, recno ) )
                        {
                           /* if within a transaction, and record is locked, it may mean
                              that the record contents have changed.  If the current tag key
                              does not match, it must be an old entry (if a unique tag),
                              and thus skip over
                           */
                           for ( ;; )
                           {
                              if ( recno > data->dataFile->minCount && verifyReccount == 1 )   /* ensure correct sequencing for multi-user */
                                 break ;  /* just let the code below take care of this case */
                              expr4context( tagFile->expr, data ) ;
                              if ( recno != d4recNo( data ) )  /* need to reposition data4 first */
                                  d4go( data, recno ) ;
                              tfile4exprKey( tagFile, &keyValue ) ;
                              if ( memcmp( tfile4key( tagFile ), keyValue, tagFile->header.keyLen ) != 0 )  /* different, so skip */
                                 nSkipped = (int)tfile4skip( tagFile, direction ) ;
                              else  /* done */
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
                              if ( tfile4recNo( tagFile ) != recno ) /* ok, may continue */
                              {
                                 recno = tfile4recNo( tagFile ) ;
                                 d4go( data, recno ) ;
                                 break ;
                              }
                           }
                        }
            #endif /* S4OFF_TRAN */

            if ( done == 1 )
               break ;

            if ( verifyReccount == 1 )
            {
               verifyReccount = 0 ;
               if ( recno > data->dataFile->minCount )   /* ensure correct sequencing for multi-user */
               {
                  while ( d4recCountLessEq( data, recno ) == 0 && done == 0 && rc == 0 )
                  {
                     rc = (int)tfile4skip( tagFile, direction ) ;
                     if ( rc < 0 )
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
                           recno = tfile4recNo( tagFile ) ;
                           if ( recno <= 0 )
                           {
                              done = 1 ;
                              rc = -1 ;
                           }
                           if ( recno > data->dataFile->minCount )  /* skip to next one */
                              continue ;
                           if ( tfile4recNo( tagFile ) != recno ) /* ok, may continue */
                           {
                              recno = tfile4recNo( tagFile ) ;
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
#endif  /* S4OFF_INDEX */

int S4FUNCTION d4skip( DATA4 *data, const long nSkip )
{
   int rc ;
   CODE4 *c4 ;
      #ifndef S4OFF_INDEX
         unsigned char *keyValue ;
         long nSkipped, recno ;
         TAG4FILE *tagFile ;
      #endif
      long n ;
         int saveFlag ;
      long startRec, newRec ;
      int oldEofFlag, c1 ;
   #ifndef S4OFF_INDEX
      TAG4 *tag ;
   #endif

   C4PARMDATA( data, E94802, -1 );

   c4 = data->codeBase ;
   if ( error4code( c4 ) < 0 )
      return error4code( c4 ) ;

   if ( data->recNum < 1L )
   {
         if ( c4->errSkip )
            error4( c4, e4info, E84801 ) ;
      return e4info ;
   }

      n = nSkip ;

      #ifndef S4OFF_INDEX
         tag = data->tagSelected ;
         if ( tag == 0 )
         {
      #endif
         data->bofFlag = 0 ;
         startRec = data->recNum ;
         newRec = startRec + n ;
         if ( newRec > 0L )
         {
               saveFlag = c4->errGo ;
               c4->errGo = 0 ;
            #ifndef S4OFF_OPTIMIZE
               data->dataFile->hiPrio = -1 ;  /* indicate d4skip - data level */
            #endif
            rc = d4go( data, newRec ) ;
            #ifndef S4OFF_OPTIMIZE
               data->dataFile->hiPrio = 0 ;
            #endif
               c4->errGo = saveFlag ;
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
            tagFile = tag->tagFile ;
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
               data->recNum = tfile4recNo( tagFile ) ;
            }

            data->bofFlag = 0 ;

            #ifndef S4OFF_WRITE
               if ( data->recordChanged )
               {
                  /* AS 04/22/97 causes problems with t4seek, unlock reset to 0 */
                  rc = d4updateRecord( data, 0 ) ;
                  if ( rc < 0 )
                     return rc ;
               }
            #endif

            t4versionCheck( tag, 1, 0 ) ;

            if ( n == 0 )
               return 0 ;

            if ( tfile4recNo( tagFile ) != data->recNum )
            {
               rc = d4go( data, data->recNum ) ;
               if ( rc )
                  return rc ;

               expr4context( tagFile->expr, data ) ;
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

            nSkipped = tfile4dskip( tagFile, n ) ;
            if ( n > 0  &&  nSkipped != n )
               return d4goEof( data ) ;

            if ( tfile4eof( tagFile ) )
            {
               data->bofFlag = 1 ;
               return d4goEof( data ) ;
            }

            recno = tfile4recNo( tagFile ) ;
            if ( recno < 0 )
               return (int)recno ;
            rc = d4go( data, recno ) ;
            if ( rc )
               return rc ;
            #ifndef S4OFF_MULTI
               #ifndef S4OFF_TRAN
                  if ( code4transEnabled( c4 ) )
                     if ( t4unique( tag ) != 0 )
                           if ( !d4lockTestFile( data ) )
                        {
                           rc = d4tagSyncDo( data, tag, n > 0 ? 1 : -1 ) ;
                           if ( rc != 0 )
                              return rc ;
                        }
               #endif
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
