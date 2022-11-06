/* e4parse.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

/* Restrictions - STR can only have a constant 2nd & 3rd parameters
                  SUBSTR can only have a constant 2nd & 3rd parameters
                  LEFT can only have a constant 2nd parameter
                  RIGHT can only have a constant 2nd paramater, and the
                    first paramater must be a field or constant only.
                  IIF must return a predictable length and type
                  TRIM, LTRIM & ALLTRIM return unpredictable lengths.
                       Its result
                       Ex. TRIM(L_NAME) + TRIM(F_NAME) is OK
                       SUBSTR( TRIM(L_NAME), 3, 2 ) is not OK
                  Memo field's evaluate to a maximum length.  Anything over
                  this maximum gets truncated.

   Known inconsistencies -
                  DTOC( x, 1 ) where x is a date value is supposed to operate
                    the same as DTOS( x ).  In actual fact, you can obtain the
                    same effect by using anything as the 2nd paramater (i.e.
                    doesn't have to be a '1').
                  The contain ($) operator does not work correctly when
                    applied with a TRIM().  eg: "TRIM( 'BOB ' ) $ 'BOBJ'"
                    returns failure.
                  ALLTRIM() and TRIM() functions do not always work as
                    expected in combination with the comparison functions.
                    For example:
                       "ALLTRIM( ' george ' ) > 'george' ) returns TRUE
                       instead of FALSE (i.e. they should be equal).
                  Fox 3.0 compatible files do not work with the new field
                    types if an index file includes a table name qualifier
                    (eg. "data->intField" where data is the data file
                     being indexed on.)  To get around this, do not use
                     the qualifer in index files (eg. index on "intField".)
                    Since CodeBase disallows other data-file qualifiers within
                    index files, this is not a major limitation.
                  String comparison of varying length appears to work different
                    than FoxPro.  For example, expr "AAAA" = "AAA" and expr
                    "AAA" = "AAAA" both return 'TRUE' in CodeBase, but only
                    the 2nd returns 'TRUE' in FoxPro.
*/

#include "d4all.h"

#include <ctype.h>

static void e4functionPop( EXPR4 * ) ;

/* e4massage
   -  Check the type returns to ensure that functions get the correct type
      result.  Use 'E4FUNCTIONS.code' to change the function where possible
      so that the correct function is used.
   -  Make sure that field parameters are put on the stack for the concatentate
      operators.
   -  Fill in the function pointers.
   -  Change (FIELD4 *) pointers in 'p1' to (char *) pointers.
   -  Where the result stack is used, make sure the correct values are filled
      into the E4INFO entires in order to adjust for the lengths needed.
   -  Check the length returns to make sure that 'codeBase->exprBufLen' is large enough
      to handle executing the expression.
   -  Calculate the length of the final result.
   -  Enforce restrictions to TRIM, STR and IIF
   -  Make sure an extra max. length character is added for e4upper() & e4trim()
*/

