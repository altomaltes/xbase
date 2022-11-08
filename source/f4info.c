/* f4info.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

FIELD4INFO *S4FUNCTION d4fieldInfo( DATA4 *data )
{
   FIELD4INFO *fieldInfo ;
   FIELD4 *field ;
   short int i ;

   #ifdef E4PARM_HIGH
      if ( data == 0 )
      {
         error4( 0, e4parmNull, E92201 ) ;
         return (FIELD4INFO *)0 ;
      }
   #endif

   if ( error4code( data->codeBase ) < 0 )
      return (FIELD4INFO *)0 ;

   fieldInfo = (FIELD4INFO *)u4allocFree( data->codeBase, ( (long)d4numFields( data ) + 1L ) * (long)sizeof( FIELD4INFO ) ) ;
   if ( fieldInfo == 0 )
      return (FIELD4INFO *)0 ;

   for ( i = 0 ; i < d4numFields( data ) ; i++ )
   {
      field = d4fieldJ( data, (short int)(i + 1) ) ;
      fieldInfo[i].name = field->name ;
      fieldInfo[i].len = field->len ;
      fieldInfo[i].dec = field->dec ;
      fieldInfo[i].type = (char)field->type ;
   }

   return fieldInfo ;
}
