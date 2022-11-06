/* *********************************************************************************************** */
/* Copyright (C) 1999-2015 by Sequiter, Inc., 9644-54 Ave, NW, Suite 209, Edmonton, Alberta Canada.*/
/* This program is free software: you can redistribute it and/or modify it under the terms of      */
/* the GNU Lesser General Public License as published by the Free Software Foundation, version     */
/* 3 of the License.                                                                               */
/*                                                                                                 */
/* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;       */
/* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.       */
/* See the GNU Lesser General Public License for more details.                                     */
/*                                                                                                 */
/* You should have received a copy of the GNU Lesser General Public License along with this        */
/* program. If not, see <https://www.gnu.org/licenses/>.                                           */
/* *********************************************************************************************** */

/* revisited by altomaltes@gmail.com
 */

/* r4relate.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"


   static int relate4currentIsChild( RELATE4 * ) ;
   static int relate4parent( RELATE4 *, RELATE4 * ) ;
   static int relate4nextRelationList( RELATION4 *, int ) ;
   static int relate4prevRecordInScan( RELATE4 * ) ;
   static int relate4prevRelationList( RELATION4 *, int ) ;

#ifdef S4SERVER
static int relate4initRelate( RELATE4 *, RELATION4 *, DATA4 *, CODE4 *, int, char * ) ;
#else
static int relate4initRelate( RELATE4 *, RELATION4 *, DATA4 *, CODE4 *, int ) ;
#endif
static int relate4lookup( RELATE4 *, const char ) ;
static int relate4readRest( RELATE4 *, char ) ;
static void relate4setNotRead( RELATE4 * ) ;
static void relate4sortFree( RELATION4 *, const int ) ;
static int relate4sortGetRecord( RELATION4 *, const long ) ;
static int relate4sortNextRecord( RELATION4 * ) ;
static int relate4sortPrevRecord( RELATION4 * ) ;

static int relate4topInit( RELATE4 * ) ;

static int f4flagIsSetFlip( F4FLAG *flagPtr, const unsigned long r )
{
   if ( flagPtr->flags == 0 )
      return 1 ;

   if ( flagPtr->isFlip )
      return ! f4flagIsSet( flagPtr, r ) ;
   else
      return f4flagIsSet( flagPtr, r ) ;
}

/* returns the position of the next flipped flag in the flag set - start at r */
static unsigned long f4flagGetNextFlip( F4FLAG *f4, const unsigned long r, const char direction )
{
   unsigned char cFlag ;
   unsigned long lowVal, onVal, highVal ;
   char i ;

   #ifdef E4PARM_LOW
      if ( direction != -1 && direction != 1 )
         return error4( 0, e4parm, E90812 ) ;
   #endif

   onVal = r ;
   if ( f4->flags == 0 || r > f4->numFlags )
      return 0 ;

   lowVal = (unsigned long)( r & 0x7 ) ;
   highVal = (unsigned long)( r >> 3 ) ;

   if ( (int)direction == -1 )
   {
      cFlag = (unsigned char)( f4->flags[highVal] ) ;
      if ( f4->isFlip )
         cFlag = (unsigned char) ~cFlag ;

      cFlag = (unsigned char)( (unsigned char)( cFlag << ( 7 - lowVal ) ) >> ( 7 - lowVal )) ;

      onVal += ( 7 - lowVal ) ;

      if ( cFlag == 0 )
         for( ; cFlag == 0 ; onVal -= 8 )
         {
            if ( highVal-- <= 1 )  /* if was zero, or is now zero */
            {
               if ( f4->flags[0] == 0 )
                  return r ;
               cFlag = f4->flags[0] ;
               if ( f4->isFlip )
                  cFlag = (unsigned char) ~cFlag ;
               onVal -= 8 ;  /* for sure if highVal == 0, else?? */
               break ;
            }
            cFlag = f4->flags[highVal] ;
            if ( f4->isFlip )
               cFlag = (unsigned char) ~cFlag ;
         }

      for( i = 7 ; (int)i >= 0 ; i--, onVal-- )
         if ( cFlag & ( 0x01 << i ) )
            break ;

      return (r - onVal) ;
   }
   else
   {
      cFlag = (unsigned char)f4->flags[highVal] ;
      if ( f4->isFlip )
         cFlag = (unsigned char) ~cFlag ;
      cFlag = (unsigned char) (cFlag >> lowVal) ;
      if ( cFlag == 0 )
      {
         onVal -= lowVal ;
         for( ; cFlag == 0 ; onVal += 8 )
         {
            if ( onVal >= f4->numFlags )
               return (f4->numFlags - r + 1) ;
            cFlag = f4->flags[++highVal] ;
            if ( f4->isFlip )
               cFlag = (unsigned char) ~cFlag ;
         }
      }

      for( i = 0 ; i <= 7 ; i++, onVal++ )
         if ( cFlag & ( 0x01 << i ) )
            break ;

      return (onVal - r) ;
   }
}

int r4dataListAdd( LIST4 *l4, DATA4 *data, RELATE4 *relate )
{
   R4DATA_LIST *r4 ;
   CODE4 *c4 ;

   c4 = relate->codeBase ;

   if ( error4code( c4 ) < 0 )
      return -1 ;

   if ( c4->relateDataListMemory == 0 )
   {
      c4->relateDataListMemory = mem4create( c4, 10, sizeof( R4DATA_LIST ), 10, 0 ) ;
      if ( c4->relateDataListMemory == 0 )
         return 0 ;
   }

   r4 = (R4DATA_LIST *)mem4alloc( c4->relateDataListMemory ) ;
   if ( r4 == 0 )
      return -1 ;
   r4->data = data ;
   r4->relate = relate ;
   l4add( l4, r4 ) ;
   return 0 ;
}

int r4dataListFind( LIST4 *l4, RELATE4 *r4 )
{
   R4DATA_LIST *link ;

   for ( link = 0 ;; )
   {
      link = (R4DATA_LIST *)l4next( l4, link ) ;
      if ( link == 0 )
         return 0 ;
      if ( link->relate == r4 )
         return 1 ;
   }
}

void r4dataListFree( LIST4 *l4 )
{
   R4DATA_LIST *r4data, *r4data2 ;

   for ( r4data = (R4DATA_LIST *)l4first( l4 ) ; r4data ; )
   {
      r4data->relate->sortType = 0 ;
      r4data2 = (R4DATA_LIST *)l4next( l4, r4data ) ;
      l4remove( l4, r4data ) ;
      mem4free( r4data->relate->codeBase->relateDataListMemory, r4data ) ;
      r4data = r4data2 ;
   }
}

/* this function takes a completed sort list, and adds data members in the
   following case:

   If (r)elate must be added, r's siblings must also be added

   Or, interpreted differently, if r is a relation, and any of it's children
   must be added, then all of its children must be added.
*/
static int r4dataListMassage( LIST4 *l4 )
{
   RELATE4 *relateChild ;
   R4DATA_LIST *r4data ;
   int addChildren, relateAdded ;

   if ( l4->nLink == 0 )  /* no work required */
      return 0 ;

   r4data = 0 ;

   for( ;; )
   {
      r4data = (R4DATA_LIST *)l4next( l4, r4data ) ;
      if ( r4data == 0 )
         break ;

      relateChild = 0 ;
      addChildren = 0 ;
      for( ;; )
      {
         relateChild = (RELATE4 *)l4next( &r4data->relate->slaves, relateChild ) ;
         if ( relateChild == 0 )
            break ;
         if ( r4dataListFind( l4, relateChild ) )
         {
            addChildren = 1 ;
            break ;
         }
      }

      if ( addChildren == 1 )
      {
         relateAdded = 0 ;
         relateChild = 0 ;
         for( ;; )
         {
            relateChild = (RELATE4 *)l4next( &r4data->relate->slaves, relateChild ) ;
            if ( relateChild == 0 )
               break ;
            if ( r4dataListFind( l4, relateChild ) == 0 )
            {
               r4dataListAdd( l4, relateChild->data, relateChild ) ;
               relateChild->sortType = relate4exact ;
               relateAdded = 1 ;
            }
         }
         if ( relateAdded == 1 )
            r4data = 0 ;   /* start at list top again to be sure none missed */
      }
   }

   return 0 ;
}

/* 1 - database added, 0 - database not added, -1 - error */
/* checkType gives the caller's status in terms of whether we should be included */
int r4dataListBuild( LIST4 *l4, RELATE4 *relate, EXPR4 *expr, int checkType )
{
   int i ;
   char mustAdd ;
   E4INFO *info ;
   RELATE4 *slaveOn ;

   if ( error4code( relate->codeBase ) < 0 )
      return -1 ;

   mustAdd = 0 ;

   /* 1st check if we must belong */
   for( i = 0 ; i < expr->infoN ; i++ )
   {
      info = expr->info + i ;
      if ( info->fieldPtr )
      {
         if ( info->fieldPtr->data == relate->data )
         {
            mustAdd = 1 ;
            break ;
         }
      }
   }

   relate->sortType = relate4exact ;

   if ( mustAdd )
      checkType = relate4exact ;
   else
   {
      if ( relate->relationType == relate4scan )
         checkType = relate4scan ;
      else
         if ( checkType != relate4scan )   /* non-scan parent must be added, so we add ourselves too, in order to save work later */
            mustAdd = 1 ;
   }

   /* if a child must be added, we must be too: */
   for ( slaveOn = 0 ;; )
   {
      slaveOn = (RELATE4 *)l4next( &relate->slaves, slaveOn ) ;
      if ( slaveOn == 0 )
         break ;
      if ( r4dataListBuild( l4, slaveOn, expr, checkType ) == 1 )
         mustAdd = 1 ;
   }

   if ( mustAdd )
      r4dataListAdd( l4, relate->data, relate ) ;
   else
      if ( relate->relationType == relate4scan )
         relate->sortType = relate4sortSkip ;

   return mustAdd ;
}

/* direction : -1 = look backwards, 0 = lookup only, 1 = look forwards */
static int relate4blankSet( RELATE4 *relate, const char direction )
{
   RELATE4 *slave ;
   CODE4 *c4 ;
   int rc ;

   #ifdef E4PARM_LOW
      if ( direction < -1 || direction > 1 )
         return error4( 0, e4parm, E94417 ) ;
   #endif

   c4 = relate->codeBase ;

   if ( error4code( c4 ) < 0 )
      return -1 ;

   relate->isRead = 1 ;
   if ( direction >= 0 )
   {
      if ( d4goEof( relate->data ) < 0 )
         return -1 ;
   }
   else
   {
      rc = d4top( relate->data ) ;
      if ( rc )
         return rc ;
      rc = d4skip( relate->data, -1L ) ;
      relate->data->recNum = -1 ;
      d4blank( relate->data ) ;
      relate->data->recordChanged = 0 ;
      if ( error4code( c4 ) < 0 )
         return -1 ;
      #ifdef S4SINGLE
         if ( rc < 0 )
      #else
         if ( rc == r4locked || rc < 0 )
      #endif
            return rc ;
   }

   for( slave = 0 ;; )
   {
      slave = (RELATE4 *)l4next( &relate->slaves, slave ) ;
      if ( slave == 0 )
         return 0 ;
      rc = relate4blankSet( slave, direction ) ;
      if ( rc < 0 )
         return rc ;
   }
}


