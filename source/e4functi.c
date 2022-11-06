/* e4functi.c  (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4NO_POW
   #include <math.h>
#endif

#ifndef S4OFF_REPORT
   double S4FUNCTION total4value( TOTAL4 *t4 );
#endif

const E4FUNCTIONS v4functions[EXPR4NUM_FUNCTIONS] =
{
   /* function, name, code, nameLen, priority, returnType, numParms, type[]*/
   { e4fieldAdd,   0,   0, 0, 0, r4str,      0, 0, 0, 0 },  /* E4FIELD_STR */
   { e4fieldAdd,   0,   0, 0, 0, r5wstr,     0, 0, 0, 0 },  /* E4FIELD_WSTR */
   { e4fieldCopy,  0,   1, 0, 0, r4str,      0, 0, 0, 0 },  /* E4FIELD_STR_CAT */
   { e4fieldCopy,  0,   1, 0, 0, r5wstr,     0, 0, 0, 0 },  /* E4FIELD_STR_CAT */
   { e4fieldLog,   0,   2, 0, 0, r4log,      0, 0, 0, 0 },  /* E4FIELD_LOG */
   { e4fieldDateD, 0,   3, 0, 0, r4dateDoub, 0, 0, 0, 0 },  /* E4FIELD_DATE_D */
   { e4fieldAdd,   0,   4, 0, 0, r4date,     0, 0, 0, 0 },  /* E4FIELD_DATE_S */
   { e4fieldNumD,  0,   5, 0, 0, r4numDoub,  0, 0, 0, 0 },  /* E4FIELD_NUM_D */
   { e4fieldAdd,   0,   6, 0, 0, r4num,      0, 0, 0, 0 },  /* E4FIELD_NUM_S */
   { e4fieldAdd,   0, 440, 0, 0, r4currency, 0, 0, 0, 0 },  /* E4FIELD_CUR */
   { e4fieldAdd,   0, 441, 0, 0, r4numDoub,  0, 0, 0, 0 },  /* E4FIELD_DOUB */
   { e4fieldAdd,   0, 442, 0, 0, r4int,      0, 0, 0, 0 },  /* E4FIELD_INT */
   { e4fieldAdd,   0, 443, 0, 0, r5ui4,      0, 0, 0, 0 },  /* E4FIELD_UNS_INT */
   { e4fieldAdd,   0, 444, 0, 0, r5i2,       0, 0, 0, 0 },  /* E4FIELD_SHORT */
   { e4fieldAdd,   0, 445, 0, 0, r5ui2,      0, 0, 0, 0 },  /* E4FIELD_UNS_SHORT */
   { e4fieldAdd,   0, 446, 0, 0, r4dateTime, 0, 0, 0, 0 },  /* E4FIELD_DTTIME */
   { e4fieldIntD,  0, 447, 0, 0, r4numDoub,  0, 0, 0, 0 },  /* E4FIELD_INT_D */
   { e4fieldCurD,  0, 448, 0, 0, r4numDoub,  0, 0, 0, 0 },  /* E4FIELD_CUR_D */

   /* *** E4LAST_FIELD IS SET TO SAME VALUE AS E4FIELD_MEMO,
          THIS MEANS ALL NEW FIELD ADDITIONS MUST GO BEFORE THIS COMMENT LINE
   */
   #ifdef S4OFF_MEMO
      { 0,           0, 7, 0,  0, r4str, 0, 0, 0, 0 },
   #else
      { e4fieldMemo, 0, 7, 0,  0, r4str, 0, 0, 0, 0 },       /* E4FIELD_MEMO */
   #endif

   { e4copyConstant, 0,  8, 0, 0, r4numDoub,  0, 0, 0, 0 },   /* E4DOUBLE */
   { e4copyConstant, 0,  9, 0, 0, r4str,      0, 0, 0, 0 },   /* E4STRING */

   /* E4FIRST_LOG STARTS AT E4LAST_FIELD + 4 (i.e. after copy constants) */
   { expr4trueFunction, ".TRUE.",  14, 6, 0, r4log, 0,     0, 0, 0 },
   { expr4trueFunction, ".T.",     14, 3, 0, r4log, 0,     0, 0, 0 },
   { e4false,           ".FALSE.", 16, 7, 0, r4log, 0,     0, 0, 0 },
   { e4false,           ".F.",     16, 3, 0, r4log, 0,     0, 0, 0 },
   { e4not,             ".NOT.",   18, 5, 5, r4log, 1, r4log, 0, 0 },

   /* E4LAST_LOG IS SET AT E4FIRST_LOG + 4 (since inclusive 10 to 14 = 5) */

   /* E4FIRST_OPERATOR IS SET AT E4LAST_LOG + 1 */
   /* E4LAST_OPERATOR IS SET AT E4FIRST_OPERATOR + 51 (52 entries less one for exclusive) */
   { e4or,            ".OR.",   20, 4, 3, r4log, -1, r4log, 0, 0 }, /* Flexible # of parms.*/
   { e4and,           ".AND.",  22, 5, 4, r4log, -1, r4log, 0, 0 },

   { e4parmRemove,   "+", 25, 1, 7, r4str,      2, r4str,     r4str,      0 },  /* Concatenate */
   { e4parmRemove,     0, 25, 0, 7, r5wstr,     2, r5wstr,    r5wstr,     0 },  /* Concatenate */
   { e4concatTrim,     0, 25, 0, 7, r4str,      2, r4str,     r4str,      0 },  /* Concatenate */
   { e4add,            0, 25, 0, 7, r4numDoub,  2, r4numDoub, r4numDoub,  0 },
   { e4addDate,        0, 25, 0, 7, r4dateDoub, 2, r4numDoub, r4dateDoub, 0 },
   { e4addDate,        0, 25, 0, 7, r4dateDoub, 2, r4dateDoub,r4numDoub,  0 },

   { e4concatTwo,    "-", 30, 1, 7, r4str,      2, r4str,      r4str,      0 },
   { e4sub,            0, 30, 0, 7, r4numDoub,  2, r4numDoub,  r4numDoub,  0 },
   { e4subDate,        0, 30, 0, 7, r4numDoub,  2, r4dateDoub, r4dateDoub, 0 },
   { e4subDate,        0, 30, 0, 7, r4dateDoub, 2, r4dateDoub, r4numDoub,  0 },

   /* E4COMPARE_START IS E4FIRST_OPEATOR + 11 */
   { e4notEqual,       "#",  50, 1, 6, r4log, 2, r4str,      r4str,      0 },
   { e4notEqual,      "<>",  50, 2, 6, r4log, 2, r4str,      r4str,      0 },
   { e4notEqual,         0,  50, 0, 6, r4log, 2, r4numDoub,  r4numDoub,  0 },
   { e4notEqual,         0,  50, 0, 6, r4log, 2, r4dateDoub, r4dateDoub, 0 },
   { e4notEqual,         0,  50, 0, 6, r4log, 2, r4log,      r4log,      0 },
   { e4notEqualCur,      0,  50, 0, 6, r4log, 2, r4currency, r4currency, 0 },
   { e4notEqualDtTime,   0,  50, 0, 6, r4log, 2, r4dateTime, r4dateTime, 0 },

   { e4greaterEq,     ">=",  60, 2, 6, r4log, 2, r4str,      r4str,      0 },
   { e4greaterEq,     "=>",  60, 2, 6, r4log, 2, r4str,      r4str,      0 },
   { e4greaterEqDoub,    0,  60, 0, 6, r4log, 2, r4numDoub,  r4numDoub,  0 },
   { e4greaterEqDoub,    0,  60, 0, 6, r4log, 2, r4dateDoub, r4dateDoub, 0 },
   { e4greaterEqCur,     0,  60, 0, 6, r4log, 2, r4currency, r4currency, 0 },
   { e4greaterEqDtTime,  0,  60, 0, 6, r4log, 2, r4dateTime, r4dateTime, 0 },

   { e4lessEq,        "<=",  70, 2, 6, r4log, 2, r4str,      r4str, 0 },
   { e4lessEq,        "=<",  70, 2, 6, r4log, 2, r4str,      r4str, 0 },
   { e4lessEqDoub,       0,  70, 0, 6, r4log, 2, r4numDoub,  r4numDoub, 0 },
   { e4lessEqDoub,       0,  70, 0, 6, r4log, 2, r4dateDoub, r4dateDoub, 0 },
   { e4lessEqCur,        0,  70, 0, 6, r4log, 2, r4currency, r4currency, 0 },
   { e4lessEqDtTime,     0,  70, 0, 6, r4log, 2, r4dateTime, r4dateTime, 0 },

   { e4equal,          "=",  40, 1, 6, r4log, 2, r4str,      r4str,      0 },
   { e4equal,            0,  40, 0, 6, r4log, 2, r4log,      r4log,      0 },
   { e4equal,            0,  40, 0, 6, r4log, 2, r4numDoub,  r4numDoub,  0 },
   { e4equal,            0,  40, 0, 6, r4log, 2, r4dateDoub, r4dateDoub, 0 },
   { e4equalCur,         0,  40, 0, 6, r4log, 2, r4currency, r4currency, 0 },
   { e4equalDtTime,      0,  40, 0, 6, r4log, 2, r4dateTime, r4dateTime, 0 },

   { e4greater,          ">",  80, 1, 6, r4log, 2, r4str, r4str, 0 },
   { e4greaterDoub,       0,  80, 0, 6, r4log, 2, r4numDoub,  r4numDoub, 0 },
   { e4greaterDoub,       0,  80, 0, 6, r4log, 2, r4dateDoub, r4dateDoub, 0 },
   { e4greaterCur,        0,  80, 0, 6, r4log, 2, r4currency, r4currency, 0 },
   { e4greaterDtTime,     0,  80, 0, 6, r4log, 2, r4dateTime, r4dateTime, 0 },

   { e4less,             "<",  90, 1, 6, r4log, 2, r4str, r4str, 0 },
   { e4lessDoub,          0,  90, 0, 6, r4log, 2, r4numDoub,  r4numDoub, 0 },
   { e4lessDoub,          0,  90, 0, 6, r4log, 2, r4dateDoub, r4dateDoub, 0 },
   { e4lessCur,           0,  90, 0, 6, r4log, 2, r4currency, r4currency, 0 },
   { e4lessDtTime,        0,  90, 0, 6, r4log, 2, r4dateTime, r4dateTime, 0 },
   /* E4COMPARE_END IS E4COMPARE_START + 35 */

   #ifdef S4NO_POW
      {       0,             0,   95, 0, 0, r4numDoub, 2, r4numDoub, 0, 0 },
      {       0,             0,   95, 0, 0, r4numDoub, 2, r4numDoub, 0, 0 },
   #else
      { e4power,           "^",  100, 1, 9, r4numDoub, 2, r4numDoub, r4numDoub, 0},
      { e4power,          "**",  100, 2, 9, r4numDoub, 2, r4numDoub, r4numDoub, 0},
   #endif

   { e4multiply,         "*", 102, 1, 8, r4numDoub, 2, r4numDoub, r4numDoub, 0},
   { e4divide,           "/", 105, 1, 8, r4numDoub, 2, r4numDoub, r4numDoub, 0},
   { e4contain,          "$", 110, 1, 6, r4log, 2, r4str, r4str, 0 },

   /* E4FIRST_FUNCTION IS E4COMPARE_END + 6 --> since compare_end was
      inclusive, should be 1 more than above functions (which is 5) */
   { e4chr,       "CHR", 120, 3, 0, r4str, 1, r4numDoub,  0, 0 },
   { e4del,       "DEL", 130, 3, 0, r4str, 0, 0,          0, 0 },
   { e4str,       "STR", 140, 3, 0, r4str, 1, r4numDoub,  0, 0 },
   { e4wideToStr,       "STR", 140, 1, 0, r4str, 1, r5wstr,  0, 0 },
   { e4substr, "SUBSTR", 150, 6, 0, r4str, 1, r4str,      0, 0 },
   { e4time,     "TIME", 160, 4, 0, r4str, 0, 0,          0, 0 },
   { e4upper,   "UPPER", 170, 5, 0, r4str, 1, r4str,      0, 0 },
   { e4copyParm, "DTOS", 180, 4, 0, r4str, 1, r4date,     0, 0 },
   { e4dtosDoub,      0, 180, 0, 0, r4str, 1, r4dateDoub, 0, 0 },
   { e4dtoc,     "DTOC", 200, 4, 0, r4str, 1, r4date,     0, 0 },
   { e4dtocDoub,      0, 200, 4, 0, r4str, 1, r4dateDoub, 0, 0 },

   { e4trim,     "TRIM",220, 4, 0, r4str, 1, r4str, 0, 0 },
   { e4ltrim,   "LTRIM",230, 5, 0, r4str, 1, r4str, 0, 0 },
   { e4alltrim, "ALLTRIM",235, 7, 0, r4str, 1, r4str, 0, 0 },
   { e4substr,   "LEFT",240, 4, 0, r4str, 1, r4str, 0, 0 },
   { e4substr,   "RIGHT",245, 5, 0, r4str, 1, r4str, 0, 0 },

   { e4iif,  "IIF", 250, 3, 0,      r4str, 3, r4log,      r4str, r4str },
   { e4iif,      0, 250, 0, 0,  r4numDoub, 3, r4log,  r4numDoub, r4numDoub},
   { e4iif,      0, 250, 0, 0, r4dateDoub, 3, r4log, r4dateDoub, r4dateDoub},
   { e4iif,      0, 250, 0, 0,      r4log, 3, r4log,      r4log, r4log },

   { e4stod,        "STOD",  260, 4, 0, r4dateDoub, 1,      r4str, 0, 0 },
   { e4ctod,        "CTOD",  270, 4, 0, r4dateDoub, 1,      r4str, 0, 0 },
   { e4date,        "DATE",  280, 4, 0, r4dateDoub, 0,          0, 0, 0 },
   { e4day,          "DAY",  290, 3, 0,  r4numDoub, 1,     r4date, 0, 0 },
   { e4dayDoub,          0,  290, 0, 0,  r4numDoub, 1, r4dateDoub, 0, 0 },
   { e4month,      "MONTH",  310, 5, 0,  r4numDoub, 1,     r4date, 0, 0 },
   { e4monthDoub,        0,  310, 0, 0,  r4numDoub, 1, r4dateDoub, 0, 0 },
   { e4year,        "YEAR",  340, 4, 0,  r4numDoub, 1,     r4date, 0, 0 },
   { e4yearDoub,         0,  340, 0, 0,  r4numDoub, 1, r4dateDoub, 0, 0 },
   { e4deleted,  "DELETED",  350, 7, 0,      r4log, 0,          0, 0, 0 },
   { e4recCount,"RECCOUNT",  360, 8, 0,  r4numDoub, 0,          0, 0, 0 },
   { e4recno,      "RECNO",  370, 5, 0,  r4numDoub, 0,          0, 0, 0 },
   { e4val,          "VAL",  380, 3, 0,  r4numDoub, 1,      r4str, 0, 0 },
   { e4calcFunction,     0,  390, 0, 0,          0, 0,          0, 0, 0 },
