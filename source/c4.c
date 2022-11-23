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


/* c4.c  Conversion Routines */

#include "d4all.h"


#ifdef S4VBASIC
int c4parm_check( const void *ptr
                , int doCheck
                , const long message )
{ char errMsg[512] ;

  if (!ptr)
      doCheck = - 1 ;

  else
  {  switch (doCheck)
     {  case 0: if ( ((INDEX4  *)ptr)->codeBase->debugInt != E4DEBUG_INT)  doCheck = -1 ;  break ;
        case 1: if ( ((CODE4   *)ptr)->debugInt           != E4DEBUG_INT)  doCheck = -1 ;  break ;              1
        case 2: if ( ((DATA4   *)ptr)->debugInt           != E4DEBUG_INT)  doCheck = -1 ;            break ;
        case 3: if ( ((FIELD4  *)ptr)->debugInt           != E4DEBUG_INT)  doCheck = -1 ;            break ;
        case 4: if ( ((TAG4    *)ptr)->index->codeBase->debugInt != E4DEBUG_INT)  doCheck = -1 ;            break ;
        case 5: if ( ((RELATE4 *)ptr)->codeBase->debugInt != E4DEBUG_INT)         doCheck = -1 ;            break ;

         #ifndef S4OFF_REPORT
         case 6:            if ( ((REPORT4 *)ptr)->codeBase->debugInt != E4DEBUG_INT)               doCheck = -1 ;            break ;
         #endif
      }
   }

   if ( doCheck < 0 )
   { strcpy( errMsg, "Function was Passed an Invalid Structure Pointer" ) ;
     return error4describe( 0, e4info, message, errMsg, 0, 0 ) ;
   }
   else return 0 ;
}

#endif  /* S4VBASIC */

#ifdef S4NO_ATOF
double c4atof( const char *bufIn )
{  double decLen = 1.0 ;
   int i = 0, baseLen, decLenHold, offset, max, sign ;
   const char *cPtr, *tptr ;
   char *ptr, buf[40] ;
   double baseDoub, decDoub, doub ;

   max = strlen( bufIn ) ;
   for ( offset = 0 ; offset < max ; offset++ )
   {
      if ( bufIn[offset] != ' ' )
         break ;
   }

   if ( bufIn[offset] == '-' )
   {
      sign = -1 ;
      u4ncpy( buf, bufIn + offset + 1, sizeof( buf ) ) ;
   }
   else
   {
      sign = 1 ;
      u4ncpy( buf, bufIn + offset + 0, sizeof( buf ) ) ;
   }

   cPtr = strchr( buf, '.' ) ;
   if ( cPtr == 0 )
   {
      tptr = buf ;
      while ( (*tptr == '-') || ((*tptr >='0')&&(*tptr <= '9')) )
         tptr++ ;
      baseLen = tptr - buf ;
      ptr = 0 ;
   }
   else
   {
      baseLen = ( cPtr - buf ) ;
      ptr = buf + baseLen ;
      if ( ptr != cPtr )
         return -1 ;
   }

   if ( ptr )
   {
      *ptr++ = '\0' ;

      tptr = ptr ;
      while ( (*tptr >='0')&&(*tptr <= '9') )
          tptr++ ;
      decLenHold = tptr - ptr ;

      for ( i = decLenHold ; i > 0 ; i-- )
         decLen = decLen * 10  ;

      if ( decLenHold > 9 )   /* too big for a long, so alternate conversion */
      {
         decDoub = 0.0 ;
         for ( ; decLenHold > 9 ; decLenHold--, i++ )
            decDoub = decDoub * 10.0 + ptr[i] - '0' ;
         decDoub = decDoub * 1000000000 + (double)atol(ptr+i) ;
      }
      else
         decDoub = (double)atol(ptr) ;

      if ( baseLen > 9 )   /* too big for a long, so alternate conversion */
      {
         baseDoub = 0.0 ;
         for ( ; baseLen > 9 ; baseLen--, i++ )
            baseDoub = baseDoub * 10.0 + buf[i] - '0' ;
         baseDoub = baseDoub * 1000000000 + (double)atol(buf+i) ;
      }
      else
         baseDoub = (double)atol(buf) ;
      decDoub = (baseDoub >= 0 ) ? decDoub : -decDoub ;
      doub = baseDoub + decDoub / decLen ;
      *(--ptr) = '.' ;
   }
   else
   {
      if ( baseLen > 9 )  /* greater than size of long */
      {
         doub = 0.0 ;
         for ( ; baseLen > 9 ; baseLen--, i++ )
            doub = doub * 10.0 + buf[i] - '0' ;
         doub = doub * 1000000000 + (double)atol(buf+i) ;
      }
      else
         doub = (double)atol(buf) ;
   }
   return sign * doub ;
}
#endif