static int e4massage( E4PARSE *p4 )
{
   E4INFO *info ;
   int parmPos, iParm, isOk, codeOn, iInfo, numParms ;
   int typeShouldBe, len, lengthStatus, i, doneTrimMemoOrCalc ;
   int position[E4MAX_STACK_ENTRIES+1] ;
   long length[E4MAX_STACK_ENTRIES] ;
   long bufLenNeeded ;
   int types[E4MAX_STACK_ENTRIES] ;
   int numEntries[E4MAX_STACK_ENTRIES] ;
   E4INFO *pointers[E4MAX_STACK_ENTRIES] ;
   CODE4 *codeBase ;
   unsigned storedKeyLen ;
   #ifdef S4DATA_ALIGN
      int delta ;
      int rem ;
   #endif

   E4PARMLOW( p4, E90901 ) ;

   #ifdef E4MISC
      memset( types, 0, sizeof( types ) ) ;
      memset( pointers, 0, sizeof( pointers ) ) ;
      memset( length, 0, sizeof( length ) ) ;
      memset( numEntries, 0, sizeof( numEntries ) ) ;
      memset( position, 0, sizeof( position ) ) ;
   #endif

   codeBase = p4->codeBase ;
   numParms = doneTrimMemoOrCalc = 0 ;
   bufLenNeeded = 0 ;

   position[0] = 0 ; /* The first parameter can be placed at position 0 */

   for( iInfo = 0; iInfo < p4->expr.infoN; iInfo++ )
   {
      info = p4->expr.info + iInfo ;

      /* Check the parameters types */
      codeOn = v4functions[info->functionI].code ;
      if ( v4functions[info->functionI].numParms != (char)info->numParms )
         if ( v4functions[info->functionI].numParms > 0 )
         {
            if( codeBase->errExpr )
               return error4describe( codeBase, e4numParms, E90901, p4->expr.source, 0, 0 ) ;
            return e4numParms ;
         }

      for( ;; )
      {
         if ( codeOn != v4functions[info->functionI].code )
         {
            if( codeBase->errExpr )
               return error4describe( codeBase, e4typeSub, E90901, p4->expr.source, 0, 0 ) ;
            return e4typeSub ;
         }

         isOk = 1 ;

         for( iParm = 0; iParm < info->numParms; iParm++ )
         {
            if ( (int)v4functions[info->functionI].numParms < 0 )
               typeShouldBe = v4functions[info->functionI].type[0] ;
            else
               typeShouldBe = v4functions[info->functionI].type[iParm] ;

            parmPos = numParms - info->numParms + iParm ;

            if ( types[parmPos] != typeShouldBe )
            {
               if ( types[parmPos] == r4date && typeShouldBe == r4dateDoub )
               {
                  pointers[parmPos]->functionI = E4FIELD_DATE_D ;
                  length[parmPos] = sizeof(double) ;
                  continue ;
               }
               if ( types[parmPos] == r4num && typeShouldBe == r4numDoub )
               {
                  pointers[parmPos]->functionI = E4FIELD_NUM_D ;
                  length[parmPos] = sizeof(double) ;
                  continue ;
               }
               if ( types[parmPos] == r4int && typeShouldBe == r4numDoub )
               {
                  pointers[parmPos]->functionI = E4FIELD_INT_D ;
                  length[parmPos] = sizeof(double) ;
                  continue ;
               }
               if ( types[parmPos] == r4currency && typeShouldBe == r4numDoub )
               {
                  pointers[parmPos]->functionI = E4FIELD_CUR_D ;
                  length[parmPos] = sizeof(double) ;
                  continue ;
               }
               info->functionI++ ;
               isOk = 0 ;
               break ;
            }
         }
         if ( isOk )
            break ;
      }

      switch( info->functionI )
      {
         case E4CONCATENATE:
         case E4CONCATENATE+1:
         case E4CONCAT_TWO:
         case E4TRIM:
         case E4LTRIM:
         case E4ALLTRIM:
         case E4UPPER:
         case E4SUBSTR:
         case E4LEFT:
         case E4RIGHT:
         case E4DESCEND:
         case E4DESCEND+1:
         case E4DESCEND+2:
         case E4DESCEND+3:
         case E4DESCEND+4:
         case E4DESCEND+5:
         case E4DESCEND+6:
         case E4DESCEND+7:
         case E4DESCEND+8:
         case E4ASCEND:
         case E4ASCEND+1:
         case E4ASCEND+2:
         case E4ASCEND+3:
         case E4ASCEND+4:
         case E4ASCEND+5:
         case E4ASCEND+6:
         case E4ASCEND+7:
         case E4ASCEND+8:
         case E4ASCEND+9:
         #ifndef S4MEMO_OFF
            case E4FIELD_MEMO:
         #endif
            for( iParm = 1; iParm <= info->numParms; iParm++ )
            {
               E4INFO *info_parm = pointers[numParms-iParm] ;
               if ( info_parm->functionI == E4FIELD_STR )  /* Make sure the parameter is put on the stack. */
                  info_parm->functionI = E4FIELD_STR_CAT ;
               if ( info_parm->functionI == E4FIELD_WSTR )   /* Make sure the parameter is put on the stack. */
                  info_parm->functionI = E4FIELD_STR_CAT + 1 ;
               if ( info->functionI == E4CONCATENATE  &&  doneTrimMemoOrCalc )
                  info->functionI = E4CONCAT_TRIM ;
            }
            break ;
         default:
            break ;
      }

      numParms -= info->numParms ;
      if ( numParms < 0 )
         if( codeBase->errExpr )
            return error4( codeBase, e4result, E90901 ) ;

      types[numParms] = v4functions[info->functionI].returnType ;

      if ( info->functionI == E4CALC_FUNCTION )
         types[numParms] = expr4type( ((EXPR4CALC *) info->p1)->expr ) ;
      switch( types[numParms] )
      {
         case r5wstr:
            switch( info->functionI )
            {
               case E4FIELD_WSTR:
                  length[numParms] = f4len( info->fieldPtr ) ;
                  break ;
               case E4CONCATENATE+1:
                  info->i1 = (int) (length[numParms]) ;
                  length[numParms] += length[numParms+1] ;
                  break ;
               default:
                  return error4( codeBase, e4result, E90901 ) ;
            }
            break ;
         case r4str:
            switch( info->functionI )
            {
               case E4FIELD_STR:
               case E4FIELD_STR_CAT:
                  length[numParms] = f4len( info->fieldPtr ) ;
                  break ;
               #ifndef S4MEMO_OFF
                  case E4FIELD_MEMO:
                     if ( p4->expr.tagPtr )  /* max size is key size */
                     {
                        /* for - ole-db memos never there, ensure limited
                           to size of supporeted key size */
                        #ifdef S4FOX
                           /* for fox, extra byte for potention null field */
                           length[numParms] = I4MAX_KEY_SIZE_COMPATIBLE - info->fieldPtr->null ;
                        #else
                           length[numParms] = I4MAX_KEY_SIZE_COMPATIBLE ;
                        #endif
                     }
                     else
                        length[numParms] = codeBase->memSizeMemoExpr ;
                     doneTrimMemoOrCalc = 1 ;
                     break ;
               #endif  /* S4MEMO_OFF */
               case E4CONCATENATE:
               case E4CONCAT_TWO:
               case E4CONCAT_TRIM:
                  info->i1 = (int) (length[numParms]) ;
                  length[numParms] += length[numParms+1] ;
                  break ;
               case E4IIF:
                  if ( length[numParms+1] != length[numParms+2] )
                     if( codeBase->errExpr )
                        return error4describe( codeBase, e4lengthErr, E90901, p4->expr.source, 0, 0 ) ;
                  length[numParms] = length[numParms+1] ;
                  break ;
               case E4DTOS:
               case E4DTOS+1:
                  length[numParms] = sizeof(double) ;
                  break ;
               case E4DTOC:
               case E4DTOC+1:
               case E4CTOD:
                  length[numParms] = sizeof(double) ;
                  info->i1 = p4->constants.pos ;
                  len = strlen( code4dateFormat( codeBase ) ) ;
                  s4stackPushStr( &p4->constants, code4dateFormat( codeBase ), len + 1 ) ;
                  if ( info->functionI == E4DTOC || info->functionI == E4DTOC+1 )
                     length[numParms] = len ;
                  break ;
               case E4CHR:
                  length[numParms] = sizeof(char) ;
                  break ;
               case E4DEL:
                  length[numParms] = sizeof(char) ;
                  if ( p4->expr.tagPtr )  /* data4 independent, so point to datafile */
                     info->p1 = (char *)&p4->expr.dataFile->record ;
                  else  /* data4 dependent, so just point to record */
                     info->p1 = (char *)&p4->expr.data->record ;
                  break ;
               case E4CALC_FUNCTION:
                  doneTrimMemoOrCalc = 1 ;
                  length[numParms] = expr4len( ((EXPR4CALC *) info->p1)->expr ) ;
                  break ;
               case E4RIGHT:
                  if ( info->i1 > (int)(length[numParms]) )
                     info->i1 = (int)(length[numParms]) ;
                  if ( info->i1 < 0 )
                     info->i1 = 0 ;
                  if ( info->len > (int)(length[numParms]) )
                     info->len = (int)(length[numParms]) ;
                  length[numParms] = info->len ;
                  break ;
               case E4SUBSTR:
               case E4LEFT:
                  if ( info->i1 > (int)(length[numParms]) )
                     info->i1 = (int)(length[numParms]) ;
                  if ( info->i1 < 0 )
                     info->i1 = 0 ;
                  length[numParms] -= info->i1 ;
                  if ( info->len > (int)(length[numParms]) )
                     info->len = (int)(length[numParms]) ;
                  length[numParms] = info->len ;
                  break ;
               case E4TIME:
                  length[numParms] = sizeof(double) ;
                  break ;
               case E4TRIM:
               case E4LTRIM:
               case E4ALLTRIM:
                  doneTrimMemoOrCalc = 1 ;
                  p4->expr.hasTrim = 1 ;
                  break ;
               case E4UPPER:
                  break ;
               case E4DESCEND:
               case E4DESCEND+1:
               case E4DESCEND+2:
               case E4DESCEND+3:
               case E4DESCEND+4:
               case E4DESCEND+5:
               case E4DESCEND+6:
               case E4DESCEND+7:
               case E4DESCEND+8:
               case E4ASCEND:
               case E4ASCEND+1:
               case E4ASCEND+2:
               case E4ASCEND+3:
               case E4ASCEND+4:
               case E4ASCEND+5:
               case E4ASCEND+6:
               case E4ASCEND+7:
               case E4ASCEND+8:
               case E4ASCEND+9:
                  /* AS 01/15/98 0 types[numParms] is always return type r4str, we are interested in the base type... */
                  switch( v4functions[info->functionI].type[0] )
                  {
                     case r4dateDoub:
                     case r4numDoub:
                     case r4dateTime:
                        length[numParms] = sizeof(double) ;
                        break ;
                     case r4log:
                        length[numParms] = sizeof(char) ;
                        break ;
                     case r4int:
                        length[numParms] = sizeof(long) ;
                        break ;
                     default:
                        break ;
                  }
                  break ;
               default:
                  length[numParms] = info->len ;
            }
            break ;
         case r4num:
            length[numParms] = f4len( info->fieldPtr ) ;
            break ;
         case r4currency:
            length[numParms] = sizeof(double) ;
            break ;
         case r4dateTime:
            length[numParms] = sizeof(double) ;
            break ;
         case r4int:
         #ifdef S5USE_EXTENDED_TYPES
            case r5ui4:
         #endif
            length[numParms] = sizeof(S4LONG) ;
            break ;
         #ifdef S5USE_EXTENDED_TYPES
            case r5i2:
            case r5ui2:
               length[numParms] = sizeof(short) ;
               break ;
         #endif
         case r4numDoub:
         case r4dateDoub:
            length[numParms] = sizeof(double) ;
            if ( info->functionI == E4CTOD )
            {
               info->i1 = p4->constants.pos ;
               s4stackPushStr( &p4->constants, code4dateFormat( codeBase ), strlen( code4dateFormat( codeBase ) ) + 1 ) ;
            }
            if ( info->functionI == E4RECCOUNT )
               info->p1 = (char *)p4->expr.dataFile ;
            if ( info->functionI == E4RECNO )
               info->p1 = 0 ;  /* updated for c/s support, just use expr data4 */
            break ;
         case r4date:
            length[numParms] = sizeof(double) ;
            break ;
         case r4log:
            if ( info->functionI != E4FIELD_LOG )
            {
               if ( info->functionI == E4DELETED )
                     if ( p4->expr.tagPtr )  /* data4 independent, so point to datafile */
                        info->p1 = (char *)&p4->expr.dataFile->record ;
                     else  /* data4 dependent, so just point to record */
                     info->p1 = (char *)&p4->expr.data->record ;
               else
               {
                  info->i1 = (int)(length[numParms+1]) ;
                  lengthStatus = 1 ;
                  if ( length[numParms] < length[numParms+1] )
                  {
                     info->i1 = (int)(length[numParms]) ;
                     lengthStatus = -1 ;
                  }
                  if ( length[numParms] == length[numParms+1] )
                     lengthStatus = 0 ;

                  if ( info->functionI == E4GREATER )
                  {
                     if ( lengthStatus > 0 )
                        info->p1 = (char *)1L ;
                     else
                        info->p1 = (char *)0L ;
                  }
                  if ( info->functionI == E4LESS )
                  {
                     if ( lengthStatus < 0 )
                        info->p1 = (char *)1L ;
                     else
                        info->p1 = (char *)0L ;
                  }
               }
            }
            length[numParms] = sizeof(int) ;
            break ;
         default:
            if ( codeBase->errExpr )
               return error4( codeBase, e4result, E90901 ) ;
            else
               return e4result ;
      }

      /* make sure there is enough key space allocated for the type,
         in case a partial evaluation occurs */
         switch( types[numParms] )
         {
            #ifdef S4FOX
               case r4num:
               case r4date:
               case r4numDoub:
                  storedKeyLen = sizeof( double ) ;
                  break ;
            #endif  /*  ifdef S4FOX      */
            #ifdef S4CLIPPER
               case r4num:  /* numeric field return, must fix length problem */
                  storedKeyLen = f4len( info->fieldPtr ) ;
                  break ;
               case r4numDoub:
                  storedKeyLen = codeBase->numericStrLen ;
                  break ;
            #endif  /*  ifdef S4CLIPPER  */
            #ifdef S4NDX
               case r4num:
               case r4date:
                  storedKeyLen = sizeof( double ) ;
                  break ;
            #endif  /*  ifdef S4NDX  */
            #ifdef S4MDX
               case r4num:
                  storedKeyLen = (int)sizeof( C4BCD ) ;
                  break ;
               case r4numDoub:
                  storedKeyLen = (int)sizeof( C4BCD ) ;
                  break ;
               case r4date:
               case r4dateDoub:
                  storedKeyLen = sizeof( double ) ;
                  break ;
            #endif  /* S4MDX */
            default:
               storedKeyLen = (unsigned)(length[numParms]) ;
         }

         #ifdef S4FOX
            storedKeyLen++ ;    /* null entry will increase length by one */
         #endif

      u4allocAgain( codeBase, &codeBase->storedKey, &codeBase->storedKeyLen, (unsigned)storedKeyLen + 1 ) ;

      if ( error4code( codeBase ) < 0 )
         return -1 ;
      #ifdef S4DATA_ALIGN
         delta = 0 ;
         switch(types[numParms])
         {
            case r4numDoub:
            case r4dateDoub:
            case r4num:
            case r4date:
            {
               if (rem=position[numParms]%sizeof(double))
                  delta=sizeof(double)-rem;
               break ;
            }
            case r4log:
            {
               if (rem=position[numParms]%sizeof(int))
                  delta=sizeof(double)-rem;
               break ;
            }
            case r4int:
            {
               if (rem=position[numParms]%sizeof(S4LONG))
                  delta=sizeof(double)-rem;
               break ;
            }
          }
         info->resultPos=position[numParms]+delta;
      #else
         info->resultPos = position[numParms] ;
      #endif

      bufLenNeeded = length[numParms] ;
      if ( info->functionI == E4CALC_FUNCTION )
         bufLenNeeded = ((EXPR4CALC *)info->p1)->expr->lenEval ;
/*         bufLenNeeded = ((EXPR4 *)info->p1)->lenEval ; */
      if( bufLenNeeded > INT_MAX )
         return error4( codeBase, e4overflow, E90901 ) ;

      if( (types[numParms] == r4num || types[numParms] == r4date || types[numParms] == r4int) && length[numParms] < sizeof(double) )
         position[numParms+1] = info->resultPos + sizeof(double) ;
      else
         position[numParms+1] = info->resultPos + (unsigned)length[numParms] ;

      if ( info->resultPos + bufLenNeeded > (long)p4->expr.lenEval )
         p4->expr.lenEval = info->resultPos + (unsigned)bufLenNeeded ;

      info->len = (int)(length[numParms]) ;
      info->numEntries = 1 ;

      for( i = 0; i < info->numParms; i++ )
         info->numEntries += numEntries[numParms+i] ;

      numEntries[numParms] = info->numEntries ;
      pointers[numParms] = info ;

      numParms++ ;
      if ( numParms >= E4MAX_STACK_ENTRIES )
         if( codeBase->errExpr )
            return error4( codeBase, e4overflow, E90901 ) ;
   }

   if ( numParms != 1 )
   {
      if( codeBase->errExpr )
         error4( codeBase, e4result, E90901 ) ;
      return -1 ;
   }

   for( i = 0; i < p4->expr.infoN; i++ )
   {
      info = p4->expr.info + i ;
      info->function = (S4OPERATOR *)v4functions[info->functionI].functionPtr ;
   }

   p4->expr.lenEval += 1 ;
   if ( codeBase->exprBufLen < (unsigned)p4->expr.lenEval )
      if ( u4allocAgain( codeBase, &codeBase->exprWorkBuf, &codeBase->exprBufLen, p4->expr.lenEval ) == e4memory )
         return error4stack( codeBase, e4memory, E90901 ) ;

   p4->expr.len = (int)(length[0]) ;
   p4->expr.type = types[0] ;
   return 0 ;
}