int S4FUNCTION relate4bottom( RELATE4 *relate )
{
   RELATION4 *relation ;
   int rc, rc2 ;
   CODE4 *c4 ;
      #ifndef S4OFF_MULTI
         char oldReadLock ;
      #endif
      long rec ;

   #ifdef S4VBASIC
      if ( c4parm_check( relate, 5, E94401 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_LOW
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94401 ) ;
   #endif

   c4 = relate->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   relation = relate->relation ;
   relate = &relation->relate ;

      #ifndef S4OFF_MULTI
         oldReadLock = c4getReadLock( c4 ) ;
         c4setReadLock( c4, 0 ) ;
      #endif
      if ( relation->skipBackwards == 0 )
      {
         relate4sortFree( relation, 0 ) ;
         relate4skipEnable( relate, 1 ) ;
      }

      for ( ;; )  /* used to minimize return code areas, just break out... */
      {
         rc = relate4topInit( relate ) ;
         if ( rc != 0 )
            break ;

         relate4setNotRead( relate ) ;

         relation->currentRelateLevel = 0 ;
         relate4prevRelationList( relation, 1 ) ;

         if ( relation->inSort == relate4sortDone )
         {
            if ( relate4sortGetRecord( relation, relation->sortRecCount ) == r4eof )
            {
               rc = r4eof ;
               break ;
            }
            else
               relation->sortRecOn = relation->sortRecCount ;
         }
         else
         {
            rc = d4bottom( relate->data ) ;
            if ( rc != 0 )
               break ;
            if ( relation->exprSource )
            {
               rec = d4recNo( relate->data ) ;
               #ifndef S4OFF_INDEX
                  if ( relate->dataTag )
                  {
                     while ( f4flagIsSetFlip( &relate->set, (unsigned long)rec ) == 0 )
                     {
                        #ifdef S4HAS_DESCENDING
                           rc = (int)tfile4dskip( relate->dataTag->tagFile, -1L ) ;
                        #else
                           rc = (int)tfile4skip( relate->dataTag->tagFile, -1L ) ;
                        #endif
                        if ( rc != -1 )
                        {
                           if ( rc == 0 )
                              rc = r4eof ;
                           break ;
                        }
                        rec = tfile4recNo( relate->dataTag->tagFile ) ;
                     }
                     if ( rc == r4eof )
                        break ;
                  }
                  else
                  {
               #endif

               if ( f4flagIsSetFlip( &relate->set, (unsigned long)rec ) == 0 )
               {
                  rec = d4recNo( relate->data ) - f4flagGetNextFlip( &relate->set, (unsigned long)d4recNo( relate->data), (char)-1 ) ;
                  if ( rec == 0 )
                  {
                     rc = r4eof ;
                     break ;
                  }
               }
               #ifndef S4OFF_INDEX
                  }
               #endif
               rc = d4go( relate->data, rec ) ;
               if ( rc < 0 )
                  break ;
            }
            relate4setNotRead( relate ) ;
         }
         rc = relate4readRest( relate, -1 ) ;
         if ( rc == relate4filterRecord )
            rc = relate4skip( relate, -1L ) ;

         if ( rc < 0 || rc == r4terminate )
            break ;

         if ( relation->exprSource )
         {
            rc2 = log4true( &relation->log ) ;
            if ( rc2 == r4terminate )
            {
               rc = r4terminate ;
               break ;
            }
            if ( rc2 == 0 )
            {
               if ( relation->inSort == relate4sortSkip )  /* must temporarily disable in order to get a matching scan if available */
               {
                  relation->inSort = 0 ;
                  rc = relate4skip( relate, -1L ) ;
                  relation->inSort = relate4sortSkip ;
               }
               else
                  rc = relate4skip( relate, -1L ) ;
            }
         }

         if ( rc == r4bof )
            rc = r4eof ;

         break ;
      }

      #ifndef S4OFF_MULTI
         c4setReadLock( c4, oldReadLock ) ;
      #endif
      return rc ;
}

static int relate4buildScanList( RELATE4 *master, RELATE4 *relate, RELATION4 *relation )
{
   RELATE4 *relateOn ;
   RELATE4LIST *ptr ;

   if ( error4code( relate->codeBase ) < 0 )
      return -1 ;

   for( relateOn = 0 ;; )
   {
      relateOn = (RELATE4 *)l4next( &relate->slaves, relateOn ) ;
      if ( relateOn == 0 )
         break ;
      if ( relate4buildScanList( relate, relateOn, relation ) < 0 )
         return -1 ;
   }

   if ( master != 0 )  /* cannot have a scan without a base master */
      if ( relate->relationType == relate4scan || relate == &relation->relate )
      {
         ptr = (RELATE4LIST *)mem4createAlloc( relate->codeBase, &relate->codeBase->relateListMemory, 5, sizeof(RELATE4LIST), 5, 0 ) ;
         if ( ptr == 0 )
            return -1 ;
         ptr->ptr = relate ;
         l4add( &master->relateList, ptr ) ;
      }
   return 0 ;
}

static void relate4freeRelateList( RELATE4 *relate )
{
   RELATE4 *slaveOn ;
   void *ptr ;

   for( ;; )
   {
      ptr = l4pop( &relate->relateList ) ;
      if ( ptr == 0 )
         break ;
      mem4free( relate->codeBase->relateListMemory, ptr ) ;
   }

   for ( slaveOn = 0 ;; )
   {
      slaveOn = (RELATE4 *)l4next( &relate->slaves, slaveOn ) ;
      if ( slaveOn == 0 )
         return ;
      relate4freeRelateList( slaveOn ) ;
   }

}

int S4FUNCTION relate4changed( RELATE4 *relate )
{
   RELATION4 *relation ;
   CODE4 *c4 ;
      int j ;

   #ifdef E4PARM_HIGH
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94402 ) ;
   #endif

   c4 = relate->codeBase ;
   if ( error4code( c4 ) < 0 )
      return -1 ;

      u4free( relate->scanValue ) ;
      relate->scanValue = 0 ;
   relation = relate->relation ;


   relation->isInitialized = 0 ;
   relate4sortFree( relation, 0 ) ;

   relate4freeRelateList( &(relation->relate) ) ;

      u4free( relation->relate.set.flags ) ;
      memset( (void *)&relation->relate.set, 0, sizeof( F4FLAG ) ) ;

      if ( relation->log.expr != 0 )
      {
         for( j = relation->log.expr->infoN; --j >= 0; )
         {
            E4INFO_REPORT *info_ptr = relation->log.infoReport + j ;
            if ( info_ptr->relateDataList != 0 )
            {
               u4free( (info_ptr->relateDataList->pointers) ) ;
               mem4free( c4->dataListMemory, info_ptr->relateDataList ) ;
            }
         }

         expr4free( relation->log.expr ) ;
         relation->log.expr = 0 ;
         u4free( relation->log.infoReport ) ;
         relation->log.infoReport = 0 ;
      }
      if ( relation->log.bufLen != 0 )
      {
         u4free( relation->log.buf ) ;
         relation->log.bufLen = 0 ;
         relation->log.bufPos = 0 ;
      }
      relation->inSort = 0 ;

   return 0 ;
}

RELATE4 *S4FUNCTION relate4createSlave( RELATE4 *master, DATA4 *slaveData, const char *masterExpr, TAG4 *slaveTag )
{
   RELATION4 *relation ;
   RELATE4 *slave ;
   CODE4 *c4 ;
   int rc ;
   #ifdef S4SERVER
      LIST4 *oldList ;
   #endif

   if ( master == 0 )
      return 0 ;

   #ifdef S4VBASIC
      if ( c4parm_check( master, 5, E94403 ) )
         return (RELATE4 *) 0 ;
   #endif

   c4 = master->codeBase ;

   if ( error4code( c4 ) < 0 )
      return 0 ;

   #ifdef E4PARM_LOW
      if ( slaveData == 0 || masterExpr == 0 )
      {
         error4( c4, e4parm_null, E94403 ) ;
         return 0 ;
      }
   #endif

   relation = master->relation ;

   #ifdef E4MISC
      /* check that the d4 doesn't belong to any existing relation */
      if ( relate4lookupRelate( &relation->relate, slaveData ) != 0 )
      {
         error4( c4, e4parm, E84403 ) ;
         return 0 ;
      }
   #endif

   relate4changed( master ) ;

   slave = (RELATE4 *)mem4createAlloc( c4, &c4->relateMemory, 5, sizeof(RELATE4), 5, 0 ) ;
   if ( slave == 0 )
      return 0 ;

   #ifdef S4SERVER
      rc = relate4initRelate( slave, relation, slaveData, c4, 0, 0 ) ;
   #else
      rc = relate4initRelate( slave, relation, slaveData, c4, 1 ) ;
   #endif
   if ( rc < 0 )
   {
      mem4free( c4->relateMemory, slave ) ;
      return 0 ;
   }
   #ifdef S4SERVER
      oldList = tran4dataList( code4trans( c4 ) ) ;
      tran4dataListSet( code4trans( c4 ), &relation->localDataList ) ;
   #endif
   slave->masterExpr = expr4parseLow( master->data, masterExpr, 0 ) ;
   #ifdef S4SERVER
      tran4dataListSet( code4trans( c4 ), oldList ) ;
   #endif
   if ( slave->masterExpr == 0 )
   {
      mem4free( c4->relateMemory, slave ) ;
      return 0 ;
   }

   #ifdef S4VFP_KEY
      if ( slaveTag != 0 )
         slave->masterExpr->vfpInfo = &slaveTag->tagFile->vfpInfo ;
   #endif

      #ifndef S4OFF_INDEX
         if ( slaveTag != 0 )
            if ( tfile4type( slaveTag->tagFile ) != expr4type( slave->masterExpr ) )
            #ifdef S4CLIPPER
               if ( !( tfile4type( slaveTag->tagFile ) == r4num && expr4type( slave->masterExpr ) == r4numDoub ) )
                  if ( !( tfile4type( slaveTag->tagFile ) == r4numDoub && expr4type( slave->masterExpr ) == r4num ) )
            #endif
               {
                     error4( c4, e4relate, E84404 ) ;
                  mem4free( c4->relateMemory, slave ) ;
                  return 0 ;
               }
      #endif

   slave->dataTag = slaveTag ;
   slave->master = master ;

   l4add( &master->slaves, slave ) ;
   relate4matchLen( slave, -1 ) ; /* Set to maximum */

   return slave ;
}

/* checks if the given dbf belongs to one of the relations in relation */
#ifdef S4CB51
static int relate4dbfInRelation( RELATE4 *relate, const DATA4 *dbf )
{
   RELATE4 *relateOn ;

   relateOn = &relate->relation->relate ;
   while( relateOn->master )
      relateOn = relateOn->master ;

   do
   {
      if ( relateOn->data == dbf )
         return 1 ;
   } while( relate4next( &relateOn ) != 2 ) ;

   return 0 ;
}
#endif /* S4CB51 */

int S4FUNCTION relate4doAll( RELATE4 *relate )
{
   int rc ;
   CODE4 *c4 ;
   #ifndef S4OFF_MULTI
      char oldReadLock ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( relate, 5, E94405 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94405 ) ;
   #endif

   c4 = relate->codeBase ;

   #ifdef E4ANALYZE
      if ( relate->master != 0 )
         return error4( c4, e4struct, E84402 ) ;
   #endif

   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   #ifndef S4OFF_MULTI
      oldReadLock = c4getReadLock( c4 ) ;
      c4setReadLock( c4, 0 ) ;
   #endif
   relate4setNotRead( relate ) ;
   rc = relate4readRest( relate, 0 ) ;
   #ifndef S4OFF_MULTI
      c4setReadLock( c4, oldReadLock ) ;
   #endif

   /* AS Feb 5/98 --> just changing flag does not free up file handles and memory, causing failure to free up */
/*   relate->relation->isInitialized = 0 ; */
   relate4changed( relate ) ;

   if ( rc == relate4filterRecord )  /* no match is an error */
   {
         if ( c4->errRelate )
            return error4describe( c4, e4lookupErr, E94404, relate->data->alias, 0, 0 ) ;
      return r4terminate ;
   }

   return rc ;
}

int S4FUNCTION relate4doOne( RELATE4 *relate )
{
   int rc ;
   CODE4 *c4 ;
   #ifndef S4OFF_MULTI
      char oldReadLock ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( relate, 5, E94405 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94405 ) ;
   #endif

   c4 = relate->codeBase ;

   #ifdef E4PARM_HIGH
      if ( relate->master == 0 )
         return error4( c4, e4parm, E84405 ) ;
   #endif

   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   if ( relate->master == 0 )   /* no master, so we must be read */
      return 0 ;

   #ifndef S4OFF_MULTI
      oldReadLock = c4getReadLock( c4 ) ;
      c4setReadLock( c4, 0 ) ;
   #endif
   relate4setNotRead( relate ) ;
   rc = relate4lookup( relate, 0 ) ;
   #ifndef S4OFF_MULTI
      c4setReadLock( c4, oldReadLock ) ;
   #endif

   /* AS Feb 5/98 --> just changing flag does not free up file handles and memory, causing failure to free up */
/*   relate->relation->isInitialized = 0 ; */
   relate4changed( relate ) ;

   relate->isRead = relate->master->isRead ;  /* we are read if master is read */
   if ( rc == relate4filterRecord )  /* no match is an error */
   {
         if ( c4->errRelate )
            return error4describe( c4, e4info, E94405, relate->data->alias, 0, 0 ) ;
      return r4terminate ;
   }
   return rc ;
}

