/* f4flag.h   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

typedef struct
{
   CODE4 S4PTR *codeBase ;
   unsigned char S4PTR *flags ;
   unsigned long  numFlags ;
   int      isFlip ;
} F4FLAG ;

#ifdef __cplusplus
   extern "C" {
#endif

S4EXPORT int  S4FUNCTION f4flagInit( F4FLAG S4PTR *, CODE4 S4PTR *, const unsigned long ) ;
S4EXPORT int  S4FUNCTION f4flagSet( F4FLAG S4PTR *, const unsigned long ) ;
S4EXPORT int  S4FUNCTION f4flagReset( F4FLAG S4PTR *, const unsigned long ) ;
S4EXPORT int  S4FUNCTION f4flagIsSet( F4FLAG S4PTR *, const unsigned long ) ;
S4EXPORT int  S4FUNCTION f4flagIsAllSet( F4FLAG S4PTR *, const unsigned long, const unsigned long ) ;
S4EXPORT int  S4FUNCTION f4flagIsAnySet( F4FLAG S4PTR *, const unsigned long, const unsigned long ) ;
S4EXPORT void S4FUNCTION f4flagSetAll( F4FLAG S4PTR * ) ;
S4EXPORT int  S4FUNCTION f4flagSetRange( F4FLAG S4PTR *, const unsigned long, const unsigned long ) ;

/* For Report Module */
S4EXPORT int  S4FUNCTION f4flagOr( F4FLAG S4PTR *, const F4FLAG S4PTR * ) ;
S4EXPORT int  S4FUNCTION f4flagAnd( F4FLAG S4PTR *, const F4FLAG S4PTR * ) ;
S4EXPORT void S4FUNCTION f4flagFlipReturns( F4FLAG S4PTR * ) ;

#ifdef __cplusplus
   }
#endif