int e4addConstant( E4PARSE *p4, const int iFunctions, const void *consPtr, const unsigned consLen )
{
   E4INFO *info ;

   E4PARMLOW( p4, E90902 ) ;

   info = e4functionAdd( &p4->expr, iFunctions ) ;
   if ( info == 0 )
      return -1 ;
   info->i1 = p4->constants.pos ;
   info->len = consLen ;
   return s4stackPushStr( &p4->constants, consPtr, (int)consLen ) ;
}

E4INFO *e4functionAdd( EXPR4 *expr, const int iFunctions )
{
   E4INFO *info ;

   E4PARMLOW( expr, E90903 ) ;

   if ( (unsigned)((expr->infoN+1)*sizeof(E4INFO)) > expr->codeBase->exprBufLen )
      if ( u4allocAgain( expr->codeBase, &expr->codeBase->exprWorkBuf, &expr->codeBase->exprBufLen, sizeof(E4INFO) * (expr->infoN+10) ) == e4memory )
         return 0 ;

   info = (E4INFO *)expr->codeBase->exprWorkBuf + expr->infoN++ ;

   info->functionI = iFunctions ;
   info->numParms = v4functions[iFunctions].numParms ;
   if ( info->numParms < 0 )
      info->numParms = 2 ;
   info->function = (S4OPERATOR *)v4functions[iFunctions].functionPtr ;
   return info ;
}