int S4FUNCTION relate4eof( RELATE4 *relate )
{
   #ifdef E4PARM_HIGH
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94406 ) ;
   #endif

   #ifdef E4MISC
      if ( relate->relation->isInitialized == 0 )
      {
         error4( relate->codeBase, e4info, E84406 ) ;
         return -1 ;
      }
   #endif

      if ( relate->relation->inSort == relate4sortDone )
         return relate->relation->sortEofFlag ;
      else
      return d4eof( relate->relation->relate.data ) ;
}

int S4FUNCTION relate4errorAction( RELATE4 *relate, const int code )
{
   int rc ;

   #ifdef E4PARM_HIGH
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94407 ) ;
      if ( code != relate4blank && code != relate4skipRec && code != relate4terminate )
         return error4( relate->codeBase, e4parm, E84407 ) ;
   #endif

   rc = relate->errorAction ;
   relate->errorAction = code ;
   return rc ;
}

int S4FUNCTION relate4freeRelate( RELATE4 *relate, const int closeFiles )
{
   int rc ;
   RELATE4 *relateOn ;
   CODE4 *c4 ;
   #ifdef S4SERVER
      LIST4 *oldList ;
   #endif

   rc = 0 ;
   if ( relate->master == 0 )
      return relate4free( relate, closeFiles ) ;

   c4 = relate->codeBase ;

   relate4changed( relate ) ;

   if( closeFiles )
   {
      #ifdef S4SERVER
         oldList = tran4dataList( code4trans( c4 ) ) ;
         tran4dataListSet( code4trans( c4 ), &relate->relation->localDataList ) ;
      #endif
      if( d4close( relate->data ) < 0 )
         rc = -1 ;
      #ifdef S4SERVER
         tran4dataListSet( code4trans( c4 ), oldList ) ;
      #endif
      relate->data = 0 ;
   }

   for( ;; )
   {
      relateOn = (RELATE4 *)l4last( &relate->slaves) ;
      if ( relateOn == 0 )
         break ;
      if( relate4freeRelate( relateOn, closeFiles ) < 0 )
         rc = -1 ;
   }

   expr4free( relate->masterExpr ) ;
   u4free( relate->scanValue ) ;
   relate->scanValue = 0 ;
      u4free( relate->set.flags ) ;
      relate->set.flags = 0 ;

   l4remove( &relate->master->slaves, relate ) ;
   mem4free( c4->relateMemory, relate ) ;
   relate = 0 ;

   return rc ;
}

int S4FUNCTION relate4free( RELATE4 *relate, const int closeFiles )
{
   int rc ;
   RELATION4 *relation ;
   RELATE4 *relateOn ;
   CODE4 *c4 ;
   #ifdef S4SERVER
      LIST4 *oldList ;
   #endif
   int oldErrorCode ;

   if ( relate == 0 )
      return -1 ;

   rc = 0 ;

   #ifdef S4VBASIC
      if ( c4parm_check( relate, 5, E94408 ) )
         return -1 ;
   #endif

   c4 = relate->codeBase ;
   oldErrorCode = error4set(c4, 0 ) ;

   #ifdef S4CB51
      #ifndef S4SINGLE
         relate4unlock( relate ) ;
      #endif
   #endif

   relate4changed( relate ) ;
   relation = relate->relation ;
   relate = &relation->relate ;

   #ifdef S4SERVER
      if( closeFiles || relate->freeData == 1 )
   #else
      if( closeFiles )
   #endif
   {
      #ifdef S4SERVER
         oldList = tran4dataList( code4trans( c4 ) ) ;
         tran4dataListSet( code4trans( c4 ), &relation->localDataList ) ;
      #endif
      if( d4close( relate->data ) < 0 )
         rc = -1 ;
      #ifdef S4SERVER
         tran4dataListSet( code4trans( c4 ), oldList ) ;
      #endif
      relate->data = 0 ;
   }

   for( relateOn = 0 ;; )
   {
      relateOn = (RELATE4 *)l4last( &relate->slaves ) ;
      if ( relateOn == 0 )
         break ;
      if( relate4freeRelate( relateOn, closeFiles ) < 0 )
         rc = -1 ;
   }

   relate4sortFree( relation, 1 ) ;
   u4free( relation->exprSource ) ;

   mem4free( c4->relationMemory, relation ) ;

   error4set(c4, oldErrorCode) ;
   return rc ;
}

#ifdef S4SERVER
static DATA4 *relate4dataOpen( RELATE4 *relate, DATA4 *oldData )
{
   DATA4 *d4 ;
   int oldAccessMode, oldReadOnly ;
   CODE4 *c4 ;

   #ifdef E4PARM_LOW
      if ( relate == 0 || oldData == 0 )
      {
         error4( 0, e4parm_null, E94427 ) ;
         return 0 ;
      }
   #endif

   c4 = relate->codeBase ;

   relate->dataOld = oldData ;

   oldAccessMode = c4->accessMode ;
   c4->accessMode = OPEN4DENY_NONE ;
   oldReadOnly = c4getReadOnly( c4 ) ;
   c4setReadOnly( c4, 1 ) ;
   d4 = d4openClone( oldData ) ;
   if ( d4 == 0 )
   {
      error4( c4, e4info, E94409 ) ;
      return 0 ;
   }
   #ifdef S4SERVER
      d4->trans = &c4->currentClient->trans ;
   #endif
   c4->accessMode = oldAccessMode ;
   c4setReadOnly( c4, oldReadOnly ) ;

   return d4 ;
}
#endif /* S4SERVER */