#ifndef S4OFF_REPORT
/* { e4calcFunction,     0,  390, 0, 0,          0, 0,          0, 0, 0 },*/
   { e4calcTotal,        0,  400, 0, 0,  r4numDoub, 0,          0, 0, 0 },
   { e4pageno,    "PAGENO",  410, 6, 0,  r4numDoub, 0,          0, 0, 0 },
#else
/* { 0,                  0,  390, 0, 0,          0, 0,          0, 0, 0 },*/
   { 0,                  0,  400, 0, 0,  r4numDoub, 0,          0, 0, 0 },
   { 0,                  0,  410, 6, 0,  r4numDoub, 0,          0, 0, 0 },
#endif
   /* DESCEND() only works like Clipper with Char parameter. */
   { e4descend,    "DESCEND",  420, 7, 0, r4str, 1, r4str,     0, 0 },
   { e4descend,            0,  420, 7, 0, r4str, 1, r4num,     0, 0 },
   { e4descend,            0,  420, 7, 0, r4str, 1, r4dateDoub,0, 0 },
   { e4descend,            0,  420, 7, 0, r4str, 1, r4log,     0, 0 },
   { e4descendBinary,      0,  420, 7, 0, r4str, 1, r4dateTime, 0, 0 },
   { e4descendBinary,      0,  420, 7, 0, r4str, 1, r4int, 0, 0 },
   { e4descendBinary,      0,  420, 7, 0, r4str, 1, r4currency, 0, 0 },
   { e4descendBinary,      0,  420, 7, 0, r4str, 1, r5wstr, 0, 0 },
   { e4descendBinary,      0,  420, 7, 0, r4str, 1, r4numDoub, 0, 0 },
   { e4ascend,      "ASCEND",  430, 6, 0, r4str, 1, r4str,     0, 0 },
   { e4ascend,             0,  430, 6, 0, r4str, 1, r5wstr,    0, 0 },
   { e4ascend,             0,  430, 6, 0, r4str, 1, r4num,     0, 0 },
   { e4ascend,             0,  430, 6, 0, r4str, 1, r4date,    0, 0 },
   { e4ascend,             0,  430, 6, 0, r4str, 1, r4dateDoub,0, 0 },
   { e4ascend,             0,  430, 6, 0, r4str, 1, r4log,     0, 0 },
   { e4ascend,             0,  430, 6, 0, r4str, 1, r4int,    0, 0 },
   { e4ascend,             0,  430, 6, 0, r4str, 1, r4dateTime, 0, 0 },
   { e4ascend,             0,  430, 6, 0, r4str, 1, r4currency,    0, 0 },
   { e4ascend,             0,  430, 6, 0, r4str, 1, r4numDoub, 0, 0 },
   { 0,0,-1, 0, 0, 0, 0, 0, 0, 0 },
} ;
/* The following function was added so that the OLEDB dll will have access to the above data.*/

