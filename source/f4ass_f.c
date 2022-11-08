/* f4ass_f.c (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_WRITE
void S4FUNCTION f4assignField( FIELD4 *fieldTo, const FIELD4 *fieldFrom )
{
   #ifdef E4PARM_HIGH
      if ( fieldTo == 0 || fieldFrom == 0 )
      {
         error4( 0, e4parmNull, E90501 ) ;
         return ;
      }
   #endif

   if ( error4code( fieldTo->data->codeBase ) < 0 )
      return ;

      #ifndef S4OFF_ENFORCE_LOCK
         if ( fieldTo->data->codeBase->lockEnforce && fieldTo->data->recNum > 0L )
            if ( d4lockTest( fieldTo->data, fieldTo->data->recNum ) != 1 )
            {
               error4( fieldTo->data->codeBase, e4lock, E90501 ) ;
               return ;
            }
      #endif

   #ifdef CLIENT_OR_FOX
      /* first check if null, if so then assign null */
      if ( f4null( fieldFrom ) )
         f4assignNull( fieldTo ) ;
      else
   #endif
      switch( fieldTo->type )
      {
         case r4str:
         case r5wstr:
            f4assignN( fieldTo, f4ptr( fieldFrom ), fieldFrom->len ) ;
            break ;
         case r4num:
         case r4float:
            if( fieldFrom->len == fieldTo->len && fieldFrom->dec == fieldTo->dec &&
                (fieldFrom->type == r4num || fieldFrom->type == r4float) )
               memcpy( f4assignPtr( fieldTo ), f4ptr( fieldFrom ), fieldTo->len ) ;
            else
               f4assignDouble( fieldTo, f4double( fieldFrom ) ) ;
            break ;
         case r4log:
            if( fieldFrom->type == r4log )
               *f4assignPtr( fieldTo ) = *f4ptr( fieldFrom ) ;
            break ;
         case r4date:
         #ifdef CLIENT_OR_FOX
            case r4currency:
            case r4dateTime:
            case r4int:
            case r4double:
         #endif
            if( fieldFrom->type == fieldTo->type )
               memcpy( f4assignPtr( fieldTo ), f4ptr( fieldFrom ), fieldTo->len ) ;
            break ;
         default:
            error4( fieldTo->data->codeBase, e4info, E80501 ) ;
      }
}
#endif  /* S4OFF_WRITE */