#ifdef S4SERVER
RELATE4 *S4FUNCTION relate4init( DATA4 *master, char *masterAliasName )
#else
RELATE4 *S4FUNCTION relate4init( DATA4 *master )
#endif
{
   RELATION4 *relation ;
   CODE4 *c4 ;
   int rc ;

   #ifdef S4VBASIC
      if ( c4parm_check( master, 2, E94410 ) )
         return (RELATE4 *) 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( master == 0 )
      {
         error4( 0, e4parm_null, E94410 ) ;
         return 0 ;
      }
   #endif

   c4 = master->codeBase ;
   if ( error4code( c4 ) < 0 )
      return 0 ;

   if ( c4->relationMemory == 0 )
   {
      c4->relationMemory = mem4create( c4, 5, sizeof( RELATION4 ), 5, 0 ) ;
      if ( c4->relationMemory == 0 )
         return 0 ;
   }
   relation = (RELATION4 *)mem4alloc( c4->relationMemory ) ;
   if ( relation == 0 )
      return 0 ;

      relation->log.relation = relation ;
      relation->log.codeBase = c4 ;
      relation->sort.file.hand = INVALID4HANDLE ;
      relation->sortedFile.hand = INVALID4HANDLE ;

   #ifdef S4SERVER
      rc = relate4initRelate( &relation->relate, relation, master, c4, 1, masterAliasName ) ;
   #else
      rc = relate4initRelate( &relation->relate, relation, master, c4, 1 ) ;
   #endif
   if ( rc < 0 )
   {
      mem4free( c4->relationMemory, relation ) ;
      return 0 ;
   }

   #ifdef S4SERVER
      relation->relate.freeData = 1 ;
   #endif

   return &relation->relate ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
#ifdef S4SERVER
static int relate4initRelate( RELATE4 *relate, RELATION4 *relation, DATA4 *data, CODE4 *c4, int doOpen, char *dataAliasName )
#else
static int relate4initRelate( RELATE4 *relate, RELATION4 *relation, DATA4 *data, CODE4 *c4, int doOpen )
#endif
{
   #ifdef S4SERVER
      LIST4 *oldList ;
   #endif

   relate->codeBase = c4 ;
   relate->relationType = relate4exact ;
   #ifdef E4ANALYZE
      if ( relate->data != 0 )
         error4( relate->codeBase, e4info, E84406 ) ;
   #endif

   #ifdef S4SERVER
      if ( doOpen == 0 )
         relate->data = data ;
      else
      {
         oldList = tran4dataList( code4trans( c4 ) ) ;
         tran4dataListSet( code4trans( c4 ), &relation->localDataList ) ;
         relate->data = relate4dataOpen( relate, data ) ;
         if ( dataAliasName != NULL )
            d4aliasSet( relate->data, dataAliasName ) ;
         tran4dataListSet( code4trans( c4 ), oldList ) ;
      }
   #else
      relate->data = data ;
   #endif
   relate->errorAction = relate4blank ;
   relate->relation = relation ;
   if ( relate->data == 0 )
      return error4( c4, e4info, E94410 ) ;

   relate->data->count = d4recCount( relate->data ) ;

   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION relate4lockAdd( RELATE4 *relate )
{
   #ifdef S4OFF_MULTI
      return 0 ;
   #else
      int rc ;
      RELATE4 *relateOn ;

      #ifdef E4PARM_HIGH
         if ( relate == 0 )
            return error4( 0, e4parm_null, E94411 ) ;
      #endif

      #ifdef S4VBASIC
         if ( c4parm_check( relate, 5, E94411 ) )
            return -1 ;
      #endif

      if ( error4code( relate->codeBase ) < 0 )
         return e4codeBase ;

      rc = 0 ;
      for( relateOn = &relate->relation->relate ;; )
      {
         if ( relateOn == 0 )
            break ;
         rc = d4lockAddFile( relateOn->data ) ;
         if ( rc != 0 )
            break ;
         if ( relate4next( &relateOn ) == 2 )
            break ;
      }

      return rc ;
   #endif
}

#ifdef S4CB51

/* new form does not allow combining lockAdds spaced around relate4locks()
   if required to do that, S4OLD_RELATE_LOCK code will allow */
int S4FUNCTION relate4lock( RELATE4 *relate )
{
   int rc ;

   rc = relate4lockAdd( relate ) ;
   if ( rc < 0 )
      return rc ;
   rc = code4lock( relate->codeBase ) ;
   if ( rc == 0 )
      relate->relation->locked = 1 ;
   return rc ;
}
#endif

#ifdef S4OLD_RELATE_LOCK
int S4FUNCTION relate4lock( RELATE4 *relate )
{
   #ifdef S4SINGLE
      return 0 ;
   #else
      CODE4 *c4 ;
      int rc, oldAttempts, count ;
      RELATE4 *relateOn ;
      #ifdef S4SERVER
         RELATE4 *relateSkip ;
         DATA4 *dataTemp ;
      #endif

      #ifdef E4PARM_HIGH
         if ( relate == 0 )
            return error4( 0, e4parm_null, E94411 ) ;
      #endif

      #ifdef S4VBASIC
         if ( c4parm_check( relate, 5, E94411 ) )
            return -1 ;
      #endif

      c4 = relate->codeBase ;
      if ( error4code( c4 ) < 0 )
         return e4codeBase ;

      relate->relation->locked = 1 ;

      count = oldAttempts = c4->lockAttempts ;  /* take care of wait here */
      c4->lockAttempts = 1 ;

      for( ;; )
      {
         rc = 0 ;
         for( relateOn = &relate->relation->relate ;; )
         {
            if ( relateOn == 0 )
               break ;
            #ifdef S4SERVER
               if ( relateOn->dataOld->dataFile->fileServerLock == data4serverId( relateOn->dataOld ) )  /* check for duplicate lock */
               {
                  dataTemp = tran4data( code4trans( c4 ), relateOn->dataOld->dataFile->fileServerLock, relateOn->dataOld->dataFile->fileClientLock ) ;
                  if ( dataTemp == 0 )
                     rc = r4locked ;
                  else
                  {
                     /* check for data in relation via relate search */
                     for ( relateSkip = &relate->relation->relate ;; )
                     {
                        if ( relateSkip == 0 )
                        {
                           rc = r4locked ;
                           break ;
                        }
                        if ( relateSkip->dataOld == dataTemp )
                           break ;
                        if ( relate4next( &relateSkip ) == 2 )
                           break ;
                     }
                  }
               }
               else
                  rc = dfile4lockAll( relateOn->dataOld->dataFile, data4clientId( relateOn->dataOld ), data4serverId( relateOn->dataOld ), relateOn->dataOld ) ;
            #else
               rc = d4lockAll( relateOn->data ) ;
            #endif
            if ( rc != 0 )
               break ;
            if ( relate4next( &relateOn ) == 2 )
               break ;
         }

         if ( rc != r4locked )
            break ;

         relate4unlock( relate ) ;
         if ( count == 0 )
            break ;

         if ( count > 0 )
            count-- ;

         #ifdef S4TEMP
            if ( d4displayQuit( &display ) )
            {
               rc = error4( c4, e4result, E84409 ) ;
               break ;
            }
         #endif

         u4delayHundredth( c4->lockDelay ) ;   /* wait a second & try lock again */
      }

      c4->lockAttempts = oldAttempts ;
      if ( error4code( c4 ) < 0 )
         return error4code( c4 ) ;

      return rc ;
   #endif
}
#endif  /* S4OLD_RELATE_LOCK */

/* direction : -1 = look backwards, 0 = lookup only, 1 = look forwards */
static int relate4lookup( RELATE4 *relate, const char direction )
{
   int rc ;
   long recno ;
   CODE4 *c4 ;
   RELATION4 *relation ;
      int oldGoError ;
   #ifndef S4OFF_INDEX
      char *ptr ;
      int len ;
      TAG4FILE *tagFile ;
   #endif

   #ifdef E4PARM_LOW
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94412 ) ;
      if ( direction < -1 || direction > 1 )
         return error4( 0, e4parm, E94417 ) ;
   #endif

   c4 = relate->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;
   relation = relate->relation ;

   #ifdef E4MISC
      if ( direction != 0 && relation->isInitialized == 0 )
         return error4( c4, e4struct, E84406 ) ;
   #endif

   relate->isRead = 1 ;
   if ( relate->master == 0 )
      return 0 ;

   #ifndef S4OFF_INDEX
      d4tagSelect( relate->data, relate->dataTag ) ;

      if ( relate->dataTag == 0 )
      {
   #endif
      recno = (long)expr4double( relate->masterExpr ) ;
      if ( error4code( c4 ) < 0 )
         return -1 ;

      if ( direction != 0 )
         if ( f4flagIsSetFlip( &relate->set, (unsigned long)recno ) == 0 )
            return relate4filterRecord ;

         oldGoError = c4->errGo ;
         c4->errGo = 0 ;
      rc = d4go( relate->data, recno ) ;
         c4->errGo = oldGoError ;
         if ( rc < 0 )
            return error4stack( c4, rc, E94412 ) ;
      if ( rc != r4entry )  /* if not error, then return */
         return 0 ;
      if ( relate->relationType == relate4approx )
      {
         d4goEof( relate->data ) ;
         return 0 ;
      }
   #ifndef S4OFF_INDEX
   }
   else
   {
      tagFile = relate->dataTag->tagFile ;

      len = expr4key( relate->masterExpr, &ptr, tagFile ) ;
      if ( len < 0 )
         return -1 ;

      len = (len < relate->matchLen) ? len : relate->matchLen ;   /* min of len and match len */

      #ifdef E4ANALYZE
         if ( len < 0 )
            return error4( c4, e4struct, E94413 ) ;
      #endif

      if ( relate->relationType == relate4scan )
      {
         #ifdef E4ANALYZE
            if ( relate->master == 0 )
               return error4( c4, e4struct, E84410 ) ;
         #endif
         if ( relate->master->scanValue == 0 )
         {
            relate->master->scanValueLen = len ;
            relate->master->scanValue = (char *)u4allocEr( c4, (long)len ) ;
            if ( relate->master->scanValue == 0 )
               return -1 ;
         }
         memcpy( relate->master->scanValue, ptr, (unsigned int)len ) ;
      }

      rc = tfile4seek( tagFile, ptr, len ) ;
      if ( rc < 0 )
         return -1 ;
      if ( relate->relationType == relate4approx || rc == 0 )
      {
         if ( tfile4eof( tagFile) )
         {
            d4goEof( relate->data ) ;
            return 0 ;
         }

         if ( (int)direction < 0 && rc == 0 && relate->relationType == relate4scan )  /* look for last one */
            for( ;; )
            {
               #ifdef S4HAS_DESCENDING
                  if ( !tfile4dskip( tagFile, 1L ) )
               #else
                  if ( !tfile4skip( tagFile, 1L ) )
               #endif
                  break ;
               #ifdef S4FOX
                  if ( u4keycmp( tfile4keyData( tagFile )->value, ptr, (unsigned int)len, (unsigned int)relate->dataTag->tagFile->header.keyLen, 0, &relate->dataTag->tagFile->vfpInfo ) )
               #else
                  if ( u4memcmp( tfile4keyData( tagFile )->value, ptr, (unsigned int)len ) )
               #endif
               {
                  #ifdef S4HAS_DESCENDING
                     tfile4dskip( tagFile, -1L ) ;
                  #else
                     tfile4skip( tagFile, -1L ) ;
                  #endif
                  break ;
               }
            }

         recno = tfile4keyData( tagFile )->num ;
         if ( direction != 0 )
            if ( f4flagIsSetFlip( &relate->set, (unsigned long)recno ) == 0 )
               return relate4filterRecord ;
         if ( d4go( relate->data, recno ) < 0 )
            return -1 ;
         return 0 ;
      }
      else
      {
         if ( tfile4eof( tagFile) )
            recno = 0 ;
         else
            recno = tfile4keyData( tagFile )->num ;
      }
   }
   #endif

   switch( relate->errorAction )  /* if got here, must be error condition */
   {
      case relate4blank:
         if ( d4goEof( relate->data ) < 0 )
            return -1 ;
         if ( direction != 0 )
         {
            if ( recno == 0 )
            {
               if ( relate->set.flags != 0 )
                  return relate4filterRecord ;
            }
            else
               if ( f4flagIsSetFlip( &relate->set, (unsigned long)recno ) == 0 )
                  return relate4filterRecord ;
         }
         return 0 ;
      case relate4skipRec:
         /* if a scan, and a failure, then, if the current relation set is
            below the current position, move it to above the current position */
         if ( relate->relationType == relate4scan )
         {
            if ( relate != &relation->relate )
               while ( relate4currentIsChild( relate ) )
                  relate4nextRelationList( relation, 0 ) ;
         }
         return relate4filterRecord ;
      case relate4terminate:
            if ( c4->errRelate )
               return error4describe( c4, e4lookupErr, E94412, relate->data->alias, 0, 0 ) ;
         return r4terminate ;
      default:  /* should never get this far */
         return error4( c4, e4info, E84411 ) ;
   }
}

RELATE4 *relate4lookupRelate( RELATE4 *relate, const DATA4 *d4 )
{
   RELATE4 *relateReturn, *relateOn ;

   if ( relate->data == d4 )
      return relate ;
   for( relateOn = 0 ;; )
   {
      relateOn = (RELATE4 *)l4next( &relate->slaves, relateOn) ;
      if ( relateOn == 0 )
         return 0 ;
      relateReturn = relate4lookupRelate( relateOn, d4 ) ;
      if ( relateReturn )
         return relateReturn ;
   }
}

int S4FUNCTION relate4matchLen( RELATE4 *relate, const int matchLenIn )
{
   int len, matchLen ;

   #ifdef E4PARM_HIGH
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94413 ) ;
      if ( relate->master == NULL )
         return error4( 0, e4parm, E84405 ) ;
   #endif

   if ( error4code( relate->codeBase ) < 0 )
      return e4codeBase ;

   matchLen = matchLenIn ;
      expr4context( relate->masterExpr, relate->master->data ) ;
   len = expr4keyLen( relate->masterExpr ) ;

   #ifdef S4CLIPPER
      if ( matchLen <= 0 )
         matchLen = len ;
   #else
      if ( matchLen <= 0 )
      {
         relate->matchLen = len ;
         #ifdef E4ANALYZE
            if ( relate->matchLen < 0 )
               return error4( 0, e4struct, E94413 ) ;
         #endif
         return len ;
      }
   #endif

      #ifndef S4OFF_INDEX
         #ifndef S4CLIPPER
            #ifdef E4MISC
               if ( relate->dataTag )
                  if( expr4type(  relate->dataTag->tagFile->expr ) != r4str )  /* make sure r4str only */
                     return error4( relate->codeBase, e4relate, E84412 ) ;
            #endif
         #endif
      #endif

   if ( matchLen > len )
      matchLen = len ;

      #ifndef S4OFF_INDEX
         if ( relate->dataTag )
         {
            expr4context( relate->dataTag->tagFile->expr, relate->data ) ;
            len = expr4keyLen( relate->dataTag->tagFile->expr ) ;
            if ( matchLen > len )
               matchLen = len ;
         }
      #endif

   relate->matchLen = matchLen ;
   relate4changed( relate ) ;

   #ifdef E4ANALYZE
      if ( relate->matchLen < 0 )
         return error4( 0, e4struct, E94413 ) ;
   #endif

   return matchLen ;
}

/* r4same = 0, r4down = 1, r4complete = 2 */
int S4FUNCTION relate4next( RELATE4 **ptrPtr )
{
   RELATE4 *cur ;
   void *nextLink ;
   int rc ;

   #ifdef E4PARM_HIGH
      if ( ptrPtr == 0 )
      {
         error4( 0, e4parm_null, E94414 ) ;
         return r4complete ;
      }
      if ( *ptrPtr == 0 )
      {
         error4( 0, e4parm_null, E94414 ) ;
         return r4complete ;
      }
   #endif

   cur = *ptrPtr ;
   rc = r4down ;

   if ( cur->slaves.nLink > 0 )
   {
      *ptrPtr = (RELATE4 *)l4first( &cur->slaves ) ;
      return r4down ;
   }

   for(;;)
   {
      rc -- ;
      if ( cur->master == 0 )
      {
         *ptrPtr = 0 ;
         return r4complete ;
      }

      nextLink = l4next( &cur->master->slaves, cur ) ;
      if ( nextLink )
      {
         *ptrPtr = (RELATE4 *)nextLink ;
         return rc ;
      }

      cur = cur->master ;
   }
}