const E4FUNCTIONS *S4FUNCTION e4functions()
{
  return (&v4functions[0]);
}

void S4FUNCTION expr4functions( const E4FUNCTIONS **fptr )
{
   *fptr = v4functions ;
}

void e4add()
{
   #ifdef S4DATA_ALIG2
      double a,b,c;
   #endif
   double *doublePptr = (double *) (expr4buf + expr4infoPtr->resultPos) ;
   #ifdef S4DATA_ALIG2
      memcpy(&a, expr4[-2], sizeof(double));
      memcpy(&b, expr4[-1], sizeof(double));
      c = a + b;
      memcpy(doublePptr, &c, sizeof(double));
   #else
      *doublePptr = *(double *)expr4[-2] + *(double *)expr4[-1] ;
   #endif
   expr4[-2] = (char *) doublePptr ;
   expr4-- ;
}

void e4addDate()
{
   if ( v4functions[expr4infoPtr->functionI].type[0] == r4dateDoub )
   {
      if ( *(double *)expr4[-2] == 0.0 )
      {
         *(double *)expr4-- = 0.0 ;
         return ;
      }
   }
   else
   {
      if ( *(double *)expr4[-1] == 0.0 )
      {
         *(double *)expr4-- = 0.0 ;
         return ;
      }
   }

   e4add() ;
}

