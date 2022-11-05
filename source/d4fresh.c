/* d4fresh.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

int S4FUNCTION d4refresh( DATA4 *data )
{
   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E94201 ) ;
   #endif

   #ifdef S4CLIENT
      return 0 ;
   #else
      if ( dfile4refresh( data->dataFile ) < 0 )
         return -1 ;
      return d4freeBlocks( data ) ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int dfile4refresh( DATA4FILE *data )
{
   #ifndef S4SINGLE
      #ifndef S4OFF_OPTIMIZE
         #ifndef S4OFF_INDEX
            #ifdef N4OTHER
               TAG4FILE *tagOn ;
            #else
               INDEX4FILE *indexOn ;
            #endif
         #endif

         #ifdef E4PARM_LOW
            if ( data == 0 )
               return error4( 0, e4parm_null, E91102 ) ;
         #endif

         file4refresh( &data->file ) ;
         #ifndef S4OFF_MEMO
            if ( data->memoFile.file.hand != INVALID4HANDLE )
               file4refresh( &data->memoFile.file ) ;
         #endif
         #ifndef S4OFF_INDEX
            #ifdef N4OTHER
               for( tagOn = 0 ;; )
               {
                  tagOn = dfile4tagNext( data, tagOn) ;
                  if ( tagOn == 0 )
                     break ;
                  file4refresh( &tagOn->file ) ;
               }
            #else
               for ( indexOn = 0 ;; )
               {
                  indexOn = (INDEX4FILE *)l4next( &data->indexes, indexOn ) ;
                  if ( indexOn == 0 )
                     break ;
                  file4refresh( &indexOn->file ) ;
               }
            #endif /* N4OTHER */
         #endif /* S4OFF_INDEX */

         if ( error4code( data->c4 ) < 0 )
            return -1 ;
      #endif
   #endif
   return 0 ;
}

#ifndef S4SERVER

#ifdef S4STAND_ALONE
   #ifndef S4SINGLE
      #ifndef S4OFF_OPTIMIZE
         #define S4DO_REFRESH
      #endif
   #endif
#endif

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4refreshRecord( DATA4 *data )
{
   #ifndef S4OFF_MEMO
      int i ;
   #endif

   #ifdef S4DO_REFRESH
      OPT4 *opt ;
      int rc ;

      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm, E94203 ) ;
      #endif

      opt = &data->codeBase->opt ;

      if ( data->recNum <= 0L || data->recNum > d4recCount( data ) )
         return 0 ;

      if ( dfile4lockTestFile( data->dataFile, data4clientId( data ), data4serverId( data ) ) != 0 ||
           data->dataFile->file.lowAccessMode != OPEN4DENY_NONE || opt == 0 )
      {
         #ifndef S4MEMO_OFF
            if ( data->dataFile->nFieldsMemo > 0 )
            {
               for ( i = 0; i < data->dataFile->nFieldsMemo; i++ )
                  f4memoReset( data->fieldsMemo[i].field ) ;
            }
         #endif
         data->recordChanged = 0 ;
         return d4go( data, data->recNum ) ;
      }

      if ( data->dataFile->file.doBuffer )  /* also makes sure 'opt' should exist */
         opt->forceCurrent = 1 ;

      #ifndef S4OFF_MEMO
         if ( data->dataFile->nFieldsMemo > 0 &&
            data->dataFile->memoFile.file.hand != INVALID4HANDLE )
         {
            file4refresh( &data->dataFile->memoFile.file ) ;
            for ( i = 0; i < data->dataFile->nFieldsMemo; i++ )
               f4memoReset( data->fieldsMemo[i].field ) ;
         }
      #endif

      data->recordChanged = 0 ;
      rc = d4goData( data, data->recNum  ) ;
      if ( rc )
         return rc ;
      data->bofFlag = data->eofFlag = 0 ;

      if ( data->dataFile->file.doBuffer )  /* also makes sure 'opt' should exist */
         opt->forceCurrent = 0 ;

      if ( d4lockTest( data, data->recNum ) )
      {
         memcpy( data->recordOld, data->record, dfile4recWidth( data->dataFile ) ) ;
         data->recNumOld = data->recNum ;
         #ifndef S4OFF_MEMO
            data->memoValidated = 1 ;
         #endif
      }
      #ifndef S4OFF_MEMO
         else
            data->memoValidated = 0 ;
      #endif

      if ( error4code( data->codeBase ) < 0 )
         return -1 ;
      return 0 ;
   #else
      #ifndef S4OFF_MEMO
         if ( data->dataFile->nFieldsMemo > 0 )
         {
            for ( i = 0; i < data->dataFile->nFieldsMemo; i++ )
               f4memoReset( data->fieldsMemo[i].field ) ;
         }
      #endif
      data->recordChanged = 0 ;
      return d4go( data, data->recNum ) ;
   #endif
}

#ifdef S4DO_REFRESH
   #undef S4DO_REFRESH
#endif

#endif /* S4SERVER */