static int relate4nextRecordInScan( RELATE4 *relate )
{
   long nextRec ;
   int rc ;
   DATA4 *d4 ;
      int  saveCode ;
   #ifndef S4OFF_INDEX
      B4KEY_DATA *key ;
      char *ptr ;
      int len ;
      unsigned char *keyValue ;
      TAG4FILE *tagFile ;
   #endif

   #ifdef E4PARM_LOW
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94415 ) ;
   #endif

   if ( error4code( relate->codeBase ) < 0 )
      return e4codeBase ;

   #ifdef E4MISC
      if ( relate->relation->isInitialized == 0 )
         return error4( relate->codeBase, e4info, E84406 ) ;
   #endif

   if ( relate->relation->inSort == relate4sortSkip && relate->sortType == relate4sortSkip )
      return r4eof ;

   d4 = relate->data ;
   /* AS 03/04/97 fix #71, if data at eof (blank record), may produce a key
      which when tfile4go() gets called goes to the 1st key (r4after), which may
      flukily match the master key, producing an unexpected match and an
      extra record in the set */
   if ( d4eof( d4 ) )
      return r4eof ;
   #ifndef S4OFF_INDEX
      if ( relate->dataTag == 0 )
      {
   #endif
         if ( d4bof( d4 ) )
            nextRec = 1 ;
         else
            nextRec = d4recNo( d4 ) + 1 ;
         nextRec += f4flagGetNextFlip( &relate->set, (unsigned long)nextRec, (char)1 ) ;
         if ( d4recCountLessEq( d4, nextRec ) == 0 )
            return r4eof ;
   #ifndef S4OFF_INDEX
      }
      else
      {
         tagFile = relate->dataTag->tagFile ;

         /* first, better make sure that tags and positions match record
            positions (in case of cloned DATA's ) */

         expr4context( tagFile->expr, relate->data ) ;
         tfile4exprKey( tagFile, &keyValue ) ;

         if ( relate->data->recNum == -1 )  /* special case because no valid record ... */
         {
            if ( !tfile4eof( tagFile ) )
               tfile4goEof( tagFile ) ;
         }
         else
         {
            rc = tfile4go( tagFile, keyValue, relate->data->recNum, 0 ) ;
            if ( rc < 0 )
               return rc ;
         }

         for(;;)
         {
            if ( d4bof( d4 ) )
            {
               if ( d4recCountLessEq( d4, 1L ) == 0L )  /* count == 0 */
                  return r4eof ;
               if ( relate->masterExpr == 0 )   /* top relate, bof */
                  return r4bof ;
               len = expr4key( relate->masterExpr, &ptr, tagFile ) ;
               if ( len < 0 )
                  return -1 ;

               len = (len < relate->matchLen) ? len : relate->matchLen ;   /* min of len and match len */

               rc = (int)tfile4seek( tagFile, relate->master->scanValue, len ) ;
               if ( rc < 0 )
                  return -1 ;
               if ( rc == 0 )
                  rc = 1 ;
               else
                  rc = 0 ;
            }
            else
               #ifdef S4HAS_DESCENDING
                  rc = (int)tfile4dskip( tagFile, 1L ) ;
               #else
                  rc = (int)tfile4skip( tagFile, 1L ) ;
               #endif
            if ( rc < 0 )
               return -1 ;
            if ( rc != 1 )
               return r4eof ;

            key = tfile4keyData( tagFile) ;
            nextRec = key->num ;

            if ( relate->master )
            {
               #ifdef S4FOX
                  if ( u4keycmp( key->value, relate->master->scanValue, (unsigned int)relate->master->scanValueLen,
                       (unsigned int)tagFile->header.keyLen, 0, &tagFile->vfpInfo ) )
               #else
                  if ( u4memcmp( key->value, relate->master->scanValue, (unsigned int)relate->master->scanValueLen ) )
               #endif
                  return r4eof ;
            }

            if ( f4flagIsSetFlip( &relate->set, (unsigned long)nextRec ) )
               break ;
         }
      }
   #endif

      saveCode = relate->codeBase->errGo ;
      relate->codeBase->errGo = 0 ;
   rc = d4go( d4, nextRec ) ;
      relate->codeBase->errGo = saveCode ;
   if ( rc < 0 )
      return -1 ;
   if ( rc == r4entry )
      return r4eof ;
   relate->isRead = 1 ;   /* we have updated this one */
   return relate4skipped ;
}

/* returns 1 if the current relation set is a child (or is itself) of the input relation */
static int relate4currentIsChild( RELATE4 *parent )
{
   RELATE4 *relateOn ;

   relateOn = parent->relation->currentRelateLevel ;

   if ( relateOn == parent )
      return 1 ;

   for ( relateOn = 0 ;; )  /* now recursively check all the descendents */
   {
      relateOn = (RELATE4 *)l4next( &parent->slaves, relateOn ) ;
      if ( relateOn == 0 )
         return 0 ;
      if ( relate4currentIsChild( relateOn ) == 1 )
         return 1 ;
   }
}

/* returns 1 if the parent is above the child at any level (grandparent, etc) */
static int relate4parent( RELATE4 *parent, RELATE4 *child )
{
   RELATE4 *slaveOn, *masterOn ;

   for ( slaveOn = child ;; )
   {
      masterOn = slaveOn->master ;
      if ( masterOn == 0 )
         return 0 ;
      if ( masterOn == parent )
         return 1 ;
      slaveOn = slaveOn->master ;  /* go up one level */
   }
}

/* returns 1 if done, 0 if positioned to a new relate */
/* if setup is true, it just does positional work (for relate4top), no reads */
static int relate4nextRelationList( RELATION4 *relation, int setup )
{
   RELATE4 *relateOn, *master ;
   int rc, rc2 ;

   relateOn = relation->currentRelateLevel ;

   if ( relateOn == 0 )  /* means get the first one */
      relateOn = &relation->relate ;
   else
      if ( setup != 1 )
      {
         /* first see if we are part of a scan ourselves, and if so that we are scanned */
         if ( relateOn->relationType == relate4scan || relateOn == &relation->relate )  /* the master is an implicit scan */
         {
            relate4setNotRead( relateOn ) ;  /* This data file & its slaves */
            if ( relation->inSort == relate4sortDone )
               if ( r4dataListFind( &relation->sortDataList, relateOn ) )
                  return relate4sortNextRecord( relation ) ;
            rc = relate4nextRecordInScan( relateOn ) ;
            if ( rc == relate4skipped )
               return 0 ;
            rc2 = relate4blankSet( relateOn, (char)1 ) ;
            if ( rc2 == r4locked || rc2 < 0 )  /* error or locked */
               return rc2 ;
            if ( relateOn->master == 0 )
               if ( d4eof( relateOn->data ) )
                  return r4eof ;
            /* are our siblings also scanned? */

            /* we are scanned, so fall through and check out our master */
         }

         if ( relateOn->master == 0 )
         {
            relation->currentRelateLevel = 0 ;
            return r4eof ;
         }

         master = relateOn->master ;

         /* try our masters next slave */
         relateOn = (RELATE4 *)l4next( &master->slaves, relateOn ) ;
         if ( relateOn == 0 )  /* no more slaves, try the master itself */
         {
            relation->currentRelateLevel = master ;
            return relate4continue ;  /* either do ourselves or go up further */
         }
      }

   /* we need to go down our own slave list to the bottom level and start seek */
   while ( relateOn->slaves.nLink != 0 )
      relateOn = (RELATE4 *)l4first( &relateOn->slaves ) ;

   /* at the bottom, so try ourselves */
   relation->currentRelateLevel = relateOn->master ;

   if ( setup == 1 )
      return relate4continue ;

   if ( relateOn->master == 0 )  /* done/ eof */
      return r4eof ;

   if ( relateOn->relationType == relate4scan )
      return relate4continue ;

   /* otherwise try our current masters other slaves--i.e. just go get next */
   return relate4nextRelationList( relation, setup ) ;
}

static int relate4nextScanRecord( RELATION4 *relation )
{
   RELATE4 *relate ;
   int rc, rc2, tryMatches ;
   LIST4 *relateList ;

   if ( error4code( relation->relate.codeBase ) < 0 )
      return -1 ;

   rc = 0 ;

   for ( ;; )
   {
      if ( relation->currentRelateLevel == 0 )
         relation->currentRelateLevel = &relation->relate ;

      for ( ;; )
      {
         relateList = &relation->currentRelateLevel->relateList ;
         tryMatches = 1 ;
         if ( d4eof( relation->currentRelateLevel->data ) ) /* we are at eof, so all children cannot match */
            tryMatches = 0 ;
         else
         {
            if ( relation->currentRelateLevel->master != 0 )
               if ( d4eof( relation->currentRelateLevel->master->data ) )  /* means no matches possible */
                  tryMatches = 0 ;
         }

         if ( tryMatches == 1 && l4numNodes( relateList ) )
         {
            if ( relateList->selected == 0 )
               relateList->selected = (LINK4 *)l4first( relateList ) ;
            for( ;; )
            {
               if ( relateList->selected == 0 )  /* finished with matches for this list */
                  break ;
               relate = ((RELATE4LIST *)relateList->selected)->ptr ;
               relate4setNotRead( relate ) ;  /* This data file & its slaves */
               if ( relation->inSort == relate4sortDone )
                  if ( r4dataListFind( &relation->sortDataList, relate ) )
                     return relate4sortNextRecord( relation ) ;

               rc = relate4nextRecordInScan( relate ) ;
               if ( rc == relate4skipped )
                  return 0 ;
               if ( rc < 0 )
                  return rc ;
               rc2 = relate4blankSet( relate, (char)1 ) ;
               if ( rc2 == r4locked || rc2 < 0 )  /* error or locked */
                  return rc2 ;
               if ( relate->master == 0 )
                  if ( d4eof( relate->data ) )
                     return r4eof ;
               relateList->selected =(LINK4 *)l4next( relateList, relateList->selected ) ;
            }
         }
         rc = relate4nextRelationList( relation, 0 ) ;
         if ( rc != relate4continue )
            return rc ;
      }

   }
}

/* returns 1 if done, 0 if positioned to a new relate */
/* if setup is true, it just does positional work (for relate4top), no reads */
static int relate4prevRelationList( RELATION4 *relation, int setup )
{
   RELATE4 *relateOn, *master ;
   int rc, rc2 ;

   relateOn = relation->currentRelateLevel ;

   if ( relateOn == 0 )  /* means get the first one */
      relateOn = &relation->relate ;
   else
      if ( setup != 1 )
      {
         /* first see if we are part of a scan ourselves, and if so that we are scanned */
         if ( relateOn->relationType == relate4scan || relateOn == &relation->relate )  /* the master is an implicit scan */
         {
            relate4setNotRead( relateOn ) ;  /* This data file & its slaves */
            if ( relation->inSort == relate4sortDone )
               if ( r4dataListFind( &relation->sortDataList, relateOn ) )
                  return relate4sortPrevRecord( relation ) ;

            rc = relate4prevRecordInScan( relateOn ) ;
            if ( rc == relate4skipped )
            {
               if ( relate4eof( relateOn ) )
               {
                  if ( relation->inSort == relate4sortDone && relation->sortEofFlag == r4eof )
                  {
                     relation->sortRecOn-- ;  /* move off eof on sort part */
                     relation->sortEofFlag = 0 ;
                  }
                  else
                  {
                     rc = d4go( relation->relate.data, d4recCount( relation->relate.data ) ) ;
                     if ( rc < 0 )
                        return rc ;
                  }
               }
               return 0 ;
            }
            if ( rc < 0 )
               return rc ;
            rc2 = relate4blankSet( relateOn, (char)-1 ) ;
            if ( rc2 == r4locked || rc2 < 0 )  /* error or locked */
               return rc2 ;
            if ( relateOn->master == 0 )
            {
               if ( d4bof( relateOn->data ) )
                  return r4bof ;
               if ( d4eof( relateOn->data ) )
                  return r4eof ;
            }
            /* are our siblings also scanned? */

            /* we are scanned, so fall through and check out our master */
         }

         if ( relateOn->master == 0 )
         {
            relation->currentRelateLevel = 0 ;
            return r4bof ;
         }

         master = relateOn->master ;

         /* try our masters prev slave */
         relateOn = (RELATE4 *)l4prev( &master->slaves, relateOn ) ;
         if ( relateOn == 0 )  /* no more slaves, try the master itself */
         {
            relation->currentRelateLevel = master ;
            return relate4continue ;  /* either do ourselves or go up further */
         }
      }

   /* we need to go down our own slave list to the bottom level and start seek */
   while ( relateOn->slaves.nLink != 0 )
      relateOn = (RELATE4 *)l4last( &relateOn->slaves ) ;

   /* at the bottom, so try ourselves */
   relation->currentRelateLevel = relateOn->master ;

   if ( setup == 1 )
      return relate4continue ;

   if ( relateOn->master == 0 )  /* done / bof */
      return r4bof ;

   if ( relateOn->relationType == relate4scan )
      return relate4continue ;

   /* otherwise try our current masters other slaves--i.e. just go get next */
   return relate4prevRelationList( relation, setup ) ;
}

