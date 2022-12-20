/* e4expr.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

char **expr4 ;
char  *expr4constants ;
E4INFO *expr4infoPtr ;
EXPR4 *expr4ptr ;

/* sets the data4 context for the expression */
int S4FUNCTION expr4context( EXPR4 *expr, DATA4 *data )
{
   short int i ;
   #ifdef E4PARM_LOW
      if ( expr == 0 || data == 0 )
         return error4( 0, e4parmNull, E90918 ) ;
   #endif

   if ( expr->data != data )
   {
      expr->data = data ;
      expr->dataFile = data->dataFile ;
      for ( i = 0 ; i < expr->infoN ; i++ )   /* update FIELD4's if reqd */
      {
         if ( expr->info[i].fieldNo != 0 )
            if ( expr->info[i].fieldPtr != 0 )
               if ( expr->info[i].localData == 1 )
                  expr->info[i].fieldPtr = d4fieldJ( data, expr->info[i].fieldNo ) ;
      }
   }
   data->dataFile->record = d4record( data ) ;
   return 0 ;
}

#ifdef __WIN32
   #ifdef S4SEMAPHORE
      /* multi-thread support */
      extern CRITICAL_SECTION critical4expression ;
   #endif
#endif

int expr4start( EXPR4 *expr )
{
   #ifdef S4SEMAPHORE
      #ifdef S4OS2
         APIRET rc ;

         rc = DosRequestMutexSem( expr->codeBase->hmtxExpr, -1 ) ;
         if ( rc != 0 )
            return error4( expr->codeBase, e4info, "OS/2 Semaphore Failure" ) ;
      #endif
      #ifdef __WIN32
         EnterCriticalSection( &critical4expression ) ;
      #endif
   #endif
   expr4buf = expr->codeBase->exprWorkBuf ;  /* initialize the working buffer pointer */
      if ( expr->tagPtr )  /* is a tag, so verift context validity */
         if ( expr->dataFile->record == 0 )
            return expr4context( expr, expr->data ) ;
   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
static void expr4stop( CODE4 *c4 )
{
   /* clear the globals to ensure they are not used without initialization */
   expr4buf = 0 ;
   expr4 = 0 ;
   expr4ptr = 0 ;
   expr4infoPtr = 0 ;
   expr4constants = 0 ;
   #ifdef S4SEMAPHORE
      #ifdef S4OS2
         DosReleaseMutexSem( c4->hmtxExpr ) ;
      #endif
      #ifdef __WIN32
         LeaveCriticalSection( &critical4expression ) ;
      #endif
   #endif
}

void t4intToFox( char *result, const long *val )
{
   int isPositive ;

   isPositive = *val > 0 ;
   *((long *)result) = x4reverseLong( val ) ;
   if ( isPositive )
      result[0] += (unsigned)0x80 ;
   else /* negative */
      result[0] -= (unsigned)0x80 ;
}

void t4curToFox( char *result, const CURRENCY4 *source )
{
   char i ;
   int isPositive ;
   char buffer[8] ;

   isPositive = ( (short int)(source->lo[3]) >= 0 ) ;

   memcpy(buffer, source, 8 ) ;
   for ( i = 0 ; i < 8 ; i++ )
      result[i] = ( buffer[7-i] ) ;

   if ( isPositive )
      result[0] += (unsigned)0x80 ;
   else /* negative */
      result[0] -= (unsigned)0x80 ;
}

void t4dateTimeToFox( char *result, const long *input )
{
   double date, time ;
   double val ;

   date = (double)input[0] ;
   time = (double)input[1] ;

   val = date + ( time / 86400000.0 ) ;
   t4dblToFox( result, val ) ;
}

void t4dblToCurFox( char *result, const double doub )
{
   CURRENCY4 hold ;

   t4dblToCur( (char *)&hold, doub ) ;
   t4curToFox( result, &hold ) ;
}

void t4dblToFox( char *result, const double doub )
{
   char i ;
   int isPositive ;

   isPositive = ( doub >= 0 ) ;

   #ifdef S4BYTEORDER_3210
      if ( isPositive )
      {
         memcpy( (char *)result, (char *)&doub, 8 ) ;
         result[0] += (unsigned)0x80 ;
      }
      else /* negative */
      {
         for ( i = 0 ; i < 8 ; i++ )
            result[i] = (char) (~(*( (unsigned char *)&doub + i ))) ;
      }
   #else
      #ifdef S4BYTEORDER_2301
         memcpy( (char *)result, ((char *)&doub) + 4, 4 ) ;
         memcpy( ((char *)result) + 4, ((char *)&doub), 4 ) ;
         memcpy( (void *)&doub, result, sizeof(double) ) ;
      #endif
      if ( isPositive )
      {
         for ( i = 0 ; i < 8 ; i++ )
            result[i] = *( (char *)&doub + 7 - i ) ;
         result[0] += (unsigned)0x80 ;
      }
      else /* negative */
         for ( i = 0 ; i < 8 ; i++ )
            result[i] = (char) (~(*( (unsigned char *)&doub + 7 - i ))) ;
   #endif
}

int S4FUNCTION expr4double2( EXPR4 *e4expr, double *result )
{
   *result = expr4double( e4expr ) ;
   if ( *result < 0.0 )
      return -1;
   return 0;
}

double S4FUNCTION expr4double( EXPR4 *e4expr )
{
   char *ptr ;
   int len ;
   #ifdef S4DATA_ALIGN
      double doub ;
   #endif

   len = expr4vary( e4expr, &ptr ) ;
   if ( len >= 0 )
      switch( expr4type( e4expr ) )
      {
         case r4numDoub:
         case r4dateDoub:
            #ifdef S4DATA_ALIGN
               memcpy( (void *)&doub, ptr, sizeof(double) ) ;
               return  doub ;
            #else
               return  *( (double *)ptr ) ;
            #endif
         case r4num:
         case r4str:
            return c4atod( ptr, len ) ;
         case r4date:
            return (double)date4long( ptr ) ;
         default:
           #ifdef E4ANALYZE
              error4( e4expr->codeBase, e4info, E90914 ) ;
           #endif
           break ;
      }

   return 0.0 ;
}

int S4FUNCTION expr4execute( EXPR4 *expr, const int pos, void **resultPtrPtr )
{
   E4INFO *lastInfo ;
   char *pointers[E4MAX_STACK_ENTRIES] ;
   int infoPos ;
   short int rc ;

   #ifdef E4PARM_LOW
      if ( expr == 0 )
         return error4( 0, e4parmNull, E90912 ) ;
      if ( pos < 0 || resultPtrPtr == 0 )
         return error4( expr->codeBase, e4parm, E90912 ) ;
   #endif

   if ( error4code( expr->codeBase ) < 0 )
      return e4codeBase ;

   rc = expr4start( expr ) ;
   if ( rc != 0 )
      return error4( expr->codeBase, rc, E90912 ) ;

   expr4 = pointers ;
   expr4constants = expr->constants ;
   expr4ptr = expr ;
   lastInfo = expr->info + pos ;
   infoPos = pos - lastInfo->numEntries + 1 ;

   for( ; infoPos <= pos ; infoPos++ )
   {
      expr4infoPtr = expr->info+ infoPos ;
      (*expr4infoPtr->function)() ;
   }

   *resultPtrPtr = pointers[0] ;
   #ifdef E4ANALYZE
      if ( pointers[0] != expr4[-1] )
         return error4( expr->codeBase, e4result, E90912 ) ;
   #endif

   expr4stop( expr->codeBase ) ;

   return error4code( expr->codeBase ) ;
}

int S4FUNCTION expr4key( EXPR4 *e4expr, char **ptrPtr, TAG4FILE *t4file )
{
   int resultLen ;

  // E4PARHIGH( expr && resultPtrPtr, E90916 ) ;
   #ifdef E4PARM_HIGH
      if ( e4expr == 0 || ptrPtr == 0 )
         return error4( 0, e4parmNull, E90913 ) ;

      if ( e4expr == 0 || ptrPtr == 0 )
         return error4( e4expr->codeBase, e4parmNull, E90913 ) ;
   #endif

   if ( error4code( e4expr->codeBase ) < 0 )
      return -1 ;

   resultLen = expr4vary( e4expr, ptrPtr ) ;
   if ( resultLen < 0 )
       return -1 ;

   return expr4keyConvert( e4expr, ptrPtr, resultLen, e4expr->type, t4file ) ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int expr4keyConvert( EXPR4 *e4expr, char **ptrPtr, const int rLen, const int exprType, TAG4FILE *t4file )
{
   int resultLen, i ;
   double d ;
   CODE4 *cb ;
   #ifdef S4MDX
      C4BCD bcd ;
      #ifdef S4DATA_ALIGN
         double dtmp;
      #endif
   #endif
   #ifdef S4CLIPPER
      long l ;
      int oldDec, tLen ;
   #endif
   #ifndef N4OTHER
      double *dPtr ;
   #endif
   #ifdef S4FOX
      CURRENCY4 *currency ;
      long *lPtr ;
   #endif
   #ifdef S4VFP_KEY
      int keyLen ;
   #endif
   #ifdef S4DATA_ALIGN
      double tempdoub ;
      int tempint ;
   #endif

   cb = e4expr->codeBase ;
   resultLen = rLen ;

   switch( exprType )
   {
      #ifdef S4FOX
         case r4int:
            lPtr = (long *)( *ptrPtr ) ;
            t4intToFox( cb->storedKey, lPtr ) ;
            break ;
         case r4currency:
            currency = (CURRENCY4 *)( *ptrPtr ) ;
            t4curToFox( cb->storedKey, currency ) ;
            resultLen = 8 ;
            break ;
         case r4dateTime:
            t4dateTimeToFox( cb->storedKey, (long *)(*ptrPtr) ) ;
            resultLen = 8 ;
            break ;
         case r4num:
            d = c4atod( *ptrPtr, resultLen ) ;
            t4dblToFox( cb->storedKey, d ) ;
            resultLen = (int)sizeof( double ) ;
            break ;
         case r4date:
            d = (double)date4long( *ptrPtr ) ;
            t4dblToFox( cb->storedKey, d ) ;
            resultLen = (int)sizeof( double ) ;
            break ;
         case r4numDoub:
         case r4dateDoub:
            #ifdef S4DATA_ALIGN
               memcpy(&tempdoub, *ptrPtr, sizeof(double) );
            #else
               dPtr = (double *) (*ptrPtr) ;
            #endif
            if ( expr4currency( e4expr ) )  /* then should be converted to a currency */
               #ifdef S4DATA_ALIGN
                  t4dblToCurFox( cb->storedKey, tempdoub ) ;
               #else
                  t4dblToCurFox( cb->storedKey, *dPtr ) ;
               #endif
            else
               #ifdef S4DATA_ALIGN
                  t4dblToFox( cb->storedKey, tempdoub ) ;
               #else
                  t4dblToFox( cb->storedKey, *dPtr ) ;
               #endif
            resultLen = (int)sizeof( double ) ;
            break ;
         case r4log:
            #ifdef S4DATA_ALIGN
               memcpy(&tempint, *ptrPtr, sizeof(int) ) ;
               switch(tempint)
            #else
               switch( *(int *)*ptrPtr )
            #endif
            {
               case 1:
                  cb->storedKey[0] = 'T' ;
                  break ;
               case 0:
                  cb->storedKey[0] = 'F' ;
                  break ;
               default:
                  #ifdef E4ANALYZE
                     return error4( e4expr->codeBase, e4info, E81002 ) ;
                  #else
                     cb->storedKey[0] = 'F' ;
                  #endif
            }
            resultLen = 1 ;
            break ;
      #endif  /*  ifdef S4FOX      */
      #ifdef S4CLIPPER
         case r4num:
            resultLen = e4expr->keyLen ;
            #ifdef E4ANALYZE
               if ( cb->storedKey == 0 )
                  return error4( cb, e4info, E80903 ) ;
            #endif
            /* AS 04/22/97 fixes for main.cpp (user test), using VAL, get failures */
            if ( t4file != 0 )
            {
               /* tLen is used for the difference between the expression len and the tag len */
               tLen = resultLen - t4file->header.keyLen ;
               memcpy( cb->storedKey, *ptrPtr + tLen, resultLen - tLen) ;
               oldDec = cb->decimals ;
               cb->decimals = t4file->expr->keyDec ;
               c4clip( cb->storedKey, t4file->header.keyLen ) ;
               cb->decimals = oldDec ;
            }
            else
            {
               memcpy( cb->storedKey, *ptrPtr, resultLen ) ;
               c4clip( cb->storedKey, resultLen ) ;
            }
            break ;
         case r4numDoub:
            if ( t4file != 0 )
            {
               oldDec = cb->decimals ;
               resultLen = t4file->header.keyLen ;
               cb->decimals = t4file->expr->keyDec ;
               c4dtoaClipper( *((double *)*ptrPtr), cb->storedKey, resultLen, cb->decimals ) ;
               cb->decimals = oldDec ;
            }
            else
            {
               resultLen = e4expr->keyLen ;
               c4dtoaClipper( *((double *)*ptrPtr), cb->storedKey, resultLen, cb->decimals ) ;
            }
            break ;
         case r4dateDoub:
            d = *( ( double  *)*ptrPtr ) ;
            l = (long)d ;
            date4assign( cb->storedKey, l ) ;
            break ;
      #endif  /*  ifdef S4CLIPPER  */
      #ifdef S4MDX
         case r4num:
            c4bcdFromA( (char *) &bcd, (char *) *ptrPtr, resultLen ) ;
            #ifdef E4ANALYZE
               if ( cb->storedKey == 0 )
                  return error4( cb, e4info, E80903 ) ;
            #endif
            memcpy( cb->storedKey, (void *)&bcd, sizeof(C4BCD) ) ;
            resultLen = (int)sizeof( C4BCD ) ;
            break ;
         case r4numDoub:
            #ifdef S4DATA_ALIGN
               memcpy(&dtmp, *ptrPtr, sizeof(double));
               c4bcdFromD( (char *)&bcd, dtmp);
            #else
               dPtr = (double *) (*ptrPtr) ;
               c4bcdFromD( (char *) &bcd, *dPtr ) ;
            #endif
            #ifdef E4ANALYZE
               if ( cb->storedKey == 0 )
                  return error4( cb, e4info, E80903 ) ;
            #endif
            memcpy( cb->storedKey, (void *)&bcd, sizeof(C4BCD) ) ;
            resultLen = (int)sizeof( C4BCD ) ;
            break ;
         case r4date:
            date4formatMdx2( *ptrPtr, &d ) ;
            if ( d == 0 ) d = 1.0E300 ;
            #ifdef E4ANALYZE
               if ( cb->storedKey == 0 )
                  return error4( cb, e4info, E80903 ) ;
            #endif
            memcpy( cb->storedKey, (void *)&d, sizeof(double) ) ;
            resultLen = (int)sizeof( double ) ;
            break ;
         case r4dateDoub:
            #ifdef E4ANALYZE
               if ( cb->storedKey == 0 )
                  return error4( cb, e4info, E80903 ) ;
            #endif
            memcpy( cb->storedKey, *ptrPtr, sizeof(double) ) ;
            dPtr = (double *)( cb->storedKey ) ;
            if ( *dPtr == 0 )
               *dPtr = 1.0E300 ;
            #ifdef WORDS_BIGENDIAN
               *(double *)cb->storedKey = x4reverseDouble((double *)cb->storedKey) ;
            #endif
            resultLen = (int)sizeof( double ) ;
            break ;
      #endif  /* S4MDX */
      case r4str:
         #ifdef E4ANALYZE
            if ( cb->storedKey == 0 )
               return error4( cb, e4info, E80903 ) ;
         #endif

         #ifdef S4VFP_KEY
            if ( t4file != 0 )
            {
               if ( t4file->expr->vfpInfo->sortType == sort4general )
               {
                  keyLen = expr4keyLen( e4expr ) ;
                  if ( cb->storedKeyLen < (unsigned)keyLen )
                  {
                     u4allocAgain( cb, &cb->storedKey, &cb->storedKeyLen, (unsigned)keyLen+1 ) ;
                     if ( error4code( cb ) < 0 )
                        return -1 ;
                     cb->storedKeyLen = keyLen + 1 ;
                  }
                  if ( expr4nullLow( e4expr, 0 ) )
                     keyLen-- ;

                  /* if trim, then replace nulls with blanks before translation */
                  if ( e4expr->hasTrim )
                     for( i = rLen - 1 ; (i >= 0) && ((*ptrPtr)[i] == 0) ; i-- )
                        (*ptrPtr)[i] = ' ' ;
                  if ( t4strToVFPKey( cb->storedKey, *ptrPtr, rLen, keyLen, e4expr->vfpInfo ) < 0 )
                     return error4( cb, e4info, E85404 ) ;
                  resultLen = keyLen ;
               }
               else
                  memcpy( cb->storedKey, *ptrPtr, (unsigned int)resultLen ) ;
            }
            else
               memcpy( cb->storedKey, *ptrPtr, (unsigned int)resultLen ) ;
         #else
            memcpy( cb->storedKey, *ptrPtr, (unsigned int)resultLen ) ;
         #endif

         /* if trim, then replace nulls with blanks */
         if ( e4expr->hasTrim )
            #ifdef S4VFP_KEY
            if ( e4expr->vfpInfo )
               if ( e4expr->vfpInfo->sortType != sort4general )
            #endif
                  for( i = resultLen - 1 ; (i >= 0) && (cb->storedKey[i] == 0) ; i-- )
                     cb->storedKey[i] = ' ' ;
         break ;
      default:
         #ifdef E4ANALYZE
            if ( cb->storedKey == 0 )
               return error4( cb, e4info, E80903 ) ;
         #endif
         memcpy( cb->storedKey, *ptrPtr, (unsigned int)resultLen ) ;
         break ;
   }

   #ifdef E4ANALYZE
      if ( (unsigned)resultLen >= cb->storedKeyLen )
         return error4( cb, e4info, E80903 ) ;
   #endif

   #ifdef S4FOX
      if ( expr4nullLow( e4expr, 0 ) )  /* maybe a null value, so check */
      {
         if ( expr4nullLow( e4expr, 1 ) )  /* value is null */
            memset( cb->storedKey, 0, resultLen + 1 ) ;
         else
         {
            c4memmove( cb->storedKey + 1, cb->storedKey, resultLen ) ;
            cb->storedKey[0] = (char)0x80 ;
         }
         resultLen++ ;
      }
   #endif

   cb->storedKey[resultLen] = 0 ;    /* null end it */
   *ptrPtr = cb->storedKey ;
   return resultLen ;
}

/* returns true if a currency field resides in the expression
   tag file type is currency if there is any currency field value within
   the expression and the result type is otherwise r4numDoub */
int S4FUNCTION expr4currency( const EXPR4 *e4expr )
{
   #ifndef S4CFOX
      FIELD4 *field ;
      int parms ;

      if ( code4indexFormat( e4expr->codeBase ) != r4cdx )
         return 0 ;

      for ( parms = 0 ; parms < e4expr->infoN ; parms++ )
      {
         field = e4expr->info[parms].fieldPtr ;
         if ( field != 0 )
            if ( f4type( field ) == r4currency )
               return 1 ;
      }
   #endif

   return 0 ;
}

#ifdef S4CFOX
/* if forAdd is 0, then returns whether or not the expression has the
   possibility of a null return.  If one, it evaluates the current
   expression to see if the current result is null */
int S4FUNCTION expr4nullLow( const EXPR4 *e4expr, const int forAdd )
{
   FIELD4 *field ;
   int parms ;
      #ifndef S4OFF_INDEX
         TAG4FILE *tag ;
      #endif


      #ifndef S4OFF_INDEX
         #ifdef S4FOX
            /* checking the tag setting doesn't apply for client since would only
               be looking at it as an expression, not a tag-related value */
            if ( forAdd == 0 )
            {
               tag = e4expr->tagPtr ;
               if ( tag != 0 )   /* candidate keys don't make room for the null since disallowed */
                  if ( tag->header.typeCode & 0x04 )  /* r4candidate */
                     return 0 ;
            }
         #endif
   #endif

   for ( parms = 0 ; parms < e4expr->infoN ; parms++ )
   {
      field = e4expr->info[parms].fieldPtr ;
      if ( field != 0 )  /* if has null now, then add 1 byte for index storage */
         if ( field->null == 1 )
         {
            if ( forAdd == 1 )  /* only want to know whether contents are null */
            {
               if ( f4null( field ) )
                  return 1 ;
            }
            else
               return 1 ;
         }
   }

   return 0 ;
}
#else
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION expr4nullLow( const EXPR4 *e4expr, const int forAdd )
{
   return 0 ;
}
#endif

int S4FUNCTION expr4keyLen( EXPR4 *e4expr )
{
   int len ;

   #ifdef E4PARM_LOW
      if ( e4expr == 0 )
         return error4( 0, e4parmNull, E90915 ) ;
   #endif

   len = expr4nullLow( e4expr, 0 ) ;   /* extra byte for nulls if required */

      switch( e4expr->type )
      {
         #ifdef S4FOX
            case r4num:
            case r4date:
            case r4numDoub:
            case r4currency:
            case r4dateTime:
               return len + (int)sizeof( double ) ;
            case r4int:
               return len + (int)sizeof( long ) ;
            case r4log:
               return len + (int)sizeof( char ) ;
         #endif  /*  ifdef S4FOX      */
         #ifdef S4CLIPPER
            case r4num:  /* numeric field return, this fixex length problem */
               return len + f4len( e4expr->info[0].fieldPtr ) ;
            case r4numDoub:
               return len + e4expr->codeBase->numericStrLen ;
         #endif  /*  ifdef S4CLIPPER  */

         #ifdef S4NDX
            case r4num:
            case r4date:
               return len + (int)sizeof( double ) ;
         #endif  /*  ifdef S4NDX  */

         #ifdef S4MDX
            case r4num:
            case r4numDoub:
               return len + (int)sizeof( C4BCD ) ;
            case r4date:
            case r4dateDoub:
               return len + (int)sizeof( double ) ;
         #endif  /* S4MDX */
         default:
            #ifdef S4VFP_KEY
               if ( e4expr->vfpInfo )
               {
                  if ( e4expr->vfpInfo->sortType == sort4general )
                     return len + expr4len( e4expr ) * 2 ;
                  else
                     return len + expr4len( e4expr ) ;
               }
               else
            #endif
                  return len + expr4len( e4expr ) ;
       }
}

static char e4nullChar = '\0' ;

S4CONST char *S4FUNCTION expr4source( const EXPR4 *e4expr )
{
   if ( e4expr == 0 )
      return &e4nullChar ;
   return e4expr->source ;
}

const char *S4FUNCTION expr4str( EXPR4 *expr )
{
   char *str ;

   E4PARM_HRET( expr, E90919, NULL ) ;

   switch( expr4type( expr ) )
   {
      case r4str:
      case r4date:
         expr4vary( expr, &str ) ;
         break ;
      default:
         error4( expr->codeBase, e4parm ,E90919 ) ;
         return 0 ;
   }

   return str ;
}

int S4FUNCTION expr4true( EXPR4 *e4expr )
{
   int resultLen ;
   int *iPtr ;

   resultLen = expr4vary( e4expr, (char **)&iPtr ) ;
   if ( resultLen < 0 )
      return -1 ;

   if ( expr4type( e4expr ) != r4log )
      return error4( e4expr->codeBase, e4result, E80905 ) ;

   /* for sure avoid returning negative values which mean true but are interpreted as errors */
   return ( ( *iPtr != 0 ) ? 1 : 0 ) ;
}

int S4FUNCTION expr4vary( EXPR4 *expr, char **resultPtrPtr )
{
   char *pointers[E4MAX_STACK_ENTRIES] ;
   int infoPos, rc ;

   E4PARHIGH( expr && resultPtrPtr, E90916 ) ;

   if ( error4code( expr->codeBase ) < 0 )
      return e4codeBase ;

   rc = expr4start( expr ) ;
   if ( rc < 0 )
      return rc ;

   expr4 = pointers ;
   expr4constants = expr->constants ;
   expr4ptr = expr ;

   for( infoPos = 0; infoPos < expr->infoN; infoPos++ )
   {
      expr4infoPtr = expr->info+ infoPos ;
      (*expr4infoPtr->function)() ;
   }

   *resultPtrPtr = pointers[0] ;
   #ifdef E4ANALYZE
      if ( pointers[0] != expr4[-1] )
         return error4( expr->codeBase, e4result, E90916 ) ;
   #endif

   expr4stop( expr->codeBase ) ;

   return expr->len ;
}
