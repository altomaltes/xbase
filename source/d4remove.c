/* d4remove.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

int S4FUNCTION d4remove( DATA4 *data )
{
   int rc ;
   CODE4 *c4 ;

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E96401 ) ;
   #endif

   /* for c/s, server takes care of determining this info... */
      #ifndef S4OFF_TRAN
         if ( code4transEnabled( data->codeBase ) )
            if ( tran4active( data->codeBase, data ) != 0 )
               return error4( data->codeBase, e4transViolation, E81521 ) ;
      #endif

   c4 = data->codeBase;
   c4setDoRemove( c4, 1 ) ;
   rc = d4close( data ) ;
   c4setDoRemove( c4, 0 ) ;

   return rc ;
}

int S4FUNCTION dfile4remove( DATA4FILE *data )
{
   int rc ;
   #ifndef S4INDEX_OFF
      #ifdef N4OTHER
         TAG4FILE *tagOn ;
      #else
         INDEX4FILE *indexOn ;
      #endif
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E96401 ) ;
   #endif

   if ( data->userCount != 0 )
      return error4( 0, e4parm_null, E86402 ) ;

   #ifndef S4INDEX_OFF
      #ifdef N4OTHER
         for( tagOn = 0;; )
         {
            tagOn = dfile4tagNext( data, tagOn ) ;
            if ( tagOn == 0 )
               break ;

            tagOn->file.isTemp = 1 ;
         }
      #else
         for( indexOn = 0 ;; )
         {
            indexOn = (INDEX4FILE *)l4next( &data->indexes, indexOn ) ;
            if ( indexOn == 0 )
               break ;

            indexOn->file.isTemp = 1 ;
         }
      #endif
   #endif

   data->file.isTemp = 1 ;

   #ifndef S4MEMO_OFF
      if (data->nFieldsMemo )
         data->memoFile.file.isTemp = 1 ;
   #endif

   rc = dfile4closeLow( data ) ;

   return rc ;
}