/* c4atod    Converts a string to a double */
double S4FUNCTION c4atod( const char *str, const int lenStr )
{
   char buffer[50] ;
   int  len ;

   len = ( lenStr >= 50 ) ? 49 : lenStr ;
   memcpy( buffer, str, (size_t)len ) ;
   buffer[len] = '\0' ;
   #ifdef S4NO_ATOF
      return c4atof( buffer ) ;
   #else
      return atof( buffer ) ;
   #endif
}

void S4FUNCTION c4atod2( char *str, int lenStr, double *result )
{
   char buffer[50] ;
   int  len ;

   len = ( lenStr >= 50 ) ? 49 : lenStr ;
   memcpy( buffer, str, (size_t)len ) ;
   buffer[len] = '\0' ;
   #ifdef S4NO_ATOF
      *result = c4atof( buffer ) ;
   #else
      *result = atof( buffer ) ;
   #endif
   return;
}

int S4FUNCTION c4atoi( const char *str, const int l )
{
   char buf[128] ;
   int lenStr ;

   if ( l >= (int)sizeof( buf ) )
      lenStr = (int)sizeof( buf ) - 1 ;
   else
      lenStr = l ;
   memcpy( buf, str, (size_t)lenStr ) ;
   buf[lenStr] = '\0' ;
   return atoi( buf ) ;
}

long S4FUNCTION c4atol( const char *str, const int l )
{
   char buf[128] ;
   int lenStr ;

   if ( l >= (int)sizeof( buf ) )
      lenStr = (int)sizeof( buf ) - 1 ;
   else
      lenStr = l ;

   memcpy( buf, str, (size_t)lenStr) ;
   buf[lenStr] = '\0' ;
   return atol( buf ) ;
}

#ifdef S4WINCE
/* Convert from normal string to Unicode */
void S4FUNCTION c4atou(const char *from, unsigned short *to,  int len)
{
   const char *end = (from+len) ;

   for (;from < end; from++, to++)
   {
      *(char *)to = *from ;
      *( ((char *)to)+1) = 0 ;
      if (*from == 0)
        break ;
   }
}

/* Convert from Unicode to a string */
void S4FUNCTION c4utoa( unsigned short *from)
{
   char *to = (char *)from ;

   for (;; from++, to++)
   {
      *(char *)to = *(char *)from ;
      if (*from == 0)
        break ;
   }
}
#endif

#ifdef S4CLIPPER
char *c4descendBinary( char *to, const char *from, int len )
{
   for(; len-- > 0; )
      to[len] = ~from[len] ;
   return to ;
}

#endif

/* note that the CLIPPER version is compatible with Clipper but does not
   properly descend names if nulls are included */
char *S4FUNCTION c4descend( char *to, const char *from, int len )
{
   for(; len-- > 0; )
      #ifdef S4CLIPPER
         to[len] = -from[len] ;
      #else
         to[len] = ~from[len] ;
      #endif
   return to ;
}

