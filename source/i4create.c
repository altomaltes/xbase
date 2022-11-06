/* i4create.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4INDEX_OFF
#ifndef S4OFF_WRITE

static INDEX4 *i4createLow( DATA4 *, const char *, const TAG4INFO * ) ;

INDEX4 *S4FUNCTION i4create( DATA4 *d4, const char *fileName, const TAG4INFO *tagData )
{
   INDEX4 *index ;

   #ifndef S4OFF_OPTIMIZE
         CODE4 *c4 ;
         #ifdef S4LOW_MEMORY
            int hasOpt ;
         #endif
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E95301 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( d4 == 0 || tagData == 0 )
      {
         error4( 0, e4parm_null, E95301 ) ;
         return 0 ;
      }
   #endif

   #ifndef S4OFF_OPTIMIZE
         c4 = d4->codeBase ;
         #ifdef S4LOW_MEMORY
            if ( c4->hasOpt )
            {
               hasOpt = 1 ;
               code4optSuspend( c4 ) ;
            }
            else
               hasOpt = 0 ;
      #endif
   #endif

   index = i4createLow( d4, fileName, tagData ) ;

      #ifndef S4OFF_OPTIMIZE
         #ifndef S4CLIPPER
            if ( index != 0 )
               file4optimizeLow( &index->indexFile->file, c4->optimize, OPT4INDEX, 0, index->indexFile ) ;
         #endif
         #ifdef S4LOW_MEMORY
            if ( hasOpt )
               code4optRestart( c4 ) ;
         #endif
      #endif

   return index ;
}

#ifndef S4CLIPPER
static INDEX4 *i4createLow( DATA4 *d4, const char *fileName, const TAG4INFO *tagData )
{
   DATA4FILE *data ;
   INDEX4FILE *indexFile ;
   TAG4 *tagPtr ;
   TAG4FILE *tFile ;
   INDEX4 *i4 ;
   CODE4 *c4 ;
   char buf[258] ;
   int i, rc ;
   FILE4LONG pos ;
   #ifdef S4FOX
      TAG4FILE *tagIndex ;
      #ifdef S4DATA_ALIGN
         unsigned int size, delta ;
      #endif
   #endif

   c4 = d4->codeBase ;

   #ifndef S4OFF_MULTI
      if ( fileName == 0 )  /* must have file open exclusively, since the lock handling changes, potentially causing multi-user failures */
         #ifdef S4FOX
         if ( d4version( d4 ) != 0x30 )    /* new locking scheme does not conflict */
         #endif
            if ( d4->dataFile->file.lowAccessMode != OPEN4DENY_RW )
            {
               error4( c4, e4exclusive, E81306 ) ;
               return 0 ;
            }
   #endif  /* S4OFF_MULTI */

   #ifdef E4ANALYZE
      if ( code4indexExtension( d4->codeBase ) == 0 )
      {
         error4( c4, e4struct, E91707 ) ;
         return 0 ;
      }
   #endif
   if ( fileName )
      u4nameCurrent( buf, sizeof( buf ), fileName ) ;
   else
      u4nameCurrent( buf, sizeof( buf ), dfile4name( d4->dataFile ) ) ;
   u4nameExt( buf, sizeof(buf), code4indexExtension( c4 ), ( fileName == 0 ? 1 : 0 ) ) ;
   #ifndef S4CASE_SEN
      c4upper( buf ) ;
   #endif
   if ( dfile4index( d4->dataFile, buf ) )
   {
      error4describe( c4, e4name, E81703, buf, 0, 0 ) ;
      return 0 ;
   }

   data = d4->dataFile ;
   if ( data == 0 )
      return 0 ;
   if ( error4code( c4 ) < 0 )
      return 0 ;
   error4set( c4, 0 ) ;  /* Make sure it is not 'r4unique' or 'r4noCreate'. */

   #ifndef S4SINGLE
      if ( d4lockFile( d4 ) )
      {
         error4( c4, e4lock, E85309 ) ;
         return 0 ;
      }
   #endif

   i4 = (INDEX4 *)mem4createAlloc( c4, &c4->indexMemory, c4->memStartIndex, sizeof(INDEX4), c4->memExpandIndex, 0 ) ;
   if ( i4 == 0 )
   {
      #ifdef E4STACK
         error4stack( c4, e4memory, E95301 ) ;
      #endif
      return 0 ;
   }

   i4->codeBase = c4 ;
   i4->data = d4 ;

   indexFile = (INDEX4FILE *)mem4createAlloc( c4, &c4->index4fileMemory, c4->memStartIndexFile, sizeof(INDEX4FILE), c4->memExpandIndexFile, 0 ) ;
   if ( indexFile == 0 )
   {
      #ifdef E4STACK
         error4stack( c4, e4memory, E95301 ) ;
      #endif
      return 0 ;
   }
   indexFile->codeBase = c4 ;
   indexFile->dataFile = data ;

   indexFile->userCount = 1 ;
   i4->indexFile = indexFile ;
   rc = file4create( &indexFile->file, c4, buf, 1 ) ;
   if ( rc )
   {
      if ( rc > 0 )
         error4set( c4, rc ) ;
       /* don't mark file as temporary here because it could have failed for safety reasons - in any case, create failed, so don't bother */
      i4closeLow( i4 ) ;
      return 0 ;
   }

   l4add( &data->indexes, indexFile ) ;
   l4add( &d4->indexes, i4 ) ;

      if ( fileName == 0 )
         u4namePiece( i4->accessName, sizeof( i4->accessName ), d4->alias, 0, 0 ) ;
      else
      {
         #ifdef E4MISC
            if ( strlen( fileName ) > sizeof( i4->accessName ) - 1 )
            {
               /* mark the file as temporary so it gets deleted since creation failed */
               indexFile->file.isTemp = 1 ;
               i4closeLow( i4 ) ;
               error4( c4, e4name, E95301 ) ;
               return 0 ;
            }
         #endif
         u4ncpy( i4->accessName, fileName, sizeof( i4->accessName ) - 1 ) ;
      }
      c4upper( i4->accessName ) ;

   #ifdef S4FOX
      indexFile->blockMemory = mem4create( c4, c4->memStartBlock,
           (sizeof(B4BLOCK)) + B4BLOCK_SIZE - (sizeof(B4STD_HEADER)) - (sizeof(B4NODE_HEADER)), c4->memExpandBlock, 0 ) ;
      if ( indexFile->blockMemory == 0 )
      {
         /* mark the file as temporary so it gets deleted since creation failed */
         indexFile->file.isTemp = 1 ;
         i4closeLow( i4 ) ;
         return 0 ;
      }

      if ( c4->tagMemory == 0 )
      {
         c4->tagMemory = mem4create( c4, c4->memStartTag, sizeof(TAG4), c4->memExpandTag, 0 ) ;
         if ( c4->tagMemory == 0 )
         {
            /* mark the file as temporary so it gets deleted since creation failed */
            indexFile->file.isTemp = 1 ;
            return 0 ;
         }
      }

      if ( c4->tagFileMemory == 0 )
      {
         c4->tagFileMemory = mem4create( c4, c4->memStartTagFile, sizeof(TAG4FILE), c4->memExpandTagFile, 0 ) ;
         if ( c4->tagFileMemory == 0 )
         {
            /* mark the file as temporary so it gets deleted since creation failed */
            indexFile->file.isTemp = 1 ;
            return 0 ;
         }
      }

      indexFile->tagIndex = (TAG4FILE *) mem4alloc( c4->tagFileMemory ) ;
      if ( indexFile->tagIndex == 0 )
      {
         /* mark the file as temporary so it gets deleted since creation failed */
         indexFile->file.isTemp = 1 ;
         return 0 ;
      }

      tagIndex = indexFile->tagIndex ;

      tagIndex->codeBase = c4 ;
      tagIndex->indexFile = indexFile ;
      tagIndex->header.typeCode = 0xE0 ;  /* compound, compact */
      tagIndex->header.filterLen = 1 ;
      tagIndex->header.filterPos = 1 ;
      tagIndex->header.exprLen = 1 ;
      tagIndex->header.exprPos = 0 ;
      tagIndex->header.keyLen = 10 ;
      tagIndex->header.signature = 0x01 ;
      #ifdef S4DATA_ALIGN
         size = (unsigned int)sizeof(S4LONG) + tagIndex->header.keyLen ;
         delta = sizeof(void *) - size % sizeof(void *) ;
         tagIndex->builtKeyMemory = mem4create( c4, 3, size + delta, 2, 0 ) ;
      #else
         tagIndex->builtKeyMemory = mem4create( c4, 3, (unsigned int)sizeof(S4LONG) + tagIndex->header.keyLen + 1, 2, 0 ) ;
      #endif
      if ( tfile4setCollatingSeq( tagIndex, sort4machine ) < 0 )     /* tag of tags is always machine */
      {
         /* mark the file as temporary so it gets deleted since creation failed */
         indexFile->file.isTemp = 1 ;
         i4closeLow( i4 ) ;
         return 0 ;
      }
      if ( tfile4setCodePage( tagIndex, d4->codePage ) < 0 )
      {
         /* mark the file as temporary so it gets deleted since creation failed */
         indexFile->file.isTemp = 1 ;
         i4closeLow( i4 ) ;
         return 0 ;
      }

      u4namePiece( tagIndex->alias, sizeof(indexFile->tagIndex->alias), buf, 0, 0 ) ;
      c4upper( tagIndex->alias ) ;

      tagPtr = 0 ;
      for ( i = 0; tagData[i].name; i++ )
      {
         tagPtr = (TAG4 *)mem4alloc( c4->tagMemory ) ;
         if ( tagPtr == 0 )
            break ;

         tagPtr->tagFile = (TAG4FILE *)mem4alloc( c4->tagFileMemory ) ;
         if ( tagPtr->tagFile == 0 )
            break ;
         tFile = tagPtr->tagFile ;

         tagPtr->index = i4 ;
         tFile->codeBase = c4 ;
         tFile->indexFile = indexFile ;

         #ifdef S4FOX
            /* if tfile4set functions fail, an error4code() is generated by them */
            if ( tfile4setCollatingSeq( tFile, c4->collatingSequence ) < 0 )
               break ;
            if ( tfile4setCodePage( tFile, d4->codePage ) < 0 )
               break ;
         #endif

         u4ncpy( tFile->alias, tagData[i].name, sizeof( tFile->alias ) ) ;
         c4upper( tFile->alias ) ;

         tFile->header.signature = 0x01 ;
         tFile->header.typeCode = 0x60 ;  /* compact */
         if ( tagData[i].unique )
         {
            #ifdef S4FOX
               if ( tagData[i].unique == r4candidate || tagData[i].unique == e4candidate )
                  tFile->header.typeCode += 0x04 ;
               else
            #endif
               tFile->header.typeCode += 0x01 ;
            tagPtr->errUnique = tagData[i].unique ;

            #ifdef S4FOX
               if ( tagData[i].unique != e4unique && tagData[i].unique != r4unique &&
                    tagData[i].unique != r4uniqueContinue && tagData[i].unique != r4candidate &&
                    tagData[i].unique != e4candidate )
            #else
               if ( tagData[i].unique != e4unique && tagData[i].unique != r4unique && tagData[i].unique != r4uniqueContinue )
            #endif
            {
               error4describe( c4, e4tagInfo, E85301, tagData[i].name, 0, 0 ) ;
               break ;
            }
         }
         if ( tagData[i].descending)
         {
            tFile->header.descending = 1 ;
            #ifdef E4PARM_HIGH
               if ( tagData[i].descending != r4descending )
               {
                  error4describe( c4, e4tagInfo, E85302, tagData[i].name, 0, 0 ) ;
                  break ;
               }
            #endif
         }

         if ( tagData[i].expression == 0 )
         {
            error4describe( c4, e4tagInfo, E85303, tagData[i].name, tagData[i].expression, 0 ) ;
            break ;
         }

         tFile->expr = expr4parseLow( d4, tagData[i].expression, tFile ) ;
         if ( tFile->expr == 0 )
         {
            if ( error4code( c4 ) >= 0 )
               error4( c4, e4memory, E95301 ) ;
            break ;
         }

         tFile->header.exprLen = (short) (strlen( tFile->expr->source ) + 1) ;
         if ( tFile->header.exprLen > I4MAX_EXPR_SIZE )
         {
            error4describe( c4, e4tagInfo, E85304, tagData[i].name, tagData[i].expression, 0 ) ;
            break ;
         }
         if ( tagData[i].filter != 0 )
            if ( *( tagData[i].filter ) != '\0' )
            {
               tFile->header.typeCode += 0x08 ;
               tFile->filter = expr4parseLow( d4, tagData[i].filter, tFile ) ;
               if (tFile->filter)
                  tFile->header.filterLen = (short)strlen( tFile->filter->source ) ;
            }
         tFile->header.filterLen++ ;  /* minimum of 1, for the '\0' */
         if ( tFile->header.filterLen > I4MAX_EXPR_SIZE )
         {
            error4describe( c4, e4tagInfo, E85304, tagData[i].name, tagData[i].filter, 0 ) ;
            break ;
         }
         tFile->header.filterPos = tFile->header.exprLen ;

         if ( error4code( c4 ) < 0 )
            break ;
         l4add( &indexFile->tags, tFile ) ;
         l4add( &i4->tags, tagPtr ) ;
         tagPtr = 0 ;
      }

      if ( error4code( c4 ) < 0 )
      {
         if ( tagPtr != 0 )
         {
            if ( tFile != 0 )
               mem4free( c4->tagFileMemory, tFile ) ;
            mem4free( c4->tagMemory, tagPtr ) ;
         }
         /* mark the file as temporary so it gets deleted since creation failed */
         indexFile->file.isTemp = 1 ;
         i4closeLow( i4 );
         return 0 ;
      }
   #else                /* if not S4FOX   */
      indexFile->header.two = 2 ;
      u4yymmdd( indexFile->header.createDate ) ;

      if ( fileName == 0 )
         indexFile->header.isProduction = 1 ;

      indexFile->header.numSlots = 0x30 ;
      indexFile->header.slotSize = 0x20 ;

      u4namePiece( indexFile->header.dataName, sizeof( indexFile->header.dataName ), data->file.name, 0, 0 ) ;
      indexFile->header.blockChunks = (short)(c4->memSizeBlock/512) ;

      #ifdef E4MISC
         if ( indexFile->header.blockChunks < 2 || indexFile->header.blockChunks > 63 )   /* disallowed for compatibility reasons */
         {
            error4describe( c4, e4info, E85305, fileName, 0, 0 ) ;
            /* mark the file as temporary so it gets deleted since creation failed */
            indexFile->file.isTemp = 1 ;
            i4closeLow( i4 );
            return 0 ;
         }
      #endif

      indexFile->header.blockRw = (short)(indexFile->header.blockChunks * I4MULTIPLY) ;
      indexFile->blockMemory = mem4create( c4, c4->memStartBlock, (sizeof(B4BLOCK)) + indexFile->header.blockRw -
         (sizeof(B4KEY_DATA)) - (sizeof(short)) - (sizeof(char[6])), c4->memExpandBlock, 0 ) ;

      if ( indexFile->blockMemory == 0 )
      {
         /* mark the file as temporary so it gets deleted since creation failed */
         indexFile->file.isTemp = 1 ;
         i4closeLow( i4 ) ;
         return 0 ;
      }

      tagPtr = 0 ;
      for ( i = 0 ; tagData[i].name ; i++ )
      {
         indexFile->header.numTags++ ;

         if ( c4->tagMemory == 0 )
         {
            c4->tagMemory = mem4create( c4, c4->memStartTag, sizeof(TAG4), c4->memExpandTag, 0 ) ;
            if ( c4->tagMemory == 0 )
               break ;
         }

         if ( c4->tagFileMemory == 0 )
         {
            c4->tagFileMemory = mem4create( c4, c4->memStartTagFile, sizeof(TAG4FILE), c4->memExpandTagFile, 0 ) ;
            if ( c4->tagFileMemory == 0 )
               break ;
         }

         tagPtr = (TAG4 *)mem4alloc( c4->tagMemory ) ;
         if ( tagPtr == 0 )
            break ;
         memset( (void *)tagPtr,0, sizeof(TAG4) ) ;
         tagPtr->index = i4 ;

         tagPtr->tagFile = (TAG4FILE *)mem4alloc( c4->tagFileMemory ) ;
         if ( tagPtr->tagFile == 0 )
            break ;
         tFile = tagPtr->tagFile ;
         memset( (void *)tFile,0, sizeof(TAG4FILE) ) ;
         tFile->codeBase = c4 ;
         tFile->indexFile = indexFile ;

         u4ncpy( tFile->alias, tagData[i].name, sizeof(tFile->alias) ) ;
         c4upper( tFile->alias ) ;

         tFile->header.typeCode  = 0x10 ;
         if ( tagData[i].unique )
         {
            tFile->header.typeCode += 0x40 ;
            tFile->header.unique = 0x4000 ;
            tagPtr->errUnique = tagData[i].unique ;

            #ifdef S4FOX
               if ( tagData[i].unique != e4unique && tagData[i].unique != r4unique &&
                    tagData[i].unique != r4uniqueContinue && tagData[i].unique != r4candidate &&
                    tagData[i].unique != e4candidate )
            #else
               if ( tagData[i].unique != e4unique && tagData[i].unique != r4unique &&tagData[i].unique != r4uniqueContinue )
            #endif
            {
               error4describe( c4, e4tagInfo, E85301, tagData[i].name, 0, 0 ) ;
               break ;
            }
         }
         if ( tagData[i].descending)
         {
            tFile->header.typeCode += 0x08 ;
            #ifdef E4PARM_HIGH
               if ( tagData[i].descending != r4descending )
               {
                  error4describe( c4, e4tagInfo, E85302, tagData[i].name, 0, 0 ) ;
                  break ;
               }
            #endif
         }

         if ( tagData[i].expression == 0 )
         {
            error4describe( c4, e4tagInfo, E85303, tagData[i].name, tagData[i].expression, 0 ) ;
            break ;
         }

         tFile->expr = expr4parseLow( d4, tagData[i].expression, tFile ) ;
         if ( tFile->expr == 0 )
         {
            if ( error4code( c4 ) >= 0 )
               error4( c4, e4memory, E95301 ) ;
            break ;
         }
         if ( expr4type( tFile->expr ) == r4log )  /* disallowed in MDX */
         {
            error4( c4, e4tagInfo, E82901 ) ;
            break ;
         }

         if ( tagData[i].filter != 0 )
            if ( *(tagData[i].filter) != '\0' )
               tFile->filter = expr4parseLow( d4, tagData[i].filter, tFile ) ;

         if ( error4code( c4 ) < 0 )
            break ;
         l4add( &indexFile->tags, tFile ) ;
         l4add( &i4->tags, tagPtr ) ;
      }

      if ( error4code( c4 ) < 0 )
      {
         if ( tagPtr != 0 )
         {
            if ( tFile != 0 )
               mem4free( c4->tagFileMemory, tFile ) ;
            mem4free( c4->tagMemory, tagPtr ) ;
         }
         /* mark the file as temporary so it gets deleted since creation failed */
         indexFile->file.isTemp = 1 ;
         i4closeLow( i4 );
         return 0 ;
      }

      if ( indexFile->header.numTags > 47 )
      {
         /* mark the file as temporary so it gets deleted since creation failed */
         indexFile->file.isTemp = 1 ;
         i4closeLow( i4 );
         error4describe( c4, e4tagInfo, E85306, fileName, 0, 0 ) ;
         return 0 ;
      }
   #endif

   i4->indexFile = indexFile ;
   rc = i4reindex( i4 ) ;

   if ( rc == r4unique || rc < 0 )
   {
      error4set( c4, r4unique ) ;
      /* mark the file as temporary so it gets deleted since creation failed */
      indexFile->file.isTemp = 1 ;
      i4closeLow( i4 ) ;
      return 0 ;
   }

   if ( rc == r4locked )   /* means data file was not opened exclusive and other users using - cannot succeed on create */
   {
      error4set( c4, e4lock ) ;
      /* mark the file as temporary so it gets deleted since creation failed */
      indexFile->file.isTemp = 1 ;
      i4closeLow( i4 ) ;
      return 0 ;
   }

   if ( fileName == 0 )
      if ( error4code( c4 ) >= 0 && error4code( c4 ) != r4unique )
      {
         data->hasMdxMemo |= 1 ;  /* or for fox 3.0 which uses this setting with 0x02 bit for memo */
         data->openMdx = 1 ;

         file4longAssign( pos, ( 4 + sizeof( S4LONG ) + 2 * sizeof( short ) + sizeof( char[16] ) ), 0 ) ;
         #ifdef S4FOX
            file4writeInternal( &data->file, pos, &(data->hasMdxMemo), sizeof( char ) ) ;
         #else
            file4writeInternal( &data->file, pos, &(data->hasMdxMemo), sizeof( data->hasMdxMemo ) ) ;
         #endif
      }

   if ( error4code( c4 ) < 0 || error4code( c4 ) == r4unique )
   {
      /* mark the file as temporary so it gets deleted since creation failed */
      indexFile->file.isTemp = 1 ;
      i4closeLow( i4 ) ;
      return 0 ;
   }

   #ifndef S4OFF_TRAN
      i4->isValid = 1 ;
   #endif
   indexFile->isValid = 1 ;
   return i4 ;
}

