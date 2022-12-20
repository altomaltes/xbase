/* f4flag.c  (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

int S4FUNCTION f4flagInit( F4FLAG *f4, CODE4 *c4, const unsigned long nFlags )
{
   E4PARMLOW( f4 && c4 , E90812 ) ;

   memset( (void *)f4, 0, sizeof(F4FLAG) ) ;

   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   f4->codeBase = c4 ;
   f4->numFlags = nFlags ;

   if ( nFlags == 0 )
      return 0 ;

   f4->flags = (unsigned char *)u4allocFree( c4, nFlags / 8 + 2 ) ;
   if ( f4->flags == 0 )
      return error4stack( c4, e4memory, E90812 ) ;
   return 0 ;
}

int S4FUNCTION f4flagReset( F4FLAG *f4, const unsigned long flagNum )
{
   unsigned char lowVal, setVal ;
   unsigned long highVal ;

   E4PARMLOW( f4, E90812 ) ;

   if ( error4code( f4->codeBase ) < 0 )
   {
      u4free( f4->flags ) ;
      f4->flags = 0 ;
      return e4codeBase ;
   }

   #ifdef E4ANALYZE
      if ( flagNum > f4->numFlags || f4->flags == 0 )
         return error4( f4->codeBase, e4info, E90812 ) ;
   #endif

   lowVal = (unsigned char) (flagNum & 0x7) ;
   highVal = flagNum >> 3 ;
   setVal = (unsigned char) (1 << lowVal) ;
   setVal = (unsigned char) ~setVal ;

   f4->flags[highVal] = (char) (setVal & f4->flags[highVal]) ;

   return 0 ;
}

int S4FUNCTION f4flagSet( F4FLAG *f4, const unsigned long flagNum )
{
   unsigned char lowVal, setVal ;
   unsigned long highVal ;

   E4PARMLOW( f4, E90812 ) ;

   if ( error4code( f4->codeBase ) < 0 )
   {
      u4free( f4->flags ) ;
      f4->flags = 0 ;
      return e4codeBase ;
   }

   #ifdef E4ANALYZE
      if ( flagNum > f4->numFlags || f4->flags == 0 )
         return error4( f4->codeBase, e4info, E90812 ) ;
   #endif

   lowVal = (unsigned char) (flagNum & 0x7) ;
   highVal = flagNum >> 3 ;
   setVal = (unsigned char) (1 << lowVal) ;

   f4->flags[highVal] = (char)(setVal | f4->flags[highVal]) ;

   return 0 ;
}

int S4FUNCTION f4flagSetRange( F4FLAG *f4, const unsigned long flagNum, const unsigned long numFlags )
{
   unsigned long iFlag ;
   int rc ;

   E4PARMLOW( f4, E90812 ) ;

   for ( iFlag = 0; iFlag < numFlags; iFlag++ )
   {
      rc = f4flagSet( f4, flagNum + iFlag ) ;
      if ( rc < 0 )
         return error4stack( f4->codeBase, (short)rc, E90812 ) ;
   }
   return 0 ;
}

int S4FUNCTION f4flagIsSet( F4FLAG *f4, const unsigned long flagNum )
{
   unsigned char lowVal, retVal ;
   unsigned long highVal ;

   E4PARMLOW( f4, E90812 ) ;

   if ( error4code( f4->codeBase ) < 0 )
   {
      u4free( f4->flags ) ;
      f4->flags = 0 ;
      return e4codeBase ;
   }

   #ifdef E4ANALYZE
      if ( flagNum > f4->numFlags || f4->flags == 0 )
         return error4( f4->codeBase, e4info, E90812 ) ;
   #endif

   lowVal = (unsigned char) (flagNum & 0x7) ;
   highVal = flagNum >> 3 ;
   retVal = (unsigned char) ((1 << lowVal) & f4->flags[highVal])  ;

   return (int) retVal ;
}

int S4FUNCTION f4flagIsAllSet( F4FLAG *f4, const unsigned long flagNum, const unsigned long nFlags )
{
   int rc ;
   unsigned long iFlag, numFlags ;

   E4PARMLOW( f4, E90812 ) ;


   if ( error4code( f4->codeBase ) < 0 )
      numFlags = 1 ;
   else
      numFlags = nFlags ;

   for ( iFlag = flagNum; iFlag <= numFlags; iFlag++ )
   {
      rc = f4flagIsSet( f4, iFlag ) ;
      if ( rc < 0 )
         return error4stack( f4->codeBase, (short)rc, E90812 ) ;
      if ( rc == 0 )
         return 0 ;
   }
   return 1 ;
}

int S4FUNCTION f4flagIsAnySet( F4FLAG *f4, const unsigned long flagNum, const unsigned long nFlags )
{
   int rc ;
   unsigned long iFlag, numFlags ;

   E4PARMLOW( f4, E90812 ) ;

   if ( error4code( f4->codeBase ) < 0 )
      numFlags = 1 ;
   else
      numFlags = nFlags ;

   for ( iFlag = flagNum; iFlag <= numFlags; iFlag++ )
      if ( (rc = f4flagIsSet( f4, iFlag )) < 0 )
         return error4stack( f4->codeBase, (short)rc, E90812 ) ;
   return 0 ;
}

int S4FUNCTION f4flagAnd( F4FLAG *flagPtr, const F4FLAG *andPtr )
{
   unsigned numBytes ;

   E4PARMLOW( flagPtr && andPtr, E90812 ) ;

   if ( andPtr->numFlags == 0 )
   {
      if ( flagPtr->numFlags == 0 )
         return 0 ;
      if ( flagPtr->isFlip != andPtr->isFlip )
         memset( (void *)flagPtr->flags, 1, (unsigned)((flagPtr->numFlags) / 8L + 2L ) ) ;
      else
         memset( (void *)flagPtr->flags, 0, (unsigned)((flagPtr->numFlags) / 8L + 2L ) ) ;
      return 0 ;
   }

   #ifdef E4ANALYZE
      if ( flagPtr->numFlags != andPtr->numFlags )
         return error4( flagPtr->codeBase, e4result, E90812 ) ;
   #endif

   numBytes = (unsigned)((flagPtr->numFlags+7)/8) ;

   #ifdef E4ANALYZE
      if ( flagPtr->numFlags != andPtr->numFlags )
         return error4( flagPtr->codeBase, e4result, E90812 ) ;
      if ( (unsigned long)numBytes != ( flagPtr->numFlags + 7L) / 8L )
         return error4( flagPtr->codeBase, e4info, E90812 ) ;
   #endif

   if ( flagPtr->isFlip != andPtr->isFlip )
   {
      if ( flagPtr->isFlip == 1 )
      {
         flagPtr->isFlip = 0 ;
         do
         {
            flagPtr->flags[numBytes] = (unsigned char) (~flagPtr->flags[numBytes] & andPtr->flags[numBytes]) ;
         } while (numBytes-- != 0 ) ;
      }
      else
         do
         {
            flagPtr->flags[numBytes] &= ~andPtr->flags[numBytes] ;
         } while (numBytes-- != 0 ) ;
   }
   else
   {
      if ( flagPtr->isFlip == 1 )
      {
         flagPtr->isFlip = 0 ;
         do
         {
            flagPtr->flags[numBytes] = (unsigned char) (~flagPtr->flags[numBytes] & ~andPtr->flags[numBytes]) ;
         } while (numBytes-- != 0 ) ;
      }
      else
         do
         {
            flagPtr->flags[numBytes] &= andPtr->flags[numBytes] ;
         } while (numBytes-- != 0 ) ;
   }

   return 0 ;
}

int S4FUNCTION f4flagOr( F4FLAG *flagPtr, const F4FLAG *orPtr )
{
   unsigned numBytes ;

   E4PARMLOW( flagPtr && orPtr, E90812 ) ;

   if ( orPtr->numFlags == 0 )
      return 0 ;

   numBytes = (unsigned)(flagPtr->numFlags / 8L + 1L ) ;

   #ifdef E4ANALYZE
      if ( flagPtr->numFlags != orPtr->numFlags )
         return error4( flagPtr->codeBase, e4result, E90812 ) ;
      if ( (unsigned long)numBytes != ( flagPtr->numFlags / 8L + 1L ) )
         return error4( flagPtr->codeBase, e4info, E90812 ) ;
   #endif
   if ( flagPtr->isFlip != orPtr->isFlip )
   {
      if ( flagPtr->isFlip == 1 )
      {
         flagPtr->isFlip = 0 ;
         do
         {
           flagPtr->flags[numBytes] = (unsigned char) (~flagPtr->flags[numBytes] | orPtr->flags[numBytes]) ;
         } while (numBytes-- != 0 ) ;
      }
      else
         do
         {
            flagPtr->flags[numBytes] |= ~orPtr->flags[numBytes] ;
         } while (numBytes-- != 0 ) ;
   }
   else
   {
      if ( flagPtr->isFlip == 1 )
      {
         flagPtr->isFlip = 0 ;
         do
         {
            flagPtr->flags[numBytes] = (unsigned char)(~flagPtr->flags[numBytes] | ~orPtr->flags[numBytes]) ;
         } while (numBytes-- != 0 ) ;
      }
      else
         do
         {
            flagPtr->flags[numBytes] |= orPtr->flags[numBytes] ;
         } while (numBytes-- != 0 ) ;
   }
   return 0 ;
}

void S4FUNCTION f4flagFlipReturns( F4FLAG *flagPtr )
{
   E4PARMLOW( flagPtr, E90812 ) ;

   flagPtr->isFlip = !flagPtr->isFlip ;
}

void S4FUNCTION f4flagSetAll( F4FLAG *flagPtr )
{
   E4PARMLOW( flagPtr, E90812 ) ;

   memset( (void *)flagPtr->flags, 0xFF, (unsigned)((flagPtr->numFlags+7)/8) ) ;
}