static void e4functionPop( EXPR4 *expr )
{
   expr->infoN-- ;
}

EXPR4 *S4FUNCTION expr4parseLow( DATA4 *d4, const char *exprPtr, TAG4FILE *tagPtr )
{
   E4PARSE parse ;
   char ops[128] ;
   char constants[512] ;
   char *src ;
   int rc, infoLen, posConstants ;
   EXPR4 *express4 ;

   #ifdef E4PARM_HIGH
      if ( d4 == 0 || exprPtr == 0 )
      {
         error4( 0, e4parm_null, E90904 ) ;
         return 0 ;
      }
   #endif

   if ( error4code( d4->codeBase ) < 0 )
      return 0 ;

   if ( d4->codeBase->exprBufLen > 0 )
      memset( d4->codeBase->exprWorkBuf, 0, d4->codeBase->exprBufLen ) ;

   memset( (void *)&parse, 0, sizeof(E4PARSE) ) ;
   memset( ops, 0, sizeof(ops));

   parse.expr.tagPtr = tagPtr ;
   #ifdef S4FOX
      parse.expr.vfpInfo = tagPtr ? &tagPtr->vfpInfo : 0 ;
   #endif
   parse.expr.data   = d4 ;
   parse.expr.source = (char *)exprPtr ;
   parse.codeBase   = d4->codeBase ;
   parse.expr.codeBase = d4->codeBase ;

   parse.op.ptr = ops ;
   parse.op.len = sizeof(ops) ;
   parse.op.codeBase = d4->codeBase ;

   parse.constants.ptr = constants ;
   parse.constants.len = sizeof(constants) ;
   parse.constants.codeBase = d4->codeBase ;

   s4scanInit( &parse.scan, (unsigned char *)exprPtr ) ;

   rc = expr4parseExpr( &parse ) ;
   if ( rc < 0 )
      return 0 ;

   if ( s4stackCur( &parse.op ) != E4NO_FUNCTION )
   {
      if( parse.codeBase->errExpr )
         error4describe( parse.codeBase, e4complete, E90904, exprPtr, 0, 0 ) ;
      return 0 ;
   }

   parse.expr.info = (E4INFO *)parse.codeBase->exprWorkBuf ;
   parse.expr.dataFile = d4->dataFile ;
   if ( e4massage( &parse ) < 0 )
      return 0 ;

   infoLen = parse.expr.infoN * sizeof(E4INFO) ;
   posConstants = sizeof(EXPR4) + infoLen ;

   express4 = (EXPR4 *)u4allocFree( d4->codeBase, (long)posConstants + parse.constants.len + parse.scan.len + 1L ) ;
   if ( express4 == 0 )
   {
      if( d4->codeBase->errExpr )
         error4( d4->codeBase, e4memory, E90904 ) ;
      return 0 ;
   }

   memcpy( (void *)express4, (void *)&parse.expr, sizeof(EXPR4) ) ;

   express4->data = d4 ;
   express4->dataFile = d4->dataFile ;
   express4->info = (E4INFO *)( express4 + 1 ) ;
   express4->constants = (char *) express4 + posConstants ;
   src = express4->constants + parse.constants.len ;
   express4->source = src ;

   memcpy( (void *)express4->info, parse.codeBase->exprWorkBuf, (unsigned int)infoLen ) ;
   memcpy( express4->constants, constants, parse.constants.len ) ;
   strcpy( src, exprPtr ) ;

   #ifdef S4CLIPPER
      express4->keyLen = parse.expr.keyLen ;
      express4->keyDec = parse.expr.keyDec ;
   #endif

   return express4 ;
}

/*EXPR4 *S4FUNCTION expr4parse( DATA4 *d4, char *exprPtr )*/
/*{*/
/*   return expr4parseLow( d4, exprPtr, 0 ) ;*/
/*}*/

/*    Looks at the input string and returns and puts a character code on the
   result stack corresponding to the next operator.  The operators all operate
   on two operands.  Ex. +,-,*,/, >=, <, .AND., ...

      If the operator is ambiguous, return the arithmatic choice.

   Returns -2 (Done), 0, -1 (Error)
*/

int e4getOperator( E4PARSE *p4, int *opReturn)
{
   char ch ;
   int op ;

   #ifdef E4PARM_LOW
      if ( p4 == 0 || opReturn == 0 )
         return error4( 0, e4parm_null, E90505 ) ;
   #endif

   s4scanRange( &p4->scan, 1, ' ' ) ;
   ch = s4scanChar(&p4->scan) ;
   if ( ch==0 || ch==')' || ch==',' )
   {
      *opReturn = E4DONE ;
      return 0 ;
   }

   op  = e4lookup( p4->scan.ptr+p4->scan.pos, -1, E4FIRST_OPERATOR, E4LAST_OPERATOR ) ;
   if ( op < 0 )
      if( p4->codeBase->errExpr )
         return error4describe( p4->codeBase, e4unrecOperator, E90905, (char *)p4->scan.ptr, 0, 0 ) ;

   p4->scan.pos += v4functions[op].nameLen ;
   *opReturn = op ;

   return 0 ;
}

/* e4lookup, searches 'v4functions' for an operator or function.

       str - the function name
       strLen - If 'strLen' is greater than or equal to zero it contains the
                 exact number of characters in 'str' to lookup.  Otherwise,
                 as many as needed, provided an ending null is not reached,
                 are compared.
*/