void e4and()
{
   int i ;

   expr4 -= expr4infoPtr->numParms ;
   for( i = expr4infoPtr->numParms-1 ; i > 0 ; i-- )
      *(int *) expr4[0] = * (int *) expr4[i]  &&  * (int *) expr4[0] ;
   expr4++ ;
}

/* The total length of the result is in 'expr4infoPtr->len'. */
void e4concatSpecial( char movePchar )
{
   int lenTwo, numChars, pos, firstLen ;
   char *ptr ;

   firstLen = expr4infoPtr[-expr4infoPtr[-1].numEntries-1].len ;
   ptr = expr4[-2] ;

   for ( pos = firstLen-1; pos >= 0; pos-- )
      if ( ptr[pos] != movePchar )
         break ;
   if ( ++pos < firstLen )
   {
      lenTwo = expr4infoPtr->len - firstLen ;
      c4memmove( ptr+ pos, expr4[-1], (unsigned int)lenTwo ) ;

      numChars = firstLen - pos ;
      memset( ptr+expr4infoPtr->len-numChars, movePchar, (unsigned int)numChars ) ;
   }
   expr4-- ;
}

void e4concatTrim()
{
   e4concatSpecial(0) ;
}

void e4concatTwo()
{
   e4concatSpecial(' ') ;
}

void e4contain()
{
   int aLen, compLen, i, logResult ;
   char firstChar, *bPtr ;

   logResult = 0 ;
   aLen = expr4infoPtr[-expr4infoPtr[-1].numEntries-1].len ;
   firstChar = *expr4[-2] ;
   compLen = expr4infoPtr[-1].len - aLen ;
   bPtr = expr4[-1] ;

   /* See if there is a match */
   for ( i=0; i <= compLen; i++ )
      if ( firstChar == bPtr[i] )
         if ( u4memcmp( expr4[-2], bPtr+i, (size_t)aLen ) == 0 )
         {
            logResult = 1 ;
            break ;
         }

   expr4[-2] = expr4buf + expr4infoPtr->resultPos ;
   *(int *) expr4[-2] = logResult ;
   expr4-- ;
}

void e4copyConstant()
{
   void *ptr = *expr4++ = expr4buf+expr4infoPtr->resultPos ;
   #ifdef E4ANALYZE
      if ( ptr == 0 )
         error4( 0, e4info, E80903 ) ;
   #endif
   memcpy( ptr, expr4constants+ expr4infoPtr->i1, (unsigned int)expr4infoPtr->len ) ;
}

void e4fieldCopy()
{
   void *ptr = *expr4++ = expr4buf+expr4infoPtr->resultPos ;
   #ifdef E4ANALYZE
      if ( ptr == 0 )
         error4( 0, e4info, E80903 ) ;
   #endif
   memcpy( ptr, *(char **)expr4infoPtr->p1 + expr4infoPtr->i1, (unsigned int)expr4infoPtr->len ) ;
}

void e4copyParm()
{
   /* DTOS */
   void *ptr = expr4[-1] ;
   expr4[-1] = expr4buf + expr4infoPtr->resultPos ;
   #ifdef E4ANALYZE
      if ( ptr == 0 )
         error4( 0, e4info, E80903 ) ;
   #endif

   /* AS 11/20/97 FoxPro verified and MDX verified, DTOS( blank date )
      should produce <blank> not <0> which was previously being produced */
   if ( (unsigned int)expr4infoPtr->len == 8 )
      if ( memcmp( ptr, "       0", 8 ) == 0 )  /* change to blank */
      {
         memset( expr4[-1], ' ', 8 ) ;
         return ;
      }

   memcpy( expr4[-1], ptr, (unsigned int)expr4infoPtr->len ) ;
}

void e4ctod()
{
   char buf[8] ;
   double d ;

   date4init( buf, expr4[-1], expr4constants+ expr4infoPtr->i1 ) ;
   expr4[-1] = expr4buf + expr4infoPtr->resultPos ;
   d = date4long( buf ) ;
   memcpy( expr4[-1], (void *)&d, sizeof(d) ) ;
}

void e4date()
{
   char datePbuf[8] ;
   date4today( datePbuf ) ;
   *expr4++ = expr4buf + expr4infoPtr->resultPos ;
   *((double *) expr4[-1]) = (double) date4long( datePbuf ) ;
}

void e4day()
{
   double d ;
   d = (double) date4day( expr4[-1] ) ;
   expr4[-1] = expr4buf + expr4infoPtr->resultPos ;
   *(double *) expr4[-1] = d ;
}

void e4dayDoub()
{
   char datePbuf[8] ;
   date4assign( datePbuf, (long) *(double *)expr4[-1] ) ;
   expr4[-1] = expr4buf + expr4infoPtr->resultPos ;
   *(double *) expr4[-1] = (double) date4day( datePbuf ) ;
}

void e4del()
{
   expr4[0] = expr4buf + expr4infoPtr->resultPos ;
   expr4[0][0] = *( *(char **)expr4infoPtr->p1) ;
   expr4++ ;
}

void e4deleted()
{
   int result = 0 ;

   #ifdef E4MISC
      if ( *( *(char **)expr4infoPtr->p1 ) != '*' && *( *(char **)expr4infoPtr->p1 ) != ' ' )
         error4( 0, e4info, E80907 ) ;
   #endif

   if ( *( *(char **)expr4infoPtr->p1 ) == '*' )
      result = 1 ;

   *(int *) (*expr4++ = expr4buf + expr4infoPtr->resultPos ) = result ;
}

void e4divide()
{
   double doub, *resultPtr ;

   resultPtr = (double *) (expr4buf + expr4infoPtr->resultPos) ;
   memcpy( (void *)&doub, (void *)expr4[-1], sizeof(double ) ) ;
   if ( doub == 0.0 )
      *resultPtr = 0.0 ;
   else
      *resultPtr = *(double *)expr4[-2] / *(double *) expr4[-1] ;
   expr4[-2] = (char *) resultPtr ;
   expr4-- ;
}

