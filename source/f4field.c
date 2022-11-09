/* f4field.c (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifdef S4VBASIC
   #ifdef __cplusplus
      extern "C" {
   #endif
   int S4FUNCTION f4len_v( FIELD4 * ) ;
   #ifdef __cplusplus
      }
   #endif
#endif

#ifndef S4OFF_WRITE
char *S4FUNCTION f4assignPtr( FIELD4 *field )
{
   #ifdef E4PARM_HIGH
      if ( field == 0 )
      {
         error4( 0, e4parmNull, E90507 ) ;
         return 0 ;
      }
   #endif

   #ifdef S4CFOX
      if ( d4version( field->data ) == 0x30 )
         f4assignNotNull( field ) ;
   #endif

   field->data->recordChanged = 1 ;

   return ( field->data->record + field->offset ) ;
}

void S4FUNCTION f4blank( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( (void *)field, 3, E90508 ) )
         return ;
   #endif

   #ifdef E4PARM_HIGH
      if ( field == 0 )
      {
         error4( 0, e4parmNull, E90508 ) ;
         return ;
      }
   #endif

   if ( error4code( field->data->codeBase ) < 0 )
      return ;

      #ifndef S4OFF_ENFORCE_LOCK
         if ( field->data->codeBase->lockEnforce && field->data->recNum > 0L )
            if ( d4lockTest( field->data, field->data->recNum ) != 1 )
            {
               error4( field->data->codeBase, e4lock, E90508 ) ;
               return ;
            }
      #endif

   #ifdef S4CFOX
      if ( field->binary )
   #else
      if ( field->type == r5wstr || field->type == r4int )
   #endif
      memset( f4assignPtr( field ), 0, field->len ) ;
   else
      memset( f4assignPtr( field ), ' ', field->len ) ;
}
#endif

DATA4 *S4FUNCTION f4data( const FIELD4 *field )
{
   #ifdef E4PARM_HIGH
      if ( field == 0 )
      {
         error4( 0, e4parmNull, E90509 ) ;
         return 0 ;
      }
   #endif

   return field->data ;
}

int S4FUNCTION f4decimals( const FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( (void *)field, 3, E90510 ) )
         return -1 ;
   #endif

   E4PARHIGH( field, E90510 ) ;

   return field->dec ;
}

unsigned S4FUNCTION f4len( const FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( (void *)field, 3, E90511 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( field == 0 )
      {
         error4( 0, e4parmNull, E90511 ) ;
         return 0 ;
      }
   #endif

   return field->len ;
}

S4CONST char *S4FUNCTION f4name( S4CONST FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( (void *)field, 3, E90512 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( field == 0 )
      {
         error4( 0, e4parmNull, E90512 ) ;
         return 0 ;
      }
   #endif

   return field->name ;
}

int S4FUNCTION f4number( const FIELD4 *field )
{
   FIELD4 *fieldOn ;
   int fNum ;

   E4PARHIGH( field, E90538 ) ;

   fieldOn = field->data->fields ;

   for ( fNum = 1 ;;  fNum++ )
   {
      if ( fieldOn == field )
         return fNum ;
      fieldOn++ ;
   }
}

int S4FUNCTION f4type( const FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( (void *)field, 3, E90513 ) )
         return -1 ;
   #endif

   E4PARHIGH( field, E90513 ) ;

   #ifdef S4CFOX
      switch( field->type )
      {
         case 'F':
            return (int)r4num ;
         case 'C':
            if ( field->binary )
               return r4charBin ;
            else
               return r4str ;
         case 'M':
            if ( field->binary == 1 )
               return r4memoBin ;
            else
               return r4memo ;
         default:
            return (int)field->type ;
      }
   #else
      if ( field->type == 'F' )
         return (int)r4num ;
      else
         return (int)field->type ;
   #endif
}

static void negate( CURRENCY4 *, const CURRENCY4 * ) ;
static void shiftLeft( long *, long *, int ) ;

int S4FUNCTION date4timeCompare( const long *dt1, const long *dt2 )
{
   long date1, date2, time1, time2 ;

   date1 = dt1[0] ;
   date2 = dt2[0] ;

   if ( date1 == date2 )  /* must compare on times */
   {
      time1 = dt1[1] ;
      time2 = dt2[1] ;

      if ( time1 > time2 )
         return 1 ;
      if ( time1 < time2 )
         return -1 ;
      return 0 ;
   }
   else  /* dates not equal */
   {
      if ( date1 > date2 )
         return 1 ;
      return -1 ;
   }
}