int S4FUNCTION e4lookup( const unsigned char *str, const int strLen, const int startI, const int endI )
{
   char uStr[9] ;  /* Maximum # of function name characters plus one. */
   int i ;

   #ifdef E4PARM_LOW
      if ( str == 0 || endI < startI )
         return error4( 0, e4parm, E90906 ) ;
   #endif

   u4ncpy( uStr, (char *)str, sizeof( uStr ) ) ;
   c4upper( uStr ) ;

   for( i=startI; i<= endI; i++)
   {
      if ( v4functions[i].code < 0 )
         break ;
      if ( v4functions[i].name == 0 )
         continue ;
      #ifdef E4ANALYZE
         if ( v4functions[i].nameLen >= (char)sizeof( uStr ) )
            return error4( 0, e4result, E90906 ) ;
      #endif

      if ( v4functions[i].name[0] == uStr[0] )
         if( strLen == v4functions[i].nameLen || strLen < 0 )
            if ( strncmp( uStr, v4functions[i].name, (size_t)v4functions[i].nameLen ) == 0 )
               return i ;
   }
   return -1 ;
}

static int opToExpr( E4PARSE *p4 )
{
   E4INFO *info ;

   E4PARMLOW( p4, E90907 ) ;

   info = e4functionAdd( &p4->expr, s4stackPop(&p4->op) ) ;
   if ( info == 0 )
      return -1 ;

   for( ; s4stackCur(&p4->op) == E4ANOTHER_PARM ; )
   {
      s4stackPop(&p4->op) ;
      info->numParms++ ;
   }

   return 0 ;
}

/*
     Parses an expression consisting of value [[operator value] ...]
   The expression is ended by a ')', a ',' or a '\0'.
   Operators are only popped until a '(', a ',' or the start of the stack.
   Left to right evaluation for operators of equal priority.

      An ambiguous operator is one which can be interpreted differently
   depending on its operands.  However, its operands depend on the
   priority of the operators and the evaluation order. Fortunately, the
   priority of an ambigous operator is constant regardless of its
   interpretation.  Consequently, the evaluation order is determined first.
   Then ambiguous operators can be exactly determined.

   Ambigous operators:+, -,  >, <, <=, >=, =, <>, #

   Return

       0  Normal
      -1  Error
*/

int expr4parseExpr( E4PARSE *p4 )
{
   int rc, opValue, opOnStack ;

   E4PARMLOW( p4, E90908 ) ;

   rc = expr4parseValue( p4 ) ;
   if ( rc < 0 )
      return error4stack( p4->codeBase, (short)rc, E90908 ) ;

   for( ;; )
   {
      rc = e4getOperator( p4, &opValue ) ;
      if ( rc < 0 )
         return error4stack( p4->codeBase, (short)rc, E90908 ) ;
      if ( opValue == E4DONE )  /* Done */
      {
         while( s4stackCur(&p4->op) != E4L_BRACKET && s4stackCur(&p4->op) != E4COMMA
                && s4stackCur(&p4->op) != E4NO_FUNCTION )
         {
            rc = opToExpr( p4 ) ;
            if ( rc < 0 )
               return error4stack( p4->codeBase, (short)rc, E90908 ) ;
         }
         return 0 ;
      }

      /* Everything with a higher or equal priority than 'opValue' must be
         executed first. (equal because of left to right evaluation order)
         Consequently, all high priority operators are sent to the result
         stack.
      */
      while ( s4stackCur( &p4->op ) >= 0 )
      {
         opOnStack = s4stackCur(&p4->op ) ;
         if ( v4functions[opValue].priority <= v4functions[opOnStack].priority )
         {
            if ( opValue == opOnStack && (int)v4functions[opValue].numParms < 0 )
            {
               /* If repeated AND or OR operator, combine them into one with an
                  extra paramter.  This makes the relate module optimization
                  algorithms easier. */
               s4stackPop( &p4->op ) ;
               s4stackPushInt( &p4->op, E4ANOTHER_PARM ) ;
               break ;
            }
            else
            {
               rc = opToExpr( p4 ) ;
               if ( rc < 0 )
                  return error4stack( p4->codeBase, (short)rc, E90908 ) ;
            }
         }
         else
            break ;
      }

      s4stackPushInt( &p4->op, opValue ) ;

      rc = expr4parseValue( p4 ) ;
      if ( rc < 0 )
         return error4stack( p4->codeBase, (short)rc, E90908 ) ;
   }
}

