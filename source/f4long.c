/* f4long.c (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_WRITE
void S4FUNCTION f4assignLong( FIELD4 *field, const long lValue )
{
   CODE4 *c4 ;

   #ifdef S4VBASIC
      if ( c4parm_check( (void *)field, 3, E90516 ) )
         return ;
   #endif

   #ifdef E4PARM_HIGH
      if ( field == 0 )
      {
         error4( 0, e4parmNull, E90516 ) ;
         return ;
      }
      switch( field->type )
      {
         case r4log:
         #ifdef S4CFOX
            case r4system:
            case r4dateTime:
         #endif
            error4( field->data->codeBase, e4parm, E81409 ) ;
            return ;
         default:
            break ;
      }
   #endif

   c4 = field->data->codeBase ;

   if ( error4code( c4 ) < 0 )
      return ;

      #ifndef S4OFF_ENFORCE_LOCK
         if ( c4->lockEnforce && field->data->recNum > 0L )
            if ( d4lockTest( field->data, field->data->recNum ) != 1 )
            {
               error4( c4, e4lock, E90516 ) ;
               return ;
            }
      #endif

   switch ( field->type )
   {
      case r4date:
         date4assign( f4assignPtr( field ), lValue ) ;
         break ;
      case r4int:
         f4assignInt( field, (int)lValue ) ;
         break ;
      #ifdef S4CFOX
         case r4double:
            f4assignDouble( field, (double)lValue ) ;
            break ;
         case r4memo:
         case r4memoBin:
         case r4gen:
            if ( f4len( field ) == 4 )
               #ifdef WORDS_BIGENDIAN
                  *((S4LONG *)f4assignPtr( field )) = x4reverseLong(&lValue) ;
               #else
                  *((S4LONG *)f4assignPtr( field )) = lValue ;
               #endif
            else
               c4ltoa45( lValue, f4assignPtr( field ), field->len ) ;
            break ;
      #endif
      default:
         if ( field->dec == 0 )
            c4ltoa45( lValue, f4assignPtr( field ), field->len ) ;
         else
            f4assignDouble( field, (double)lValue ) ;
         break ;
   }
}
#endif

long S4FUNCTION f4long( const FIELD4 *field )
{
   #ifdef S4CFOX
      const char *ptr ;
   #endif
   #ifdef S4VBASIC
      if ( c4parm_check( (void *)field, 3, E90517 ) )
         return 0L ;
   #endif

   #ifdef E4PARM_HIGH
      if ( field == 0 )
      {
         error4( 0, e4parmNull, E90517 ) ;
         return -1L ;
      }
      switch( field->type )
      {
         case r4log:
         #ifdef S4CFOX
            case r4system:
            case r4dateTime:
         #endif
            error4( field->data->codeBase, e4parm, E81409 ) ;
            return -1L ;
         default:
            break ;
      }
   #endif

   switch( field->type )
   {
      case r4date:
         return date4long( f4ptr( field ) ) ;
      case r4int:
         #ifdef WORDS_BIGENDIAN
            return x4reverseLong(f4ptr(field)) ;
         #else
            return *((S4LONG *)f4ptr( field )) ;
         #endif
      #ifdef S4CFOX
         case r4memo:
         case r4memoBin:
         case r4gen:
            if ( f4len( field ) == 4 )
               #ifdef WORDS_BIGENDIAN
                  return x4reverseLong(f4ptr(field)) ;
               #else
                  return *((S4LONG *)f4ptr( field )) ;
               #endif
            break ;
         case r4double:
            return (long)(*((double *)f4ptr( field ))) ;
         case r4currency:
            ptr = f4currency( field, 0 ) ;
            return c4atol( ptr, strlen( ptr ) ) ;
      #endif
      default:
         break ;
   }
   return c4atol( f4ptr( field ), field->len ) ;
}