/* returns -1 if c1 < c2, 0 if equal, 1 if c1 > c2 */
int S4FUNCTION currency4compare( const CURRENCY4 *c1, const CURRENCY4 *c2 )
{
   char sign1, sign2 ;
   short int loop ;
   long result ;

   sign1 = (short int)(c1->lo[3]) >= 0 ? 1 : -1 ;
   sign2 = (short int)(c2->lo[3]) >= 0 ? 1 : -1 ;

   if ( sign1 != sign2 )   /* one positive, one negative, so comparison easy */
   {
      if ( sign1 == -1 )
         return -1 ;
      else
         return 1 ;
   }

   if ( sign1 == -1 )   /* negative comparison, larger value is smaller */
   {
      for ( loop = 3 ; loop > 0 ; loop-- )
      {
         result = c1->lo[loop] ;
         result -= c2->lo[loop] ;

         if ( result < 0 )
            return 1 ;
         if ( result > 0 )
            return -1 ;
      }
      return 0 ;
   }

   /* otherwise both values positive, larger is larger */
   for ( loop = 3 ; loop > 0 ; loop-- )
   {
      result = c1->lo[loop] ;
      result -= c2->lo[loop] ;

      if ( result < 0 )
         return -1 ;
      if ( result > 0 )
         return 1 ;
   }
   return 0 ;
}

int S4FUNCTION currency4add( CURRENCY4 *result, const CURRENCY4 *c1, const CURRENCY4 *c2 )
{
   char sign1, sign2, carry ;
   short int loop ;
   long val1, val2, resultVal ;

   sign1 = (short int)(c1->lo[3]) >= 0 ? 1 : -1 ;
   sign2 = (short int)(c2->lo[3]) >= 0 ? 1 : -1 ;

   carry = 0 ;
   for ( loop = 0 ; loop < 4 ; loop++ )
   {
      if ( sign1 == -1 && sign2 == -1 )
      {
         val1 = -(short int)(c1->lo[loop]) ;
         val2 = -(short int)(c2->lo[loop]) ;
         if ( val1 == 0 )
            val1 = 65536 ;
         if ( val2 == 0 )
            val2 = 65536 ;
         if ( loop != 0 )
            val2-- ;
      }
      else
      {
         val1 = c1->lo[loop] ;
         val2 = c2->lo[loop] ;
      }
      resultVal = val1 + val2 + carry ;
      carry = 0 ;
      if ( resultVal > USHRT_MAX )
      {
         #ifdef S4TESTING
            if ( ( resultVal - (long)USHRT_MAX ) > USHRT_MAX )
               return error4( 0, e4info, E99999 ) ;
         #endif
         carry = 1 ;
         result->lo[loop] = (int)(resultVal - (long)USHRT_MAX - 1) ;
      }
      else
         result->lo[loop] = (int)resultVal ;
      if ( sign1 == -1 && sign2 == -1 )
         result->lo[loop] = -((short int)result->lo[loop]) ;
   }

   return 0 ;
}

int S4FUNCTION currency4subtract( CURRENCY4 *result, const CURRENCY4 *c1, const CURRENCY4 *c2 )
{
   CURRENCY4 temp ;

   memcpy( &temp, c2, sizeof( CURRENCY4 ) ) ;
   negate( &temp, &temp ) ;

   currency4add( result, c1, &temp ) ;

   return 0 ;
}

