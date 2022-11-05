/* i4init.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifdef __TURBOC__
   #pragma hdrstop
#endif

long time4long( const char *time, int strLen )
{
   long val, hold ;

   hold = c4atoi( time, 2 ) ;
   val = hold*3600000 ;
   hold = c4atoi( time+3, 2 ) ;
   val += hold*60000 ;
   hold = c4atoi( time+6, 2 ) ;
   val += hold*1000 ;
   if ( time[8] != 0 && strLen > 8 )
   {
      hold = c4atoi( time+9, 3 ) ;
      val += hold ;
   }

   return val ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
/* result must be big enough to hold a CURRENCY4 structure - 8 bytes */
void t4dblToCur( char *result, const double d )
{
   char currencyBuffer[21] ;

   c4dtoa45( d, currencyBuffer, 20, 4 ) ;
   c4atoCurrency( (CURRENCY4 *)result, currencyBuffer, 20 ) ;
}

#ifndef S4CLIENT
#ifndef S4CLIPPER

#ifdef S4FOX
#ifdef P4ARGS_USED
   #pragma argsused
#endif
void t4strToCur( char *result, const char *input, const int len )
{
   CURRENCY4 hold ;

   c4atoCurrency( &hold, input, len ) ;
   t4curToFox( result, &hold ) ;
}

