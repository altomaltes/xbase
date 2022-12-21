/* f4true.c (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved.

   Returns a true or false.
*/

#include "d4all.h"

#include <ctype.h>

int S4FUNCTION f4true( const FIELD4 *field )
{
   char charValue ;

   C4PARMFIELD( field, E90537, -1 );

   #ifdef E4PARM_HIGH
      if ( f4type( field ) != r4log )
         error4( field->data->codeBase, e4parm, E81409 ) ;
   #endif

   charValue = (char) toupper( *f4ptr( field ) ) ;
   return ( charValue == 'Y' || charValue == 'T' ) ? 1 : 0 ;
}
