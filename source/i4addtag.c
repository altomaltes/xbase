/* i4addtag.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_INDEX

#ifndef S4OFF_WRITE

#ifdef S4CLIPPER
int tfile4add( TAG4FILE *t4, unsigned char *keyInfo, const long recNum, short int errUnique )
{
   CODE4 *c4 ;
   B4BLOCK *oldBlock, *rootBlock, *newBlock, *wchBlock ;
   int rc ;
   long  oldFileBlock, extendBlock, newFileBlock, rec ;
   B4KEY_DATA *atNew ;
   unsigned char oldKeyPtr[ I4MAX_KEY_SIZE ] ;
   unsigned char keyData[I4MAX_KEY_SIZE+1] ;    /* temporary storage for the key data (max size 100) */
   unsigned short int oldDesc ;
   #ifndef S4OFF_MULTI
      int dSet, incPos ;
      #ifdef E4ANALYZE_ALL
         long findVal ;
         int trc ;
      #endif
   #endif

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E95402 ) ;
      if ( keyInfo == 0 || recNum < 1 )
         return error4( t4->codeBase, e4parm_null, E95402 ) ;
   #endif

   rec = recNum ;
   c4 = t4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   oldDesc = t4->header.descending ;
   tfile4descending( t4, 0 ) ;

   rc = tfile4seek( t4, (char *)keyInfo, t4->header.keyLen ) ;

   #ifndef S4OFF_MULTI
      /* for run-time multi-user compatibility with Clipper, must perform
         the insertion at the end of the list of keys.  This is slower than
         CodeBase's insertion at the beginning (i.e. S4OFF_MULTI is defined) */
      if ( rc == 0 && errUnique == 0 )
      {
         for( dSet = 0, incPos = t4->header.keyLen - 1 ; dSet == 0 && incPos >=0 ; incPos-- )
            if ( keyInfo[incPos] != 0xFF )
            {
               keyInfo[incPos]++ ;
               dSet = 1 ;
               if ( tfile4seek( t4, (char *)keyInfo, t4->header.keyLen ) < 0 )
                  return -1 ;
               keyInfo[incPos]-- ;
               #ifdef E4ANALYZE_ALL
                  findVal = tfile4recNo( t4 ) ;
               #endif
            }
         #ifdef E4ANALYZE_ALL
            rc = tfile4seek( t4, (char *)keyInfo, t4->header.keyLen ) ;
            if ( rc != 0 || errUnique != 0 )
               findVal = -2L ;
            for(;;)
            {
               if ( (*t4->cmp)( tfile4keyData( t4 )->value, keyInfo, t4->header.keyLen ) == 0 )
               {
                  trc = tfile4skip( t4, 1L ) ;
                  if ( trc == 0L )
                  {
                     b4goEof( tfile4block( t4 ) ) ;
                     break ;
                  }
               }
               else
                  break ;
            }
            if ( tfile4recNo( t4 ) != findVal )
               return error4( c4, e4index, E85402 ) ;
         #endif
      }
   #endif

   tfile4descending( t4, oldDesc ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E95402 ) ;

   if ( rc == 0 )
   {
      switch ( errUnique )
      {
         case e4unique:
            return error4describe( c4, e4unique, E95402, tfile4alias( t4 ), t4->file.name, 0 ) ;
         case r4unique:
            return r4unique ;
         case r4uniqueContinue:
            return r4uniqueContinue ;
         default:
            break ;
      }
   }

   oldBlock = tfile4block(t4) ;
   oldFileBlock = 0 ;
   newFileBlock = 0 ;

   t4->header.version = (short)(t4->header.oldVersion + 1L) ;

   while( !b4leaf( oldBlock ) )
   {
      rc = tfile4down( t4 ) ;
      if ( rc < 0 || rc == 2 )
         return -1 ;
      oldBlock = tfile4block( t4 ) ;
      if ( b4leaf( oldBlock ) )
         oldBlock->keyOn = b4lastpos( oldBlock ) + 1 ;
      else
         oldBlock->keyOn = b4lastpos( oldBlock ) ;
   }

   for(;;)
   {
      if ( oldBlock == 0 )
      {
         /* Must create a new root block */
         extendBlock = tfile4extend( t4 ) ;
         if ( extendBlock < 0 )
            return (int) extendBlock ;

         rootBlock = b4alloc( t4, extendBlock) ;
         if ( rootBlock == 0 )
            return -1 ;

         l4add( &t4->blocks, rootBlock ) ;

         /* need to set root first so that b4insert() will record that current
            block is the root block, which is vital to its functioning */
         t4->header.root = rootBlock->fileBlock * 512 ;
         b4insert( rootBlock, keyInfo, rec, oldFileBlock ) ;
         b4append( rootBlock, newFileBlock ) ;

         t4->rootWrite  = 1 ;
         return 0 ;
      }

      if ( b4room( oldBlock ) )
      {
         if ( b4leaf( oldBlock ) )
            b4insert( oldBlock, keyInfo, rec, 0L ) ;
         else   /* update the current pointer, add the new branch */
         {
            #ifdef E4ANALYZE
               if ( oldBlock->nKeys == 0 )
                  return error4( t4->codeBase, e4index, E95402 ) ;
            #endif
               atNew = b4key( oldBlock, oldBlock->keyOn ) ;
               atNew->pointer = newFileBlock * 512 ;
               b4insert( oldBlock, keyInfo, rec, oldFileBlock ) ;
         }
         return 0 ;
      }

      l4pop( &t4->blocks ) ;

      /* NNNNOOOO  N - New, O - Old */
      /* The new block's end key gets added to the block just up */
      if ( oldBlock->keyOn < (t4->header.keysHalf + ( b4leaf( oldBlock ) ? 0 : 1 ) ) )
      {
         /* AS 05/05/98 had 0 as 3rd param here, 1 for split below, I think this was incorrect, so changed
          maybe should be dependent on leaf? */
         newBlock= tfile4split( t4, oldBlock, 1 ) ;
         if ( newBlock == 0 )
            return -1 ;
         wchBlock = oldBlock ;
      }
      else
      {
         newBlock= tfile4split( t4, oldBlock, 0 ) ;
         if ( newBlock == 0 )
            return -1 ;
         wchBlock = newBlock ;
      }

      if ( b4leaf( wchBlock ) )
      {
         b4insert( wchBlock, keyInfo, rec, 0L ) ;
         if ( newBlock->nKeys <= t4->header.keysHalf )   /* add a key from the old block!, must have info in newBlock because oldBlock gets deleted below */
         {
            #ifdef E4ANALYZE
               if ( oldBlock->nKeys <= t4->header.keysHalf )  /* impossible */
                  #ifdef S4CLIPPER
                     return error4describe( t4->codeBase, e4index, E81601, tfile4alias( t4 ), t4->file.name, 0 ) ;
                  #else
                     return error4describe( t4->codeBase, e4index, E81601, tfile4alias( t4 ), t4->indexFile->file.name, 0 ) ;
                  #endif
            #endif
            oldBlock->keyOn = oldBlock->nKeys - 1 ;
            memcpy( keyData, b4keyKey( oldBlock, oldBlock->keyOn ), t4->header.keyLen ) ;
            keyInfo = keyData ;
            rec = b4key( oldBlock, oldBlock->keyOn )->num ;
            b4remove( oldBlock ) ;
            newBlock->keyOn = 0 ;
            b4insert( newBlock, keyInfo, rec, 0 ) ;
         }
         newBlock->keyOn = 0 ;
         memcpy( keyData, b4keyKey( newBlock, newBlock->keyOn ), t4->header.keyLen ) ;
         keyInfo = keyData ;
         rec = b4key( newBlock, newBlock->keyOn )->num ;
         b4remove( newBlock ) ;
      }
      else
      {
         /* now get the key to place upwards */
         if ( wchBlock->nKeys == 0 )   /* treat like a root block */
         {
            if ( wchBlock == oldBlock )  /* not a problem if number of keys very small ?? */
            {
               if ( wchBlock->keyOn == 1 )   /* at end, so must combine with new block alterations */
               {
                  wchBlock->nKeys = 1 ;  /* now reset to the proper value */
                  /* don't actually need to do an insert, just set the file block value */
                  /* but need to copy values in so that later copy for data gives correct results */
                  memcpy( b4keyKey( oldBlock, 1), keyInfo, t4->header.keyLen ) ;
                  b4key( oldBlock, 1 )->num = rec ;
                  atNew = b4key( oldBlock, 1 ) ;
                  atNew->pointer = oldFileBlock * 512 ;
                  atNew = b4key( newBlock, 0 ) ;  /* currently set to old value, reset to new */
                  atNew->pointer = newFileBlock * 512 ;
               }
               else  /* simple insert */
               {
                  wchBlock->nKeys = 1 ;  /* to get a proper insert */
                  b4insert( wchBlock, keyInfo, rec, oldFileBlock ) ;
                  atNew = b4key( wchBlock, 1 ) ;
                  atNew->pointer = newFileBlock * 512 ;
                  memcpy( oldKeyPtr, b4keyKey( oldBlock, 1 ), t4->header.keyLen ) ;
                  keyInfo = oldKeyPtr ;
                  rec = b4key( oldBlock, 1 )->num ;
                  wchBlock->nKeys = 1 ;  /* now reset to the proper value */
               }
            }
            else
            {
               b4insert( wchBlock, keyInfo, rec, oldFileBlock ) ;
               b4append( wchBlock, newFileBlock ) ;
            }
         }
         else
         {
            if ( wchBlock->keyOn > wchBlock->nKeys && wchBlock == oldBlock )
            {
               atNew = b4key( newBlock, 0 ) ;
               atNew->pointer = newFileBlock * 512 ;
            }
            else
            {
               atNew = b4key( wchBlock, wchBlock->keyOn ) ;
               atNew->pointer = newFileBlock * 512 ;
            }
            b4insert( wchBlock, keyInfo, rec, oldFileBlock ) ;
         }
         memcpy( oldKeyPtr, b4keyKey( oldBlock, b4lastpos( oldBlock )), t4->header.keyLen ) ;
         keyInfo = oldKeyPtr ;
         rec = b4key( oldBlock, b4lastpos( oldBlock ) )->num ;
      }

      #ifdef E4INDEX_VERIFY
         rc = b4verify( oldBlock ) ;
         if ( rc < 0 )
            return error4stack( t4->codeBase, rc, E91642 ) ;
         rc = b4verify( newBlock ) ;
         if ( rc < 0 )
            return error4stack( t4->codeBase, rc, E91642 ) ;
      #endif

      l4add( &t4->saved, newBlock ) ;
      newFileBlock = newBlock->fileBlock ;
      oldFileBlock = oldBlock->fileBlock ;
      if ( b4flush( oldBlock ) < 0 )
         return -1 ;
      b4free( oldBlock ) ;
      oldBlock = (B4BLOCK *) t4->blocks.lastNode ;
   }
}