#else /* S4CLIPPER */

static INDEX4 *i4createLow( DATA4 *d4, const char *fileName, const TAG4INFO *tagData )
{
   INDEX4 *i4 ;
   CODE4 *c4 ;
   char buf[258], ext[4], name[10] ;
   int i, rc, flen ;
   char buffer[1024] ;
   FILE4SEQ_WRITE seqwrite ;
   FILE4LONG pos ;

   #ifdef E4PARM_HIGH
      if ( fileName )
      {
         if ( d4index( d4, fileName ) )
         {
            error4describe( d4->codeBase, e4name, E81703, fileName, 0, 0 ) ;
            return 0 ;
         }
         u4namePiece( buf, sizeof( buf ), fileName, 0, 1 ) ;
      }
   #endif

   c4 = d4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return 0 ;
   error4set( c4, 0 ) ;  /* Make sure it is not 'r4unique' or 'r4noCreate'. */

   if ( fileName != 0 )
      if ( code4indexFormat( c4 ) == r4ntx )   /* disallow .ntx extensions */
      {
         u4nameRetExt( ext, 3, fileName ) ;
         #ifndef S4CASE_SEN
            if ( memcmp( ext, "NTX", 3 ) == 0 )
         #else
            if ( memcmp( ext, "ntx", 3 ) == 0 )
         #endif
         {
            ext[sizeof(ext)-1] = 0 ;
            error4describe( c4, e4name, E81720, fileName, ext, 0 ) ;
            return 0 ;
         }
      }

   #ifndef S4SINGLE
      if ( d4lockFile( d4 ) != 0 )
         return 0 ;
   #endif

   i4 = (INDEX4 *)mem4createAlloc( c4, &c4->indexMemory, c4->memStartIndex, sizeof(INDEX4), c4->memExpandIndex, 0 ) ;

   if ( i4 == 0 )
   {
      #ifdef E4STACK
         error4stack( c4, e4memory, E95301 ) ;
      #endif
      return 0 ;
   }

   i4->codeBase = c4 ;
   i4->data = d4 ;

   memset( buf, 0, sizeof( buf ) ) ;

   if ( fileName )  /* create a group file */
   {
      u4ncpy( buf, fileName, sizeof( buf ) ) ;
      u4ncpy( i4->accessName, fileName, sizeof( i4->accessName ) ) ;
      c4upper( i4->accessName ) ;

      #ifndef S4CASE_SEN
         u4nameExt( buf, sizeof( buf ), "CGP", 0 ) ;
         c4upper( buf ) ;
      #else
         u4nameExt( buf, sizeof( buf ), "cgp", 0 ) ;
      #endif

      rc = file4create( &i4->file, c4, buf, 1 ) ;
      if ( rc )
      {
         if ( rc > 0 )
            error4set( c4, rc ) ;

         file4close( &i4->file ) ;
         return 0 ;
      }

      file4longAssign( pos, 0, 0 ) ;
      file4seqWriteInitLow( &seqwrite, &i4->file, pos, buffer, sizeof( buffer ) ) ;

      /* create the group file */
      for ( i = 0; tagData[i].name; i++ )
      {
         u4namePiece(name, 10, tagData[i].name, 0, 0 ) ;
         c4upper(name) ;
         flen = strlen(name) ;
         file4seqWrite( &seqwrite, tagData[i].name, strlen( tagData[i].name ) - flen ) ;
         file4seqWrite( &seqwrite, name, flen ) ;
         file4seqWrite( &seqwrite, "\r\n", 2 ) ;
      }

      file4seqWriteFlush( &seqwrite ) ;

      file4close ( &i4->file ) ;

      #ifndef S4SINGLE
         if ( rc )
         {
            if ( rc > 0 )
               error4set( c4, rc ) ;
            return 0 ;
         }
      #endif
      i4->path = buf ;
   }
   else
   {
      u4ncpy( i4->accessName, d4->alias, sizeof( i4->accessName ) ) ;
      c4upper( i4->accessName ) ;
   }

   l4add( &d4->indexes, i4 ) ;

   /* now create the actual tag files */
   for ( i = 0 ; tagData[i].name ; i++ )
      if ( t4create( d4, &tagData[i], i4, (c4->createTemp == 1 && fileName == 0 ) ? 1 : 0 ) == 0 )
      {
         i4closeLow( i4 ) ;
         return 0 ;
      }

   if ( error4code( c4 ) < 0 || error4code( c4 ) == r4unique )
   {
      i4closeLow( i4 ) ;
      return 0 ;
   }

   rc = i4reindex( i4 ) ;
   if ( rc == r4unique || rc < 0 )
   {
      i4closeLow( i4 ) ;
      return 0 ;
   }

   #ifndef S4OFF_TRAN
      i4->isValid = 1 ;
   #endif
   return i4 ;
}