int expr4parseFunction( E4PARSE *p4, const char *startPtr, const int fLen )
{
   int fNum, numParms, infoI1, infoLen, rc, rVal ;
   char ch ;
   #ifdef S4DATA_ALIGN
      double doubVal ;
   #endif
   E4INFO *info, *rInfo ;
   void *newOrTotalPtr = 0 ;
   EXPR4CALC *calc ;
   short isWide ;

   #ifdef E4PARM_LOW
      if ( p4 == 0 || startPtr == 0 || fLen < 0 )
         return error4( 0, e4parm, E90909 ) ;
   #endif

   infoI1 = infoLen = 0 ;

   if ( error4code( p4->codeBase ) < 0 )
      return e4codeBase ;

   fNum = e4lookup( (unsigned char *)startPtr, fLen, E4FIRST_FUNCTION, 0x7FFF) ;
   if( fNum < 0 )
   {
      newOrTotalPtr = calc = expr4calcLookup( p4->codeBase, startPtr, fLen ) ;
      if( calc == 0 )
      {
         if( p4->codeBase->errExpr )
            return error4describe( p4->codeBase, e4unrecFunction, E90909, (char *)p4->scan.ptr, 0, 0 ) ;
         return e4unrecFunction ;
      }
      else
      {
         fNum = E4CALC_FUNCTION ;
            if( calc->total != 0 )
            {
               fNum = E4TOTAL ;
               newOrTotalPtr = calc->total ;
            }
      }
   }

   s4stackPushInt( &p4->op, E4L_BRACKET ) ;
   p4->scan.pos++ ;

   numParms = 0 ;
   for( ;; )
   {
      ch = s4scanChar( &p4->scan ) ;
      if ( ch == 0 )
      {
         if( p4->codeBase->errExpr )
            return error4describe( p4->codeBase, e4rightMissing, E90909, (char *)p4->scan.ptr, 0, 0 ) ;
         return e4rightMissing ;
      }
      if ( ch == ')')
      {
         p4->scan.pos++ ;
         break ;
      }

      rc = expr4parseExpr( p4 ) ;
      if ( rc < 0 )
         return error4stack( p4->codeBase, (short)rc, E90909 ) ;
      numParms++ ;

      while( s4scanChar( &p4->scan ) <= ' ' && s4scanChar( &p4->scan ) >='\1')
         p4->scan.pos++ ;

      if ( s4scanChar( &p4->scan ) == ')')
      {
         p4->scan.pos++ ;
         break ;
      }
      if ( s4scanChar( &p4->scan ) != ',')
      {
         if( p4->codeBase->errExpr )
            return error4describe( p4->codeBase, e4commaExpected, E90909, (char *)p4->scan.ptr, 0, 0 ) ;
         return e4commaExpected ;
      }
      p4->scan.pos++ ;
   }

   s4stackPop( &p4->op ) ;  /* pop the left bracket */

   if ( fNum == E4STR )
   {
      infoLen= 10 ;
      isWide = 0 ;
      info = (E4INFO *) p4->codeBase->exprWorkBuf + p4->expr.infoN -1 ;

      if ( info->fieldPtr != NULL )
         if ( f4type ( info->fieldPtr ) == r5wstr ) /* WIDE STRING */
            isWide = 1 ;

      if ( isWide ) /* wide string to character */
      {
         infoLen = f4len( info->fieldPtr ) ;
/*         fNum++ ; */
      }
      else
      {
         if ( numParms == 3  )
         {
            if ( info->functionI != E4DOUBLE )
            {
               if( p4->codeBase->errExpr )
                  return error4describe( p4->codeBase, e4notConstant, E90909, p4->expr.source, 0, 0 ) ;
               return e4notConstant ;
            }
            #ifdef S4DATA_ALIGN
               memcpy( (void *)&doubVal, (p4->constants.ptr + info->i1), sizeof(double) ) ;
               infoI1 = (int) doubVal ;
            #else
               infoI1 = (int) *(double *) (p4->constants.ptr + info->i1) ;
            #endif
            e4functionPop( &p4->expr ) ;
            numParms-- ;
         }
         if ( numParms == 2  )
         {
            info = (E4INFO *) p4->codeBase->exprWorkBuf + p4->expr.infoN -1 ; /* may have changed due to nParms == 3 ) */
            if ( info->functionI != E4DOUBLE )
            {
               if( p4->codeBase->errExpr )
                  return error4describe( p4->codeBase, e4notConstant, E90909, p4->expr.source, 0, 0 ) ;
               return e4notConstant ;
            }
            #ifdef S4DATA_ALIGN
               memcpy( (void *)&doubVal, (p4->constants.ptr + info->i1), sizeof(double) ) ;
               infoLen = (int) doubVal ;
            #else
               infoLen = (int) *(double *) (p4->constants.ptr + info->i1) ;
            #endif

            e4functionPop( &p4->expr ) ;
            numParms-- ;
         }
         if ( infoLen < 0 )
            infoLen = 10 ;
         if ( infoLen <= infoI1+1 )
            infoI1 = infoLen - 2 ;
         if ( infoI1 < 0 )
            infoI1 = 0 ;
      }
   }
   if ( numParms == 2  &&  fNum == E4RIGHT )
   {
      info = (E4INFO *)p4->codeBase->exprWorkBuf + p4->expr.infoN -1 ;
      rInfo = info - 1 ;   /* get the constant/field len */
      if ( info->functionI != E4DOUBLE )
      {
         if( p4->codeBase->errExpr )
            return error4describe( p4->codeBase, e4notConstant, E90909, p4->expr.source, 0, 0 ) ;
         return e4notConstant ;
      }
      #ifdef S4DATA_ALIGN
         memcpy( (void *)&doubVal, (p4->constants.ptr + info->i1), sizeof(double) ) ;
         infoI1 = (int) doubVal ;
      #else
         infoI1 = (int) *(double *) (p4->constants.ptr + info->i1) ;
      #endif
      if ( rInfo->fieldPtr != 0 )   /* is a field */
         rVal = f4len( rInfo->fieldPtr ) - infoI1 ;
      else  /* assume constant */
         rVal = rInfo->len - infoI1 ;
      infoLen = infoI1 ;
      infoI1 = rVal ;
      if ( infoLen < 0 )
        infoLen = 0 ;
      e4functionPop( &p4->expr ) ;
      numParms-- ;
   }
   if ( numParms == 2  &&  fNum == E4SUBSTR )   /* case where no 3rd paramater on substr */
   {
      info = (E4INFO *)p4->codeBase->exprWorkBuf + p4->expr.infoN -1 ;
      rInfo = info - 1 ;   /* get the constant/field len */
      if ( info->functionI != E4DOUBLE )
      {
         if( p4->codeBase->errExpr )
            return error4describe( p4->codeBase, e4notConstant, E90909, p4->expr.source, 0, 0 ) ;
         return e4notConstant ;
      }
      #ifdef S4DATA_ALIGN
         memcpy( (void *)&doubVal, (p4->constants.ptr + info->i1), sizeof(double) ) ;
         infoI1 = (int) doubVal ;
      #else
         infoI1 = (int) *(double *) (p4->constants.ptr + info->i1) ;
      #endif
      infoI1-- ;
      if ( rInfo->fieldPtr != 0 )   /* is a field */
         rVal = f4len( rInfo->fieldPtr ) - infoI1 ;
      else  /* assume constant */
         rVal = rInfo->len - infoI1 ;
      infoLen = rVal ;
      if ( infoLen < 0 )
        infoLen = 0 ;
      e4functionPop( &p4->expr ) ;
      numParms-- ;
   }
   if ( numParms == 3  &&  fNum == E4SUBSTR || numParms == 2  &&  fNum == E4LEFT )
   {
      info = (E4INFO *)p4->codeBase->exprWorkBuf + p4->expr.infoN -1 ;
      if ( info->functionI != E4DOUBLE )
      {
         if( p4->codeBase->errExpr )
            return error4describe( p4->codeBase, e4notConstant, E90909, p4->expr.source, 0, 0 ) ;
         return e4notConstant ;
      }
      #ifdef S4DATA_ALIGN
         memcpy( (void *)&doubVal, (p4->constants.ptr + info->i1), sizeof(double) ) ;
         infoLen = (int) doubVal ;
      #else
         infoLen = (int) *(double *) (p4->constants.ptr + info->i1) ;
      #endif
      e4functionPop( &p4->expr ) ;
      numParms-- ;
   }
   if ( numParms == 2  &&  fNum == E4SUBSTR )
   {
      info = (E4INFO *) p4->codeBase->exprWorkBuf + p4->expr.infoN -1 ;
      if ( info->functionI != E4DOUBLE )
      {
         if( p4->codeBase->errExpr )
            error4describe( p4->codeBase, e4notConstant, E90909, p4->expr.source, 0, 0 ) ;
         return e4notConstant ;
      }
      #ifdef S4DATA_ALIGN
         memcpy( (void *)&doubVal, (p4->constants.ptr + info->i1), sizeof(double) ) ;
         infoI1 = (int) doubVal ;
      #else
         infoI1 = (int) *(double *) (p4->constants.ptr + info->i1) ;
      #endif
      infoI1-- ;
      e4functionPop( &p4->expr ) ;
      numParms-- ;
   }

   if ( error4code( p4->codeBase ) < 0 )
      return -1 ;

   if ( numParms != v4functions[fNum].numParms && (int)v4functions[fNum].numParms >= 0 )
   {
      if( fNum == E4DTOC && numParms == 2 )
      {
         e4functionPop( &p4->expr ) ;
         numParms-- ;
         fNum = E4DTOS ;
      }
      else
      {
         if( p4->codeBase->errExpr )
            return error4describe( p4->codeBase, e4numParms, E80906, v4functions[fNum].name, (char *)p4->scan.ptr, 0 ) ;
         return e4numParms ;
      }
   }

   info = e4functionAdd( &p4->expr, fNum ) ;
   if ( info == 0 )
      return -1 ;

   info->i1  = infoI1 ;
   info->len = infoLen ;

   info->numParms = numParms ;
   if ( fNum == E4CALC_FUNCTION || fNum == E4TOTAL )
      info->p1 = (char *)newOrTotalPtr ;
   return 0 ;
}