/* S4CLIPPER */
int t4addCalc( TAG4 *t4, long rec )
{
   int rc ;
   char *ptr ;
   TAG4FILE *tfile ;
   #ifndef S4OFF_TRAN
      TAG4KEY_REMOVED *removed ;
   #endif

   #ifdef E4PARM_LOW
      if ( t4 == 0 || rec < 1 )
         return error4( 0, e4parm, E95403 ) ;
   #endif

   tfile = t4->tagFile ;

   if ( error4code( tfile->codeBase ) < 0 )
      return e4codeBase ;

   if ( tfile->filter )
      if ( !expr4true( tfile->filter ) )
         return 0;

   rc = tfile4exprKey( tfile, (unsigned char **)&ptr ) ;
   if ( rc < 0 )
      return error4stack( tfile->codeBase, rc, E95403 ) ;

   #ifndef S4OFF_TRAN
      if ( code4tranStatus( tfile->codeBase ) == r4active && ( t4unique( t4 ) == r4unique ||
           t4unique( t4 ) == e4unique ) )  /* just remove from the removed list */
      {
         removed = t4keyFind( t4, 0L, ptr ) ;
         if ( removed != 0 )
         {
            if ( tfile4remove( tfile, removed->key, removed->recno ) < 0 )
               return -1 ;
            l4remove( &t4->removedKeys, removed ) ;
            u4free( removed ) ;
            return 0 ;
         }
      }
   #endif
   return tfile4add( tfile, (unsigned char *)ptr, rec, t4unique( t4 ) ) ;
}