int S4FUNCTION currency4multiplyShort( CURRENCY4 *result, const CURRENCY4 *c1, const short c2 )
{
   int loop, sign1, sign2, signResult, pos ;
   CURRENCY4 cur1, hold ;
   unsigned long val1, valResult ;
   short int cur2 ;

   sign1 = (short int)(c1->lo[3]) >= 0 ? 1 : -1 ;
   sign2 = c2 >= 0 ? 1 : -1 ;

   if ( sign1 == -1 )
   {
      negate( &cur1, c1 ) ;
      signResult = -1 ;
   }
   else
   {
      memcpy( &cur1, c1, sizeof( CURRENCY4 ) ) ;
      signResult = 1 ;
   }

   if ( sign2 == -1 )
   {
      cur2 = -c2 ;
      signResult *= -1 ;
   }
   else  /* no change to sign */
      cur2 = c2 ;

   memset( result, 0, sizeof( CURRENCY4 ) ) ;

   for ( loop = 0 ; loop < 4 ; loop++ )
   {
      val1 = cur1.lo[loop] ;
      pos = loop ;
      if ( pos >= 4 )  /* too large to store results */
         break ;
      if ( val1 == 0 )
         continue ;
      valResult = val1 * cur2 ;
      memset( &hold, 0, sizeof( CURRENCY4 ) ) ;
      if ( pos == 3 )  /* only least significant bytes important */
         memcpy( &(hold.lo[pos]), &valResult, 2 ) ;
      else
         memcpy( &(hold.lo[pos]), &valResult, 4 ) ;
      currency4add( result, result, &hold ) ;
   }

   if ( signResult == -1 )
      negate( result, result ) ;

   return 0 ;
}
#ifdef NOT_IMPLEMENTED_YET
int currency4multiply( CURRENCY4 *result, const CURRENCY4 *c1, const CURRENCY4 *c2 )
{
   int loop, sign1, sign2, signResult, jLoop, pos ;
   CURRENCY4 cur1, cur2, hold, tenThousand ;
   unsigned long val1, val2, valResult ;

   sign1 = (short int)(c1->lo[3]) >= 0 ? 1 : -1 ;
   sign2 = (short int)(c2->lo[3]) >= 0 ? 1 : -1 ;

   if ( sign1 == -1 )
   {
      negate( &cur1, c1 ) ;
      signResult = -1 ;
   }
   else
   {
      memcpy( &cur1, c1, sizeof( CURRENCY4 ) ) ;
      signResult = 1 ;
   }

   if ( sign2 == -1 )
   {
      negate( &cur2, c2 ) ;
      signResult *= -1 ;
   }
   else  /* no change to sign */
      memcpy( &cur2, c2, sizeof( CURRENCY4 ) ) ;

   memset( result, 0, sizeof( CURRENCY4 ) ) ;
   memset( &tenThousand, 0, sizeof( CURRENCY4 ) ) ;
   tenThousand.lo[0] = 0x2710 ;

   for ( jLoop = 0 ; jLoop < 4 ; jLoop++ )
   {
      val2 = cur2.lo[jLoop] ;
      if ( val2 == 0 )
         continue ;
      for ( loop = 0 ; loop < 4 ; loop++ )
      {
         val1 = cur1.lo[loop] ;
         pos = loop + jLoop ;
         if ( pos >= 4 )  /* too large to store results */
            break ;
         if ( val1 == 0 )
            continue ;
         valResult = val1 * val2 ;
         memset( &hold, 0, sizeof( CURRENCY4 ) ) ;
         if ( pos == 3 )  /* only least significant bytes important */
            memcpy( &(hold.lo[pos]), &valResult, 2 ) ;
         else
            memcpy( &(hold.lo[pos]), &valResult, 4 ) ;
         currency4add( result, result, &hold ) ;
      }
   }

   if ( signResult == -1 )
      negate( result, result ) ;

   return 0 ;
}
#endif

static void negate( CURRENCY4 *result, const CURRENCY4 *source )
{
   int loop ;
   CURRENCY4 one ;

   if ( ( (short int)(source->lo[3]) >= 0 ? 1 : -1 ) == -1 )  /* negative */
   {
      memset( &one, 0, sizeof( CURRENCY4 ) ) ;
      one.lo[0] = 0x0001 ;

      for ( loop = 0; loop < 4 ; loop++ )
         result->lo[loop] = ~source->lo[loop] ;
      currency4add( result, result, &one ) ;
   }
   else /* positive */
   {
      memset( &one, (unsigned char)0xFF, sizeof( CURRENCY4 ) ) ;

      memcpy( result, source, sizeof( CURRENCY4 ) ) ;
      currency4add( result, result, &one ) ;
      for ( loop = 0; loop < 4 ; loop++ )
         result->lo[loop] = ~result->lo[loop] ;
   }
}

static void shiftLeft( long *left, long *right, int numShift )
{
   int loop ;

   for ( loop = 0 ; loop < numShift ; loop++ )
   {
      *left = *left << 1 ;
      *left = *left | ( 1 * ( *right & 0x80000000 ) ) ;
      *right = *right << 1 ;
   }
}

static void shiftRight( long *left, long *right, int numShift )
{
   int loop ;

   for ( loop = 0 ; loop < numShift ; loop++ )
   {
      *right = *right >> 1 ;
      *right = *right | ( 0x80000000 * ( *left & 0x00000001 ) ) ;
      *left = *left >> 1 ;
   }
}