void e4dtoc()
{
   char buf[LEN4DATE_FORMAT] ;

   date4format( expr4[-1], buf, expr4constants+ expr4infoPtr->i1 ) ;
   expr4[-1] = expr4buf + expr4infoPtr->resultPos ;
   memcpy( expr4[-1], buf, (unsigned int)expr4infoPtr->len ) ;
}

void e4dtocDoub()
{
   e4dtosDoub() ;
   e4dtoc() ;
}

void e4chr()
{
   double doub ;

   memcpy( (void *) &doub, (void *) expr4[-1], sizeof(double) ) ;
   *expr4[-1] = (char) doub ;
}

void e4dtosDoub()
{
   date4assign( expr4buf + expr4infoPtr->resultPos, (long) *(double *) expr4[-1] ) ;
   expr4[-1] = expr4buf + expr4infoPtr->resultPos ;
}

void e4equalCur()
{
   int *intPtr ;

   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;
   *intPtr = !currency4compare( (CURRENCY4 *)expr4[-2], (CURRENCY4 *)expr4[-1] )  ;

   expr4[-2] = (char *)intPtr ;
   expr4-- ;
}

void e4equalDtTime()
{
   int *intPtr ;

   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;
   *intPtr = !date4timeCompare( (long *)expr4[-2], (long *)expr4[-1] )  ;

   expr4[-2] = (char *)intPtr ;
   expr4-- ;
}

void e4equal()
{
   int *intPtr ;

   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;
   *intPtr = !u4memcmp( expr4[-2], expr4[-1], (unsigned int)expr4infoPtr->i1 )  ;

   expr4[-2] = (char *)intPtr ;
   expr4-- ;
}

void e4false()
{
   int *ptr ;

   ptr = (int *) (*expr4++ = expr4buf+expr4infoPtr->resultPos) ;
   *ptr = 0 ;
}

void e4fieldDateD()
{
   void *ptr = *expr4++ = expr4buf+expr4infoPtr->resultPos ;
   double d = date4long( *(char **)expr4infoPtr->p1 + expr4infoPtr->i1 ) ;
   #ifdef E4ANALYZE
      if ( ptr == 0 )
         error4( 0, e4info, E80903 ) ;
   #endif
   memcpy( ptr, (void *)&d, sizeof(d) ) ;
}

void e4fieldLog()
{
   int *ptr = (int *) (*expr4++ = expr4buf+expr4infoPtr->resultPos) ;
   char charValue = *(* (char **)expr4infoPtr->p1 + expr4infoPtr->i1 ) ;
   if ( charValue == 'Y'  ||  charValue == 'y'  ||
         charValue == 'T'  ||  charValue == 't'  )
      *ptr = 1 ;
   else
      *ptr = 0 ;
}

#ifndef S4OFF_MEMO
void e4fieldMemo()
{
   char *ptr, *memoPtr ;
   unsigned memoLen, copyLen, zeroLen ;

   ptr = *expr4++ = expr4buf + expr4infoPtr->resultPos ;
/*   memoLen = f4memoLen( d4fieldJ( expr4ptr->data, expr4infoPtr->fieldNo ) ) ;*/
/*   memoPtr = f4memoPtr( d4fieldJ( expr4ptr->data, expr4infoPtr->fieldNo ) ) ;*/
   memoLen = f4memoLen( expr4infoPtr->fieldPtr ) ;
   memoPtr = f4memoPtr( expr4infoPtr->fieldPtr ) ;
   if( error4code( expr4ptr->codeBase ) < 0 )
      return ;

   copyLen = memoLen ;
   zeroLen = 0 ;
   if( copyLen > (unsigned) expr4infoPtr->len )
      copyLen = (unsigned int)expr4infoPtr->len ;
   else
      zeroLen = expr4infoPtr->len - copyLen ;

   #ifdef E4ANALYZE
      if ( ptr == 0 )
         error4( 0, e4info, E80903 ) ;
   #endif
   memcpy( ptr, memoPtr, copyLen ) ;
   memset( ptr + copyLen, 0, zeroLen ) ;
}
#endif

void e4fieldCurD()
{
   void *ptr ;
   double d ;
   CURRENCY4 *val ;
   char currencyBuffer[21] ;

   ptr = *expr4++ = expr4buf + expr4infoPtr->resultPos ;
   val = ((CURRENCY4 *)( *(char **)expr4infoPtr->p1 + expr4infoPtr->i1 ) ) ;

   /*
      this algorithm produces values which might be off at the last point
      of double precision (comparet to string calculations), so instead
      the slower string conversion routines were inserted */
   /*
      d = val->lo[3] ;
      d = d * 65536 + (double)val->lo[2] ;
      d = d * 65536 + (double)val->lo[1] ;
      d = d * 65536 + (double)val->lo[0] ;
      d /= 10000.0 ;
   */

   memset( currencyBuffer, 0, sizeof( currencyBuffer ) ) ;
   c4currencyToA( currencyBuffer, sizeof( currencyBuffer ), val, 4 ) ;
   d = c4atod( currencyBuffer, strlen( currencyBuffer ) ) ;

   #ifdef E4ANALYZE
      if ( ptr == 0 )
         error4( 0, e4info, E80903 ) ;
   #endif
   memcpy( ptr, (void *)&d, sizeof(d) ) ;
}

void e4fieldIntD()
{
   void *ptr ;
   double d ;
   long val ;

   ptr = *expr4++ = expr4buf + expr4infoPtr->resultPos ;
   val = *((long *)( *(char **)expr4infoPtr->p1 + expr4infoPtr->i1 ) ) ;
   d = (double)val ;
   #ifdef E4ANALYZE
      if ( ptr == 0 )
         error4( 0, e4info, E80903 ) ;
   #endif
   memcpy( ptr, (void *)&d, sizeof(d) ) ;
}

void e4fieldNumD()
{
   void *ptr ;
   double d ;

   ptr = *expr4++ = expr4buf + expr4infoPtr->resultPos ;
   d = c4atod( *(char **)expr4infoPtr->p1 + expr4infoPtr->i1, expr4infoPtr->len ) ;
   #ifdef E4ANALYZE
      if ( ptr == 0 )
         error4( 0, e4info, E80903 ) ;
   #endif
   memcpy( ptr, (void *)&d, sizeof(d) ) ;
}