int expr4parseValue( E4PARSE *p4 )
{
   FIELD4 *fieldPtr ;
   char ch, searchChar ;
   const unsigned char *startPtr ;
   int  rc, iFunctions, len, iFunction, savePos ;
   double d ;
   E4INFO *expr, *info ;
   DATA4 *basePtr ;
   char bName[11], fName[11] ;
   int tempErrFieldName ;
   CODE4 *c4 ;

   E4PARMLOW( p4, E90910 ) ;

   c4 = p4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   s4scanRange( &p4->scan, ' ', ' ' ) ;

   /* expression */

   if ( s4scanChar( &p4->scan ) == '(')
   {
      p4->scan.pos++ ;

      s4stackPushInt( &p4->op, E4L_BRACKET) ;
      rc = expr4parseExpr( p4 ) ;
      if ( rc < 0 )
         return error4stack( c4, (short)rc, E90910 ) ;

      while ( s4scanChar( &p4->scan ) <= ' ' &&
         s4scanChar( &p4->scan ) != 0)   p4->scan.pos++ ;

      if ( s4scanChar( &p4->scan ) != ')' )
      {
         if( c4->errExpr )
            return error4describe( c4, e4rightMissing, E90910, (char *)p4->scan.ptr, 0, 0 ) ;
         return e4rightMissing ;
      }
      p4->scan.pos++ ;
      s4stackPop( &p4->op ) ;
      return 0 ;
   }

   /* logical */
   if ( s4scanChar( &p4->scan ) == '.' )
   {
      iFunctions = e4lookup( p4->scan.ptr+p4->scan.pos, -1, E4FIRST_LOG, E4LAST_LOG ) ;
      if ( iFunctions >= 0 )
      {
         p4->scan.pos += v4functions[iFunctions].nameLen ;

         if ( strcmp( v4functions[iFunctions].name, ".NOT." ) == 0 )
         {
            rc = expr4parseValue( p4 ) ;   /* One operand operation */
            if ( rc < 0 )
               return error4stack( c4, (short)rc, E90910 ) ;
            s4stackPushInt( &p4->op, iFunctions ) ;
            return 0 ;
         }

         expr = e4functionAdd( &p4->expr, iFunctions ) ;
         if ( expr == 0 )
            return -1 ;
         return 0 ;
      }
   }

   /* string */
   ch = s4scanChar( &p4->scan ) ;
   if ( ch == '\'' || ch == '\"' || ch == '[' )
   {
      if ( ch == '[' )
         searchChar = ']' ;
      else
         searchChar = ch ;

      p4->scan.pos++ ;
      startPtr = p4->scan.ptr + p4->scan.pos ;

      len = s4scanSearch( &p4->scan, searchChar ) ;
      if ( s4scanChar( &p4->scan ) != searchChar )
         if ( len < 0 )
         {
            if( c4->errExpr )
               return error4describe( c4, e4unterminated, E90910, (char *)p4->scan.ptr, 0, 0 ) ;
            return e4unterminated ;
         }
      p4->scan.pos++ ;

      rc = e4addConstant( p4, E4STRING, startPtr, (unsigned int)len ) ;
      if ( rc < 0 )
         return error4stack( c4, (short)rc, E90910 ) ;
      return 0 ;
   }

   /* real */
   ch = s4scanChar( &p4->scan ) ;
   if ( ((ch >='0') && (ch <='9')) || (ch == '-') || (ch == '+') || (ch == '.') )
   {
      startPtr = p4->scan.ptr + p4->scan.pos ;
      savePos = p4->scan.pos ;
      p4->scan.pos++ ;
      len = 1 ;

      while( ((s4scanChar( &p4->scan ) >= '0') && (s4scanChar( &p4->scan ) <= '9')) || (s4scanChar( &p4->scan ) == '.') )
      {
         if ( s4scanChar( &p4->scan ) == '.' )
         {
            if ( strnicmp( (char *)p4->scan.ptr + p4->scan.pos, ".AND.", 5) == 0 ||
                 strnicmp( (char *)p4->scan.ptr + p4->scan.pos, ".OR.", 4) == 0 ||
                 strnicmp( (char *)p4->scan.ptr + p4->scan.pos, ".NOT.", 5) == 0 )
               break ;
            /* if the next value is a character, then we have a database
               with a number as its name/alias.  (i.e. 111.afld), since
               numerics are invalid to being a field name, MUST be a
               number if a numeric after the decimal point... */
            /* AS 03/03/97 fix to numerics, fix #70 in changes.60 */
            p4->scan.pos++ ;
            if ( toupper( s4scanChar( &p4->scan ) ) >= 'A' && toupper( s4scanChar( &p4->scan ) ) <= 'Z' )
               break ;
            else
               p4->scan.pos-- ;   /* retract ++ which was just done to see next character */
         }
         len++ ;
         p4->scan.pos++ ;
      }

      /* check to see if maybe actually a database name starting with a numeric... */
      if ( toupper( s4scanChar( &p4->scan ) ) >= 'A' && toupper( s4scanChar( &p4->scan ) ) <= 'Z' )
         p4->scan.pos = savePos ;
      else
      {
         d = c4atod( (char *)startPtr, len ) ;
         rc = e4addConstant( p4, E4DOUBLE, &d, sizeof(d) ) ;
         if ( rc < 0 )
            return error4stack( c4, (short)rc, E90910 ) ;
         return 0 ;
      }
   }

   /* function or field */
   if ( u4nameChar( s4scanChar( &p4->scan ) ) )
   {
      startPtr = p4->scan.ptr + p4->scan.pos ;

      for( len = 0 ; u4nameChar( s4scanChar( &p4->scan ) ) ; len++ )
         p4->scan.pos++ ;

      s4scanRange( &p4->scan, (char)0, ' ' ) ;

      if ( s4scanChar( &p4->scan ) == '(' )
         return expr4parseFunction( p4, (char *)startPtr, len ) ;

      basePtr = 0 ;

      #ifdef S4FOX
         if ( s4scanChar( &p4->scan ) == '.' )
         {  /* for fox, same as -> */
            if ( len > 10 )
               len = 10 ;
            c4memmove( bName, startPtr, (size_t)len ) ;
            bName[len] = '\0' ;
            #ifndef S4CASE_SEN
               c4upper( bName ) ;
            #endif

            basePtr = tran4dataName( code4trans( c4 ), bName, 0L, 1 ) ;
         }
      #endif

      if ( s4scanChar( &p4->scan ) == '-' )
         if ( p4->scan.ptr[p4->scan.pos+1] == '>')
         {
            if ( len > 10 )
               len = 10 ;
            c4memmove( bName, startPtr, (size_t)len ) ;
            bName[len] = '\0' ;
            #ifndef S4CASE_SEN
               c4upper( bName ) ;
            #endif

            basePtr = tran4dataName( code4trans( c4 ), bName, 0L, 1 ) ;

            if ( basePtr == 0 )
            {
               if( c4->errExpr )
                  return error4describe( c4, e4dataName, E90910, bName, (char *)p4->scan.ptr, (char *) 0 ) ;
               return e4dataName ;
            }
            p4->scan.pos++ ;
         }


      if ( basePtr != 0 )
      {
         if ( p4->expr.tagPtr )  /* data4 independent, so point to datafile */
            if ( basePtr != (DATA4 *)p4->expr.data )   /* allow if it points to itself */
               return error4( c4, e4tagExpr, E80909 ) ;

         p4->scan.pos++ ;

         startPtr = p4->scan.ptr + p4->scan.pos ;
         for( len = 0 ; u4nameChar( s4scanChar( &p4->scan ) ) ; len++ )
            p4->scan.pos++ ;
      }
      else
         basePtr = (DATA4 *)p4->expr.data ;

      if ( len <= 10)
      {
         c4memmove( fName, startPtr, (size_t)len ) ;
         fName[len] = 0 ;
         if( !c4->errExpr )
         {
            tempErrFieldName = c4->errFieldName;
            c4->errFieldName = 0;
            fieldPtr = d4field( basePtr, fName ) ;
            c4->errFieldName = tempErrFieldName;
         }
         else
            fieldPtr = d4field( basePtr, fName ) ;
         if ( fieldPtr == 0 )
         {
            if ( c4->errExpr == 1 && c4->errFieldName == 0 )  /* generate an error since it wasn't by d4field */
               return error4( c4, e4fieldName, E90910 ) ;
            return -1 ;
         }

         #ifdef S4CLIPPER
            p4->expr.keyLen = fieldPtr->len ;
            p4->expr.keyDec = fieldPtr->dec ;
         #endif

         iFunction = 0 ;
         switch( fieldPtr->type )
         {
            case r4num:
            case r4float:
               iFunction = E4FIELD_NUM_S ;
               break ;
            case r5wstr:
               iFunction = E4FIELD_WSTR ;
               break ;
            case r4str:
               iFunction = E4FIELD_STR ;
               break ;
            case r4date:
               iFunction = E4FIELD_DATE_S ;
               break ;
            case r4log:
               iFunction = E4FIELD_LOG ;
               break ;
            case r4memo:
               #ifdef S4MEMO_OFF
                  return error4( c4, e4notMemo, E90910 ) ;
               #else
                  iFunction = E4FIELD_MEMO ;
                  break ;
               #endif
            /* visual Fox 3.0 new field types - also used for ole-db */
            case r4currency:
               iFunction = E4FIELD_CUR ;
               break ;
            case r4dateTime:
               iFunction = E4FIELD_DTTIME ;
               break ;
            case r4double:
               iFunction = E4FIELD_DOUB ;
               break ;
            case r4int:
               iFunction = E4FIELD_INT ;
               break ;
            #ifdef S5USE_EXTENDED_TYPES
               case r5i2:
                  iFunction = E4FIELD_SHORT ;
                  break ;
               case r5ui2:
                  iFunction = E4FIELD_UNS_SHORT ;
                  break ;
               case r5ui4:
                  iFunction = E4FIELD_UNS_INT ;
                  break ;
            #endif
            default:
               if( c4->errExpr )
                  return error4( c4, e4typeSub, E80901 ) ;
               return -1 ;
         }

         info = e4functionAdd( &p4->expr, iFunction ) ;
         if ( info == 0 )
            return -1 ;
         info->fieldNo = f4number( fieldPtr ) ;
         info->fieldPtr = fieldPtr ;
         if ( fieldPtr->data == (DATA4 *)p4->expr.data )  /* data is local */
            info->localData = 1 ;
            if ( p4->expr.tagPtr )
               info->p1 = (char *)&basePtr->dataFile->record ;
            else
         info->p1 = (char *)&basePtr->record ;
         info->i1 = fieldPtr->offset ;

         return 0 ;
      }
   }

   if( c4->errExpr )
      return error4describe( c4, e4unrecValue, E90910, (char *)p4->scan.ptr, 0, 0 ) ;
   return e4unrecValue ;
}