static int relate4prevRecordInScan( RELATE4 *relate )
{
   long nextRec ;
   int  rc ;
   DATA4 *d4 ;
      int  saveCode ;
   #ifndef S4OFF_INDEX
      TAG4FILE *tagFile ;
      B4KEY_DATA *key ;
      unsigned char *keyValue ;
      #ifdef S4HAS_DESCENDING
         unsigned short int oldDesc ;
         int len ;
         char *ptr ;
      #endif
   #endif

   #ifdef E4PARM_LOW
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94416 ) ;
   #endif

   #ifdef E4MISC
      if ( relate->relation->isInitialized == 0 )
         return error4( relate->codeBase, e4info, E84406 ) ;
   #endif

   d4 = relate->data ;

   #ifndef S4OFF_INDEX
   if ( relate->dataTag == 0 )
   {
   #endif
      nextRec = d4recNo( d4 ) - 1 ;
      nextRec -= f4flagGetNextFlip( &relate->set, (unsigned long)nextRec, (char)-1 ) ;
      if ( nextRec <= 0 )
         return r4bof ;
      if ( d4recCountLessEq( d4, nextRec ) == 0 )
         return r4eof ;
   #ifndef S4OFF_INDEX
   }
   else
   {
      tagFile = relate->dataTag->tagFile ;

      /* first, better make sure that tags and positions match record
         positions (in case of cloned DATA's ) */

      expr4context( tagFile->expr, relate->data ) ;
      tfile4exprKey( tagFile, &keyValue ) ;

      if ( relate->data->recNum == -1 )  /* special case because no valid record ... */
      {
         if ( !tfile4eof( tagFile ) )
            tfile4goEof( tagFile ) ;
      }
      else
      {
         rc = tfile4go( tagFile, keyValue, relate->data->recNum, 0 ) ;
         if ( rc < 0 )
            return rc ;
      }

      for(;;)
      {
         if ( relate4eof( relate ) )   /* if eof in relate, just leave on last tag entry */
            rc = tfile4eof( tagFile ) ? 0 : -1 ;
         else
         {
            if ( d4eof( d4 ) == 1 )
            {
               if ( d4recCountLessEq( d4, 1L ) == 0L )  /* count == 0 */
                  return r4bof ;
               if ( relate->masterExpr == 0 )   /* top relate, bof */
                  return r4eof ;
               #ifdef S4HAS_DESCENDING
                  len = expr4key( relate->masterExpr, &ptr, tagFile ) ;
                  if ( len < 0 )
                     return -1 ;
                  len = (len < relate->matchLen) ? len : relate->matchLen ;   /* min of len and match len */
                  oldDesc = tagFile->header.descending ;
                  tfile4descending( tagFile, ((unsigned short int)(1 - oldDesc)) ) ;  /* invert the descending */
                  rc = (int)tfile4seek( tagFile, relate->master->scanValue, len ) ;
                  tfile4descending( tagFile, oldDesc ) ;
               #else
                  /* need to find the last matching entry, without seek */
                  rc = (int)tfile4bottom( tagFile ) ;
                  if ( rc == 0 )
                  {
                     rc = -1 ;
                     while ( rc == -1 )
                     {
                        key = tfile4keyData( tagFile) ;
                        if ( u4memcmp( key->value, relate->master->scanValue, (unsigned int)relate->master->scanValueLen ) == 0 )
                        {
                           rc = 0 ;
                           break ;
                        }
                        if ( u4memcmp( key->value, relate->master->scanValue, (unsigned int)relate->master->scanValueLen ) < 0 )
                           return r4bof ;
                        rc = (int)tfile4skip( tagFile, -1L ) ;
                     }
                  }
               #endif
               if ( rc < 0 )
                  return -1 ;
               if ( rc == 0 )
                  rc = -1 ;
               else
                  rc = 0 ;
            }
            else
               #ifdef S4HAS_DESCENDING
                  rc = (int)tfile4dskip( tagFile, -1L ) ;
               #else
                  rc = (int)tfile4skip( tagFile, -1L ) ;
               #endif
         }
         if ( rc > 0 )
            return -1 ;
         if ( rc != -1L )
            return r4bof ;

         key = tfile4keyData( tagFile) ;
         nextRec = key->num ;

         if ( relate->master )
            #ifdef S4FOX
               if ( u4keycmp( key->value, relate->master->scanValue, (unsigned int)relate->master->scanValueLen,
                    (unsigned int)tagFile->header.keyLen, 0, &tagFile->vfpInfo ) )
            #else
               if ( u4memcmp( key->value, relate->master->scanValue, (unsigned int)relate->master->scanValueLen ) )
            #endif
               return r4bof ;

         if ( f4flagIsSetFlip( &relate->set, (unsigned long)nextRec ) )
            break ;
      }
   }
   #endif

      saveCode = relate->codeBase->errGo ;
      relate->codeBase->errGo = 0 ;
   rc = d4go( d4, nextRec ) ;
      relate->codeBase->errGo = saveCode ;
   if ( rc < 0 )
      return -1 ;
   if ( rc == r4entry )
      return r4eof ;
   relate->isRead = 1 ;   /* we have updated this one */
   return relate4skipped ;
}

static int relate4prevScanRecord( RELATION4 *relation )
{
   RELATE4 *relate ;
   int rc, rc2, tryMatches ;
   LIST4 *relateList ;

   if ( error4code( relation->relate.codeBase ) < 0 )
      return -1 ;

   rc = 0 ;

   for ( ;; )
   {
      if ( relation->currentRelateLevel == 0 )
         relation->currentRelateLevel = &relation->relate ;

      for ( ;; )
      {
         relateList = &relation->currentRelateLevel->relateList ;
         tryMatches = 1 ;
         if ( d4eof( relation->currentRelateLevel->data ) ) /* we are at eof, so all children cannot match */
            tryMatches = 0 ;
         else
         {
            if ( relation->currentRelateLevel->master != 0 )
               if ( d4eof( relation->currentRelateLevel->master->data ) )  /* means no matches possible */
                  tryMatches = 0 ;
         }

         if ( tryMatches == 1 && l4numNodes( relateList ) )
         {
            if ( relateList->selected == 0 )
               relateList->selected = (LINK4 *)l4last( relateList ) ;
            for (;; )
            {
               if ( relateList->selected == 0 )  /* finished with matches for this list */
                  break ;
               relate = ((RELATE4LIST *)l4last( relateList ))->ptr ;
               if ( relate4eof( relate ) )  /* at eof means we must read this record */
                  if ( relation->inSort != relate4sortDone )
                  {
                     rc = relate4bottom( relate ) ;
                     if ( rc == r4eof )   /* no records, so can't skip back */
                        return r4bof ;
                     else
                        return rc ;
                  }

               relate = ((RELATE4LIST *)relateList->selected)->ptr ;
               relate4setNotRead( relate ) ;  /* This data file & its slaves */
               if ( relation->inSort == relate4sortDone )
                  if ( r4dataListFind( &relation->sortDataList, relate ) )
                     return relate4sortPrevRecord( relation ) ;

               rc = relate4prevRecordInScan(relate) ;
               if ( rc == relate4skipped )
               {
                  if ( relate4eof( relate ) )
                  {
                     if ( relation->inSort == relate4sortDone && relation->sortEofFlag == r4eof )
                     {
                        relation->sortRecOn-- ;  /* move off eof on sort part */
                        relation->sortEofFlag = 0 ;
                     }
                     else
                     {
                        rc = d4go( relation->relate.data, d4recCount( relation->relate.data ) ) ;
                        if ( rc < 0 )
                           return rc ;
                     }
                  }
                  return 0 ;
               }
               if ( rc < 0 )
                  return rc ;
               rc2 = relate4blankSet( relate, (char)-1 ) ;
               if ( rc2 == r4locked || rc2 < 0 )  /* error or locked */
                  return rc2 ;
               if ( relate->master == 0 )
               {
                  if ( d4bof(relate->data) )
                     return r4bof ;
                  if ( d4eof(relate->data) )
                     return r4eof ;
               }
               relateList->selected =(LINK4 *)l4prev( relateList, relateList->selected ) ;
            }
         }
         rc = relate4prevRelationList( relation, 0 ) ;
         if ( rc != relate4continue )
            return rc ;
      }
   }
}

int S4FUNCTION relate4querySet( RELATE4 *relate, const char *expr )
{
   int len ;

   if ( relate == 0 )
      return -1 ;

   #ifdef S4VBASIC
      if ( c4parm_check( relate, 5, E94428 ) )
         return -1 ;
   #endif

   if ( error4code( relate->codeBase ) < 0 )
      return -1 ;

   relate4changed( relate ) ;
   u4free( relate->relation->exprSource ) ;
   relate->relation->exprSource = 0 ;
   if ( expr == 0 )
      return 0 ;
   if ( expr[0] == 0 )
      return 0 ;
   len = strlen( expr ) + 1 ;
   relate->relation->exprSource = (char *)u4allocEr( relate->codeBase, (long)len ) ;
   if ( relate->relation->exprSource == 0 )
      return -1 ;
   memcpy( relate->relation->exprSource, expr, (unsigned int)len ) ;
   return 0 ;
}

int relate4readIn( RELATE4 *relate )
{
   int rc ;

   if ( error4code( relate->codeBase ) < 0 )
      return -1 ;
   if ( relate->isRead )
      return 0 ;
   if ( relate->master )
      if ( relate->master->isRead == 0 )
      {
         rc = relate4readIn( relate->master ) ;
         if ( rc == relate4filterRecord || rc == r4terminate )
            return rc ;
      }

   return relate4lookup( relate, 1 ) ;
}

/* direction : -1 = look backwards, 0 = lookup only, 1 = look forwards */
static int relate4readRest( RELATE4 *relate, char direction )
{
   RELATE4 *slave ;
   int rc, scanDone ;

   #ifdef E4PARM_LOW
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94417 ) ;
      if ( direction < -1 || direction > 1 )
         return error4( 0, e4parm, E94417 ) ;
   #endif

   #ifdef E4ANALYZE
      if ( error4code( relate->codeBase ) < 0 )
         return e4codeBase ;
   #endif

   rc = 0 ;

   if ( relate->isRead == 0 )
   {
      rc = relate4lookup( relate, direction );
      if ( rc < 0 ||rc == r4terminate )
      {
         relate4changed( relate ) ;  /* mark it as non-initialized, and free-up resources */
         return rc ;
      }
      if ( rc == relate4filterRecord )
         return rc ;
   }

   scanDone = 0 ;
   for( slave = 0 ;; )
   {
      if ( direction == 1 )
         slave = (RELATE4 *)l4next( &relate->slaves, slave ) ;
      else
         slave = (RELATE4 *)l4prev( &relate->slaves, slave ) ;
      if ( slave == 0 )
         break ;
      if ( slave->isRead == 0 )
         if ( slave->relationType == relate4scan )
         {
            /* if the currentRelateLevel is an upward master of ourselves,
               then make our master the currentRelateLevel so that the next
               skip will scan through us */
            if ( relate4parent( slave->relation->currentRelateLevel, slave->master ) )
               slave->relation->currentRelateLevel = slave->master ;
            if ( direction == 1 )
            {
               d4top( slave->data ) ;
               #ifndef S4OFF_INDEX
                  tfile4top( slave->dataTag->tagFile ) ;
               #endif
            }
            else
            {
               d4bottom( slave->data ) ;
               #ifndef S4OFF_INDEX
                  tfile4bottom( slave->dataTag->tagFile ) ;
               #endif
            }
         }
   }
   for( slave = 0 ;; )
   {
      if ( direction == 1 )
         slave = (RELATE4 *)l4next( &relate->slaves, slave ) ;
      else
         slave = (RELATE4 *)l4prev( &relate->slaves, slave ) ;
      if ( slave == 0 )
         return 0 ;
      if ( slave->relationType == relate4scan && scanDone == 1 )
      {
         if ( slave->isRead == 0 )
         {
            relate4blankSet( slave, (char)(-direction) ) ;  /* do reverse of direction */
            slave->isRead = 1 ;
            rc = relate4readRest( slave, direction ) ;
         }
      }
      else
      {
         rc = relate4readRest( slave, direction ) ;
         if ( slave->relationType == relate4scan && rc == 0 )
         {
            switch ( direction )
            {
               case -1:
                  if ( !d4bof( slave->data ) )
                     scanDone = 1 ;
                  break ;
               case 1:
                  if ( !d4eof( slave->data ) )
                     scanDone = 1 ;
                  break ;
               default:
                  break ;
            }
         }
      }
      if ( rc < 0 || rc == relate4filterRecord || rc == r4terminate )
         return rc ;
   }
}

static void relate4setNotRead( RELATE4 *relate )
{
   RELATE4 *slaveOn ;

   if ( relate->isRead )
   {
      relate->isRead = 0 ;
      for( slaveOn = 0 ;; )
      {
         slaveOn = (RELATE4 *)l4next(&relate->slaves,slaveOn) ;
         if ( slaveOn == 0 )
            return ;
         relate4setNotRead( slaveOn ) ;
      }
   }
}

