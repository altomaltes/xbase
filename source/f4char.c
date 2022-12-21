/* f4char.c (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_WRITE
void S4FUNCTION f4assignChar( FIELD4 *field, const int chr )
{
   C4PARMFIELD( field, E90502,  );

   #ifdef E4PARM_HIGH
      switch( field->type )
      {
         case r4date:
         case r4float:
         case r4num:
         case r4memo:
         case r4gen:
         #ifdef S4FOX
            case r4currency:
            case r4dateTime:
            case r4double:
            case r4int:
            case r4system:
            case r4memoBin:
         #endif
            error4( field->data->codeBase, e4parm, E81409 ) ;
            return ;
         default:
            break ;
      }
   #endif

   if ( error4code( field->data->codeBase ) < 0 )
      return ;

      #ifndef S4OFF_ENFORCE_LOCK
         if ( field->data->codeBase->lockEnforce && field->data->recNum > 0L )
            if ( d4lockTest( field->data, field->data->recNum ) != 1 )
            {
               error4( field->data->codeBase, e4lock, E90502 ) ;
               return ;
            }
      #endif

   f4blank( field ) ;
   *f4assignPtr( field ) = (char)chr ;
}
#endif

int S4FUNCTION f4char( const FIELD4 *field )
{
   C4PARMFIELD( field, E90503,  );

   #ifdef E4PARM_HIGH
      #ifdef S4FOX
         switch( field->type )
         {
            case r4currency:
            case r4dateTime:
            case r4double:
            case r4int:
               return error4( field->data->codeBase, e4parm, E81409 ) ;
            default:
               break ;
         }
      #endif
   #endif

   /* Return the first character of the record buffer. */
   return *f4ptr( field ) ;
}