#ifdef S4CLIPPER
/* S4CLIPPER */
void c4dtoaClipper( double val, char *result, int len, int decimals )
{
   int digLen, isNeg, zerosLen, decPos, decLen, resultLen, i ;
   char *str, *ptr ;

   resultLen = len ;

   #ifdef S4NO_FCVT
      str = f4fcvt( val, decimals, &digLen, &isNeg) ;
   #else
      str = fcvt( val, decimals, &digLen, &isNeg) ;
   #endif

   zerosLen = resultLen - digLen - decimals - (decimals > 0 ) ;
   if ( zerosLen > 0 )
      memset( result, '0', zerosLen ) ;

   if ( decimals > 0 )
   {
      decPos = resultLen - decimals - 1 ;
      result[decPos] = '.' ;
   }
   else
      decPos = resultLen ;

   if ( zerosLen > 0 )
      ptr = result + zerosLen ;
   else
      ptr = result ;

   if ( digLen >= 0 )
   {
      if ( (decPos - digLen) < 0 )
      {
         memset( ptr, (int) '*', (size_t)resultLen) ;
         return ;
      }
      memcpy( ptr, str, (size_t) digLen ) ;
      if ( zerosLen >= 0 && decimals > 0 )
      {
         ptr += digLen ;
         *ptr = '.' ;
         memcpy( ++ptr, str+digLen, (size_t)decimals ) ;
      }
   }
   else
   {
      decLen = decimals + digLen ;
      if ( decLen > 0 )
         memcpy( ptr - digLen, str, decLen ) ;
   }

   if ( isNeg )
   {
      for ( i=0; i< resultLen; i++ )
         result[i] = (char) 0x5c - result[i] ;
   }
}
#endif

/* Clipper, DESCEND() & ASCEND() */
/* Numeric Key Database Output is Converted to Numeric Key Index File format */
int S4FUNCTION c4clip( char *ptr, int len)
{
   int i, negative ;
   char *p ;

   for ( i= negative= 0, p= ptr
       ; i< len
       ; i++, p++ )
   {
      if ( *p == ' ' )
      {
         *p = '0' ;
      }
      else
      {
         if ( *p == '-' )
         {
            *p = '0' ;
            negative = 1 ;
         }
         else
            break ;
      }
   }

   if ( negative )
   {
      for ( i= 0, p= ptr; i< len; i++, p++ )
         *p = (char) 0x5c - *p ;
   }

   return 0 ;
}

/* c4dtoa45
   - formats a double to a string
   - if there is an overflow, '*' are returned
*/
int S4FUNCTION c4dtoa45( double doubVal, char *outBuffer, int len, int dec)
{
   int preLen, postLen, signPos, decVal, signVal ;
   char *result ;

   #ifdef E4PARM_LOW
      if ( len < 0 || len >128 || dec < 0 || dec >= len )
         return error4( 0, e4parm, E95106 ) ;
   #endif

   memset( outBuffer, (int) '0', (size_t) len) ;

   if (dec > 0)
   {
      postLen = dec ;
      if (postLen > 15)
         postLen = 15 ;
      if (postLen > len-1)
         postLen = len-1 ;
      preLen = len -postLen -1 ;

      outBuffer[ preLen] = '.' ;
   }
   else
   {
      preLen = len ;
      postLen = 0 ;
   }

   #ifdef S4NO_FCVT
      result = (char *)f4fcvt( doubVal, postLen, &decVal, &signVal) ;
   #else
      result = (char *)fcvt( doubVal, postLen, &decVal, &signVal) ;
   #endif

   if (decVal > 0)
      signPos = preLen-decVal -1 ;
   else
   {
      signPos = preLen - 2 ;
      if ( preLen == 1 )
         signPos = 0 ;
   }

   if ( (decVal > preLen) ||  (preLen<0)  ||  ((signPos< 0) && signVal) )
   {
      /* overflow */
      memset( outBuffer, (int)'*', (size_t)len) ;
      return e4overflow ;  /* don't generate an error, but return state */
   }

   if (decVal > 0)
   {
      memset( outBuffer, (int)' ', (size_t)(preLen - decVal) ) ;
      c4memmove( outBuffer + preLen - decVal, result, (size_t)decVal) ;
      if ( ( outBuffer[preLen-1] == '\0' ) && (preLen > 0) )
         outBuffer[preLen-1] = '0' ;
   }
   else
   {
      if ( preLen> 0 )
         memset( outBuffer, (int) ' ', (size_t) (preLen-1)) ;
   }
   if ( signVal )
      outBuffer[signPos] = '-' ;


   outBuffer += preLen+1 ;
   if (decVal >= 0)
      result+= decVal ;
   else
   {
      outBuffer    -= decVal ;
      postLen += decVal ;
   }

   if ( postLen > (int) strlen(result) )
      postLen = (int) strlen( result) ;

   /*  - outBuffer   points to where the digits go to
       - result       points to where the digits are to be copied from
       - postLen     is the number to be copied
   */

   if ( postLen > 0 )
      c4memmove( outBuffer, result, (size_t)postLen ) ;

   return 0 ;
}