int S4FUNCTION relate4skip( RELATE4 *relate, const long numSkip )
{
   int rc ;
   long numskip ;
   RELATION4 *relation ;
   CODE4 *c4 ;
      int sortStatus, rc2 ;
      signed char sign ;
      #ifdef S4REPORT
         #ifdef S4WINDOWS
            char countstring[22];
            static long int scanrecCount = 0, selectrecCount = 0, sortrecCount = 0;
            HWND statwin;
         #endif
      #endif
      #ifndef S4SINGLE
         char oldReadLock ;
      #endif

   #ifdef S4VBASIC
      if ( c4parm_check( relate, 5, E94418 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94418 ) ;
   #endif

   c4 = relate->codeBase ;

   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   numskip = numSkip ;
   relation = relate->relation ;

   if ( relation->isInitialized == 0 )
      return error4( c4, e4info, E84406 ) ;

   relate = &relation->relate ;

      if ( numskip < 0 )
      {
         if ( relation->skipBackwards == 0 )
            return error4( c4, e4info, E84417 ) ;
         sign = -1 ;
      }
      else
         sign = 1 ;

      sortStatus = 0 ;
      rc = 0 ;
      #ifndef S4SINGLE
         /* suspend auto read locking from within the relate */
         oldReadLock = c4getReadLock( c4 ) ;
         c4setReadLock( c4, 0 ) ;
      #endif
      for( ; numskip ; )
      {
         #ifdef S4REPORT
         #ifdef S4WINDOWS
            if( GetWindowWord( c4->hWnd, 8 ) == 666 )
               statwin = c4->hWnd ;

            if ( statwin )
            {
               if ( GetWindowWord( statwin, 6 ) == 0 )
               {
                  SetWindowWord( statwin, 6, 1 ) ;
                  scanrecCount = sortrecCount = selectrecCount = 0 ;
               }

               scanrecCount++ ;
               if ( scanrecCount < 20 || ( scanrecCount % 20 ) == 0 )
               {
                  c4ltoa45( scanrecCount, countstring, sizeof( countstring ) -1 ) ;
                  countstring[21] = 0 ;
                  SendMessage( (HWND)GetWindowWord( statwin, 0 ), WM_SETTEXT, 0, (LPARAM)((LPSTR)countstring ) ) ;
               }
            }
         #endif
         #endif

         if ( sign > 0 )
         {
            rc = relate4nextScanRecord( relation ) ;
            if ( rc == r4eof )
               break ;
         }
         else
         {
            rc = relate4prevScanRecord( relation ) ;
            if ( rc == r4bof )
               break ;
         }

         #ifdef S4SINGLE
            if ( rc < 0 || rc == r4terminate )
         #else
            if ( rc < 0 || rc == r4locked || rc == r4terminate )
         #endif
               break ;

         rc = relate4readRest( relate, sign ) ;
         if ( rc == relate4filterRecord )
            continue ;

         if ( rc < 0 || rc == r4terminate )
            break ;

         if ( relation->exprSource )
         {
            rc2 = log4true(&relation->log ) ;
            if ( rc2 == r4terminate )
            {
               rc = r4terminate ;
               break ;
            }
            if ( rc2 == 0 )
            {
               if ( relation->inSort == relate4sortSkip )  /* must temporarily disable in order to get a matching scan if available */
               {
                  sortStatus = 1 ;
                  relation->inSort = 0 ;
               }
               continue ;
            }
         }

         numskip -= sign ;
      }
      #ifndef S4SINGLE
         /* suspend auto read locking from within the relate */
         c4setReadLock( c4, oldReadLock ) ;
      #endif

      #ifdef S4WINDOWS
      #ifdef S4REPORT
         if(GetWindowWord( c4->hWnd, 8 ) == 666 )
            statwin = c4->hWnd;

         if ( statwin )
         {
            selectrecCount++;
            if ( selectrecCount < 20 || (selectrecCount % 20) == 0 )
            {
               c4ltoa45(selectrecCount,countstring,sizeof(countstring)-1);
               countstring[21] = 0;
               SendMessage((HWND)GetWindowWord(statwin,2),WM_SETTEXT,0,(LPARAM)((LPSTR)countstring));
            }
            if ( relation->inSort )
            {
               sortrecCount++;
               if( sortrecCount < 20 || (sortrecCount % 20) == 0)
               {
                  c4ltoa45(sortrecCount,countstring,sizeof(countstring)-1);
                  countstring[21] = 0;
                  SendMessage((HWND)GetWindowWord(statwin,4),WM_SETTEXT,0,(LPARAM)((LPSTR)countstring));
               }
            }
         }
      #endif
      #endif

      if ( sortStatus == 1 )
         relation->inSort = relate4sortSkip ;
      if ( relate->relation->inSort == relate4sortDone && rc == r4eof )
         relate->relation->sortEofFlag = r4eof ;

      return rc ;
}

int S4FUNCTION relate4skipEnable( RELATE4 *relate, const int doEnable )
{
   if ( relate == 0 )
      return -1 ;

   if ( error4code( relate->codeBase ) < 0 )
      return -1 ;

   if ( relate->relation->skipBackwards != (char) doEnable )
   {
      relate->relation->skipBackwards = (char) doEnable ;
      relate4changed( relate ) ;
   }
   return 0 ;
}

static void relate4sortFree( RELATION4 *relation, const int deleteSort )
{
   if ( relation == 0 )
      return ;

      sort4free( &relation->sort ) ;
      u4free( relation->otherData ) ;
      relation->otherData = 0 ;
      if ( relation->sortedFile.hand != INVALID4HANDLE )
         file4close( &relation->sortedFile ) ;
      r4dataListFree( &relation->sortDataList ) ;
      relation->inSort = 0 ;

   if ( deleteSort )
   {
      u4free( relation->sortSource ) ;
      relation->sortSource = 0 ;
   }
}

static int relate4sort( RELATE4 *relate )
{
   EXPR4 *sortExpr ;
   int rc, i, len ;
   long j, zero ;
   char nDbf, *sortKey ;
   R4DATA_LIST *r4data ;
   RELATION4 *relation ;
   CODE4 *c4 ;
   #ifdef S4SERVER
      LIST4 *oldList ;
   #endif

   zero = 0L ;

   #ifdef E4PARM_LOW
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94419 ) ;
   #endif

   c4 = relate->codeBase ;

   #ifdef E4ANALYZE
      if ( error4code( c4 ) < 0 )
         return e4codeBase ;
   #endif

   relation = relate->relation ;
   relate = &relation->relate ;
   rc = 0 ;
   #ifdef S4SERVER
      oldList = tran4dataList( code4trans( c4 ) ) ;
      tran4dataListSet( code4trans( c4 ), &relation->localDataList ) ;
   #endif
   sortExpr = expr4parseLow( relate->data, relation->sortSource, 0 ) ;
   #ifdef S4SERVER
      tran4dataListSet( code4trans( c4 ), oldList ) ;
   #endif

   relation->inSort = relate4sortSkip ;
   relation->sortDoneFlag = 0 ;

   rc = relate4top( relate ) ;
   if ( rc )   /* no records satisfy the relate, or error */
   {
      expr4free( sortExpr ) ;
      return rc ;
   }

   expr4context( sortExpr, relate->relation->relate.data ) ;
   /* AS sort has nothing to do with tag and ordering because tag is not coming into this.
   if ( relate->dataTag != NULL )
      len = expr4key( sortExpr, &sortKey, relate->dataTag->tagFile ) ;
   else
   */
      len = expr4key( sortExpr, &sortKey, 0 ) ;
   if ( len <= 0 )
   {
      expr4free( sortExpr ) ;
      return -1 ;
   }

   #ifdef E4ANALYZE
      if ( relation->sortDataList.nLink != 0 )
         return error4( c4, e4struct, E84413 ) ;
   #endif

   if ( r4dataListBuild( &relation->sortDataList, relate, sortExpr, relate4exact ) < 0 )
   {
      expr4free( sortExpr ) ;
      return -1 ;
   }

   if ( r4dataListMassage( &relation->sortDataList ) < 0 )
   {
      expr4free( sortExpr ) ;
      return -1 ;
   }

   nDbf = (char)relation->sortDataList.nLink ;

   relation->sortOtherLen = (int)(nDbf * sizeof(S4LONG )) ;
   relation->otherData = (char *)u4alloc( (long)relation->sortOtherLen ) ;
   if ( relation->otherData == 0 )
      return -1 ;

   rc = sort4initFree( &relation->sort, c4, len, relation->sortOtherLen, relate ) ;
   if ( rc )
   {
      expr4free( sortExpr ) ;
      return rc ;
   }

   #ifdef S4MDX
      switch( expr4type( sortExpr ) )
      {
         case r4num:
            relation->sort.cmp = (S4CMP_FUNCTION *)c4bcdCmp ;
            break ;
         case r4date:
            relation->sort.cmp = (S4CMP_FUNCTION *)t4cmpDoub ;
            break ;
         default:
            break ;
      }
   #endif

   /* call relate4top() again in case free-ups occurred */
   rc = relate4top( relate ) ;
   if ( rc )   /* no records satisfy the relate, or error */
   {
      expr4free( sortExpr ) ;
      return rc ;
   }

   for ( j = 0L, rc = 0 ; !rc ; j++, rc = relate4skip( relate, 1L ) )
   {
      for ( i = 0, r4data = 0 ;; i++ )
      {
         r4data = (R4DATA_LIST *)l4next( &relation->sortDataList, r4data ) ;
         if ( r4data == 0 )
            break ;
         if ( d4eof( r4data->data ) || d4bof( r4data->data ) )   /* relate4blank case */
            memcpy( relation->otherData + i * sizeof(S4LONG), (void *)&zero, sizeof(S4LONG ) ) ;
         else
            memcpy( relation->otherData + i * sizeof(S4LONG), (void *)&r4data->data->recNum, sizeof(S4LONG) ) ;
      }
   /* AS sort has nothing to do with tag and ordering because tag is not coming into this.
      if ( expr4key( sortExpr, &sortKey, relate->dataTag == NULL ? 0 : relate->dataTag->tagFile ) < 0 ) */
      if ( expr4key( sortExpr, &sortKey, 0 ) < 0 )
      {
         expr4free( sortExpr ) ;
         u4free( relation->otherData ) ;
         relation->otherData = 0 ;
         return -1 ;
      }
      if ( sort4put( &relation->sort, j, sortKey, relation->otherData ) < 0 )
      {
         expr4free( sortExpr ) ;
         u4free( relation->otherData ) ;
         relation->otherData = 0 ;
         return -1 ;
      }
   }

   expr4free( sortExpr ) ;

   if ( rc < 0 || rc == r4terminate )
   {
      u4free( relation->otherData ) ;
      relation->otherData = 0 ;
      return rc ;
   }

   relation->sortRecCount = j ;
   relation->inSort = relate4sortDone ;

   if ( relation->skipBackwards )
      if ( file4tempLow( &relation->sortedFile, c4, 1, 1, NULL ) < 0 )
      {
         u4free( relation->otherData ) ;
         relation->otherData = 0 ;
         return -1 ;
      }

   if ( sort4getInitFree( &relation->sort, relate ) < 0 )
      return -1 ;

   relation->sortRecOn = relation->sortFilePos = relation->sortRecTo = 0L ;

   return 0 ;
}