/* seek string must be:  CCYYMMDDHH:MM:SS */
void t4strToDateTime( char *result, const char *input, const int len )
{
   long dt[2] ;

   if ( len < 16 )
   {
      memset( result, 0, 8 ) ;   /* 4 byte double */
      return ;
   }

   dt[0] = date4long( input ) ;
   dt[1] = time4long( input + 8, len - 8 ) ;
   t4dateTimeToFox( result, dt ) ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
void t4strToInt( char *result, const char *input, const int len )
{
   long val ;

   val = c4atol( input, len ) ;
   t4intToFox( result, &val ) ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
void t4dblToInt( char *result, const double d )
{
   long val ;

   val = (int)d ;
   t4intToFox( result, &val ) ;
}
#else
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4CALL t4cmpDoub( S4CMP_PARM dataPtr, S4CMP_PARM searchPtr, size_t len )
{
   double dif ;
   #ifdef S4DATA_ALIGN
      double d1, d2 ;

      memcpy( (void *)&d1, dataPtr, sizeof(double) ) ;
      memcpy( (void *)&d2, searchPtr, sizeof(double) ) ;
      #ifdef S4BYTE_SWAP
         d1 = x4reverseDouble( &d1 ) ;
         d2 = x4reverseDouble( &d2 ) ;
      #endif
      dif = d1 - d2 ;
   #else
      #ifdef S4BYTE_SWAP
         dif = x4reverseDouble((double *)dataPtr) -
            x4reverseDouble((double *)searchPtr) ;
      #else
         dif = *((double *)dataPtr) - *((double *)searchPtr) ;
      #endif
   #endif

   if ( dif > E4ACCURACY )
      return r4after ;
   if ( dif < -E4ACCURACY )
      return -1 ;
   return r4success ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4CALL t4descCmpDoub( S4CMP_PARM dataPtr, S4CMP_PARM searchPtr, size_t len )
{
   return -1 * t4cmpDoub( dataPtr, searchPtr, 0 ) ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4CALL t4descBcdCmp( S4CMP_PARM dataPtr, S4CMP_PARM searchPtr, size_t len )
{
   return -1 * c4bcdCmp( dataPtr, searchPtr, 0 ) ;
}

int S4CALL t4descMemcmp( S4CMP_PARM dataPtr, S4CMP_PARM searchPtr, size_t len )
{
   return -1 * u4memcmp( dataPtr, searchPtr, len ) ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
void t4strToDateMdx( char *result, const char *input, const int dummy )
{
   double d ;
   d = (double) date4long(input) ;
   #ifdef S4BYTE_SWAP
      d = x4reverseDouble( &d) ;
   #endif
   memcpy( result, (void *)&d, sizeof(double) ) ;
}

void t4noChangeDouble( char *result, const double d )
{
   memcpy( result, (void *)&d, sizeof(double) ) ;
}

void t4noChangeStr( char *a, const char *b, const int l)
{
   memcpy(a,b,l) ;
}

#ifndef S4OFF_INDEX
/* S4MDX */
int tfile4init( TAG4FILE *t4, INDEX4 *i4, T4DESC *tagInfo )
{
   CODE4 *c4 ;
   FILE4SEQ_READ seqread ;
   char buffer[1024], garbageBuffer[518], exprBuf[I4MAX_EXPR_SIZE+1], *ptr ;
   int len ;
   FILE4LONG pos ;

   #ifdef E4PARM_LOW
      if ( i4 == 0 || t4 == 0 || tagInfo == 0 )
         return error4( 0, e4parm, E94904 ) ;
   #endif

   c4 = i4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return -1 ;

   t4->indexFile = i4->indexFile ;
   t4->codeBase = c4 ;
   t4->cmp = (S4CMP_FUNCTION *)u4memcmp ;

   t4->headerOffset = tagInfo->headerPos * 512 ;

   file4longAssign( pos, t4->headerOffset, 0 ) ;
   file4seqReadInitDo( &seqread, &i4->indexFile->file, pos, buffer, sizeof(buffer), 1 ) ;
   if ( file4seqReadAll( &seqread, &t4->header, sizeof(T4HEADER)) < 0 )
      return -1 ;

   #ifdef S4BYTE_SWAP
      t4->header.keyLen = x4reverseShort( (void *)&t4->header.keyLen ) ;
      t4->header.keysMax = x4reverseShort( (void *)&t4->header.keysMax ) ;
      t4->header.groupLen = x4reverseShort( (void *)&t4->header.groupLen ) ;
      t4->header.isDate = x4reverseShort( (void *)&t4->header.isDate ) ;
      t4->header.unique = x4reverseShort( (void *)&t4->header.unique ) ;
   #endif

   t4->header.root = -1 ;

   u4ncpy( t4->alias, tagInfo->tag, sizeof(t4->alias) ) ;
   c4trimN( t4->alias, sizeof(t4->alias) ) ;
   c4upper( t4->alias ) ;

   file4seqReadAll( &seqread, exprBuf, sizeof(exprBuf)-1 ) ;
   c4trimN( exprBuf, sizeof(exprBuf) ) ;
   t4->expr = expr4parseLow( i4->data, exprBuf, t4 ) ;
   if ( !t4->expr )
      return -1 ;

   len = expr4keyLen( t4->expr ) ;
   if ( len < 0 )
      return error4stack( c4, len, E94904 ) ;

   if ( t4->header.keyLen != (short)len )
      return error4describe( c4, e4index, E84901, i4->indexFile->file.name, 0, 0 ) ;

   tfile4initSeekConv( t4, t4->header.type ) ;

   file4seqReadAll( &seqread, garbageBuffer, sizeof(garbageBuffer) ) ;

   file4seqReadAll( &seqread, exprBuf, sizeof(exprBuf)-1 ) ;
   c4trimN( exprBuf, sizeof(exprBuf) ) ;

   if ( garbageBuffer[1] == 1 )   /* Q&E support ... has filter */
   {
      if ( exprBuf[0] != 0 )
      {
         if ( garbageBuffer[2] == 1 )
            t4->hasKeys = t4->hadKeys = 1 ;
         else
            t4->hasKeys = t4->hadKeys = 0 ;

         t4->filter = expr4parseLow( i4->data, exprBuf, t4 ) ;
         if ( t4->filter == 0 )
            return -1 ;

         #ifdef S4VFP_KEY
            len = expr4key( t4->filter, &ptr, t4 ) ;
         #else
            len = expr4key( t4->filter, &ptr, 0 ) ;
         #endif
         if ( len < 0 )
            return -1 ;
         if ( expr4type( t4->filter ) != 'L' )
            error4describe( c4, e4index, E84903, i4->indexFile->file.name, 0, 0 ) ;
      }
   }
   return 0 ;
}

int tfile4initSeekConv( TAG4FILE *t4, int keyType )
{
   int isDesc ;

   isDesc = t4->header.typeCode & 8 ;

   switch( keyType )
   {
      case r4num:
         if ( isDesc )
            t4->cmp = (S4CMP_FUNCTION *)t4descBcdCmp ;
         else
            t4->cmp = (S4CMP_FUNCTION *)c4bcdCmp ;

         t4->stok = c4bcdFromA ;
         t4->dtok = c4bcdFromD ;
         break ;
      case r4date:
         if ( isDesc )
            t4->cmp = (S4CMP_FUNCTION *)t4descCmpDoub ;
         else
            t4->cmp = (S4CMP_FUNCTION *)t4cmpDoub ;
         t4->stok = t4strToDateMdx ;
         t4->dtok = t4noChangeDouble ;
         break ;
      case r4str:
      case r5wstr:
         if ( isDesc )
            t4->cmp = (S4CMP_FUNCTION *)t4descMemcmp ;
         else
            t4->cmp = (S4CMP_FUNCTION *)u4memcmp ;
         t4->stok = t4noChangeStr ;
         t4->dtok = 0 ;
         break ;
      default:
         return error4( t4->codeBase, e4info, E82901 ) ;
   }
   #ifdef S4UNIX
      switch( keyType )
      {
         case r4num:
            t4->keyType = r4num ;
            break ;
         case r4date:
            t4->keyType = r4date ;
            break ;
         case r4str:
            t4->keyType = r4str ;
            break ;
      }
   #endif

   return 0 ;
}
#endif  /* ifndef S4OFF_INDEX */
#endif  /* ifndef S4FOX */

#ifdef S4FOX
#ifndef S4OFF_INDEX

#ifdef S4LANGUAGE
   extern unsigned char v4map[256];
#else
   #ifdef S4ANSI
      extern unsigned char v4map[256];
   #endif
#endif

int S4CALL t4cdxCmp( S4CMP_PARM dataPtr, S4CMP_PARM searchPtr, size_t len )
{
   unsigned char *data = (unsigned char *)dataPtr ;
   unsigned char *search = (unsigned char *)searchPtr ;
   unsigned on ;

   for( on = 0 ; on < len ; on++ )
   {
      if ( data[on] != search[on] )
      {
         #ifdef S4VMAP
            if ( v4map[data[on]] > v4map[search[on]] ) return -1 ;  /* gone too far */
         #else
            if ( data[on] > search[on] ) return -1 ;  /* gone too far */
         #endif
         break ;
      }
   }

   return on ;
}

void t4noChangeStr( char *a, const char *b, const int l )
{
   memcpy( a, b, (unsigned int)l ) ;
}

void t4strToLog( char *dest, const char *src, const int l )
{
   int pos = 0 ;

   for ( ; l != pos ; pos++ )
      switch( src[pos] )
      {
         case 't':
         case 'T':
         case 'y':
         case 'Y':
            dest[0] = 'T' ;
            return ;
         case 'f':
         case 'F':
         case 'n':
         case 'N':
            dest[0] = 'F' ;
            return ;
         default:
            break ;
      }

   dest[0] = 'F' ;
}

/* S4FOX */
int tfile4init( TAG4FILE *t4, INDEX4 *i4, long filePos, unsigned char *name )
{
   CODE4 *c4 ;
   char exprBuf[I4MAX_EXPR_SIZE+1], *ptr ;
   int len ;
   unsigned int topSize ;
   FILE4LONG pos ;
   #ifdef S4DATA_ALIGN
      unsigned int size, delta ;
   #endif

   #ifdef E4PARM_LOW
      if ( i4 == 0 || t4 == 0 || name == 0 || filePos < 0 )
         return error4( 0, e4parm, E94904 ) ;
   #endif

   c4 = i4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return -1 ;

   t4->indexFile = i4->indexFile ;
   t4->codeBase = c4 ;
   t4->headerOffset = filePos ;
   t4->header.root = -1 ;
   t4->cmp = (S4CMP_FUNCTION *)t4cdxCmp ;

   file4longAssign( pos, filePos, 0 ) ;
   topSize = 2 * sizeof(S4LONG) + 4*sizeof(char) + sizeof(short) + 2 * sizeof(unsigned char) ;
   if ( file4readAllInternal( &i4->indexFile->file, pos, &t4->header, topSize ) < 0 )
      return 0 ;
   file4longAssign( pos, filePos + (long)topSize + 478L, 0 ) ;
   if ( file4readAllInternal( &i4->indexFile->file, pos, &t4->header.sortSeq, ( 8 * sizeof(char) ) ) < 0 )
      return 0 ;
   t4->header.sortSeq[7] = '\0' ;
   file4longAssign( pos, filePos + (long)topSize + 486L, 0 ) ;
   if ( file4readAllInternal( &i4->indexFile->file, pos, &t4->header.descending, ( 5 * sizeof(short) ) ) < 0 )
      return 0 ;
   #ifdef S4BYTE_SWAP
      t4->header.root = x4reverseLong( (void *)&t4->header.root ) ;
      t4->header.freeList = x4reverseLong( (void *)&t4->header.freeList ) ;
      /* version is already stored in intel format */
      t4->header.keyLen = x4reverseShort( (void *)&t4->header.keyLen ) ;
      t4->header.descending = x4reverseShort( (void *)&t4->header.descending ) ;
      t4->header.filterPos = x4reverseShort( (void *)&t4->header.filterPos ) ;
      t4->header.filterLen = x4reverseShort( (void *)&t4->header.filterLen ) ;
      t4->header.exprPos = x4reverseShort( (void *)&t4->header.exprPos ) ;
      t4->header.exprLen = x4reverseShort( (void *)&t4->header.exprLen ) ;
   #else
      t4->header.version = x4reverseLong( (void *)&t4->header.version ) ;
   #endif

   u4ncpy( t4->alias, (char *)name, sizeof(t4->alias) ) ;
   c4trimN( t4->alias, sizeof(t4->alias) ) ;
   c4upper( t4->alias ) ;

   if ( *t4->header.sortSeq == '\0' )
   {
      if ( tfile4setCollatingSeq( t4, sort4machine ) < 0 )
         return error4( c4, e4index, E84907 ) ;
   }
   #ifdef S4GENERAL
   else if ( strcmp( t4->header.sortSeq, "GENERAL" ) == 0 )
   {
      if ( tfile4setCollatingSeq( t4, sort4general ) < 0 )
         return error4( c4, e4index, E84907 ) ;
   }
   #endif
   else
      return error4( c4, e4index, E84907 ) ;

   if ( t4->header.typeCode < 0x80 )  /* non-compound header; so expression */
   {
      #ifdef E4ANALYZE
         if ( t4->header.exprLen+1 > sizeof( exprBuf ) )
             return error4( c4, e4info, E84902 ) ;
      #endif
      file4longAssign( pos, filePos+B4BLOCK_SIZE, 0 ) ;
      file4readAllInternal( &i4->indexFile->file, pos, exprBuf, (unsigned int)t4->header.exprLen ) ;
      exprBuf[t4->header.exprLen] = '\0' ;
      t4->expr = expr4parseLow( i4->data, exprBuf, t4 ) ;
      if ( t4->expr == 0 )
         return -1 ;

      len = expr4keyLen( t4->expr ) ;
      if ( len < 0 )
         return -1 ;

      if ( t4->header.keyLen != len )
         return error4describe( c4, e4index, E84901, i4->indexFile->file.name, 0, 0 ) ;

      tfile4initSeekConv(t4, t4->expr->type ) ;

      if ( t4->header.typeCode & 0x08 )   /* For clause (filter) exists */
      {
         file4longAssign( pos, filePos + B4BLOCK_SIZE + t4->header.exprLen, 0 ) ;
         file4readAllInternal( &i4->indexFile->file, pos, exprBuf, (unsigned int)t4->header.filterLen ) ;
         exprBuf[t4->header.filterLen] = '\0' ;

         t4->filter = expr4parseLow( i4->data, exprBuf, t4 ) ;
         if ( t4->filter == 0 )
            return -1 ;
         #ifdef S4VFP_KEY
            len = expr4key( t4->filter, &ptr, t4 ) ;
         #else
            len = expr4key( t4->filter, &ptr, 0 ) ;
         #endif
         if ( len < 0 )
            return -1 ;
         if ( expr4type( t4->filter ) != 'L' )
            error4describe( c4, e4index, E84903, i4->indexFile->file.name, 0, 0 ) ;
      }
   }

   #ifdef S4DATA_ALIGN
      size = (unsigned int)sizeof(S4LONG) + t4->header.keyLen ;
      delta = sizeof(void *) - size % sizeof(void *);
      t4->builtKeyMemory = mem4create( c4, 3, size + delta, 2, 0 ) ;
   #else
      t4->builtKeyMemory = mem4create( c4, 3, (unsigned int)sizeof(S4LONG) + t4->header.keyLen + 1, 2, 0 ) ;
   #endif

   return 0 ;
}

int tfile4initSeekConv( TAG4FILE *t4, int type )
{
   t4->cmp = (S4CMP_FUNCTION *)t4cdxCmp ;

   switch( type )
   {
      case r4date:
      case r4dateDoub:
         t4->stok = t4dtstrToFox ;
         t4->dtok = t4dblToFox ;
         t4->pChar = '\0' ;
         break ;
      case r4numDoub:
         if ( expr4currency( t4->expr ) )  /* then should be converted to a currency */
         {
            t4->stok = t4strToCur ;
            t4->dtok = t4dblToCurFox ;
            t4->pChar = '\0' ;
            break ;
         }
         /* else fall through, same as r4num */
      case r4num:
         t4->stok = t4strToFox ;
         t4->dtok = t4dblToFox ;
         t4->pChar = '\0' ;
         break ;
      case r4str:
      case r5wstr:
         t4->stok = t4noChangeStr ;
         t4->dtok = 0 ;
         t4->pChar = ' ' ;
         #ifdef S4VFP_KEY
            if ( tfile4vfpKey( t4 ) )
               t4->pChar = '\0' ;   /* if this changes check t4strToVFPKey() to ensure correctness */
         #endif
         break ;
      case r4log:
         t4->stok = t4strToLog ;
         t4->dtok = 0 ;
         break ;
      #ifdef S4FOX
         case r4int:
            t4->stok = t4strToInt ;
            t4->dtok = t4dblToInt ;
            t4->pChar = '\0' ;
            break ;
         case r4dateTime:
            t4->stok = t4strToDateTime ;
            t4->dtok = 0 ;
            t4->pChar = '\0' ;
            break ;
         case r4currency:
            t4->stok = t4strToCur ;
            t4->dtok = t4dblToCur ;
            t4->pChar = '\0' ;
            break ;
      #endif
      default:
         return error4( t4->codeBase, e4info, E82901 ) ;
   }
   #ifdef S4UNIX
      switch( type )
      {
         case r4num:
         case r4numDoub:
            t4->keyType = r4num ;
            break ;

         case r4date:
         case r4dateDoub:
            t4->keyType = r4date ;
            break ;

         case r4str:
            t4->keyType = r4str ;
            break ;

         case r4log:
            t4->keyType = r4log ;
            break ;
      }
   #endif

   return 0 ;
}

#endif   /*  ifdef S4FOX  */
#endif   /*  ifndef S4OFF_INDEX  */
#endif   /*  ifndef S4CLIPPER  */

#ifdef S4CLIPPER

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4CALL t4cmpDoub( S4CMP_PARM dataPtr, S4CMP_PARM searchPtr, size_t len )
{
   double dif ;
   #ifdef S4DATA_ALIGN
      double d1, d2 ;
      memcpy( &d1, dataPtr, sizeof(double) ) ;
      memcpy( &d2, searchPtr, sizeof(double) ) ;
      dif = d1 - d2 ;
   #else
      dif = *((double *)dataPtr) - *((double *)searchPtr) ;
   #endif

   if ( dif > E4ACCURACY )  return r4after ;
   if ( dif < -E4ACCURACY ) return -1 ;
   return r4success ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
void  t4strToDoub( char *result, const char *input, const int dummy )
{
   double d ;
   d = c4atod( input, strlen( input )) ;
   memcpy( result, &d, sizeof(double) ) ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
void t4strToDateMdx( char *result, const char *input, const int dummy )
{
   double d = (double) date4long(input) ;
   memcpy( result, &d, sizeof(double) ) ;
}

void t4noChangeDouble( char *result, const double d )
{
   memcpy( result, &d, sizeof(double) ) ;
}

void t4noChangeStr( char *a, const char *b, const int l)
{
   memcpy(a,b,l) ;
}

#ifdef S4CLIPPER
/* if input len != 0 then it is used to position value to right-justify */
void  t4strToClip( char *result, const char *input, const int len )
{
   int iLen, dLen ;

   iLen = strlen( input ) ;
   dLen = len - iLen ;
   if ( dLen > 0 )
   {
      memset( result, ' ', dLen ) ;
      memcpy( result + dLen, input, iLen ) ;
   }
   else
      memcpy( result, input, iLen ) ;
   c4clip( result, strlen( result ) ) ;
}
#endif

void  t4dateDoubToStr( char *result, const double d )
{
   long  l ;

   l = (long) d ;
   date4assign( result, l ) ;
}

#ifndef S4OFF_INDEX

int tfile4initSeekConv( TAG4FILE *t4, int keyType )
{
   switch( keyType )
   {
      case r4date:
      case r4dateDoub:
         t4->cmp = (S4CMP_FUNCTION *)u4memcmp ;
         t4->stok = t4noChangeStr ;
         t4->dtok = t4dateDoubToStr ;
         #ifdef S4UNIX
            t4->keyType = r4date ;
         #endif
         break ;
      case r4num:
      case r4numDoub:
         t4->cmp = (S4CMP_FUNCTION *)u4memcmp ;
         t4->stok = t4strToClip ;
         t4->dtok = 0 ;
         #ifdef S4UNIX
            t4->keyType = r4num ;
         #endif
         break ;
      case r4str:
      case r5wstr:
         t4->cmp = (S4CMP_FUNCTION *)u4memcmp ;
         t4->stok = t4noChangeStr ;
         t4->dtok = 0 ;
         #ifdef S4UNIX
            t4->keyType = r4str ;
         #endif
         break ;
      default:
         return error4( t4->codeBase, e4tagInfo, E82901 ) ;
   }
   return 0 ;
}
#endif   /* S4OFF_INDEX */
#endif   /* S4CLIPPER */
#endif   /* S4CLIENT */
