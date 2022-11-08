/* d4positi.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

double S4FUNCTION d4position( DATA4 *data )
{
      long count ;
      #ifndef S4OFF_INDEX
         int len, rc ;
         unsigned char *result ;
      #endif

   #ifndef S4OFF_INDEX
      TAG4 *tag ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E94701 ) )
         return -1.0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return (double)error4( 0, e4parmNull, E94701 ) ;
   #endif

   if( error4code( data->codeBase ) < 0 )
      return -1.0 ;

   if ( d4eof( data ) )
      return 1.1 ;

      #ifndef S4OFF_INDEX
         tag = data->tagSelected ;
         if ( tag == 0 || data->recNum <= 0L )
         {
      #endif
         count = d4recCount( data ) ;
         if ( count < 0 )
            return -1.0 ;
         if ( count == 0 || data->recNum <= 0L )
            return 0.0 ;

         return (double)( data->recNum - 1 ) / ( count - (count != 1 ) ) ;
      #ifndef S4OFF_INDEX
         }
         else
         {
            expr4context( tag->tagFile->expr, data ) ;
            len = tfile4exprKey( tag->tagFile, &result ) ;
            if ( len < 0 )
               return -1.0 ;
            t4versionCheck( tag, 0, 0 ) ;
            rc = tfile4go( tag->tagFile, result, data->recNum, 0 ) ;
            if ( rc != 0  && rc != r4eof && rc != r4after )
               return -1.0 ;
            return tfile4position( tag->tagFile ) ;
         }
      #endif
}

int S4FUNCTION d4position2( DATA4 *data, double *result )
{
   *result = d4position( data ) ;
   if ( *result < 0.0 )
      return -1;
   return 0;
}

int S4FUNCTION d4positionSet( DATA4 *data, const double per )
{
   int rc ;
   CODE4 *c4 ;
      long newRec ;
      long count ;
   #ifndef S4OFF_INDEX
      TAG4 *tag ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E94702 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parmNull, E94702 ) ;
   #endif

   c4 = data->codeBase ;
   if( error4code( c4 ) < 0 )
      return e4codeBase ;

   if ( per > 1.0 )
      return d4goEof( data ) ;
   if ( per <= 0 )
      return d4top( data ) ;

      #ifndef S4OFF_INDEX
         tag = data->tagSelected ;
         if ( tag == 0 )
         {
      #endif
         count = d4recCount( data ) ;
         if ( count <= 0L )
            return d4goEof( data ) ;

         newRec = (long)( per * ( (double)count - 1 ) + 1.5 ) ;
         if ( newRec > count )
            newRec = count ;
      #ifndef S4OFF_INDEX
         }
         else
         {
            rc = tfile4positionSet( tag->tagFile, per ) ;
            if ( rc )
               return rc ;
            if ( rc == r4eof )
               return d4goEof( data ) ;

            newRec = tfile4recNo( tag->tagFile ) ;
         }
      #endif
      rc = d4go( data, newRec ) ;
      if ( rc != 0 )
         return rc ;
      #ifndef S4OFF_INDEX
         if ( tag != 0 )
            rc = d4tagSyncDo( data, tag, 1 ) ;
      #endif
      #ifndef S4OFF_MULTI
         if ( rc != 0 )  /* failed, try synching the other way */
         {
            rc = d4go( data, newRec ) ;
            #ifndef S4OFF_INDEX
               #ifndef S4OFF_TRAN
                  if ( rc == 0 )
                     if ( code4transEnabled( c4 ) )
                        if ( t4unique( tag ) != 0 )
                              if ( !d4lockTestFile( data ) )
                              rc = d4tagSyncDo( data, tag, -1 ) ;
               #endif
            #endif
         }
      #endif
      return rc ;
}