#else  /* S4CLIPPER */

#ifdef S4FOX
/* (temporary) fix for FoxPro multi-user compatibility
   swaps parent and right blocks */
#ifndef S4OFF_MULTI
static long tfile4swap( B4BLOCK *parent, B4BLOCK *left )
{
   long tempFb ;
   FILE4LONG pos ;
   #ifdef S4BYTE_SWAP
      S4LONG longVal ;
   #endif

   tempFb = left->fileBlock ;
   left->fileBlock = parent->fileBlock ;
   parent->fileBlock = tempFb ;

   /* now update neighbours */
   if ( left->header.rightNode != -1 )
   {
      file4longAssign( pos, left->header.rightNode + 2*sizeof(short), 0 ) ;
      #ifdef S4BYTE_SWAP
         longVal = x4reverseLong( (void *)&left->fileBlock ) ;
         file4writeInternal( &parent->tag->indexFile->file, pos, &longVal, sizeof( left->header.leftNode ) ) ;
      #else
         file4writeInternal( &parent->tag->indexFile->file, pos, &left->fileBlock, sizeof( left->header.leftNode ) ) ;
      #endif
   }

   if ( left->header.leftNode != -1 )
   {
      file4longAssign( pos, left->header.leftNode + 2*sizeof(short), 0 ) ;
      #ifdef S4BYTE_SWAP
         longVal = x4reverseLong( (void *)&left->fileBlock ) ;
         file4write( &parent->tag->indexFile->file, pos, &longVal, sizeof( left->header.rightNode ) ) ;
      #else
         file4writeInternal( &parent->tag->indexFile->file, pos, &left->fileBlock, sizeof( left->header.rightNode ) ) ;
      #endif
   }

   if ( parent->header.rightNode != -1 )
   {
      file4longAssign( pos, parent->header.rightNode + 2*sizeof(short), 0 ) ;
      #ifdef S4BYTE_SWAP
         longVal = x4reverseLong( (void *)&parent->fileBlock ) ;
         file4writeInternal( &parent->tag->indexFile->file, pos, &longVal, sizeof( parent->header.leftNode ) ) ;
      #else
         file4writeInternal( &parent->tag->indexFile->file, pos, &parent->fileBlock, sizeof( parent->header.leftNode ) ) ;
      #endif
   }

   if ( parent->header.leftNode != -1 )
   {
      file4longAssign( pos, parent->header.leftNode + 2*sizeof(short), 0 ) ;
      #ifdef S4BYTE_SWAP
         longVal = x4reverseLong( (void *)&parent->fileBlock ) ;
         file4writeInternal( &parent->tag->indexFile->file, pos, &longVal, sizeof( parent->header.rightNode ) ) ;
      #else
         file4writeInternal( &parent->tag->indexFile->file, pos, &parent->fileBlock, sizeof( parent->header.rightNode ) ) ;
      #endif
   }

   return left->fileBlock ;
}
#endif /* S4OFF_MULTI */