void e4greaterCur()
{
   int *intPtr, rc ;

   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;
   rc = currency4compare( (CURRENCY4 *)expr4[-2], (CURRENCY4 *)expr4[-1] )  ;

   if( rc > 0 )
      *intPtr = 1 ;
   else
   {
      if( rc < 0 )
         *intPtr = 0 ;
      else
         *intPtr = (int)((long)expr4infoPtr->p1) ;
   }
   expr4[-2] = (char *) intPtr ;
   expr4-- ;
}

void e4greaterDtTime()
{
   int *intPtr, rc ;

   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;
   rc = date4timeCompare( (long *)expr4[-2], (long *)expr4[-1] )  ;

   if( rc > 0 )
      *intPtr = 1 ;
   else
   {
      if( rc < 0 )
         *intPtr = 0 ;
      else
         *intPtr = (int)((long)expr4infoPtr->p1) ;
   }
   expr4[-2] = (char *) intPtr ;
   expr4-- ;
}

void e4greater()
{
   int *intPtr, rc ;
   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;
   rc = u4memcmp( expr4[-2], expr4[-1], (unsigned int)expr4infoPtr->i1 ) ;

   if( rc > 0 )
      *intPtr = 1 ;
   else
   {
      if( rc < 0 )
         *intPtr = 0 ;
      else
         *intPtr = (int)((long)expr4infoPtr->p1) ;
   }
   expr4[-2] = (char *) intPtr ;
   expr4-- ;
}

void e4greaterDoub()
{
   e4lessEqDoub() ;
   *(int *)expr4[-1] = ! *(int *)expr4[-1] ;
}

void e4greaterEqCur()
{
   e4lessCur() ;
   *((int *)expr4[-1]) = ! *((int *)expr4[-1]) ;
}

void e4greaterEqDtTime()
{
   e4lessDtTime() ;
   *((int *)expr4[-1]) = ! *((int *)expr4[-1]) ;
}

void e4greaterEq()
{
   e4less() ;
   *((int *)expr4[-1]) = ! *((int *)expr4[-1]) ;
}

void e4greaterEqDoub()
{
   int *intPtr ;
   #ifdef S4DATA_ALIG2
      double a,b;
   #endif

   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;
   #ifdef S4DATA_ALIG2
      memcpy(&a, (double *)expr4[-2], sizeof(double));
      memcpy(&b, (double *)expr4[-1], sizeof(double));
      if (a>=b)
   #else
      if ( *(double *)expr4[-2] >= *(double *)expr4[-1] )
   #endif /* !S4DATA_ALIG2 */
      *intPtr = 1 ;
   else
      *intPtr = 0 ;
   expr4[-2] = (char *) intPtr ;
   expr4-- ;
}

void e4iif()
{
   if ( *(int *) expr4[-3] )
      c4memmove( expr4buf + expr4infoPtr->resultPos, expr4[-2], (unsigned int)expr4infoPtr->len ) ;
   else
      c4memmove( expr4buf + expr4infoPtr->resultPos, expr4[-1], (unsigned int)expr4infoPtr->len ) ;
   expr4[-3] = expr4buf + expr4infoPtr->resultPos ;
   expr4-= 2 ;
}

void e4lessCur()
{
   int *intPtr, rc ;

   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;
   rc = currency4compare( (CURRENCY4 *)expr4[-2], (CURRENCY4 *)expr4[-1] )  ;

   if( rc < 0 )
      *intPtr = 1 ;
   else
   {
      if( rc > 0 )
         *intPtr = 0 ;
      else
         *intPtr = (int)((long)expr4infoPtr->p1) ;
   }

   expr4[-2] = (char *)intPtr ;
   expr4-- ;
}

void e4lessDtTime()
{
   int *intPtr, rc ;

   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;
   rc = date4timeCompare( (long *)expr4[-2], (long *)expr4[-1] )  ;

   if( rc < 0 )
      *intPtr = 1 ;
   else
   {
      if( rc > 0 )
         *intPtr = 0 ;
      else
         *intPtr = (int)((long)expr4infoPtr->p1) ;
   }

   expr4[-2] = (char *)intPtr ;
   expr4-- ;
}

void e4less()
{
   int *intPtr, rc ;

   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;
   rc = u4memcmp( expr4[-2], expr4[-1], (unsigned int)expr4infoPtr->i1 ) ;

   if( rc < 0 )
      *intPtr = 1 ;
   else
   {
      if( rc > 0 )
         *intPtr = 0 ;
      else
         *intPtr = (int)((long)expr4infoPtr->p1) ;
   }

   expr4[-2] = (char *)intPtr ;
   expr4-- ;
}

void e4lessDoub()
{
   e4greaterEqDoub() ;
   *(int *)expr4[-1] = ! *(int *)expr4[-1] ;
}

void e4lessEqCur()
{
   e4greaterCur() ;
   *((int *)expr4[-1]) = ! *((int *)expr4[-1]) ;
}

void e4lessEqDtTime()
{
   e4greaterDtTime() ;
   *((int *)expr4[-1]) = ! *((int *)expr4[-1]) ;
}

void e4lessEq()
{
   e4greater() ;
   *((int *)expr4[-1]) = ! *((int *)expr4[-1]) ;
}

void e4lessEqDoub()
{
   int *intPtr ;
   #ifdef S4DATA_ALIG2
      double doub1, doub2 ;

      memcpy( (void *)&doub1, (void *)expr4[-2], sizeof(double) ) ;
      memcpy( (void *)&doub2, (void *)expr4[-1], sizeof(double) ) ;
   #endif

   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;

   #ifdef S4DATA_ALIG2
      if ( doub1 <= doub2 )
   #else
      if ( *(double *)expr4[-2] <= *(double *)expr4[-1] )
   #endif
      *intPtr = 1 ;
   else
      *intPtr = 0 ;
   expr4[-2] = (char *) intPtr ;
   expr4-- ;
}

