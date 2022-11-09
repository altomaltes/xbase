/* f4memo.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

extern char f4memoNullChar ;

#ifdef S4VBASIC
   #ifdef __cplusplus
      extern "C" {
   #endif
   long S4FUNCTION f4memoLenVB( FIELD4 * ) ;
   #ifdef __cplusplus
      }
   #endif
#endif

#ifndef S4OFF_MEMO
/* used internally only */
void f4memoAssignField( FIELD4 *fieldTo, FIELD4 *fieldFrom )
{
   if ( fieldFrom->type == fieldTo->type && fieldFrom->type == r4memo )
      f4memoAssignN( fieldTo, f4memoPtr( fieldFrom ), f4memoLen( fieldFrom ) ) ;
   else
      f4assignField( fieldTo, fieldFrom ) ;
}
#endif

#ifndef S4OFF_WRITE
int S4FUNCTION f4memoAssign( FIELD4 *field, const char *ptr )
{
   E4PARHIGH( field, E90518 ) ;

   if ( error4code( field->data->codeBase ) < 0 )
      return e4codeBase ;

   #ifdef S4OFF_MEMO
      if ( ptr == 0 )
         f4assignN( field, (char *)0, 0 ) ;
      else
         f4assignN( field, ptr, (unsigned int)strlen( ptr ) ) ;
      return 0 ;
   #else
      if ( ptr == 0 )
         return f4memoAssignN( field, (char *)0, 0 ) ;
      else
         return f4memoAssignN( field, ptr, (unsigned int)strlen( ptr ) ) ;
   #endif
}

int S4FUNCTION f4memoAssignN( FIELD4 *field, const char *ptr, const unsigned int ptrLen )
{
   #ifndef S4OFF_MEMO
      CODE4 *c4 ;
      F4MEMO *mfield ;
      int rc ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( (void *)field, 3, E90519 ) )
         return -1 ;
   #endif

   E4PARHIGH( field, E90519 ) ;

   #ifdef S4OFF_MEMO
      f4assignN( field, ptr, ptrLen ) ;
   #else
      c4 = field->data->codeBase ;
      if ( error4code( c4 ) < 0 )
         return e4codeBase ;

         #ifndef S4OFF_ENFORCE_LOCK
            if ( c4->lockEnforce && field->data->recNum > 0L )
               if ( d4lockTest( field->data, field->data->recNum ) != 1 )
                  return error4( c4, e4lock, E90519 ) ;
         #endif

      if ( ptrLen > UINT_MAX - 128 )
         return error4( c4, e4memory, E85202 ) ;

      mfield = field->memo ;
      if ( !mfield )
         f4assignN( field, ptr, ptrLen ) ;
      else
      {
         #ifdef S4FOX
            if ( d4version( field->data ) == 0x30 )
               f4assignNotNull( field ) ;
         #endif
         rc = f4memoSetLen( field, ptrLen ) ;
         if ( rc )
            return error4stack( c4, rc, E90519 ) ;
         memcpy( mfield->contents, ptr, (size_t)ptrLen ) ;
      }
   #endif

   return 0 ;
}
#endif

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION f4memoFree( FIELD4 *field )
{
   #ifndef S4OFF_MEMO
      F4MEMO *mfield ;

   E4PARHIGH( field, E90521 ) ;

      mfield = field->memo ;

      #ifdef E4ANALYZE
         if ( !mfield )
            return error4( 0, e4struct, E90521 ) ;
      #endif

      if ( mfield->lenMax > 0 )
         u4free( mfield->contents ) ;

      mfield->contents = &f4memoNullChar ;

      mfield->status = 1 ;
      mfield->lenMax = 0 ;
   #endif
   return 0 ;
}