int S4FUNCTION currency4divideShort( CURRENCY4 *result, const CURRENCY4 *c1, const short c2 )
{
   unsigned short int shortResult ;
   short int cur2 ;
   unsigned long shortMod ;
   int sign1, sign2, signResult, loop ;
   CURRENCY4 add, cur1 ;

   sign1 = (short int)(c1->lo[3]) >= 0 ? 1 : -1 ;
   sign2 = c2 >= 0 ? 1 : -1 ;

   if ( sign1 == -1 )
   {
      negate( &cur1, c1 ) ;
      signResult = -1 ;
   }
   else
   {
      memcpy( &cur1, c1, sizeof( CURRENCY4 ) ) ;
      signResult = 1 ;
   }

   if ( sign2 == -1 )
   {
      cur2 = -c2 ;
      signResult *= -1 ;
   }
   else  /* no change to sign */
      cur2 = c2 ;

   if ( cur2 == 0 )
      return -1 ;

   memset( result, 0, sizeof( CURRENCY4 ) ) ;

   if ( *((long *)(&cur1.lo[2])) == 0 )  /* value smaller than max long */
   {
      *((long *)(&result->lo[0])) = *((long *)(&cur1.lo[0])) / cur2 ;
      if ( signResult == -1 )
         negate( result, result ) ;
      return 0 ;
   }

   shortMod = 0 ;
   for ( loop = 3 ; loop >= 0 ; loop-- )
   {
      memset( &add, 0, sizeof( CURRENCY4 ) ) ;
      shortResult = (unsigned short)(((unsigned long)cur1.lo[loop] + shortMod )/ cur2) ;
      memcpy( ((unsigned short int *)(&add)) + loop, &shortResult, sizeof( short int ) ) ;
      if ( loop != 0 )
         shortMod = ((unsigned long)(USHRT_MAX)+1) * ((cur1.lo[loop] + shortMod) % cur2 ) ;
      currency4add( result, result, &add) ;
   }

   if ( signResult == -1 )
      negate( result, result ) ;

   return 0 ;
}
#ifdef NOT_IMPLEMENTED_YET
int currency4divide( CURRENCY4 *result, const CURRENCY4 *c1, const CURRENCY4 *c2 )
{
   long topLeft, topRight, bottomLeft, bottomRight, resultRight ;
   long stopShiftVal, holdLeft, holdRight ;
   int sign1, sign2, signResult, numShift, signHold ;
   CURRENCY4 add, subtract, hold, hold2, cur1, cur2 ;

   sign1 = (short int)(c1->lo[3]) >= 0 ? 1 : -1 ;
   sign2 = (short int)(c2->lo[3]) >= 0 ? 1 : -1 ;

   if ( sign1 == -1 )
   {
      negate( &cur1, c1 ) ;
      signResult = -1 ;
   }
   else
   {
      memcpy( &cur1, c1, sizeof( CURRENCY4 ) ) ;
      signResult = 1 ;
   }

   if ( sign2 == -1 )
   {
      negate( &cur2, c2 ) ;
      signResult *= -1 ;
   }
   else  /* no change to sign */
      memcpy( &cur2, c2, sizeof( CURRENCY4 ) ) ;

   topLeft = *((long *)(&cur1.lo[2])) ;
   topRight = *((long *)(&cur1.lo[0])) ;
   bottomLeft = *((long *)(&cur2.lo[2])) ;
   bottomRight = *((long *)(&cur2.lo[0])) ;

   if ( bottomLeft == 0 && bottomRight == 0 )  /* divide by zero */
      return -1 ;

   memset( result, 0, sizeof( CURRENCY4 ) ) ;

   if ( topLeft == 0 )  /* value smaller than max long */
   {
      if ( bottomLeft != 0 )  /* bottom larger than top */
         return 0 ;
      if ( bottomRight == 0 )
         return -1 ;
      resultRight = topRight / bottomRight ;
      memcpy( &(result->lo[0]), &resultRight, sizeof(S4LONG ) ) ;
      if ( signResult == -1 )
         negate( result, result ) ;
      return 0 ;
   }

   if ( topLeft < bottomLeft )   /* zero return */
   {
      if ( signResult == -1 )
         negate( result, result ) ;
      return 0 ;
   }

   if ( topLeft == bottomLeft )  /* either one or zero return */
   {
      if ( topRight >= bottomRight )
      {
         result->lo[0] = 1 ;
         if ( signResult == -1 )
            negate( result, result ) ;
      }
      return 0 ;
   }

   /* case where topLeft > 0 && topLeft > bottomLeft */
   memcpy( &(hold.lo[0]), &topRight, sizeof(S4LONG ) ) ;
   memcpy( &(hold.lo[2]), &topLeft, sizeof(S4LONG ) ) ;
   stopShiftVal = topLeft / 2 ;
   for ( ;; )
   {
      numShift = -1 ;
      while( bottomLeft < stopShiftVal )
      {
         holdLeft = bottomLeft ;
         holdRight = bottomRight ;
         shiftLeft( &bottomLeft, &bottomRight, 1 ) ;
         numShift++ ;
      }

      bottomLeft = holdLeft ;
      bottomRight = holdRight ;

      memset( &add, 0, sizeof( CURRENCY4 ) ) ;

      currency4add( result, result, &add ) ;

      #ifdef E4DEBUG
         if ( ( (short int)(hold->lo[3]) >= 0 ? 1 : -1 ) == -1 )
            return error4( 0, e4info, E90542 ) ;
      #endif

      for ( signHold = 1 ;; )
      {
         memcpy( &(subtract.lo[0]), &holdRight, sizeof(S4LONG ) ) ;
         memcpy( &(subtract.lo[2]), &holdLeft, sizeof(S4LONG ) ) ;
         memcpy( &hold2, &hold, sizeof( CURRENCY4 ) ) ;
         currency4subtract( &hold, &hold, &subtract ) ;
         signHold = (short int)(hold.lo[3]) >= 0 ? 1 : -1 ;
         if ( signHold == -1 )
            break ;
      }

      memcpy( &hold, &hold2, sizeof( CURRENCY4 ) ) ;
      topLeft = *((long *)(&hold.lo[2])) ;
      topRight = *((long *)(&hold.lo[0])) ;

      stopShiftVal = topLeft / 2 ;
      while ( bottomLeft > topLeft )
      {
         shiftRight( &bottomLeft, &bottomRight, 1 ) ;
         numShift-- ;
      }
   }

   if ( signResult == -1 )
      negate( result, result ) ;

   return 0 ;
}
#endif