/* this function does not reindex if an 'i4ndx' is passed as a parameter */
/* this allows several creations before an actual reindex must occur */
/* if useTempTagFileNames is true then the tag files created use temporary names */
TAG4 *S4FUNCTION t4create( DATA4 *d4, const TAG4INFO *tagData, INDEX4 *i4ndx, int useTempTagFileNames )
{
   CODE4  *c4 ;
   INDEX4 *i4 ;
   char   buf[258] ;
   TAG4 *t4 ;
   TAG4FILE *tfile ;
   int rc, oldTagErr ;
   #ifndef S4OFF_OPTIMIZE
      #ifdef S4LOW_MEMORY
         int hasOpt ;
      #endif
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E95302 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( d4 == 0 || tagData == 0 )
      {
         error4( 0, e4parm, E95302 ) ;
         return 0 ;
      }
      u4namePiece( buf, sizeof( buf ), tagData->name, 0, 0 ) ;
   #endif

   c4 = d4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return 0 ;

   oldTagErr = c4->errTagName ;
   c4->errTagName = 0 ;
   if ( d4tag( d4, buf ) )
   {
      error4describe( c4, e4name, E85308, buf, 0, 0 ) ;
      c4->errTagName = oldTagErr ;
      return 0 ;
   }
   c4->errTagName = oldTagErr ;

   error4set( c4, 0 ) ;  /* Make sure it is not 'r4unique' or 'r4noCreate'. */

   #ifndef S4OFF_OPTIMIZE
      #ifdef S4LOW_MEMORY
         if ( c4->hasOpt )
         {
            hasOpt = 1 ;
            code4optSuspend( c4 ) ;
         }
         else
            hasOpt = 0 ;
      #endif
   #endif

   if ( i4ndx == 0 )   /* must create an index for the tag */
   {
      if ( c4->indexMemory == 0 )
         c4->indexMemory = mem4create( c4, c4->memStartIndex, sizeof(INDEX4),
                                        c4->memExpandIndex, 0 ) ;
      if ( c4->indexMemory == 0 )
         return 0 ;
      i4 = (INDEX4 *) mem4alloc( c4->indexMemory ) ;
      if ( i4 == 0 )
      {
         #ifdef E4STACK
            error4stack( c4, e4memory, E95302 ) ;
         #endif
         return 0 ;
      }
      i4->data = d4 ;
      i4->codeBase = c4 ;
      #ifdef E4PARM_HIGH
         u4namePiece( buf, sizeof( buf ), i4->accessName, 0, 0 ) ;
         if ( d4index( d4, buf ) )
         {
            error4( d4->codeBase, e4parm, E81704 ) ;
            return 0 ;
         }
      #endif
      u4namePiece( i4->accessName, sizeof( i4->accessName ), tagData->name, 0, 0 ) ;
      c4upper( i4->accessName ) ;
   }
   else
      i4 = i4ndx ;

   if ( c4->tagMemory == 0 )
   {
      c4->tagMemory = mem4create( c4, c4->memStartTag, sizeof(TAG4), c4->memExpandTag, 0 ) ;
      if ( c4->tagMemory == 0 )
         return 0 ;
   }

   if ( c4->tagFileMemory == 0 )
   {
      c4->tagFileMemory = mem4create( c4, c4->memStartTagFile, sizeof(TAG4FILE), c4->memExpandTagFile, 0 ) ;
      if ( c4->tagFileMemory == 0 )
         return 0 ;
   }

   t4 = (TAG4 *)mem4alloc( c4->tagMemory ) ;
   if ( t4 == 0 )
   {
      #ifdef E4STACK
         error4stack( c4, e4memory, E95302 ) ;
      #endif
      return 0 ;
   }

   t4->index = i4 ;

   t4->tagFile = (TAG4FILE *) mem4alloc( c4->tagFileMemory ) ;
   if ( t4->tagFile == 0 )
   {
      #ifdef E4STACK
         error4stack( c4, e4memory, E95302 ) ;
      #endif
      return 0 ;
   }

   tfile = t4->tagFile ;
   tfile->codeBase = c4 ;
   tfile->userCount = 1 ;

   if ( tfile->blockMemory == 0 )
      tfile->blockMemory = mem4create( c4, c4->memStartBlock, sizeof(B4BLOCK) + B4BLOCK_SIZE -
         (sizeof(B4KEY_DATA)) - (sizeof(short)) - (sizeof(char[2])), c4->memExpandBlock, 0 ) ;

   if ( tfile->blockMemory == 0 )
   {
      if ( tfile != 0 )
         mem4free( c4->tagFileMemory, tfile ) ;
      mem4free( c4->tagMemory, t4 ) ;
      return 0 ;
   }

   if ( i4ndx != 0 && i4->path != 0)
   {
      rc = u4namePath( buf, sizeof(buf), i4ndx->path ) ;
      u4ncpy( buf+rc, tagData->name, sizeof(buf)-rc - 1 ) ;
   }
   else
      u4ncpy( buf, tagData->name, sizeof(buf) - 1 ) ;

   c4upper(buf) ;

   #ifdef S4CASE_SEN
      u4nameExt( buf, sizeof(buf), "ntx", 0 ) ;
   #else
      u4nameExt( buf, sizeof(buf), "NTX", 0 ) ;
   #endif

   u4namePiece( tfile->alias, sizeof( tfile->alias ), tagData->name, 0, 0 ) ;
   #ifndef S4CASE_SEN
      c4upper( tfile->alias ) ;
   #endif

   #ifdef E4ANALYZE
      if ( tfile == 0 )
      {
         mem4free( c4->tagMemory, t4 ) ;
         error4( c4, e4info, E95302 ) ;
         return 0 ;
      }
   #endif

   if ( useTempTagFileNames )
      rc = file4create( &tfile->file, c4, 0, 1 ) ;
   else
      rc = file4create( &tfile->file, c4, buf, 1 ) ;
   if ( rc )
   {
      t4close( t4 ) ;
      if ( rc > 0 )
         error4set( c4, rc ) ;
      return 0 ;
   }

   #ifndef S4OFF_OPTIMIZE
      file4optimizeLow( &tfile->file, c4->optimize, OPT4INDEX, 0, tfile ) ;
      #ifdef S4LOW_MEMORY
         if ( hasOpt )
            code4optRestart( c4 ) ;
      #endif
   #endif

   if ( tagData->unique )
   {
      tfile->header.unique = 0x01 ;

      t4->errUnique = tagData->unique ;

      #ifdef E4PARM_HIGH
         #ifdef S4FOX
            if ( tagData->unique != e4unique && tagData->unique != r4unique
                 tagData->unique != r4uniqueContinue && tagData->unique != r4candidate &&
                 tagData[i].unique != e4candidate )
         #else
            if ( tagData->unique != e4unique && tagData->unique != r4unique && tagData->unique != r4uniqueContinue )
         #endif
         {
            t4close( t4 ) ;
            error4describe( c4, e4tagInfo, E85301, tagData->name, 0, 0 ) ;
            return 0 ;
         }
      #endif
   }

   if ( tagData->descending)
   {
      tfile->header.descending = 1 ;
      #ifdef E4PARM_HIGH
         if ( tagData->descending != r4descending )
         {
            t4close( t4 ) ;
            error4describe( c4, e4tagInfo, E85302, tagData->name, 0, 0 ) ;
            return 0 ;
         }
      #endif
   }

   #ifdef E4PARM_HIGH
      if ( tagData->expression == 0 )
      {
         t4close( t4 ) ;
         error4describe( c4, e4tagInfo, E85303, tagData->name, tagData->expression, 0 ) ;
         return 0 ;
      }
   #endif

   if ( strlen( tagData->expression ) > I4MAX_EXPR_SIZE )
   {
      t4close( t4 ) ;
      error4describe( c4, e4tagInfo, E85304, tagData->name, tagData->expression, 0 ) ;
      return 0 ;
   }
   tfile->expr = expr4parseLow( d4, tagData->expression, tfile ) ;
   if ( tfile->expr == 0 )
   {
      t4close( t4 ) ;
      return 0 ;
   }

   if ( tagData->filter != 0 )
   {
      if ( strlen( tagData->filter ) > I4MAX_EXPR_SIZE )
      {
         t4close( t4 ) ;
         error4describe( c4, e4tagInfo, E85304, tagData->name, tagData->filter, 0 ) ;
         return 0 ;
      }
      if ( *( tagData->filter ) != '\0' )
         tfile->filter = expr4parseLow( d4, tagData->filter, tfile ) ;
   }

   tfile->header.eof = 0 ;
   tfile->header.root = 1024 ;
   tfile->header.keyLen = c4->numericStrLen ;
   tfile->header.keyDec = c4->decimals ;
   if ( tfile->expr->type == r4num )
   {
      tfile->header.keyLen = tfile->expr->keyLen ;
      tfile->header.keyDec = tfile->expr->keyDec ;
   }

   if ( error4code( c4 ) < 0 )
   {
      t4close( t4 ) ;
      return 0 ;
   }

   /* add the tag to the index list */
   l4add( &i4->tags, t4 ) ;

   if ( i4ndx == 0 )   /* single create, so reindex now */
   {
      if ( t4reindex( t4 ) == r4unique )
      {
         t4close( t4 ) ;
         error4set( c4, r4unique ) ;
         return 0 ;
      }

      l4add( &i4->data->indexes, i4 ) ;
   }

   l4add( &d4->dataFile->tagfiles, tfile ) ;
   if ( d4->dataFile->indexLocked == 1 )   /* index locked, so lock this tag as well */
      tfile4lock( tfile, data4serverId( d4 ) ) ;
   #ifndef S4OFF_TRAN
      t4->isValid = 1 ;
   #endif
   return t4 ;
}
#endif   /* S4CLIPPER */

#endif   /* S4OFF_WRITE */
#endif   /* S4OFF_INDEX */

#ifdef S4VB_DOS

INDEX4 * i4create_v ( DATA4  *d4, char near *name, TAG4INFO *t4 )
{
   return i4create( d4, c4str(name), t4 ) ;
}

INDEX4 * i4createProd ( DATA4  *d4, TAG4INFO *t4 )
{
   return i4create( d4, 0, t4 ) ;
}

#endif