/* c4encode

   - From CCYYMMDD to CCYY.MM.DD

   Ex.        c4encode( to, from, "CCYY.MM.DD", "CCYYMMDD" ) ;
*/

void S4FUNCTION c4encode( char *to, const char *f, char *tTo, const char *tF )
{
   int pos ;
   char chr ;
   char *chrPos ;
   char const * tFrom, * from ;

   tFrom = tF ;
   from = f ;

   strcpy( to, tTo ) ;

   while ( (chr = *tFrom++) != 0)
   {
      if ( ( chrPos= strchr( tTo, chr ) ) == 0 )
      {
         from++;
         continue ;
      }

      pos = (int)( chrPos - tTo ) ;
      to[pos++] = *from++ ;

      while (chr == *tFrom)
      {
         if (chr == tTo[pos] )
            to[pos++] = *from ;
         tFrom++ ;
         from++ ;
} } }

/*  c4ltoa45

    Converts a long to a string.  Fill with '0's rather than blanks if
    'num' is less than zero.
*/

void S4FUNCTION c4ltoa45( long lVal, char *ptr, int num)
{
   int   n, numPos ;
   long  iLong ;

   iLong = (lVal>0) ? lVal : -lVal ;
   numPos = n = (num > 0) ? num : -num ;

   while (n-- > 0)
   {
      ptr[n] = (char) ('0'+ iLong%10) ;
      iLong = iLong/10 ;
   }

   if ( iLong > 0 )
   {
     memset( ptr, (int) '*', (size_t) numPos ) ;
     return ;
   }

   num--;
   for (n=0; n<num; n++)
      if (ptr[n]=='0')
         ptr[n]= ' ';
      else
         break ;

   if (lVal < 0)
   {
      if ( ptr[0] != ' ' )
      {
         memset( ptr, (int) '*', (size_t) numPos ) ;
         return ;
      }
      for (n=num; n>=0; n--)
         if (ptr[n]==' ')
         {
            ptr[n]= '-' ;
            break ;
         }
   }
}

void S4FUNCTION c4trimN( char *str, int nCh )
{
   int len ;

   if ( nCh <= 0 )
      return ;

   /* Count the Length */
   len = 0 ;
   while ( len< nCh )
   {
      len++ ;
      if ( *str++ == '\0' )
         break ;
   }

   if ( len < nCh )
      nCh = len ;

   *(--str) = '\0' ;

   while( --nCh > 0 )
   {
      str-- ;
      if ( *str == '\0' ||  *str == ' ' )
         *str = '\0' ;
      else
         break ;
   }
}

#ifndef S4LANGUAGE

void S4FUNCTION c4lower( char *str )
{
#ifdef S4NO_STRLWR
   char *ptr ;

   ptr = str ;

   while ( *ptr != '\0' )
   {
      if ( *ptr >= 'A' && *ptr <= 'Z' )
       *ptr |= 0x20 ;
      ptr++ ;
   }
#else
   #ifdef S4ANSI
      AnsiLower( str ) ;
   #else
      (void)strlwr( str ) ;
   #endif
#endif
}

#ifdef S4NO_TOUPPER
char toupper(char c)
{
   char ch = c;
   if ( (c)>='a' && (c)<='z' )
      ch &= 0xDF ;
   return ch ;
}
#endif