/*
int currency4mod( CURRENCY4 *result, const CURRENCY4 *c1, const CURRENCY4 *c1 )
{

}
*/

int S4FUNCTION c4currencyToA( char *out, int outLen, const CURRENCY4 *sourceIn, int numDec )
{
   int sign, pos, loop, reqdLen ;
   CURRENCY4 hold, old, mod, sv, source ;
   unsigned char buf[21] ;
   unsigned long l1 ;

   memcpy( &source, sourceIn, sizeof( CURRENCY4 ) ) ;
   sign = (short int)(source.lo[3]) >= 0 ? 1 : -1 ;
   if ( sign == -1 )
      negate( &source, &source ) ;

   memcpy( &hold, &source, sizeof( CURRENCY4 ) ) ;
   memset( buf, 0, sizeof( buf ) ) ;

   for ( pos = 0 ; pos < 20 ; pos++ )
   {
      if ( *((long *)(&hold.lo[2])) == 0L )  /* only right portion */
         break ;
      memcpy( &old, &hold, sizeof( CURRENCY4 ) ) ;
      currency4divideShort( &hold, &hold, 10 ) ;
      currency4multiplyShort( &sv, &hold, 10 ) ;
      currency4subtract( &mod, &old, &sv ) ;
      buf[pos] = (char)mod.lo[0] ;
   }

   /* if any portion is left, it is only the right portion */
   if ( *((long *)(&hold.lo[2])) == 0L )  /* only right portion */
   {
      l1 = *((long *)(&hold.lo[0])) ;
      if ( l1 == 0 )
      {
         if ( pos == 0 )   /* zero */
            pos = -1 ;
      }
      else
      {
         for ( ; pos < 20 ; )
         {
            buf[pos] = (unsigned char) (l1 % 10) ;
            l1 = l1 / 10 ;
            if ( l1 == 0 )
               break ;
            pos++ ;
         }
      }
   }

   memset( out, 0, outLen ) ;

   if ( pos == -1 )
   {
      reqdLen = 6 ;  /* x.xxxx */
      memset( out, '0', 5 ) ;
   }
   else
   {
      reqdLen = pos + 2 ; /* for decimal point and because pos is 1 less than amount */
      if ( reqdLen < 6 )
      {
         memset( out, '0', 5 ) ;
         reqdLen = 6 ;  /* x.xxxx */
      }
   }


   if ( (reqdLen+1) > outLen + (sign == -1) )   /* insufficient space */
      return -1 ;

   loop = 5 - (pos+1) ;
   if ( loop < 0 )
      loop = 0 ;
   for ( ; pos >= 0 ; pos--, loop++ )  /* reverse the string */
      out[loop] = buf[pos] + '0' ;

   /* now reposition for the decimal points */
   c4memmove( &(out[reqdLen-4]), &(out[reqdLen-5]), 4 ) ;
   out[reqdLen-5] = '.' ;

   /* now remove any decimal points according to input */
   if ( numDec == 0 )
   {
      reqdLen -= 5 ;
      out[reqdLen] = 0 ;
   }
   else
   {
      if ( numDec > 4 || numDec < 0 )
         numDec = 4 ;
      out[reqdLen-4+numDec] = 0 ;
      reqdLen -= (4 - numDec) ;
   }

   if ( sign == -1 )
   {
      c4memmove( out+1, out, reqdLen + 1 ) ;
      out[0] = '-' ;
   }

   return 0 ;
}