void e4ltrim()
{
   int n ;
   char *ptr ;

   for( n = 0; n < expr4infoPtr->len; n++ )
      if ( expr4[-1][n] != ' ' && expr4[-1][n] != 0 )
         break ;
   ptr = expr4buf + expr4infoPtr->resultPos ;
   c4memmove( ptr, expr4[-1]+n, (unsigned int)(expr4infoPtr->len - n) ) ;
   memset( ptr+ expr4infoPtr->len - n, 0, (unsigned int)n ) ;
   expr4[-1] = ptr ;
}

void e4alltrim()
{
   e4ltrim() ;
   e4trim() ;
}

void e4month()
{
   double *doublePptr = (double *)(expr4buf + expr4infoPtr->resultPos) ;
   *doublePptr = (double)date4month( expr4[-1] ) ;
   expr4[-1] = (char *) doublePptr ;
}

void e4monthDoub()
{
   char datePbuf[8] ;
   double *doublePptr ;

   doublePptr = (double *) (expr4buf + expr4infoPtr->resultPos) ;
   date4assign( datePbuf, (long) *(double *)expr4[-1] ) ;
   *doublePptr = (double) date4month( datePbuf ) ;
   expr4[-1] = (char *) doublePptr ;
}

void e4multiply()
{
   double *doublePptr ;

   doublePptr = (double *)(expr4buf + expr4infoPtr->resultPos) ;
   *doublePptr = *(double *)expr4[-2] * *(double *)expr4[-1] ;
   expr4[-2] = (char *) doublePptr ;
   expr4-- ;
}

void e4nop()
{
}

void e4not()
{
   int *ptr ;

   ptr = (int *)expr4[-1] ;
   *ptr = !*ptr ;
}

void e4notEqualCur()
{
   int *intPtr ;

   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;
   *intPtr = currency4compare( (CURRENCY4 *)expr4[-2], (CURRENCY4 *)expr4[-1] )  ;
   expr4[-2] = (char *) intPtr ;
   expr4-- ;
}

void e4notEqualDtTime()
{
   int *intPtr ;

   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;
   *intPtr = date4timeCompare( (long *)expr4[-2], (long *)expr4[-1] )  ;
   expr4[-2] = (char *) intPtr ;
   expr4-- ;
}

void e4notEqual()
{
   int *intPtr ;

   intPtr = (int *)(expr4buf + expr4infoPtr->resultPos) ;
   *intPtr = u4memcmp( expr4[-2], expr4[-1], (unsigned int)expr4infoPtr->i1 ) != 0 ? 1 : 0  ;
   expr4[-2] = (char *) intPtr ;
   expr4-- ;
}

void e4or()
{
   int i ;

   expr4 -= expr4infoPtr->numParms ;
   for( i = expr4infoPtr->numParms-1 ; i > 0 ; i-- )
      *(int *) expr4[0] = * (int *) expr4[i]  ||  * (int *) expr4[0] ;
   expr4++ ;
}

void e4fieldAdd()
{
   *expr4++ = *(char **)expr4infoPtr->p1 + expr4infoPtr->i1 ;
}

void e4parmRemove()
{
   expr4-- ;
}

#ifndef S4NO_POW
void e4power()
{
   double *doublePptr ;
   #ifdef S4DATA_ALIG2
      double doub1,doub2,doub3 ;
   #endif
   doublePptr = (double *) (expr4buf + expr4infoPtr->resultPos) ;
   #ifdef S4DATA_ALIG2
      memcpy( &doub2, expr4[-2], sizeof(double) ) ;
      memcpy( &doub1, expr4[-1], sizeof(double) ) ;
      doub3 = pow( doub2, doub1 ) ;
      memcpy( doublePptr, &doub3, sizeof(double) ) ;
   #else
      *doublePptr = pow( *(double *) expr4[-2], *(double *) expr4[-1] ) ;
   #endif
   expr4[-2] = (char *) doublePptr ;
   expr4-- ;
}
#endif

void e4recCount()
{
   double d ;
   long serverId = -2L ;
   DATA4FILE *dataFile ;

   dataFile = (DATA4FILE *)expr4infoPtr->p1 ;

   d = (double)dfile4recCount( dataFile, serverId ) ;
   memcpy( *expr4++ = expr4buf+ expr4infoPtr->resultPos, (void *)&d, sizeof(d) ) ;
}

void e4recno()
{
   double d ;

   d = (double)d4recNo( expr4ptr->data ) ;  /* updated for c/s */
/*   d = (double)d4recNo( (DATA4 *) expr4infoPtr->p1 ) ;*/
   memcpy( *expr4++ = expr4buf+ expr4infoPtr->resultPos, (void *)&d, sizeof(d) ) ;
}

void e4stod()
{
   double *doublePptr ;
   #ifdef S4DATA_ALIG2
      double a;
   #endif

   doublePptr = (double *)(expr4buf + expr4infoPtr->resultPos) ;
   #ifdef S4DATA_ALIG2
      a=(double)date4long(expr4[-1]);
      memcpy(doublePptr, &a, sizeof(double));
   #else
      *doublePptr = (double) date4long( expr4[-1] ) ;
   #endif  /* !S4DATA_ALIG2  */
   expr4[-1] = (char *) doublePptr ;
}

void e4str()
{
   char *ptr ;
   #ifdef S4DATA_ALIG2
        double doub;
   #endif

   ptr = expr4buf + expr4infoPtr->resultPos ;
   #ifdef S4DATA_ALIG2
      memcpy( (void *)&doub, expr4[-1], sizeof(double) );
      c4dtoa45( doub, ptr, expr4infoPtr->len, expr4infoPtr->i1 ) ;
   #else
      c4dtoa45( *(double *) expr4[-1], ptr, expr4infoPtr->len, expr4infoPtr->i1 ) ;
   #endif
   expr4[-1] = ptr ;
}

void e4wideToStr()
{
   char *ptr ;

   ptr = expr4buf + expr4infoPtr->resultPos ;
   #ifndef S4WINCE
      #ifdef __WIN32
         WideCharToMultiByte( CP_OEMCP, 0, (LPCWSTR)expr4[-1], expr4infoPtr->len, ptr, expr4infoPtr->len, NULL, NULL ) ;
      #else
         /* no widestring conversion available, just memcpy */
         memcpy( ptr, expr4[-1], expr4infoPtr->len ) ;
      #endif
   #else
      c4atou(expr4[-1], (unsigned short *)ptr, expr4infoPtr->len ) ;
   #endif
   expr4[-1] = ptr ;
}