void S4FUNCTION c4upper( char *str )
{
#ifdef S4NO_STRUPR
   char *ptr ;

   ptr = str ;

   while ( *ptr != '\0' )
   {
      if ( *ptr >= 'a'  &&  *ptr <= 'z' )
       *ptr &= 0xDF ;
      ptr++ ;
   }
#else
   #ifdef S4ANSI
      AnsiUpper( str ) ;
   #else
      (void)strupr( str ) ;
   #endif
#endif
}

#else  /* ifdef S4LANGUAGE  */

#ifdef S4GERMAN
   void  S4FUNCTION c4upper( char *str )
   {
   #ifdef S4ANSI
      AnsiUpper(str) ;
   #else
      unsigned char *ptr ;

      ptr = (unsigned char *)str ;

      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'a'  &&  *ptr <= 'z' )
            *ptr &= 0xDF ;

         if ( *ptr >= 0x81 )
         {
            switch( *ptr )
            {
                   case 0x84:                      *ptr = 0x8E ;                      break ;
                   case 0x94:                      *ptr = 0x99 ;                      break ;
                   case 0x81:                      *ptr = 0x9A ;                      break ;
            }
         }
         ptr++ ;
      }
   #endif
   }

   void S4FUNCTION c4lower( char *str )
   {
   #ifdef S4ANSI
      AnsiLower(str) ;
   #else
      unsigned char *ptr ;

      ptr = (unsigned char *)str ;

      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'A' && *ptr <= 'Z' )
            *ptr |= 0x20 ;
         if ( *ptr >= 0x81 )
         {
            switch( *ptr )
            {
               case 0x8E:                        *ptr = 0x84 ;                        break ;
               case 0x99:                        *ptr = 0x94 ;                        break ;
               case 0x9A:                        *ptr = 0x81 ;                        break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
#endif /* S4GERMAN */

#ifdef S4FRENCH
   void  S4FUNCTION c4upper( char *str )
   {
   #ifdef S4ANSI
      AnsiUpper(str) ;
   #else
      unsigned char *ptr ;

      ptr = (unsigned char *)str ;

      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'a'  &&  *ptr <= 'z' )
            *ptr &= 0xDF ;

         /* Les accents sont laiss‚ afin d'ˆtre compatible avec toute p‚riph‚rie */
         if ( *ptr >= E4C_CED )
         {
            switch( *ptr )
            {
               case E4A_TRE :
               case E4A_GRA :
               case E4A_CIR :
               case E4A_CI2 :
               case E4A_EGU : *ptr = 'A' ;        /* A */                        break ;

               case E4C_CED : *ptr = 'C' ;        /* C */                  break ;

               case E4E_EGU :
               case E4E_GRA :
               case E4E_CIR :
               case E4E_TRE : *ptr = 'E' ;        /* E */                  break ;

               case E4I_TRE :
               case E4I_EGU :
               case E4I_GRA :
               case E4I_CIR : *ptr = 'I' ;        /* I */                  break ;

               case E4U_CIR :
               case E4U_TRE :
               case E4U_GRA :
               case E4U_EGU : *ptr = 'O' ;        /* O */                  break ;

               case E4O_CIR :
               case E4O_GRA :
               case E4O_TRE :
               case E4O_EGU : *ptr = 'U' ;        /* U */                  break ;

               case E4Y_TRE : *ptr = 'Y' ;        /* Y */                  break ;
         }  }
         ptr++ ;
      }
   #endif
   }

   void S4FUNCTION c4lower( char *str )
   {
   #ifdef S4ANSI
      AnsiLower(str) ;
   #else
      unsigned char *ptr ;

      ptr = (unsigned char *)str ;

      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'A' && *ptr <= 'Z' )
            *ptr |= 0x20 ;

         /* Les accents sont laiss‚ afin d'ˆtre compatible avec toute p‚riph‚rie */
         if ( *ptr >= E4C_CED )
         {
            switch( *ptr )
            {
               case E4CM_CED: *ptr = 'c' ;       /* c */                  break ;
               case E4AM_TRE:
               case E4AM_CIR: *ptr = 'a' ;       /* a */                  break ;
               case E4EM_EGU: *ptr = 'e' ;       /* e */                  break ;
               case E4OM_TRE: *ptr = 'o' ;       /* o */                  break ;
               case E4UM_TRE: *ptr = 'u' ;       /* u */                  break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
#endif   /*  S4FRENCH  */

#ifdef S4SWEDISH
   void  S4FUNCTION c4upper( char *str )
   {
   #ifdef S4ANSI
      AnsiUpper(str) ;
   #else
      unsigned char *ptr ;

      ptr = (unsigned char *)str ;

      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'a'  &&  *ptr <= 'z' )
            *ptr &= 0xDF ;

         if ( *ptr >= 0x81 )
         {
            switch( *ptr )
            {
                   case 0x81: *ptr = 0x9A; break;
                   case 0x82: *ptr = 0x90; break;
                   case 0x84: *ptr = 0x8E; break;
                   case 0x86: *ptr = 0x8F; break;
                   case 0x91: *ptr = 0x92; break;
                   case 0x94: *ptr = 0x99; break;
            }
         }
         ptr++ ;
      }
   #endif
   }

   void S4FUNCTION c4lower( char *str )
   {
   #ifdef S4ANSI
      AnsiLower( str ) ;
   #else
      unsigned char *ptr ;

      ptr = (unsigned char *)str ;

      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'A' && *ptr <= 'Z' )
            *ptr |= 0x20 ;
         if ( *ptr >= 0x81 )
         {
            switch( *ptr )
            {
               case 0x8E: *ptr = 0x84;  break ;
               case 0x8F: *ptr = 0x86;  break ;
               case 0x90: *ptr = 0x82;  break ;
               case 0x92: *ptr = 0x91;  break ;
               case 0x99: *ptr = 0x94;  break ;
               case 0x9A: *ptr = 0x81;  break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
#endif /* S4SWEDISH */

#ifdef S4FINNISH
   void  S4FUNCTION c4upper( char *str )
   {
   #ifdef S4ANSI
      AnsiUpper(str) ;
   #else
      unsigned char *ptr ;

      ptr = (unsigned char *)str ;

      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'a'  &&  *ptr <= 'z' )
            *ptr &= 0xDF ;

         if ( *ptr >= 0x81 )
         {
            switch( *ptr )
            {
                   case 0x81:
                      *ptr = 0x9A ;
                      break ;
                   case 0x82:
                      *ptr = 0x90 ;
                      break ;
                   case 0x84:
                      *ptr = 0x8E ;
                      break ;
                   case 0x86:
                      *ptr = 0x8F ;
                      break ;
                   case 0x91:
                      *ptr = 0x92 ;
                      break ;
                   case 0x94:
                      *ptr = 0x99 ;
                      break ;
            }
         }
         ptr++ ;
      }
   #endif
   }

   void S4FUNCTION c4lower( char *str )
   {
   #ifdef S4ANSI
      AnsiLower(str) ;
   #else
      unsigned char *ptr ;

      ptr = (unsigned char *)str ;

      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'A' && *ptr <= 'Z' )
            *ptr |= 0x20 ;
         if ( *ptr >= 0x81 )
         {
            switch( *ptr )
            {
               case 0x8E:
                        *ptr = 0x84 ;
                        break ;
               case 0x8F:
                        *ptr = 0x86 ;
                        break ;
               case 0x90:
                        *ptr = 0x82 ;
                        break ;
               case 0x92:
                        *ptr = 0x91 ;
                        break ;
               case 0x99:
                        *ptr = 0x94 ;
                        break ;
               case 0x9A:
                        *ptr = 0x81 ;
                        break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
#endif /* S4FINNISH */

#ifdef S4NORWEGIAN
   void  S4FUNCTION c4upper( char *str )
   {
   #ifdef S4ANSI
      AnsiUpper(str) ;
   #else
      unsigned char *ptr ;

      ptr = (unsigned char *)str ;

      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'a'  &&  *ptr <= 'z' )
            *ptr &= 0xDF ;

         if ( *ptr >= 0x81 )
         {
            switch( *ptr )
            {
                   case 0x81:
                      *ptr = 0x9A ;
                      break ;
                   case 0x82:
                      *ptr = 0x90 ;
                      break ;
                   case 0x84:
                      *ptr = 0x8E ;
                      break ;
                   case 0x86:
                      *ptr = 0x8F ;
                      break ;
                   case 0x91:
                      *ptr = 0x92 ;
                      break ;
                   case 0x94:
                      *ptr = 0x99 ;
                      break ;
            }
         }
         ptr++ ;
      }
   #endif
   }

   void S4FUNCTION c4lower( char *str )
   {
   #ifdef S4ANSI
      AnsiLower(str) ;
   #else
      unsigned char *ptr ;

      ptr = (unsigned char *)str ;

      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'A' && *ptr <= 'Z' )
            *ptr |= 0x20 ;
         if ( *ptr >= 0x81 )
         {
            switch( *ptr )
            {
               case 0x8E:
                        *ptr = 0x84 ;
                        break ;
               case 0x8F:
                        *ptr = 0x86 ;
                        break ;
               case 0x90:
                        *ptr = 0x82 ;
                        break ;
               case 0x92:
                        *ptr = 0x91 ;
                        break ;
               case 0x99:
                        *ptr = 0x94 ;
                        break ;
               case 0x9A:
                        *ptr = 0x81 ;
                        break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
#endif /* S4NORWEGIAN */

#endif   /*  S4LANGUAGE  */


#ifdef S4NO_MEMMOVE
void *c4memmove(void *dest, const void *src, size_t count )
{
   if ( dest < src )
      if ( (char *)dest + count  <= (char *) src )
      {
         memcpy( dest, src, count ) ;
         return( dest ) ;
      }
      else
      {
         /* Start at beginning of 'src' */
         int  i ;
         for ( i=0; i< count; i++ )
            ((char *) dest)[i] = ((char *)src)[i] ;
         return( dest ) ;
      }

   if ( src < dest )
      if ( (char *) src + count  <= (char *) dest )
      {
         memcpy( dest, src, count ) ;
         return( dest ) ;
      }
      else
      {
         /* Start at end of 'src' */
         for(;count!=0;)
         {
            --count ;
            ((char *)dest)[count] = ((char *) src)[count] ;
         }
         return( dest ) ;
      }
   return( dest ) ;
}
#endif  /* S4NO_MEMMOVE */

#ifdef S4MEMCMP
int c4memcmp(const void * __s1, const void * __s2, size_t __n)
{
   unsigned char *p1, *p2 ;
   int cnt ;

   p1 = (unsigned char *)__s1 ;
   p2 = (unsigned char *)__s2 ;

   for ( cnt = 0 ; cnt < __n ; cnt++ )
   {
      if ( p1[cnt] != p2[cnt] )
      {
         if ( p1[cnt] > p2[cnt] )
            return 1 ;
         return -1 ;
      }
   }
   return 0 ;
}
#endif   /* S4MEMCMP */

#ifdef S4NO_STRNICMP
int strnicmp(char *a, char *b, size_t n )
{
   unsigned char aChar, bChar ;

   for ( ; *a != '\0'  &&  *b != '\0' && n != 0; a++, b++, n-- )
   {
      aChar = (unsigned char) *a  & 0xDF ;
      bChar = (unsigned char) *b  & 0xDF ;

      if ( aChar < bChar )  return -1 ;
      if ( aChar > bChar )  return  1 ;
   }
   return 0 ;
}
#endif  /* S4NO_STRNICMP */

#ifndef S4OPTIMIZE_OFF
/* used by optimization */
int S4FUNCTION c4calcType( unsigned long l )
{
   int i ;

   for ( i = 0 ; l > 1 ; i++ )
      l >>= 1 ;
   return i ;
}

#endif
