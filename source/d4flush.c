/* d4flush.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_WRITE
int S4FUNCTION d4changed( DATA4 *data, int flag )
{
   int previous ;

   C4PARMDATA( data, E94101, -1 );

   #ifndef S4OFF_ENFORCE_LOCK
      if ( flag > 0 )
         if ( data->codeBase->lockEnforce && data->recNum > 0L )
            if ( d4lockTest( data, data->recNum ) != 1 )
               return error4( data->codeBase, e4lock, E94101 ) ;
   #endif

   previous = data->recordChanged ;

   if ( flag >= 0 )
      data->recordChanged = ( flag > 0 ) ;
   return previous ;
}
#endif /* S4OFF_WRITE */

int S4FUNCTION d4flush( DATA4 *data )
{
   #ifdef S4OFF_WRITE
      return 0 ;
   #else
         int rc, saveRc ;

   E4PARHIGH( data, E94102 ) ;

         saveRc = d4flushData( data ) ;
         rc = dfile4flushIndex( data->dataFile ) ;
         if ( rc < 0 )
            saveRc = rc ;

         return saveRc ;
   #endif /* S4OFF_WRITE */
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int d4flushData( DATA4 *data )
{
   #ifdef S4OFF_WRITE
      return 0 ;
   #else
         int rc, saveRc ;

   E4PARHIGH( data, E94105 ) ;

         saveRc = d4update( data ) ;   /* returns -1 if error4code( codeBase ) < 0 */
         rc = dfile4flushData( data->dataFile ) ;
         if ( rc < 0 )
            saveRc = rc ;
         return saveRc ;
   #endif
}

int dfile4flush( DATA4FILE *data )
{
   int rc, saveRc ;

   #ifdef E4PARM_LOW
      if ( data == 0 )
         return error4( 0, e4parmNull, E91102 ) ;
   #endif

   saveRc = dfile4flushData( data ) ;
   rc = dfile4flushIndex( data ) ;
   if ( rc < 0 )
      saveRc = rc ;

   return saveRc ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int dfile4flushIndex( DATA4FILE *data )
{
   #ifdef S4OFF_WRITE
      return 0 ;
   #else
      int rc ;
      #ifndef S4INDEX_OFF
         #ifndef N4OTHER
            INDEX4FILE *indexOn ;
         #else
            TAG4FILE *tagOn ;
         #endif
      #endif

      if ( data == 0 )
         return error4( 0, e4parm, E91102 ) ;

      rc = 0 ;

      #ifndef S4INDEX_OFF
         #ifndef S4CLIPPER
            indexOn = (INDEX4FILE *)l4first( &data->indexes ) ;
            if ( indexOn )
               do
               {
                  if ( index4flush( indexOn ) )
                     rc = -1 ;
                  indexOn = (INDEX4FILE *)l4next( &data->indexes, indexOn ) ;
               } while ( indexOn != 0 ) ;
         #else
            for( tagOn = 0 ;; )
            {
               tagOn = dfile4tagNext( data, tagOn ) ;
               if ( !tagOn )
                  break ;
               if ( tfile4flush( tagOn ) )
                  rc = - 1 ;
            }
         #endif
      #endif

      return rc ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int dfile4flushData( DATA4FILE *data )
{
   #ifdef S4OFF_WRITE
      return 0 ;
   #else
      int rc ;

      #ifdef E4PARM_LOW
         if ( data == 0 )
            return error4( 0, e4parm, E91102 ) ;
      #endif

      /* 03/14/97 AS if file opened excl. with opt, was not flushing header */
      #ifndef S4OFF_MULTI
      if ( data->file.lowAccessMode == OPEN4DENY_RW )
      #endif
         rc = dfile4updateHeader( data, 1, 1 ) ;
      rc = file4flush( &data->file ) ;
      #ifndef S4OFF_MEMO
         if ( data->memoFile.file.hand != INVALID4HANDLE )
            return file4flush( &data->memoFile.file ) ;
      #endif
      return rc ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION code4flush( CODE4 *c4 )
{
   #ifdef S4OFF_WRITE
      return 0 ;
   #else
      DATA4 *dataOn ;
      int rc, rcReturn ;
      LIST4 *list ;

   C4PARMCODE( c4, E94107, -1 );

      rcReturn = 0 ;

      list = tran4dataList( (&(c4->c4trans.trans)) ) ;
      dataOn = (DATA4 *)l4first( list ) ;
      while ( dataOn )
      {
         rc = d4flush( dataOn ) ;   /* returns -1 if error4code( codeBase ) < 0 */
         if ( rc )
            rcReturn = rc ;
         dataOn = (DATA4 *)l4next( list, dataOn) ;
      }

      return rcReturn ;
   #endif
}

#ifndef S4OFF_WRITE

#ifndef S4INDEX_OFF
int dfile4updateIndexes( DATA4FILE *data )
{
   #ifdef S4FOX
      INDEX4FILE *indexOn ;
   #else
      TAG4FILE *tagOn ;
   #endif
   int rc ;

   #ifdef E4PARM_LOW
      if ( data == 0 )
         return error4( 0, e4parmNull, E91102 ) ;
   #endif

   rc = 0 ;

   #ifdef S4FOX
      indexOn = (INDEX4FILE *)l4first( &data->indexes ) ;
      if ( indexOn )
      do
      {
         if ( index4update( indexOn ) < 0 )
            rc = -1 ;
         indexOn = (INDEX4FILE *)l4next( &data->indexes, indexOn ) ;
      } while ( indexOn != 0 ) ;
   #else
      for( tagOn = 0 ;; )
      {
         tagOn = dfile4tagNext( data, tagOn ) ;
         if ( !tagOn )
            break ;
         if ( tfile4update(tagOn) < 0 )
            rc = -1 ;
      }
   #endif

   return rc ;
}
#endif

int d4update( DATA4 *data )
{
   int rc ;

   C4PARMDATA( data, E94109, -1 );

   if ( error4code( data->codeBase ) < 0 )
      return -1 ;

   rc = d4updateRecord( data, 0 ) ;

      if ( rc == 0 )
         if ( data->dataFile->fileChanged )   /* if the header is outdated */
            #ifndef S4SINGLE
               if ( d4lockTestAppend( data ) == 1 )   /* if we have changed the record count */
            #endif
                  rc = dfile4updateHeader( data->dataFile, 1, 1 ) ;
      #ifndef S4INDEX_OFF
         if ( rc == 0 )
            dfile4updateIndexes( data->dataFile ) ;
      #endif

   return rc ;
}

int d4updateRecord( DATA4 *data, const int doUnlock )
{
   int rc ;
   #ifndef S4SINGLE
      int explicitUnlock ;
   #endif
   #ifndef S4OFF_MEMO
      int i ;
   #endif

   C4PARMDATA( data, E94110, -1 );

   #ifdef E4ANALYZE
      if ( data->codeBase == 0 )
         return error4describe( 0, e4struct, E94110, data->alias, 0, 0 ) ;
   #endif

   if ( error4code( data->codeBase ) < 0 )
      return e4codeBase ;

   if ( data->recNum <= 0 || data->eofFlag )
   {
      #ifndef S4OFF_MEMO
         if ( data->fieldsMemo != 0 )
            for ( i = 0; i < data->dataFile->nFieldsMemo; i++ )
               f4memoReset( data->fieldsMemo[i].field ) ;
      #endif
      data->recordChanged = 0 ;
      return 0 ;
   }

   if ( data->recordChanged )
   {
      rc = d4writeLow( data, data->recNum, doUnlock ) ;
      if ( rc )
         return rc ;
      #ifndef S4SINGLE
         explicitUnlock = 0 ;
      #endif

      #ifndef S4OFF_MEMO
         for ( i = 0; i < data->dataFile->nFieldsMemo; i++ )
            f4memoReset( data->fieldsMemo[i].field ) ;
      #endif
      data->recNumOld = -1 ;
   }
   else
   {
      #ifndef S4SINGLE
         explicitUnlock = doUnlock ;
      #endif
      #ifndef S4OFF_MEMO
         for ( i = 0; i < data->dataFile->nFieldsMemo; i++ )
            data->fieldsMemo[i].status = 1 ;
      #endif
   }

   #ifndef S4SINGLE
      if ( explicitUnlock )   /* unlock records (unless entire file is locked)... */
      {
         if ( code4unlockAuto( data->codeBase ) != 0 )
         {
               #ifndef S4OFF_TRAN
                  if ( code4transEnabled( data->codeBase ) )
                     if ( code4tranStatus( data->codeBase ) == r4active )
                        return 0 ;
               #endif
               if ( d4lockTestFile( data ) == 0 )
            {
               rc = d4unlockLow( data, data4clientId( data ), 0 ) ;
               if ( rc == r4active )  /* just a transactional notification */
                  return 0 ;
               return rc ;
            }
         }
      }
   #endif /* S4SINGLE */

   return 0 ;
}
#endif  /* S4OFF_WRITE */
