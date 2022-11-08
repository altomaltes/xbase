/* d4field.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

FIELD4 *S4FUNCTION d4field( DATA4 *data, const char *fieldName )
{
   int i ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E94001 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
      {
         error4( 0, e4parmNull, E94001 ) ;
         return 0 ;
      }
   #endif

   i =  d4fieldNumber( data, fieldName ) - 1 ;
   if ( i < 0 )
      return 0 ;

   return data->fields + i ;
}

FIELD4 *S4FUNCTION d4fieldJ( DATA4 *data, const int jField )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E94002 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
      {
         error4( 0, e4parmNull, E94002 ) ;
         return (FIELD4 *)0 ;
      }
      if ( jField <= 0 || data->fields == 0 || jField > data->dataFile->nFields )
      {
         error4( data->codeBase, e4parm, E94002 ) ;
         return (FIELD4 *)0 ;
      }
   #endif

   return data->fields + jField - 1 ;
}

int S4FUNCTION d4fieldNumber( DATA4 *data, const char *fieldName )
{
   char buf[256] ;
   int i ;

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parmNull, E94003 ) ;
   #endif

   if ( fieldName )
   {
      u4ncpy( buf, fieldName, sizeof( buf ) ) ;
      c4trimN( buf, sizeof( buf ) ) ;
      c4upper( buf ) ;

      for ( i = 0 ; i < data->dataFile->nFields ; i++ )
         if ( !strcmp( buf, data->fields[i].name ) )
            return i + 1 ;
   }

   if ( data->codeBase->errFieldName )
      return error4describe( data->codeBase, e4fieldName, E94003, fieldName, 0, 0 ) ;
   return e4fieldName ;
}

#ifdef S4VB_DOS

FIELD4 *d4field_v( DATA4 *d4, char *fldName )
{
   return d4field( d4, c4str(fldName) ) ;
}

int d4fieldNumber( DATA4 *d4, char *fldName )
{
   return d4fieldNumber( d4, c4str(fldName) ) ;
}

#endif