unsigned S4FUNCTION f4memoLen( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( (void *)field, 3, E90522 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( field == 0 )
      {
         error4( 0, e4parmNull, E90522 ) ;
         return 0 ;
      }
   #endif


   #ifdef S4OFF_MEMO
      return (unsigned)field->len ;
   #else
      if ( !field->memo )
         return (unsigned)field->len ;
      if ( field->memo->status == 1 )
      {
         if ( f4memoRead( field ) != 0 )
            return 0 ;
         field->memo->status = 0 ;
      }
      return field->memo->len ;
   #endif
}

unsigned S4FUNCTION f4memoNcpy( FIELD4 *field, char *memPtr, const unsigned len )
{
   #ifndef S4OFF_MEMO
      CODE4 *c4 ;
      unsigned numCpy ;
   #endif

   #ifdef E4PARM_HIGH
      if ( field == 0 )
      {
         error4( 0, e4parmNull, E90523 ) ;
         return 0 ;
      }
   #endif

   #ifdef S4OFF_MEMO
      return f4ncpy( field, memPtr, len ) ;
   #else
      c4 = field->data->codeBase ;
      if ( !field->memo )
         return f4ncpy( field, memPtr, len ) ;

      if ( len == 0 )
         return 0 ;

      if ( error4code( c4 ) < 0 )
         return 0 ;
      error4set( c4, 0 ) ;

      numCpy = f4memoLen( field ) ;
      if ( len <= numCpy )
         numCpy = len - 1 ;

      memcpy( memPtr, f4memoPtr( field ), (size_t)numCpy ) ;

      memPtr[numCpy] = '\000' ;

      return( numCpy ) ;
   #endif
}

char *S4FUNCTION f4memoPtr( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( (void *)field, 3, E90524 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( field == 0 )
      {
         error4( 0, e4parmNull, E90524 ) ;
         return 0 ;
      }
   #endif

   #ifdef S4OFF_MEMO
      return f4ptr(field) ;
   #else
      if ( !field->memo )
         return f4ptr(field) ;
      if ( field->memo->status == 1 )
      {
         if ( f4memoRead( field ) )
            return 0 ;
         field->memo->status = 0 ;
      }
      return field->memo->contents ;
   #endif
}

#ifndef S4OFF_MEMO
int f4memoRead( FIELD4 *field )
{
   int rc ;
   F4MEMO *mfield ;
   CODE4 *c4 ;

   E4PARHIGH( field, E90518 ) ;

   c4 = field->data->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   mfield = field->memo ;
   mfield->isChanged = 0 ;

   if ( d4recNo( field->data ) < 0 )
   {
      mfield->len = 0 ;
      return mfield->len ;
   }

      #ifndef S4OFF_MULTI
         if ( c4getReadLock( c4 ) )
         {
            rc = d4validateMemoIds( field->data ) ;
            if ( rc < 0 )
               return error4stack( c4, rc, E90525 ) ;
         }
      #endif

      rc = f4memoReadLow( field ) ;
      if ( rc < 0 )
         return error4stack( c4, rc, E90525 ) ;

      return 0 ;
}

int f4memoReadLow( FIELD4 *field )
{
   F4MEMO *mfield ;
   int rc ;
   #ifdef S4MFOX
      long mType ;
   #endif

   E4PARMLOW( field, E90526 ) ;

   if ( error4code( field->data->codeBase ) < 0 )
      return e4codeBase ;

   mfield = field->memo ;
   #ifdef E4ANALYZE
      if ( !mfield )
         return error4( field->data->codeBase, e4info, E80502 ) ;
   #endif

   mfield->len = mfield->lenMax  ;

   #ifdef S4MFOX
      rc = memo4fileRead( &field->data->dataFile->memoFile, f4long( field ), &mfield->contents, &mfield->len, &mType ) ;
   #else
      rc = memo4fileRead( &field->data->dataFile->memoFile, f4long( field ), &mfield->contents, &mfield->len ) ;
   #endif

   if ( rc == 0 )
   {
      #ifdef S4MNDX
         if ( mfield->lenMax > 0 )
            if ( mfield->len == 0 )
            {
               u4free( mfield->contents ) ;
               mfield->contents = 0 ;
            }
         if ( mfield->len > mfield->lenMax )
            mfield->lenMax = mfield->len  ;
         if ( mfield->len == 0 )
         {
            mfield->lenMax = 0 ;
            mfield->contents = &f4memoNullChar ;
         }
      #else
         if ( mfield->len > mfield->lenMax )
            mfield->lenMax = mfield->len  ;
         if ( mfield->lenMax > 0 )
            mfield->contents[mfield->len] = 0 ;
         else
            mfield->contents = &f4memoNullChar ;
      #endif
   }

   return rc ;
}

int f4memoReset( FIELD4 *field )
{
   E4PARMLOW( field, E90527 ) ;


   #ifdef E4ANALYZE
      if ( field->memo == 0 )
         return error4( 0, e4struct, E90527 ) ;
   #endif

   field->memo->len = 0 ;
   field->memo->status = 1 ;
   field->memo->isChanged = 0 ;

   return 0 ;
}

#ifndef S4OFF_WRITE
int S4FUNCTION f4memoSetLen( FIELD4 *field, const unsigned len )
{
   F4MEMO *mfield ;
   CODE4 *c4 ;

   E4PARMLOW( field, E90528 ) ;


   c4 = field->data->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

      #ifndef S4OFF_ENFORCE_LOCK
         if ( c4->lockEnforce  && field->data->recNum > 0L )
            if ( d4lockTest( field->data, field->data->recNum ) != 1 )
               return error4( c4, e4lock, E90528 ) ;
      #endif

   mfield = field->memo ;
   if ( mfield == 0 )
      return -1 ;

   if ( mfield->lenMax < len )
   {
      if ( mfield->lenMax > 0 )
         u4free( mfield->contents ) ;
      mfield->lenMax = len ;

      mfield->contents = (char *)u4allocEr( c4, (long)mfield->lenMax + 1L ) ;
      if ( mfield->contents == 0 )
      {
         mfield->contents = &f4memoNullChar ;
         mfield->lenMax = 0 ;
         mfield->status = 1 ;
         return e4memory ;
      }
   }

   mfield->len = len ;

   if ( mfield->lenMax == 0 )
      mfield->contents = &f4memoNullChar ;
   else
      mfield->contents[mfield->len] = 0 ;

   mfield->status = 0 ;
   mfield->isChanged = 1 ;
   field->data->recordChanged = 1 ;
   return 0 ;
}
#endif  /* S4OFF_WRITE */
#endif  /* S4OFF_MEMO */

S4CONST char *S4FUNCTION f4memoStr( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( (void *)field, 3, E90529 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( field == 0 )
      {
         error4( 0, e4parmNull, E90529 ) ;
         return 0 ;
      }
   #endif

   #ifdef S4OFF_MEMO
      return f4str( field ) ;
   #else
      if ( field->memo == 0 )
         return f4str( field ) ;
      return f4memoPtr( field ) ;
   #endif
}
#ifndef S4OFF_WRITE
#ifndef S4OFF_MEMO
int f4memoUpdate( FIELD4 *field )
{
   E4PARMLOW( field, E90530 ) ;


   if ( error4code( field->data->codeBase ) < 0 )
      return e4codeBase ;

   if ( field->memo )
      if ( field->memo->isChanged )
         return f4memoWrite( field ) ;

   return 0 ;
}

int f4memoWrite( FIELD4 *field )
{
   int rc ;
   long memoId, newId ;

   E4PARMLOW( field, E90531 ) ;


   if ( error4code( field->data->codeBase ) < 0 )
      return e4codeBase ;

   #ifndef S4OFF_MULTI
      rc = d4validateMemoIds( field->data ) ;
      if ( rc < 0 )
         return error4stack( field->data->codeBase, rc, E90531 ) ;
   #endif

   memoId = f4long( field ) ;
   newId = memoId ;

   rc = memo4fileWrite( &field->data->dataFile->memoFile, &newId, field->memo->contents, field->memo->len ) ;

   if ( rc < 0 )
      return error4stack( field->data->codeBase, rc, E90531 ) ;

   if ( newId != memoId )
   {
      if ( newId )
         f4assignLong( field, newId ) ;
      else
         f4blank( field ) ;   /* for FOX 3.0, must set to 0, else spaces */
   }

   field->memo->isChanged = 0 ;
   return 0 ;
}
#endif  /* S4OFF_MEMO */
#endif  /* S4OFF_WRITE */


#ifdef S4VBASIC

long S4FUNCTION f4memoLenVB( FIELD4 *f4 )
{
   return (long)f4memoLen( f4 ) ;
}

#ifdef S4VB_DOS

int f4memoAssign ( FIELD4 *fld, char *data )
{
   return f4memoAssignN ( fld, data, StringLength( data ) ) ;
}


int f4memoAssignN( FIELD4 *fld, char *data, int len )
{
   char *cBuf ;
   int rc ;

   if( (cBuf = (char *) u4alloc(len + 1) ) )
   {
      u4vtoc( cBuf, len+1, data ) ;
      rc = f4memoAssignN( fld, cBuf, len ) ;
      u4free( cBuf ) ;
      return rc ;
   }
   else
      return error4( fld->codeBase, e4memory, E4_MEMORY_MEMO );
}

char * f4memoStr( FIELD4 *fld )
{
   return v4str( f4memoStr(fld) ) ;
}

#endif /* S4VB_DOS */

#endif /* S4VBASIC */
