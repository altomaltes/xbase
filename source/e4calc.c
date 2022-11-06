/* e4calc.c (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

void S4FUNCTION expr4calcMassage( EXPR4CALC *calc )
{
   EXPR4 *exp4 = calc->expr;

   if( exp4->type == r4num )
   {
      /* Must consist of a single numeric field */
      exp4->type =  r4numDoub ;
      exp4->len =  sizeof(double) ;
      exp4->info->len =  f4len( exp4->info->fieldPtr ) ;
      exp4->info->functionI =  E4FIELD_NUM_D ;
      exp4->info->function =  (S4OPERATOR *)v4functions[E4FIELD_NUM_D].functionPtr ;
   }
}

EXPR4CALC *S4FUNCTION code4calcCreate( CODE4 *c4, EXPR4 *exp4, const char *name )
{
   EXPR4CALC *calcPtr ;

   #ifdef E4PARM_HIGH
      if ( c4 == 0 || exp4 == 0 || name == 0 )
      {
         error4( 0, e4parm_null, E90920 ) ;
         return 0 ;
      }
   #endif

   calcPtr = (EXPR4CALC *)mem4createAlloc( c4, &c4->calcMemory, 5, sizeof(EXPR4CALC), 5, 0 ) ;
   if ( calcPtr == 0 )
      return 0 ;

      l4add( &c4->calcList, calcPtr ) ;
   calcPtr->expr = exp4 ;
   u4ncpy( calcPtr->name, name, sizeof(calcPtr->name) ) ;
   c4upper( calcPtr->name ) ;

   expr4calcMassage( calcPtr ) ;


   return calcPtr ;
}

EXPR4CALC *S4FUNCTION expr4calcLookup( CODE4 *c4, const char *name, const unsigned int nameLenIn )
{
   EXPR4CALC *calcOn ;
   char buf[sizeof(calcOn->name)] ;
   unsigned int nameLen ;

   nameLen = nameLenIn ;

   if ( nameLen >= sizeof(calcOn->name) )
      nameLen = sizeof(calcOn->name)-1 ;
   u4ncpy( buf, name, nameLen+1 ) ;
   c4upper( buf ) ;
   for( calcOn = 0 ;; )
   {
         calcOn = (EXPR4CALC *)l4next( &c4->calcList, calcOn ) ;
      if ( calcOn == 0 )
         return 0 ;
      if ( strcmp( calcOn->name, buf) == 0 )
         return calcOn ;
   }
}

void S4FUNCTION expr4calcResultPos( EXPR4CALC *calcPtr, const int newResultPos )
{
   E4INFO *info ;
   int i, offset = newResultPos - calcPtr->curResultPos ;
   if ( offset == 0 )
      return ;

   calcPtr->curResultPos = newResultPos ;

   info = calcPtr->expr->info ;
   for( i = calcPtr->expr->infoN; --i >= 0; )
      info[i].resultPos += offset ;
}

#ifdef S4WINTEL
/* Expression source must be updated to refect the fact that a calculation had a name change */
/* Caller must ensure names are trimmed in front and back & are upper case */
int S4FUNCTION expr4calcNameChange( EXPR4 **exprOn, const char *oldName, const char *newName )
{
   EXPR4 *newExpr ;
   char bufMem[50] ;
   char *buf ;
   unsigned pos, bufLen ;
   int oldNameLen, ptrLen, newNameLen, didAlloc, bufPos, didChange ;
   const char *ptr ;

   ptr = expr4source( *exprOn ) ;
   oldNameLen = strlen( oldName ) ;
   ptrLen = strlen( ptr ) ;
   bufLen = sizeof( bufMem ) ;
   buf = bufMem ;
   didAlloc = bufPos = didChange = 0 ;

   for( pos = 0; ptr[pos]; pos++)
   {
      buf[bufPos++] = ptr[pos] ;
      if( (unsigned)bufPos == bufLen )
      {
         if( didAlloc )
         {
            u4allocAgain( (*exprOn)->codeBase, &buf, &bufLen, bufLen+50 ) ;
            if( buf == 0 )
               return -1 ;
         }
         else
         {
            bufLen += 50 ;
            buf = (char *) u4allocEr( (*exprOn)->codeBase, bufLen + 50 ) ;
            if( buf == 0 )
               return -1 ;
            memcpy( buf, bufMem, sizeof(bufMem) ) ;
            didAlloc = 1 ;
         }
      }

      if( ((unsigned) ptrLen - pos) < (unsigned) oldNameLen )
         continue ;
      #ifdef S4WINCE
         if( _memicmp( (void *)(ptr+pos), (void *)oldName, oldNameLen ) != 0 )
      #else
         if( memicmp( (void *)(ptr+pos), (void *)oldName, oldNameLen ) != 0 )
      #endif
            continue ;
      if( u4nameChar( ptr[pos+oldNameLen] )  )
         continue ;
      if( pos > 0 )
         if( u4nameChar(ptr[pos-1]) )
            continue ;

      didChange = 1 ;
      newNameLen = strlen(newName) ;
      if( bufLen <= (unsigned) (bufPos + newNameLen) )
      {
         if( didAlloc )
         {
            u4allocAgain( (*exprOn)->codeBase, &buf, &bufLen, bufLen+ newNameLen + 50 ) ;
            if( buf == 0 )
               return -1 ;
         }
         else
         {
            bufLen += newNameLen + 50 ;
            buf = (char *)u4allocEr( (*exprOn)->codeBase, bufLen ) ;
            if( buf == 0 )
               return -1 ;
            memcpy( buf, bufMem, sizeof(bufMem) ) ;
            didAlloc = 1 ;
         }
      }

      memcpy( buf+(--bufPos), newName, newNameLen ) ;
      bufPos += newNameLen ;
      pos += oldNameLen-1 ;
   }

   if( didChange )
   {
      buf[bufPos] = 0 ;
      newExpr = expr4parseLow( (*exprOn)->data, buf, 0 ) ;
      if( newExpr )
      {
         expr4free( *exprOn ) ;
         *exprOn = newExpr ;
      }
      if( didAlloc )
         u4free( buf ) ;
      return 0 ;
   }

   return 1 ;
}
#endif  /* S4WINTEL */