/* returns -1 if an invalid currency input string */
int S4FUNCTION c4atoCurrency( CURRENCY4 *result, const char *str, int strLen )
{
   char buf[21] ;
   int loop, len, numDecimals, numWhole, sign, numDigits ;
   short int multiplier, mPower ;
   long val ;
   const char *decPt ;
   CURRENCY4 temp, mult ;
   char *ptr ;

   memset( result, 0, sizeof( CURRENCY4 ) ) ;

   decPt = 0 ;
   len = strLen ;
   if ( len == 0 )
      return 0 ;
   for ( ;; )
   {
      if ( str[0] != ' ' )
         break ;
      len-- ;
      str++ ;
      if ( len == 0 )
         return 0 ;
   }
   if ( str[0] == '-' )
   {
      sign = -1 ;
      str++ ;
      len-- ;
   }
   else
   {
      sign = 1 ;
      if ( str[0] == '+' )
      {
         str++ ;
         len-- ;
      }
   }
   while ( str[0] == '0' && str[1] == '0' )  /* get rid of leading zeros */
   {
      str++ ;
      len-- ;
   }
   numDecimals = 0 ;
   numWhole = len ;
   for ( loop = len - 1 ; loop >= 0 ; loop-- )
   {
      if ( str[loop] == '.' )
      {
         numDecimals = len - loop - 1 ;
         numWhole = len - numDecimals - 1 ;
         decPt = &str[loop]+1 ;
         break ;
      }
   }

   if ( numDecimals > 4 )
      return -1 ;

   numDigits = numWhole + 4 ;

   if ( numDigits > 20 )
      return -1 ;

   memcpy( buf, str, numWhole ) ;
   if ( decPt != 0 )
      c4memmove( buf+numWhole, decPt, numDecimals ) ;
   for ( loop = numDecimals ; loop < 4 ; loop++ )
   {
      buf[numWhole+loop] = '0' ;
   }

   buf[numDigits] = 0 ;

   if ( numDigits < 10 || ( numDigits == 10 && ( str[0] <= '3' && str[0] >= '0' ) ) )
   {
      /* fits in a single long */

      val = c4atol( buf, strlen( buf ) ) ;
      if ( sign == -1 )
         val = -val ;

      if ( val == 0 )   /* negative 0 is still zero */
         sign = 1 ;

      if ( sign == 1 )   /* positive value */
      {
         result->lo[3] = 0 ;
         result->lo[2] = 0 ;
      }
      else   /* negative value */
      {
         result->lo[3] = 0xFFFF ;
         result->lo[2] = 0xFFFF ;
      }

      memcpy( &result->lo[0], &val, sizeof(S4LONG ) ) ;

      return 0 ;
   }

   /* more complex scenario because number is larger than a long */
   /* first get the first 9 bytes and store to long */
   val = c4atol( buf, 9 ) ;

   memcpy( &result->lo[0], &val, sizeof(S4LONG) ) ;

   ptr = buf + 9 ;
   numDigits -= 9 ;
   while ( numDigits > 4 )
   {
      multiplier = c4atoi( ptr, 4 ) ;
      memset( &mult, 0, sizeof( CURRENCY4 ) ) ;
      mult.lo[0] = 10000 ;
      currency4multiplyShort( &temp, result, 10000 ) ;
      *result = temp ;
      if ( multiplier != 0 )  /* add */
      {
         memset( &temp, 0, sizeof( temp ) ) ;
         memcpy( &(temp.lo[0]), &multiplier, sizeof( short ) ) ;
         currency4add( result, result, &temp ) ;
      }
      numDigits -= 4 ;
      ptr += 4 ;
   }

   if ( numDigits != 0 )
   {
      multiplier = c4atoi( ptr, numDigits ) ;
      mPower = 1 ;
      for ( ; numDigits > 0 ; numDigits-- )
         mPower *= 10 ;
      currency4multiplyShort( &temp, result, mPower ) ;
      *result = temp ;
      if ( multiplier != 0 )  /* add */
      {
         memset( &temp, 0, sizeof( temp ) ) ;
         memcpy( &(temp.lo[0]), &multiplier, sizeof( short ) ) ;
         currency4add( result, result, &temp ) ;
      }
   }

   if ( sign == -1 )
   {
      memset( &mult, 0, sizeof( CURRENCY4 ) ) ;
      mult.lo[0] = 0x0001 ;
      currency4subtract( result, result, &mult ) ;
      for ( loop = 0; loop < 4 ; loop++ )
         result->lo[loop] = ~result->lo[loop] ;
   }

   return 0 ;
}

#ifndef S4OFF_WRITE
void S4FUNCTION f4assignCurrency( FIELD4 *field, char *str )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E90541 ) )
         return ;
   #endif

   #ifdef E4PARM_HIGH
      if ( field == 0 || str == 0 )
      {
         error4( 0, e4parmNull, E90541 ) ;
         return ;
      }
   #endif

   if ( field->type != r4currency )
   {
      #ifdef E4PARM_HIGH
         error4( field->data->codeBase, e4parm, E81409 ) ;
      #endif
      return ;
   }

   #ifdef E4ANALYZE
      if ( field->data == 0 )
      {
         error4( 0, e4struct, E90541 ) ;
         return ;
      }
      if ( field->data->codeBase == 0 )
      {
         error4( 0, e4struct, E90541 ) ;
         return ;
      }
   #endif

   #ifdef E4PARM_HIGH
      if ( field->type != r4currency )
      {
         error4( field->data->codeBase, e4parm, E81404 ) ;
         return ;
      }
   #endif

   if ( error4code( field->data->codeBase ) < 0 )
      return ;

      #ifndef S4OFF_ENFORCE_LOCK
         if ( field->data->codeBase->lockEnforce && field->data->recNum > 0L )
            if ( d4lockTest( field->data, field->data->recNum ) != 1 )
            {
               error4( field->data->codeBase, e4lock, E90541 ) ;
               return ;
            }
      #endif

   c4atoCurrency( (CURRENCY4 *)f4assignPtr( field ), str, strlen( str ) ) ;
}
#endif /* S4OFF_WRITE */