static int relate4sortGetRecord( RELATION4 *relation, const long num )
{
   int len, i, rc ;
   char *key ;
   char *other = 0 ;
   R4DATA_LIST *linkOn ;
   long j, numLeft ;
   FILE4LONG pos ;
   #ifdef S4DATA_ALIGN
      S4LONG longPtr ;
   #endif

   if ( error4code( relation->relate.codeBase ) < 0 )
      return -1 ;

   if ( num <= 0 )
      return r4bof ;

   relation->sortEofFlag = 0 ;
   numLeft = num - relation->sortRecTo ;

   if ( numLeft <= 0 )  /* already read, so just return from file */
   {
      if ( relation->skipBackwards == 0 )
         return -1 ;
      file4longAssign( pos, ( num - 1 ) * relation->sortOtherLen, 0 ) ;
      len = file4readInternal( &relation->sortedFile, pos, relation->otherData, (unsigned int)relation->sortOtherLen ) ;
      if ( len != relation->sortOtherLen )  /* free up and exit */
         return -1 ;
      other = relation->otherData ;
   }
   else
      while ( numLeft-- )
      {
         if ( relation->sortDoneFlag == 1 )  /* sort is finished, therefore must be eof */
            return r4eof ;

         rc = sort4get( &relation->sort, &j, (void **)&key, (void **)&other ) ;
         if ( rc )  /* no more items, or error */
         {
            sort4free( &relation->sort ) ;
            if ( rc == r4done )
            {
               relation->sortEofFlag = r4eof ;
               relation->sortDoneFlag = 1 ;
               return r4eof ;
            }
            else
               return rc ;
         }
         relation->sortRecTo++ ;
         if ( relation->skipBackwards )
         {
            file4longAssign( pos, relation->sortFilePos, 0 ) ;
            file4writeInternal( &relation->sortedFile, pos, other, (unsigned int)relation->sortOtherLen ) ;
            relation->sortFilePos += relation->sortOtherLen ;
         }
      }

   /* now read the database records in */
   for ( i = 0, linkOn = 0 ;; i++ )
   {
      linkOn = (R4DATA_LIST *)l4next( &relation->sortDataList, linkOn ) ;
      if ( linkOn == 0 )
         return 0 ;
      /* note that the sort positions all blanks to eof whereas in non-sort it is not
         consistent whether it is positioned bof or eof.  For sort it doesn't matter
         because further skips all go through the sort, therefore we already know the
         record order thus it doesn't need to be calculated.  This is a small efficience. */
      #ifdef S4DATA_ALIGN
         memcpy( &longPtr, (void *)(other + i*sizeof(longPtr)), sizeof(longPtr) ) ;
         if ( longPtr == 0 )  /* relate4blank case */
            rc = d4goEof( linkOn->data ) ;
         else
            rc = d4go( linkOn->data, longPtr ) ;
      #else
         if ( *((long *)(other) + i ) == 0 )  /* relate4blank case */
            rc = d4goEof( linkOn->data ) ;
         else
            rc = d4go( linkOn->data, *((long *)(other) + i ) ) ;
      #endif
      if ( rc < 0 )
         return rc ;

      linkOn->relate->isRead = 1 ;
   }
}

static int relate4sortNextRecord( RELATION4 *relation )
{
   int rc ;

   if ( error4code( relation->relate.codeBase ) < 0 )
      return -1 ;

   rc = relate4sortGetRecord( relation, relation->sortRecOn + 1 ) ;
   if ( rc == 0 )
      relation->sortRecOn++ ;
   if ( rc == r4eof )
      relation->sortRecOn = relation->sortRecCount + 1 ;
   return rc ;
}

static int relate4sortPrevRecord( RELATION4 *relation )
{
   int rc ;

   if ( error4code( relation->relate.codeBase ) < 0 )
      return -1 ;

   rc = relate4sortGetRecord( relation, relation->sortRecOn - 1 ) ;
   if ( rc == 0 )
      relation->sortRecOn-- ;
   return rc ;
}

int S4FUNCTION relate4sortSet( RELATE4 *relate, const char *expr )
{
   RELATION4 *relation ;
   int len ;

   if ( relate == 0 )
      return -1 ;

   #ifdef S4VBASIC
      if ( c4parm_check( relate, 5, E94429 ) )
         return -1 ;
   #endif

   if ( error4code( relate->codeBase ) < 0 )
      return -1 ;

   relation = relate->relation ;
   relate = &relation->relate ;
   relate4changed( relate ) ;
   u4free( relation->sortSource ) ;
   relation->sortSource = 0 ;
   if ( expr )
      if ( expr[0] )
      {
         len = strlen( expr ) ;
         relation->sortSource = (char *)u4allocEr( relate->codeBase, (long)len + 1L ) ;
         if ( relation->sortSource == 0 )
            return -1 ;
         memcpy( relation->sortSource, expr, (unsigned int)len ) ;
      }

   return 0 ;
}


static int relate4topInit( RELATE4 *relate )
{
   RELATION4 *relation ;
   int rc ;
   CODE4 *c4 ;
   #ifndef S4OPTIMIZE_OFF
      int has_opt ;
   #endif
   #ifdef S4SERVER
      LIST4 *oldList ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( relate, 5, E94422 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if( relate == 0 )
         return error4( 0, e4parm_null, E94422 ) ;
   #endif

   c4 = relate->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   relation = relate->relation ;
   relate = &relation->relate ;

   rc = 0 ;

   if ( relation->inSort == relate4sortDone )
      if ( relation->skipBackwards == 0 )
         relate4changed( relate ) ;

   relate->dataTag = relate->data->tagSelected ;
   if ( relation->isInitialized == 0 )
   {
      #ifndef S4OPTIMIZE_OFF
         has_opt = (char)c4->hasOpt ;
      #endif
      if ( rc < 0 )
         return rc ;
      relation->bitmapsFreed = 0 ;
      if ( relation->exprSource )
      {
         #ifdef S4SERVER
            oldList = tran4dataList( code4trans( c4 ) ) ;
            tran4dataListSet( code4trans( c4 ), &relation->localDataList ) ;
         #endif
         relation->log.expr = expr4parseLow( relate->data, relation->exprSource, 0 ) ;
         #ifdef S4SERVER
            tran4dataListSet( code4trans( c4 ), oldList ) ;
         #endif
         if ( relation->log.expr == 0 )
            return error4( c4, e4relate, E83703 ) ;
         if ( expr4type( relation->log.expr ) != r4log )
            return error4 ( c4, e4relate, E80905 ) ;

         if ( log4bitmapDo( &relation->log ) < 0 )
            relation->bitmapsFreed = 1 ;
         log4determineEvaluationOrder( &relation->log ) ;
      }

      if ( relate4buildScanList( 0, relate, relation ) < 0 )
         return -1 ;

      relation->isInitialized = 1 ;
      if ( relation->sortSource )
      {
         rc = relate4sort( relate ) ;
         if ( rc < 0 || rc == r4terminate )
            return rc ;
      }

      #ifndef S4OPTIMIZE_OFF
         if ( has_opt )
            code4optRestart( c4 ) ;
      #endif
   }

   return 0 ;
}

int S4FUNCTION relate4top( RELATE4 *relate )
{
   RELATION4 *relation ;
   int rc ;
   CODE4 *c4 ;
   DATA4 *d4 ;
      long rec ;
      int rc2 ;
      #ifndef S4OFF_MULTI
         char oldReadLock ;
      #endif

   #ifdef S4VBASIC
      if ( c4parm_check( relate, 5, E94422 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if( relate == 0 )
         return error4( 0, e4parm_null, E94422 ) ;
   #endif

   c4 = relate->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   relation = relate->relation ;
   relate = &relation->relate ;
   d4 = relate->data ;

   rc = 0 ;

      #ifndef S4OFF_MULTI
         oldReadLock = c4getReadLock( c4 ) ;
         c4setReadLock( c4, 0 ) ;
      #endif

      for ( ;; )  /* used to minimize return code areas, just break out... */
      {
         rc = relate4topInit( relate ) ;
         if (  rc != 0 )
            break ;

         relate4setNotRead( relate ) ;

         relation->currentRelateLevel = 0 ;
         relate4nextRelationList( relation, 1 ) ;

         if ( relation->inSort == relate4sortDone )
         {
            relation->sortRecOn = 0 ;
            rc = relate4sortNextRecord( relation ) ;
         }
         else
            rc = d4top( d4 ) ;

         if ( rc )    /* eof or error */
            break ;

         if ( relation->exprSource )
         {
            rec = d4recNo( d4 ) ;
            if ( f4flagIsSetFlip( &relate->set, (unsigned long)rec ) == 0 )
            {
               #ifndef S4OFF_INDEX
                  if ( relate->dataTag )
                  {
                     while ( f4flagIsSetFlip( &relate->set, (unsigned long)rec ) == 0 )
                     {
                        #ifdef S4HAS_DESCENDING
                           rc = (int)tfile4dskip( relate->dataTag->tagFile, 1L ) ;
                        #else
                           rc = (int)tfile4skip( relate->dataTag->tagFile, 1L ) ;
                        #endif
                        if ( rc != 1 )
                        {
                           if ( rc == 0 )
                           {
                              d4goEof( d4 ) ;
                              rc = r4eof ;
                           }
                           break ;
                        }
                        rec = tfile4recNo( relate->dataTag->tagFile ) ;
                     }
                     if ( rc == r4eof )
                        break ;
                  }
                  else
                  {
               #endif
                  rec = f4flagGetNextFlip( &relate->set, 1L, 1 ) + 1L ;
                  if ( d4recCountLessEq( d4, rec ) == 0 )
                  {
                     d4goEof( d4 ) ;
                     rc = r4eof ;
                     break ;
                  }
               #ifndef S4OFF_INDEX
                  }
               #endif
            }
            rc = d4go( d4, rec ) ;
            if ( rc != 0 )
               break ;
         }

         rc = relate4readRest( relate, 1 ) ;
         if ( rc == relate4filterRecord )
         {
            rc = relate4skip( relate, 1L ) ;
            break ;
         }

         if ( rc < 0 || rc == r4terminate )
            break ;

         if ( relation->exprSource )
         {
            rc2 = log4true( &relation->log ) ;
            if ( rc2 == r4terminate )
            {
               rc = r4terminate ;
               break ;
            }
            if ( rc2 == 0 )
            {
               if ( relation->inSort == relate4sortSkip )  /* must temporarily disable in order to get a matching scan if available */
               {
                  relation->inSort = 0 ;
                  rc = relate4skip( relate, 1L ) ;
                  relation->inSort = relate4sortSkip ;
               }
               else
                  rc = relate4skip( relate, 1L ) ;
            }
         }
         break ;
      }

      #ifndef S4OFF_MULTI
         c4setReadLock( c4, oldReadLock ) ;
      #endif
      return rc ;
}

int S4FUNCTION relate4type( RELATE4 *relate, int relateType )
{
   int rc ;

   #ifdef E4PARM_HIGH
      if ( relate == 0 )
         return error4( 0, e4parm_null, E94423 ) ;
      if ( relateType != relate4exact && relateType != relate4scan && relateType != relate4approx )
         return error4( relate->codeBase, e4parm, E84416 ) ;
   #endif
   rc = relate->relationType ;
   if ( rc != relateType )
   {
      relate->relationType = relateType ;
      relate4changed( relate ) ;
   }
   return rc ;
}

#ifdef S4CB51
int S4FUNCTION relate4unlock( RELATE4 *relate )
{
   #ifndef S4SINGLE
      DATA4 *dataOn ;

      #ifdef E4PARM_HIGH
         if ( relate == 0 )
            return error4( 0, e4parm_null, E94424 ) ;
      #endif

      if ( !relate->relation->locked )
         return 0 ;

      for ( dataOn = (DATA4 *)l4first( tran4dataList( code4trans( relate->codeBase ) ) ) ;
            dataOn ; dataOn = (DATA4 *)l4next( tran4dataList ( code4trans( relate->codeBase ) ), dataOn ) )
         if ( relate4dbfInRelation( relate, dataOn ) )
            d4unlockLow( dataOn, 0, 0 ) ;

      relate->relation->locked = 0 ;
   #endif
   return 0 ;
}
#endif

#ifdef S4VB_DOS

RELATE4 *S4FUNCTION relate4createSlave_v( RELATE4 *master, DATA4 *slave, char *masterExpr, TAG4 *slaveTag )
{
   return relate4createSlave( master, slave, c4str( masterExpr ), slaveTag ) ;
}

char * relate4masterExpr_v( RELATE4 *r4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( r4, 5, "relate4masterExpr():" ) ) return 0 ;
   #endif

   return v4str(r4->masterExpr->source) ;
}

int S4FUNCTION relate4querySet_v ( RELATE4 *relate, char *expr )
{
   return relate4querySet( relate, c4str(expr) ) ;
}

int S4FUNCTION relate4sortSet_v ( RELATE4 *relate, char *expr )
{
   return relate4sortSet( relate, c4str( expr ) ) ;
}
#endif