void e4sub()
{
   double *doublePptr ;
   #ifdef S4DATA_ALIG2
      double a,b;
   #endif

   doublePptr = (double *)(expr4buf + expr4infoPtr->resultPos) ;
   #ifdef S4DATA_ALIG2
      memcpy(&a, expr4[-2], sizeof(double));
      memcpy(&b, expr4[-1], sizeof(double));
      *doublePptr = a-b;
   #else
      *doublePptr = *(double *)expr4[-2] - *(double *)expr4[-1] ;
   #endif
   expr4[-2] = (char *) doublePptr ;
   expr4-- ;
}

void e4subDate()
{
   if ( v4functions[expr4infoPtr->functionI].type[0] == r4dateDoub )
   {
      if ( *(double *)expr4[-2] == 0.0 )
      {
         *(double *)expr4-- = 0.0 ;
         return ;
      }
   }

   if ( v4functions[expr4infoPtr->functionI].type[1] == r4dateDoub )
   {
      if ( *(double *)expr4[-1] == 0.0 )
      {
         *(double *)expr4-- = 0.0 ;
         return ;
      }
   }

   e4sub() ;
}

void e4substr()
{
   c4memmove( expr4buf + expr4infoPtr->resultPos, expr4buf + expr4infoPtr->resultPos + expr4infoPtr->i1, (unsigned int)expr4infoPtr->len ) ;
}

void e4time()
{
   date4timeNow( *expr4++ = expr4buf + expr4infoPtr->resultPos ) ;
}

void e4trim()
{
   c4trimN( expr4[-1], expr4infoPtr->len + 1 ) ;
}

void expr4trueFunction()
{
   int *ptr ;
   ptr = (int *)(*expr4++ = expr4buf+expr4infoPtr->resultPos) ;
   *ptr = 1 ;
}

void e4upper()
{
   expr4[-1][expr4infoPtr->len] = 0 ;
   c4upper( expr4[-1] ) ;
}

void e4val()
{
   char *ptr ;
   double d ;

   ptr = expr4buf + expr4infoPtr->resultPos ;
   d = c4atod( expr4[-1], expr4infoPtr[-1].len ) ;
   #ifdef E4ANALYZE
      if ( ptr == 0 )
         error4( 0, e4info, E80903 ) ;
   #endif
   memcpy( ptr, (void *)&d, sizeof(d) ) ;
   expr4[-1] = (char *) ptr ;
}

void e4year()
{
   double d ;

   d = (double) date4year( expr4[-1] ) ;
   expr4[-1] = expr4buf + expr4infoPtr->resultPos ;
   *(double *)expr4[-1] = d ;
}

void e4yearDoub()
{
   char datePbuf[8] ;

   date4assign( datePbuf, (long) *(double *)expr4[-1] ) ;
   expr4[-1] = expr4buf + expr4infoPtr->resultPos ;
   *(double *) expr4[-1] = date4year( datePbuf ) ;
}

void e4ascend()
{
   double d ;
   char *ptr ;

   /* modified 09/15/95 AS -- need to always perform ascend in the expression
      buffer in case of concatanations */
   ptr = expr4buf + expr4infoPtr->resultPos ;
   if ( expr4[-1] != ptr )
   {
      memcpy( ptr, expr4[-1], expr4infoPtr->len ) ;
      expr4[-1] = ptr ;
   }

   switch( v4functions[expr4infoPtr->functionI].type[0] )
   {
      case r4num:
         c4clip( expr4[-1], expr4infoPtr->len ) ;
         break ;
      case r4dateDoub:
         d = *(double *)expr4[-1] ;
         date4assign( expr4[-1], (long)d ) ;
         break ;
      case r4log:
         if( *(int *) expr4[-1] )
            *expr4[-1] = '1' ;
         else
            *expr4[-1] = '0' ;
         return ;
      case r4numDoub:
         t4dblToFox( expr4[-1], *((double *)expr4[-1]) ) ;   /* converts a double to an ordered byte */
         break ;
      case r4int:
         t4intToFox( expr4[-1], ((long *)expr4[-1]) ) ;
         break ;
      case r4currency:
         t4curToFox( expr4[-1], ((CURRENCY4 *)expr4[-1]) ) ;
         break ;
      case r4dateTime:
         t4dateTimeToFox( expr4[-1], ((long *)expr4[-1]) ) ;
         return ;
      default:  /* r4str, r4date */
         break ;
   }
}

#ifdef S4CLIPPER
void e4descendBinary()
{
   e4ascend() ;
   c4descendBinary( expr4[-1], expr4[-1], expr4infoPtr->len ) ;
}
#endif

void e4descend()
{
   e4ascend() ;
   c4descend( expr4[-1], expr4[-1], expr4infoPtr->len ) ;
}

void e4calcFunction()
{
   EXPR4CALC *e4calcPtr = (EXPR4CALC *) expr4infoPtr->p1 ;
   char **e4save = expr4 ;
   char *expr4constantsSave = expr4constants ;
   char *resultPtr ;

   expr4calcResultPos( e4calcPtr, expr4infoPtr->resultPos ) ;
   expr4vary( e4calcPtr->expr, &resultPtr ) ;
   expr4start( e4calcPtr->expr ) ;  /* restart from vary */

   expr4 = e4save ;
   expr4constants = expr4constantsSave ;
   *expr4++ = resultPtr ;
}

#ifndef S4OFF_REPORT
void e4pageno()
{
   double d = (double) expr4ptr->codeBase->pageno ;
   memcpy( *expr4++ = expr4buf + expr4infoPtr->resultPos, (void *)&d, sizeof(d) ) ;
}

void e4calcTotal()
{
   double d = total4value( (struct TOTAL4st *) expr4infoPtr->p1 ) ;
   *expr4 = expr4buf + expr4infoPtr->resultPos ;
   memcpy( *expr4++, (void *)&d, sizeof(d) ) ;
}

double S4FUNCTION total4value( TOTAL4 *t4 )
{
   switch( t4->totalType )
   {
      case total4sum:
         return t4->total ;
      case total4count:
         return (double) t4->count ;
      case total4average:
         if( t4->count == 0 )
            return 0.0 ;
         return t4->total/t4->count ;
      case total4highest:
         return t4->high ;
      case total4lowest:
         return t4->low ;
      default:
         break ;
   }

   return t4->total ;
}
#endif
