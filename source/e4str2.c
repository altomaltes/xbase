/* e4str2.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

extern S4CONST char *bad4data ;

#ifdef P4ARGS_USED
   #pragma argsused
#endif
const char *S4FUNCTION error4text( CODE4 *c4, const long errCode2 )
{
   #ifdef E4OFF
      return bad4data ;
   #else
      #ifndef E4OFF_STRING
         ERROR4INFO_ARRAY *array ;
         long errCode ;
         unsigned int szArray ;
      #endif

      if ( errCode2 < 0L )   /* 1st level error code */
         return e4text( (int)errCode2 ) ;
      #ifdef E4OFF_STRING
         return bad4data ;
      #else
         if ( errCode2 < 10000 )
            return bad4data ;

         switch( errCode2 / 10000 )
         {
            #ifdef S4VBASIC
               case 4:
                  array = error4array4 ;
                  szArray = sizeof( error4array4 ) ;
                  break ;
            #endif
            #ifdef S4TESTING
               case 5:
                  array = error4array5 ;
                  szArray = sizeof( error4array5 ) ;
                  break ;
            #endif
            #ifdef S4CBPP
               case 6:
                  array = error4array6 ;
                  szArray = sizeof( error4array6 ) ;
                  break ;
            #endif
            case 8:
               array = error4array8 ;
               szArray = sizeof( error4array8 ) ;
               break ;
            case 9:
               array = error4array9 ;
               szArray = sizeof( error4array9 ) ;
               break ;
            default:
               return bad4data ;
         }
         errCode = (errCode2 % 10000) + 1;
         if ( errCode > ( (long)szArray / (long)sizeof( array[1] ) ) )
            return bad4data ;
         return array[(int)errCode].errorString ;
      #endif
   #endif
}

long S4FUNCTION error4number2( const long errCode2 )
{
   #ifndef E4OFF_STRING
      ERROR4INFO_ARRAY *array ;
      long errCode ;
      unsigned int szArray ;

      if ( errCode2 < 10000 )
         return 0 ;
      switch( errCode2 / 10000 )
      {
         #ifdef S4VBASIC
            case 4:
               array = error4array4 ;
               szArray = sizeof( error4array4 ) ;
               break ;
         #endif
         #ifdef S4TESTING
            case 5:
               array = error4array5 ;
               szArray = sizeof( error4array5 ) ;
               break ;
         #endif
         #ifdef S4CBPP
            case 6:
               array = error4array6 ;
               szArray = sizeof( error4array6 ) ;
               break ;
         #endif
         case 8:
            array = error4array8 ;
            szArray = sizeof( error4array8 ) ;
            break ;
         case 9:
            array = error4array9 ;
            szArray = sizeof( error4array9 ) ;
            break ;
         default:
            return 0 ;
      }
      errCode = (errCode2 % 10000) + 1;
      if ( errCode > ( (long)szArray / (long)sizeof( array[1] ) ) )
         return 0 ;
      return array[ errCode ].error_number ;
   #else
      return errCode2 ;
   #endif
}

#ifndef E4OFF_STRING
long error4seek( long errCode2 )
{
   long ePos, pos, sPos, nPos ;
   unsigned int szArray ;
   long arrayUsed ;
   ERROR4INFO_ARRAY *array ;

   if ( errCode2 < 10000 )
      return -1 ;
   arrayUsed = errCode2 / 10000 ;
   switch( arrayUsed )
   {
      #ifdef S4VBASIC
         case 4:
            array = error4array4 ;
            szArray = sizeof( error4array4 ) ;
            break ;
      #endif
      #ifdef S4TESTING
         case 5:
            array = error4array5 ;
            szArray = sizeof( error4array5 ) ;
            break ;
      #endif
      #ifdef S4CBPP
         case 6:
            array = error4array6 ;
            szArray = sizeof( error4array6 ) ;
            break ;
      #endif
      case 8:
         array = error4array8 ;
         szArray = sizeof( error4array8 ) ;
         break ;
      case 9:
         array = error4array9 ;
         szArray = sizeof( error4array9 ) ;
         break ;
      default:
         return -1 ;
   }

   ePos = szArray / sizeof( array[1] ) ;
   pos = ePos / 2 ;
   sPos = 0 ;

   for ( ;; )
   {
      if ( pos < 0L || pos >= ePos )  /* code not found */
         return 0 ;

      if ( array[pos].error_number == errCode2 )
         return (pos - 1 + 10000 * arrayUsed );

      if ( array[pos].error_number > errCode2 )
      {
         ePos = pos ;
         nPos = pos - ( pos - sPos ) / 2 ;
         if ( nPos == pos )
            pos-- ;
         else
            pos = nPos ;
      }
      else
      {
         sPos = pos ;
         nPos = pos + ( ePos - pos ) / 2 ;
         if ( nPos == pos )
            pos++ ;
         else
            pos = nPos ;
      }
   }
}

#endif /* S4OFF_STRING */