int tfile4add( TAG4FILE *t4, const unsigned char *keyInfo, const long recIn, short int errUnique )
{
   int rc ;

   /* in fox case the tfile4add might not actually add the key */
   for( ;; )
   {
      rc = tfile4addDo( t4, keyInfo, recIn, errUnique ) ;
      if ( rc != 1 )
         return rc ;
   }
}
#endif /* S4FOX */

/* errUnique must be 0 if the tag is not unique, otherwise must contain unique error */
int tfile4addDo( TAG4FILE *t4, const unsigned char *keyInfo, const long recIn, short int errUnique )
{
   CODE4 *c4 ;
   INDEX4FILE *i4 ;
   B4BLOCK *oldBlock, *rootBlock, *newBlock ;
   int rc ;
   long  oldFileBlock, extendBlock, rec ;
   FILE4LONG pos ;
   #ifdef S4FOX
      int  keyOn, didAdd ;
      long rec1 = 0L ;
      long rec2 = 0L ;
      const unsigned char *tempKey = 0 ;
      int doInsert, updateReqd ;
   #else
      int isBranch ;
   #endif

   rec = recIn ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E95402 ) ;
      if ( keyInfo == 0 || rec < 1 )
         return error4( t4->codeBase, e4parm_null, E95402 ) ;
   #endif

   c4 = t4->codeBase ;
   i4 = t4->indexFile ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   #ifdef S4FOX
      if ( t4->expr != 0 )
         switch( errUnique )  /* ensure not a null add if r4/e4 candidate */
         {
            case r4candidate:
               if ( expr4nullLow( t4->expr, 1 ) == 1 )
                  return r4unique ;
               break ;
            case e4candidate:
               if ( expr4nullLow( t4->expr, 1 ) == 1 )
                  return error4describe( c4, e4unique, E95402, tfile4alias( t4 ), i4->file.name, 0 ) ;
               break ;
            default:
               break ;
         }

      rc = tfile4go( t4, keyInfo, rec, 1 ) ;
   #else
      rc = tfile4seek( t4, keyInfo, t4->header.keyLen ) ;
   #endif
   if ( rc < 0 )
      return error4stack( c4, rc, E95402 ) ;

   #ifdef S4FOX
      if ( rc == 0 )
      {
         switch( errUnique )
         {
            case e4unique:
            case e4candidate:
               return error4describe( c4, e4unique, E95402, tfile4alias( t4 ), i4->file.name, 0 ) ;
            case r4candidate :
               return r4unique ;
            default:
               break ;
         }
      }

      if ( errUnique && rc == r4found )
   #else
      if ( errUnique && rc == 0 )
   #endif
      {
         switch ( errUnique )
         {
            #ifdef S4FOX
               case e4candidate:
            #endif
            case e4unique:
               return error4describe( c4, e4unique, E95402, tfile4alias( t4 ), i4->file.name, 0 ) ;
            #ifdef S4FOX
               case r4candidate:
            #endif
            case r4unique:
               return r4unique ;
            case r4uniqueContinue:
               return r4uniqueContinue ;
            default:
               break ;
         }
      }

   if ( t4->filter && !t4->hasKeys )
   {
      file4longAssign( pos, t4->headerOffset + sizeof( t4->header ) + 222, 0 ) ;
      file4writeInternal( &t4->indexFile->file, pos, (char *) "\0", 1 ) ;
      t4->hasKeys = (char)1 ;
      #ifdef S4MDX
         t4->hadKeys = (char)0 ;
      #endif
   }

   oldBlock = tfile4block( t4 ) ;
   oldFileBlock = 0 ;

   #ifdef S4FOX
      doInsert = 1 ;
      updateReqd = 0 ;
      didAdd = 1 ;

      for( ;; )
      {
         if ( doInsert == 1 )
         {
            i4->tagIndex->header.version = i4->versionOld + 1 ;
            if ( oldBlock == 0 )
            {
               /* Must create a new root block */
               extendBlock = index4extend( i4 ) ;
               if ( extendBlock < 0 )
                  return (int)extendBlock ;

               rootBlock = b4alloc( t4, extendBlock) ;
               if ( rootBlock == 0 )
                  return -1 ;

               rootBlock->header.leftNode = -1 ;
               rootBlock->header.rightNode = -1 ;
               rootBlock->header.nodeAttribute = 1 ;

               l4add( &t4->blocks, rootBlock ) ;

               #ifndef S4OFF_MULTI
                  if ( t4->indexFile->file.lowAccessMode != OPEN4DENY_RW )
                  {
                     oldFileBlock = tfile4swap( rootBlock, (B4BLOCK *)l4last( &t4->saved ) ) ;
                     if ( oldFileBlock < 0 )
                        return -1 ;
                  }

               #endif

               b4top( rootBlock ) ;
               b4insert( rootBlock, tempKey, rec, rec2, 1 ) ;
               b4insert( rootBlock, keyInfo, oldFileBlock, rec1, 1 ) ;
               t4->header.root = rootBlock->fileBlock ;
               t4->rootWrite = 1 ;
               if ( didAdd == 0 )
                  return 1 ;
               return 0 ;
            }

            if ( (rc = b4insert( oldBlock, keyInfo, rec, rec1, 0 )) != 1 )
            {
               if ( rc == 0 )
               {
                  if ( b4leaf( oldBlock ) )
                  {
                     tempKey = keyInfo ;
                     rec2 = rec ;
                  }
                  if ( oldBlock->keyOn == oldBlock->header.nKeys - 1 )
                     updateReqd = 1 ;
                  doInsert = 0 ;
                  continue ;
               }
               else
                  return rc ;
            }
            else
            {
               l4pop( &t4->blocks ) ;
               keyOn = oldBlock->keyOn ;

               /* The new block's end key gets added to the block just up */
               newBlock= tfile4split( t4, oldBlock ) ;
               if ( newBlock == 0 )
                  return -1 ;

               l4add( &t4->saved, oldBlock ) ;

               if ( keyOn < oldBlock->header.nKeys )
               {
                  b4go( oldBlock, (long)keyOn ) ;
                  rc = b4insert( oldBlock, keyInfo, rec, rec1, 0 ) ;
               }
               else
               {
                  b4go( newBlock, (long)(keyOn - oldBlock->header.nKeys) ) ;
                  rc = b4insert( newBlock, keyInfo, rec, rec1, 0 ) ;
                  if ( rc == 0 )  /* if there was room to insert and on the key, need to change the upper block entry */
                     if ( newBlock->keyOn == newBlock->header.nKeys - 1 )
                        updateReqd = 1 ;
               }

               if ( rc == 1 )  /* was not possible to insert the key */
                  didAdd = 0 ;

               #ifdef E4INDEX_VERIFY
                  rc = b4verify( oldBlock ) ;
                  if ( rc < 0 )
                     return error4stack( t4->codeBase, rc, E91642 ) ;
                  rc = b4verify( newBlock ) ;
                  if ( rc < 0 )
                     return error4stack( t4->codeBase, rc, E91642 ) ;
               #endif

               /* Now add to the block just up */
               b4goEof( oldBlock ) ;
               oldBlock->keyOn-- ;

               keyInfo = b4keyKey( oldBlock, oldBlock->keyOn ) ;
               oldFileBlock = oldBlock->fileBlock ;
               rec1 = b4recNo( oldBlock, oldBlock->keyOn ) ;

               rec = newBlock->fileBlock ;
               rc = b4flush(newBlock) ;
               if ( rc < 0 )
                  return error4stack( t4->codeBase, rc, E91642 ) ;

               b4goEof( newBlock ) ;
               newBlock->keyOn-- ;

               tempKey = (unsigned char *)c4->savedKey ;
               memcpy( (void *)tempKey, (void *)b4keyKey( newBlock, newBlock->keyOn ), (unsigned int)t4->header.keyLen ) ;
               rec2 = b4recNo( newBlock, newBlock->keyOn ) ;
               if( newBlock->keyOn == newBlock->header.nKeys - 1 )
                  updateReqd = 1 ;
               b4free( newBlock ) ;
            }
         }
         else
            l4add( &t4->saved, l4pop( &t4->blocks ) ) ;

         oldBlock = (B4BLOCK *)t4->blocks.lastNode ;

         if ( oldBlock == 0 )
         {
            if ( doInsert == 0 )
            {
               if ( didAdd == 0 )
                  return 1 ;
               return 0 ;
            }
         }
         else
            if ( updateReqd )  /* may have to update a parent block */
            {
               if ( b4brReplace( oldBlock, tempKey, rec2 ) < 0 )
                  return -1 ;
               if ( oldBlock->keyOn != oldBlock->header.nKeys - 1 )  /* done reqd updates */
                  updateReqd = 0 ;
            }
      }
   #else              /* if not S4FOX  */
      i4->changed = 1 ;
      t4->changed = 1 ;
      t4->header.version++ ;

      for(;;)
      {
         if ( oldBlock == 0 )
         {
            /* Must create a new root block */
            extendBlock = index4extend(i4) ;
            if ( extendBlock < 0 )
               return (int) extendBlock ;

            rootBlock = b4alloc( t4, extendBlock) ;
            if ( rootBlock == 0 )
               return -1 ;

            l4add( &t4->blocks, rootBlock ) ;

            b4insert( rootBlock, keyInfo, oldFileBlock ) ;
            b4insert( rootBlock, keyInfo, rec ) ;
            rootBlock->nKeys-- ;
            t4->header.root = rootBlock->fileBlock ;
            t4->rootWrite  = 1 ;
            return 0 ;
         }

         if ( oldBlock->nKeys < oldBlock->tag->header.keysMax )
         {
            b4insert( oldBlock, keyInfo, rec ) ;
            return 0 ;
         }

         l4pop( &t4->blocks ) ;

         isBranch  = b4leaf( oldBlock )  ?  0 : 1 ;

         /* NNNNOOOO  N - New, O - Old */
         /* The new block's end key gets added to the block just up */
         newBlock= tfile4split( t4, oldBlock ) ;
         if ( newBlock == 0 )
            return -1 ;

         l4add( &t4->saved, newBlock ) ;

         newBlock->nKeys -= (short)isBranch ;
         if ( newBlock->keyOn < (newBlock->nKeys+isBranch) )
            b4insert( newBlock, keyInfo, rec ) ;
         else
            b4insert( oldBlock, keyInfo, rec ) ;

         #ifdef E4INDEX_VERIFY
            rc = b4verify( oldBlock ) ;
            if ( rc < 0 )
               return error4stack( t4->codeBase, rc, E91642 ) ;
            rc = b4verify( newBlock ) ;
            if ( rc < 0 )
               return error4stack( t4->codeBase, rc, E91642 ) ;
         #endif

         /* Now add to the block just up */
         newBlock->keyOn = b4lastpos(newBlock) ;

         keyInfo = b4keyKey( newBlock, newBlock->keyOn ) ;
         rec = newBlock->fileBlock ;

         oldFileBlock = oldBlock->fileBlock ;
         if ( b4flush(oldBlock) < 0 )
            return -1 ;

         b4free( oldBlock ) ;
         oldBlock = (B4BLOCK *) t4->blocks.lastNode ;
      }
   #endif
}