/* this function uses the same memory as f4str() */
char *S4FUNCTION f4currency( const FIELD4 *field, int numDec )
{
   CODE4 *codeBase ;

   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E90542 ) )
         return 0 ;
   #endif

   E4PARM_TRET( field == 0 || numDec < 0 || numDec > 4, E90542, NULL ) ;

   codeBase = field->data->codeBase ;

   E4PARM_TRET( field->type != r4currency, E90542, NULL ) ;

   if ( error4code( codeBase ) < 0 )
      return 0 ;

   if ( codeBase->bufLen <= 20 )   /* not room for field length + null */
   {
      if ( u4allocAgain( codeBase, &codeBase->fieldBuffer, &codeBase->bufLen, 21 ) < 0 )
      {
         #ifdef E4STACK
            error4stack( codeBase, e4memory, E90542 ) ;
         #endif
         return 0 ;
      }
   }
   else
      codeBase->fieldBuffer[20] = 0 ;

   c4currencyToA( codeBase->fieldBuffer, 20, (CURRENCY4 *)f4ptr( field ), numDec ) ;
   return codeBase->fieldBuffer ;
}

#ifdef S4CFOX

#ifndef S4OFF_WRITE
void S4FUNCTION f4assignNotNull( FIELD4 *field )
{
   FIELD4 *nullFlags ;
   unsigned short int byteNum, offset ;
   char mask ;
   char *fPtr ;

   if ( d4version( field->data ) != 0x30 )
      return ;

   if ( field->null == 1 )
   {
      if ( f4null( field ) )
      {
         nullFlags = &(field->data->fields[d4numFields(field->data)]) ;
         #ifdef E4MISC
            if ( nullFlags->type != r4system )
            {
               error4( field->data->codeBase, e4data, E83805 ) ;
               return ;
            }
         #endif
         byteNum = field->nullBit / 8 ;
         #ifdef E4MISC
            if ( nullFlags->len < ( byteNum + 1 ) )
            {
               error4( field->data->codeBase, e4data, E83805 ) ;
               return ;
            }
         #endif
         offset = ( field->nullBit - (byteNum * 8) ) ;
         mask = (unsigned int)0x01 << offset ;
         mask = ~mask ;
         fPtr = f4ptr( nullFlags ) ;
         fPtr[byteNum] = fPtr[byteNum] & mask ;
      }
   }
}

void S4FUNCTION f4assignNull( FIELD4 *field )
{
   FIELD4 *nullFlags ;
   unsigned short int byteNum, offset ;
   char mask ;
   char *fPtr ;

   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E90539 ) )
         return ;
   #endif

   #ifdef E4PARM_HIGH
      if ( field == 0 )
      {
         error4( 0, e4parmNull, E90539 ) ;
         return ;
      }
   #endif

   if ( error4code( field->data->codeBase ) < 0 )
      return ;

   if ( d4version( field->data ) != 0x30 )
   {
      error4( field->data->codeBase, e4parm, E81409 ) ;
      return ;
   }

      #ifndef S4OFF_ENFORCE_LOCK
         if ( field->data->codeBase->lockEnforce && field->data->recNum > 0L )
            if ( d4lockTest( field->data, field->data->recNum ) != 1 )
            {
               error4( field->data->codeBase, e4lock, E90539 ) ;
               return ;
            }
      #endif

   if ( field->null == 1 )
   {
      nullFlags = &(field->data->fields[d4numFields(field->data)]) ;
      #ifdef E4MISC
         if ( nullFlags->type != r4system )
         {
            error4( field->data->codeBase, e4data, E83805 ) ;
            return ;
         }
      #endif
      byteNum = field->nullBit / 8 ;
      #ifdef E4MISC
         if ( nullFlags->len < ( byteNum + 1 ) )
         {
            error4( field->data->codeBase, e4data, E83805 ) ;
            return ;
         }
      #endif
      offset = ( field->nullBit - (byteNum * 8 ) ) ;
      mask = 0x01 << offset ;
      fPtr = f4assignPtr( nullFlags ) ;
      fPtr[byteNum] = fPtr[byteNum] | mask ;
   }
   #ifdef E4PARM_HIGH
      else
         error4( field->data->codeBase, e4parm, E81409 ) ;
   #endif
}
#endif  /* S4OFF_WRITE */

