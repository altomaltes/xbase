/* f4str.c (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

/* Returns a pointer to static string corresponding to the field.
   This string will end in a NULL character.
*/

#include "d4all.h"

#ifndef S4OFF_WRITE
void S4FUNCTION f4assign( FIELD4 *field, const char *str )
{
 
   C4PARMFIELD( field, E90533, );

   #ifdef E4ANALYZE
      if ( field->data == 0 )
      {
         error4( 0, e4struct, E90533 ) ;
         return ;
      }
      if ( field->data->codeBase == 0 )
      {
         error4( 0, e4struct, E90533 ) ;
         return ;
      }
   #endif

   if ( error4code( field->data->codeBase ) < 0 )
      return ;

      #ifndef S4OFF_ENFORCE_LOCK
         if ( field->data->codeBase->lockEnforce && field->data->recNum > 0L )
            if ( d4lockTest( field->data, field->data->recNum ) != 1 )
            {
               error4( field->data->codeBase, e4lock, E90533 ) ;
               return ;
            }
      #endif

   f4assignN( field, str, (unsigned)strlen(str) ) ;
}

void S4FUNCTION f4assignN( FIELD4 *field, const char *ptr, const unsigned ptrLen )
{
   char *fPtr ;
   unsigned pLen ;

   C4PARMFIELD( field, E90534, NULL );

   #ifdef E4PARM_HIGH
      if ( field == 0 || ( ptr == 0 && ptrLen ) )
      {
         error4( 0, e4parmNull, E90534 ) ;
         return ;
      }
   #endif

   if ( error4code( field->data->codeBase ) < 0 )
      return ;

      #ifndef S4OFF_ENFORCE_LOCK
         if ( field->data->codeBase->lockEnforce && field->data->recNum > 0L )
            if ( d4lockTest( field->data, field->data->recNum ) != 1 )
            {
               error4( field->data->codeBase, e4lock, E90534 ) ;
               return ;
            }
      #endif

   fPtr = f4assignPtr( field ) ;

   if ( ptrLen > field->len )
      pLen = field->len ;
   else
   {
      pLen = ptrLen ;

      /* in this case, also need to blank out extra bytes in field. */
      /* use f4blank() because of possible null */
      f4blank( field ) ;
   }

   /* Copy the data into the record buffer. */
   memcpy( fPtr, ptr, (size_t)pLen ) ;
}
#endif

unsigned int S4FUNCTION f4ncpy( FIELD4 *field, char *memPtr, const unsigned int memLen )
{
   unsigned numCpy ;

   if ( memLen == 0 )
      return 0 ;

   C4PARMFIELD( field, E90535, NULL );

   numCpy = field->len ;
   if ( memLen <= numCpy )
      numCpy = memLen - 1 ;

   /* 'f4ptr' returns a pointer to the field within the database record buffer. */
   memcpy( memPtr, f4ptr( field ), (size_t)numCpy ) ;

   memPtr[numCpy] = '\000' ;

   return numCpy ;
}

char *S4FUNCTION f4str( FIELD4 *field )
{
   CODE4 *codeBase ;

   C4PARMFIELD( field, E90536, NULL );

   codeBase = field->data->codeBase ;

   if ( codeBase->bufLen <= field->len )   /* not room for field length + null */
   {
      if ( u4allocAgain( codeBase, &codeBase->fieldBuffer, &codeBase->bufLen, field->len + 1 ) < 0 )
      {
         #ifdef E4STACK
            error4stack( codeBase, e4memory, E90536 ) ;
         #endif
         return 0 ;
      }
   }
   else
      codeBase->fieldBuffer[field->len] = 0 ;

   memcpy( codeBase->fieldBuffer, f4ptr( field ), field->len ) ;
   return codeBase->fieldBuffer ;
}

#ifdef S4VB_DOS

void f4assign_v( FIELD4 *fld, const char *data )
{
   f4assignN ( fld, data, StringLength((char near *)data) ) ;
}

int f4assignN ( FIELD4 *fld, const char *data, const int len )
{
   char *cBuf;

   if( (cBuf = (char *) u4alloc(len + 1) ) )
   {
      u4vtoc( cBuf, len+1, data ) ;
      f4assignN( fld, cBuf, len ) ;
      u4free( cBuf );
   }
   else
      return error4( fld->codeBase, e4memory, E90534 );
}

char *f4str_v( FIELD4 *fld )
{
   return v4str( f4str(fld) ) ;
}

#endif