int t4addCalc( TAG4 *t4, long rec )
{
   int len ;
   unsigned char *ptr ;
   TAG4FILE *tfile ;
   #ifndef S4OFF_TRAN
      TAG4KEY_REMOVED *removed ;
   #endif

   #ifdef E4PARM_LOW
      if ( t4 == 0 || rec < 1 )
         return error4( 0, e4parm, E95403 ) ;
   #endif

   tfile = t4->tagFile ;

   if ( error4code( tfile->codeBase ) < 0 )
      return e4codeBase ;

   if ( tfile->filter )
      if ( !expr4true( tfile->filter ) )
         return 0;

   len = tfile4exprKey( tfile, (unsigned char **)&ptr ) ;
   if ( len < 0 )
      return error4stack( tfile->codeBase, len, E95403 ) ;

   #ifdef E4ANALYZE
      if ( len != tfile->header.keyLen )
         return error4describe( tfile->codeBase, e4index, E95403, tfile4alias( tfile ), 0, 0 ) ;
   #endif

   #ifndef S4OFF_TRAN
      if ( code4tranStatus( tfile->codeBase ) == r4active && ( t4unique( t4 ) == r4unique ||
           t4unique( t4 ) == e4unique
           #ifdef S4FOX
              || t4unique( t4 ) == r4candidate
           #endif
           ) )  /* just remove from the removed list */
      {
         removed = t4keyFind( t4, rec,(char *)ptr ) ;
         /* if found it means it is already there, so just remove from list */
         if ( removed != 0 )
         {
            l4remove( &t4->removedKeys, removed ) ;
            u4free( removed ) ;
            return 0 ;
         }
         removed = t4keyFind( t4, 0L, (char *)ptr ) ;
         if ( removed != 0 )
         {
            if ( tfile4remove( tfile, removed->key, removed->recno ) < 0 )
               return -1 ;
            l4remove( &t4->removedKeys, removed ) ;
            u4free( removed ) ;
         }
      }
   #endif
   return tfile4add( tfile, (unsigned char *)ptr, rec, t4unique( t4 ) ) ;
}

#endif  /*  ifndef S4CLIPPER  */

#endif  /* S4OFF_WRITE */
#endif  /* S4OFF_INDEX */