int S4FUNCTION f4null( const FIELD4 *field )
{
   FIELD4 *nullFlags ;
   unsigned short int byteNum, offset ;
   char mask ;

   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E90540 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( field == 0 )
         return error4( 0, e4parmNull, E90540 ) ;
   #endif

   if ( d4version( field->data ) != 0x30 )
      return 0 ;

   if ( field->null == 1 )
   {
      nullFlags = &(field->data->fields[d4numFields(field->data)]) ;
      #ifdef E4MISC
         if ( nullFlags->type != r4system )
            return error4( field->data->codeBase, e4data, E83805 ) ;
      #endif
      byteNum = field->nullBit / 8 ;
      #ifdef E4MISC
         if ( nullFlags->len < ( byteNum + 1 ) )
            return error4( field->data->codeBase, e4data, E83805 ) ;
      #endif
      offset = ( field->nullBit - (byteNum * 8 ) ) ;
      mask = 0x01 << offset ;
      if ( ( f4ptr( nullFlags )[byteNum] & mask ) != 0 )
         return 1 ;
   }
   return 0 ;
}
#endif /* S4CFOX */

static void time4assign( char *ptr, unsigned long inVal )
{
   long seconds, minutes, hours, milliSeconds, val ;

   val = inVal / 1000 ;  /* remove the thousandths's of seconds */
   milliSeconds = inVal - ( val * 1000 ) ;
   seconds = val % 60 ;
   val /= 60 ;
   minutes = val % 60 ;
   val /= 60 ;
   hours = val ;

   c4ltoa45( hours, ptr, -2 ) ;  /* - means fill with 0s */
   ptr[2] = ':' ;
   c4ltoa45( minutes, ptr+3, -2 ) ;  /* - means fill with 0s */
   ptr[5] = ':' ;
   c4ltoa45( seconds, ptr+6, -2 ) ;  /* - means fill with 0s */
   if ( milliSeconds != 0 )
   {
      ptr[8] = ':' ;
      c4ltoa45( milliSeconds, ptr+9, -3 ) ;  /* - means fill with 0s */
   }
}

/* this function uses the same memory as f4str() */
char S4PTR *S4FUNCTION f4dateTime( const FIELD4 *field )
{
   CODE4 *codeBase ;
   long val ;
   char *fPtr ;

   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E90543 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( field == 0 )
      {
         error4( 0, e4parm, E90543 ) ;
         return 0 ;
      }
   #endif

   codeBase = field->data->codeBase ;

   #ifdef E4PARM_HIGH
      if ( field->type != r4dateTime )
      {
         error4( codeBase, e4parm, E81409 ) ;
         return 0 ;
      }
   #endif

   if ( error4code( codeBase ) < 0 )
      return 0 ;

   if ( codeBase->bufLen <= 20 )   /* not room for field length + null (incl. millisecs) */
   {
      if ( u4allocAgain( codeBase, &codeBase->fieldBuffer, &codeBase->bufLen, 21 ) < 0 )
      {
         #ifdef E4STACK
            error4stack( codeBase, e4memory, E90542 ) ;
         #endif
         return 0 ;
      }
   }
   memset( codeBase->fieldBuffer, ' ', 20 ) ;
   codeBase->fieldBuffer[20] = 0 ;

   fPtr = f4ptr( field ) ;
   val = *((long *)fPtr) ; /* date */
   date4assign( codeBase->fieldBuffer, val ) ;
   val = *(((long *)fPtr) + 1) ; /* time */
   time4assign( codeBase->fieldBuffer + 8, val ) ;
   c4trimN( codeBase->fieldBuffer, 21 ) ;
   return codeBase->fieldBuffer ;
}

/* input date time of format: "YYYYMMDDHH:MM:SS:MMM" */
void S4FUNCTION f4assignDateTime( FIELD4 *field, char *dateTime )
{
   #ifdef S4OFF_WRITE
      error4( field->data->codeBase, e4notSupported, E90543 ) ;
   #else
      long date, time ;

      if ( field->type != r4dateTime )
      {
         #ifdef E4PARM_HIGH
            error4( field->data->codeBase, e4parm, E81409 ) ;
         #endif
         return ;
      }

      date = date4long( dateTime ) ;
      time = time4long( dateTime + 8, strlen( dateTime ) - 8 ) ;

      *((long *)f4assignPtr( field )) = date ;
      *(((long *)f4assignPtr( field ))+1) = time ;
   #endif /* S4OFF_WRITE */
}

#ifdef S4VBASIC
   int S4FUNCTION f4len_v( FIELD4 *f4 )
   {
      return (int)f4len( f4 );
   }

   #ifdef S4VB_DOS
      char * f4name_v( FIELD4 *fld )
      {
         return v4str( f4name(fld) ) ;
      }
   #endif /* S4VB_DOS */
#endif /* S4VBASIC */