int s4stackPop( S4STACK *s4 )
{
   int retValue ;

   retValue = s4stackCur(s4) ;

   if ( s4->pos >= sizeof(int) )
      s4->pos -= sizeof(int) ;
   return retValue ;
}

int s4stackCur( S4STACK *s4 )
{
   int pos, curData ;

   if ( s4->pos < sizeof(int) )
      return E4NO_FUNCTION ;
   pos = s4->pos - sizeof(int) ;
   memcpy( (void *)&curData, s4->ptr+pos, sizeof(int) ) ;
   return curData ;
}

int s4stackPushInt( S4STACK *s4, const int i )
{
   return s4stackPushStr( s4, &i, sizeof(i)) ;
}

int s4stackPushStr( S4STACK *s4, const void *p, const int len )
{
   char *oldPtr ;
   CODE4 *c4 ;

   c4 = s4->codeBase ;

   if ( error4code( c4 ) < 0 )
      return -1 ;

   if ( s4->pos+len > s4->len )
   {
      oldPtr = s4->ptr ;
      if ( ! s4->doExtend )
         s4->ptr = 0 ;
      else
         s4->ptr = (char *)u4allocFree( c4, (long)s4->len + 256L ) ;
      if ( s4->ptr == 0 )
      {
         s4->ptr = oldPtr ;
         if ( c4->errExpr )
            return error4( c4, e4memory, E90911 ) ;
         return e4memory ;
      }
      memcpy( s4->ptr, oldPtr, s4->len ) ;
      u4free( oldPtr ) ;
      s4->len += 256 ;

      return s4stackPushStr( s4, p, len ) ;
   }
   else
   {
      memcpy( s4->ptr+s4->pos, p, (unsigned int)len ) ;
      s4->pos += len ;
   }
   return 0 ;
}

unsigned char s4scanChar( S4SCAN *s4 )
{
   if ( s4->pos >= s4->len )
      return 0 ;
   return s4->ptr[s4->pos] ;
}

void s4scanInit( S4SCAN *s4, const unsigned char *p )
{
   s4->ptr = p ;
   s4->pos = 0 ;
   s4->len = strlen( (char *)p ) ;
}

int s4scanRange( S4SCAN *s4, const int startChar, const int endChar )
{
   int count ;

   for ( count = 0; s4->pos < s4->len; s4->pos++, count++ )
      if ( s4->ptr[s4->pos] < startChar || s4->ptr[s4->pos] > endChar )
         return count ;
   return count ;
}

int s4scanSearch( S4SCAN *s4, const char searchChar )
{
   int count ;

   for ( count = 0; s4->pos < s4->len; s4->pos++, count++ )
      if ( s4->ptr[s4->pos] == searchChar )
         return count ;
   return count ;
}

#ifdef S4VB_DOS

EXPR4 *expr4parse_v( DATA4 *d4, char *expr )
{
   return expr4parseLow( d4, c4str(expr), 0 ) ;
}

#endif
