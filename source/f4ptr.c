/* f4ptr.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

char *S4FUNCTION f4ptr( const FIELD4 *field )
{
   C4PARMFIELD( field, E90532, NULL );

   return ( field->data->record + field->offset ) ;
}
