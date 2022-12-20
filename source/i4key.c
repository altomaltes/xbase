/* i4key.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"


#ifndef S4INDEX_OFF

char *S4FUNCTION tfile4key( TAG4FILE *t4 )
{
   B4BLOCK *b4 ;

   #ifdef S4VBASIC
      if ( c4parm_check( t4->codeBase, 1, E91630 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( t4 == 0 )
      {
         error4( 0, e4parmNull, E91630 ) ;
         return 0 ;
      }
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return 0 ;

   b4 = (B4BLOCK *)(t4->blocks.lastNode) ;

   if ( b4 == 0 )
      return 0 ;

   if ( b4->keyOn >= b4numKeys( b4 ) )  /* eof */
      return 0 ;

   return (char *)b4keyKey( b4, b4->keyOn ) ;
}

#endif  /* S4INDEX_OFF */

#ifdef S4VB_DOS

char * tfile4key_v( TAG4FILE *t4 )
{
   int len ;
   char *key ;

   #ifdef S4VBASIC
      if ( c4parm_check( t4->codeBase, 1, "tfile4key():" ) ) return 0 ;
   #endif

   StringRelease( basic_desc );

   if( ! (key = tfile4key(t4)) )
      return basic_desc ;

   StringAssign( key, expr4keyLen(t4->expr), basic_desc, 0 );

   return basic_desc ;

}

#endif  /* S4VB_DOS */

