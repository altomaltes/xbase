/* d4opt.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_OPTIMIZE
   /* ensure we have at least 8 buffers -- 4 for special purpose, 4 for general purpose */
   #define MEM4MIN_BUFFERS 8
   static void opt4freeAlloc( OPT4 * ) ;
   static int opt4initAlloc( CODE4 *, int ) ;
   #ifdef E4ANALYZE_ALL
      int file4copyx( CODE4 *, FILE4 *, char * ) ;
   #endif
#endif

#ifndef S4OFF_OPTIMIZE
/* if possible, it gets the physical amount of available memory and sets the
   memStartMax to a percentage of that amount (the input percent)
   It won't auto-start optimizatino if it was suspended physically */
void code4memStartMaxSet( CODE4 *c4, const int percent )
{
   double availMem ;
   #ifdef __WIN32
      MEMORYSTATUS memory;
   #endif

   if ( c4->hadOpt == 1 || c4->opt.numBuffers || c4->hasOpt )   /* don't enable if suspend was requested or is already initialized */
      return ;

   availMem = 0 ;

   #ifdef __WIN32
      memory.dwLength = sizeof( MEMORYSTATUS ) ;   /* reqd by Windows to set */
      GlobalMemoryStatus( &memory ) ;
      availMem = memory.dwAvailPhys ;
   #endif

   if ( availMem != 0 && percent <= 100 && percent >= 0 )
   {
      c4->memStartMax = (long) ( ((double)percent / 100) * availMem ) ;
      code4optStart( c4 ) ;
   }
   else
      c4->hadOpt = 1 ;    /* had chance to enable optimization but it was denied */

   return ;
}
#endif

static int code4freeBlocks( CODE4 *c4 )
{
      DATA4 *data ;

   #ifdef E4PARM_HIGH
      if ( c4 == 0 )
         return error4( 0, e4parmNull, E92510 ) ;
   #endif

      for ( data = 0 ;; )
      {
         data = (DATA4 *)l4next( tran4dataList( &c4->c4trans.trans ), data ) ;
         if ( data == 0 )
            break ;
         d4freeBlocks( data ) ;
      }

   return 0 ;
}

#ifndef S4OFF_OPTIMIZE
int S4FUNCTION code4optAll( CODE4 *c4 )
{
   LIST4 *list ;
   int rc ;
   DATA4FILE *dfile ;
   DATA4 *data ;

   #ifdef E4PARM_HIGH
      if ( c4 == 0 )
         return error4( 0, e4parmNull, E92531 ) ;
   #endif

   list = tran4dataList( code4trans( c4 ) ) ;
   for ( dfile = 0 ;; )
   {
      dfile = (DATA4FILE *)l4next( &c4->dataFileList, dfile ) ;
      if ( dfile == 0 )
         break ;
      #ifdef S4OPTIMIZE_STATS
         /* don't optimize the stat file or nested d4appends occur */
         if ( c4->statusDbf != 0 )
            if ( &dfile->file == &c4->statusDbf->dataFile->file )  /* don't do for the stat file! */
               continue ;
      #endif
      for ( data = 0 ;; )
      {
         data = (DATA4 *)l4next( list, data ) ;
         if ( data == 0 )
         {
            #ifdef E4ANALYZE
               #ifndef S4OFF_MULTI
                  code4lockClear( c4 ) ;
               #endif
               return error4( c4, e4info, E92531 ) ;
            #else
               break ;
            #endif
         }
         if ( data->dataFile == dfile )
         {
            #ifndef S4OFF_MULTI
               rc = d4lockAddAll( data ) ;
               if ( rc != 0 )
                  return rc ;
            #endif
            rc = d4optimize( data, OPT4ALL ) ;
            if ( rc != 0 )
               return rc ;
            rc = d4optimizeWrite( data, OPT4ALL ) ;
            if ( rc != 0 )
               return rc ;
            break ;
         }
      }
   }

   #ifndef S4OFF_MULTI
      rc = code4lock( c4 ) ;
      if ( rc != 0 )
         return rc ;
   #endif

   code4optStart( c4 ) ;

   return 0 ;
}
#endif /* S4OFF_OPTIMIZE */

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION code4optStart( CODE4 *c4 )
{
   #ifndef S4OFF_OPTIMIZE
      int rc ;

      #ifdef E4PARM_HIGH
         if ( c4 == 0 )
            return error4( 0, e4parmNull, E92501 ) ;
      #endif

      rc = code4optRestart( c4 ) ;
         if ( rc == 0 )
            code4freeBlocks( c4 ) ;

      return rc ;
   #else
      return 0 ;
   #endif /* S4OFF_OPTIMIZE */
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int code4optRestart( CODE4 *c4 )
{
   #ifndef S4OFF_OPTIMIZE
      OPT4 *opt ;
      unsigned numBuffers ;
      int numAlloc ;
      FILE4 *fileOn ;
      FILE4LONG len ;
      double hitCountAdd ;

      #ifdef __WIN32
         DWORD sectorsPerCluster, bytesPerSector, numberFreeClusters, totalNumberClusters ;
      #endif

      C4PARMCODE( c4, E92502, -1 );

      #ifdef E4ANALYZE
         if ( c4->debugInt != 0x5281 )
            return error4( 0, e4struct, E81301 ) ;
      #endif

      if ( error4code( c4 ) < 0 )
         return e4codeBase ;

      opt = &c4->opt ;

      if ( opt->numBuffers || c4->hasOpt )  /* no initialization required */
         return 0 ;

      #ifdef __WIN32
         /* set block sizes based on sector-size values */
         /* if used, final version must address different drives for different
            files */
         /* use current directory for now */
         if ( GetDiskFreeSpace( 0, &sectorsPerCluster
                              , &bytesPerSector
                              , &numberFreeClusters
                              , &totalNumberClusters ) == TRUE )
         {
            if ( c4->memSizeBlock < bytesPerSector )  /* ok if larger than sector size */
               c4->memSizeBlock = bytesPerSector ;
            else
            {
               /* round down to nearest multiple sector size */
               c4->memSizeBlock = bytesPerSector * (c4->memSizeBlock / bytesPerSector ) ;
               /* round down to nearest multiple block size */
               c4->memSizeBuffer = c4->memSizeBlock * ( c4->memSizeBuffer / c4->memSizeBlock ) ;
            }
            if ( c4->memSizeBlock > c4->memSizeBuffer )
               c4->memSizeBuffer = c4->memSizeBlock ;
         }
      #endif

      #ifdef E4ANALYZE
         if ( c4->memSizeBlock == 0 || c4->memSizeBuffer == 0 || c4->memSizeBlock > c4->memSizeBuffer )
            return error4( c4, e4struct, E82501 ) ;
      #endif

      opt->blockSize = c4->memSizeBlock ;

      if ( c4->memSizeBlock != 0 )
         opt->bufferSize = (unsigned long)(c4->memSizeBlock * ( (unsigned long)c4->memSizeBuffer / c4->memSizeBlock )) ;
      else
         opt->bufferSize = 0 ;

      opt->hashTrail = 0 ;
      opt->prio[0] = &opt->other ;
      opt->prio[1] = &opt->dbfLo ;
      opt->prio[2] = &opt->indexLo ;
      opt->prio[3] = &opt->dbfHi ;
      opt->prio[4] = &opt->indexHi ;
      opt->doUpdate = 1 ;
      opt->checkCount = OPT4CHECK_RATE ;   /* set to do analysis when first block removed */

      for( ; ( (numBuffers = (unsigned int)((unsigned long)c4->memStartMax / (unsigned long)(opt->bufferSize - 2UL) )) < MEM4MIN_BUFFERS) ; )
      {
         opt->bufferSize -= opt->blockSize ;
         if ( opt->bufferSize == 0 )
            return -1 ;
      }

      numBuffers -= 4 ;   /* 4 special use buffers are taken into account elsewhere */

      for( ;; )
      {
         c4->hasOpt = 1 ;
         opt->minLink = opt->maxBlocks = (unsigned int)( opt->bufferSize / opt->blockSize ) ;
         opt->blockPower = (char)c4calcType( opt->blockSize ) ;
         opt->numShift = (char)(8*sizeof(S4LONG ) - (opt->blockPower)) ;
         opt->numLists = OPT4BLOCK_DENSITY << c4calcType( (long)numBuffers * opt->maxBlocks ) ;

         numAlloc = opt4initAlloc( c4, numBuffers ) ;

         if ( numAlloc <= 0 )
         {
            code4optSuspend( c4 ) ;
            return -1 ;
         }

         opt->numBuffers = numAlloc ;

         if ( numAlloc < 4 )   /* couldn't do a minimum allocation, try again */
         {
            opt4freeAlloc( opt ) ;   /* free allocs */
            if ( numBuffers > 4 )
               numBuffers = 4 ;
            opt->bufferSize /= 2 ;
            opt->bufferSize = opt->bufferSize - opt->bufferSize % opt->blockSize ;  /* round it down to a blockSize multiple */
            if ( opt->bufferSize == 0 )
               return -1 ;
            continue ;
         }
         break ;
      }

      opt->numBlocks = (unsigned long)opt->numBuffers * opt->maxBlocks ;
      opt->numLists = OPT4BLOCK_DENSITY << c4calcType( opt->numBlocks ) ;
      opt->mask = opt->numLists - 1 ;

      /* now actually optimize those files reqd */
      for ( fileOn = 0 ;; )
      {
         fileOn = (FILE4 *)l4next( &opt->optFiles, fileOn ) ;
         if ( fileOn == 0 )
            break ;
         file4longAssignError( fileOn->len ) ;   /* in case the file length changed during suspension */
         len = file4lenLow( fileOn ) ;
         /* if either error or > 4 Gigs, either case we don't optimize */
         if ( file4longGetHi( len ) == 0 )  /* file is ok to optimize */
         {
            fileOn->doBuffer = 1 ;   /* re-add the reference */
            #ifndef S4SINGLE
               if ( fileOn->lowAccessMode == OPEN4DENY_RW )
            #endif
            file4setWriteOpt( fileOn, 1 ) ;

            if ( fileOn->type == OPT4DBF )
            {
               hitCountAdd = (double)fileOn->expectedReadSize / (double)opt->blockSize ;
               if ( hitCountAdd > 1.0 )
                   fileOn->hitCountAdd = 1.0 ;
                else
                   fileOn->hitCountAdd = hitCountAdd ;
            }

            if ( fileOn->hashInit == -1 )
            {
               #ifdef E4ANALYZE
                  FILE4LONG lPos ;
               #endif
               fileOn->hashInit = opt->hashTrail * opt->blockSize ;
               #ifdef E4ANALYZE
                  lPos = file4lenLow( fileOn ) ;
                  if ( ( file4longError( lPos ) < 0L ) || ( opt->blockSize == 0 ) )
                     return error4( c4, e4info, E92502 ) ;
               #endif
               opt->hashTrail = (opt->hashTrail + file4longGetLo( len ) / opt->blockSize) % opt->numBlocks ;
            }
            #ifdef E4ANALYZE_ALL
               file4copyx( c4, fileOn, fileOn->dupName ) ;
            #endif
         }
      }

      opt->minAccessTimeVariation = (unsigned int)opt->numBlocks / 100 ;
      if ( opt->minAccessTimeVariation < 2 )  /* provide a basic minimum */
         opt->minAccessTimeVariation = 2 ;
      opt->dbfLo.minLink  = (unsigned short int) ((double)opt->numBlocks * OPT4DBF_LO_MIN_LINK) ;
      opt->dbfLo.maxTime  = (unsigned      long) ((double)opt->numBlocks * OPT4DBF_LO_MAX_TIME) ;
      opt->dbfLo.minTime  = (unsigned      long) ((double)opt->numBlocks * OPT4DBF_LO_MIN_TIME) ;
      opt->dbfHi.minLink  = (unsigned short int) ((double)opt->numBlocks * OPT4DBF_HI_MIN_LINK) ;
      opt->dbfHi.maxTime  = (unsigned      long) ((double)opt->numBlocks * OPT4DBF_HI_MAX_TIME) ;
      opt->dbfHi.minTime  = (unsigned      long) ((double)opt->numBlocks * OPT4DBF_HI_MIN_TIME) ;
      opt->indexLo.minLink= (unsigned short int) ((double)opt->numBlocks * OPT4INDEX_LO_MIN_LINK) ;
      opt->indexLo.maxTime= (unsigned      long) ((double)opt->numBlocks * OPT4INDEX_LO_MAX_TIME) ;
      opt->indexLo.minTime= (unsigned      long) ((double)opt->numBlocks * OPT4INDEX_LO_MIN_TIME) ;
      opt->indexHi.minLink= (unsigned short int) ((double)opt->numBlocks * OPT4INDEX_HI_MIN_LINK) ;
      opt->indexHi.maxTime= (unsigned      long) ((double)opt->numBlocks * OPT4INDEX_HI_MAX_TIME) ;
      opt->indexHi.minTime= (unsigned      long) ((double)opt->numBlocks * OPT4INDEX_HI_MIN_TIME) ;
      opt->other.minLink  = (unsigned short int) ((double)opt->numBlocks * OPT4OTHER_MIN_LINK) ;
      opt->other.maxTime  = (unsigned      long) ((double)opt->numBlocks * OPT4OTHER_MAX_TIME) ;
      opt->other.minTime  = (unsigned      long) ((double)opt->numBlocks * OPT4OTHER_MIN_TIME) ;

   #endif /* S4OFF_OPTIMIZE */
   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION code4optSuspend( CODE4 *c4 )
{
   #ifndef S4OFF_OPTIMIZE
      OPT4 *opt ;
      FILE4 *fileOn ;
      int rc, saveRc ;

      C4PARMCODE( c4, E92503, -1 );

      opt = &c4->opt ;
      if ( opt->numBuffers == 0 || c4->hasOpt == 0 )
         return 0 ;

      rc = 0 ;
      saveRc = error4set( c4, 0 ) ;

      /* first remove any optimized files */
      for ( fileOn = 0 ;; )
      {
         fileOn = (FILE4 *)l4next( &opt->optFiles, fileOn ) ;
         if ( fileOn == 0 )
            break ;
         rc = opt4fileFlush( fileOn, 1 ) ;
         fileOn->doBuffer = 0 ;  /* remove the reference */
         file4setWriteOpt( fileOn, 0 ) ;
      }
      c4->hasOpt = 0 ;
      c4->hadOpt = 1 ;   /* indicate that at one time optimization was enabled */

      opt4freeAlloc( opt ) ;

      opt->numBuffers = 0 ;  /* mark as freed */

      if ( saveRc < 0 )
         error4set( c4, saveRc ) ;

      if ( rc < 0 )
         return error4stack( c4, e4optSuspend, E92503 ) ;
      else
   #endif
   return 0 ;
}

int S4FUNCTION d4optimize( DATA4 *d4, const int optFlag )
{
   C4PARMDATA( d4, E92504, -1 );

      #ifdef E4PARM_HIGH
         if ( optFlag < -1 || optFlag > 1 )
            return error4( 0, e4parm, E92504 ) ;
      #endif
      return dfile4optimize( d4->dataFile, optFlag ) ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int dfile4optimize( DATA4FILE *d4, const int optFlag )
{
   #ifndef S4OFF_OPTIMIZE
      int rc ;
      #ifndef S4OFF_INDEX
         #ifdef N4OTHER
            TAG4FILE *tagOn ;
         #else
            INDEX4FILE *indexOn ;
         #endif
      #endif

      #ifdef E4PARM_LOW
         if ( d4 == 0 || optFlag < -1 || optFlag > 1 )
            return error4( 0, e4parm, E91102 ) ;
      #endif

      if ( dfile4recWidth( d4 ) > d4->c4->opt.bufferSize )  /* don't optimize records larger than the buffer size */
      {
         rc = file4optimizeLow( &d4->file, optFlag, OPT4DBF, dfile4recWidth( d4 ), d4 ) ;
         if ( rc < 0 )
            return rc ;
      }

      #ifndef S4OFF_INDEX
         #ifdef N4OTHER
            for( tagOn = 0;;)
            {
               tagOn = dfile4tagNext( d4, tagOn ) ;
               if ( tagOn == 0 )
                  break ;

               rc = file4optimizeLow( &tagOn->file, optFlag, OPT4INDEX, 0, tagOn ) ;
               if ( rc < 0 )
                  return rc ;
            }
         #else
            for ( indexOn = 0 ;; )
            {
               indexOn = (INDEX4FILE *)l4next( &d4->indexes, indexOn ) ;
               if ( indexOn == 0 )
                  break ;
               rc = file4optimizeLow( &indexOn->file, optFlag, OPT4INDEX, 0, indexOn ) ;
               if ( rc < 0 )
                  return rc ;
            }
         #endif
      #endif

      #ifndef S4OFF_MEMO
         if ( d4->memoFile.file.hand != INVALID4HANDLE )
            return file4optimize( &d4->memoFile.file, optFlag, OPT4OTHER ) ;
      #endif

      return 0 ;
   #else
      return 0 ;
   #endif
}

int S4FUNCTION d4optimizeWrite( DATA4 *d4, const int optFlag )
{
   C4PARMDATA( d4, E92506, -1 );

      #ifdef E4PARM_HIGH
         if ( optFlag < -1 || optFlag > 1 )
            return error4( 0, e4parm, E92506 ) ;
      #endif
      return dfile4optimizeWrite( d4->dataFile, optFlag ) ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int dfile4optimizeWrite( DATA4FILE *d4, const int optFlag )
{
   #ifndef S4OFF_WRITE
      #ifndef S4OFF_OPTIMIZE
         int rc ;
         #ifndef S4OFF_INDEX
            #ifdef N4OTHER
               TAG4FILE *tagOn ;
            #else
               INDEX4FILE *indexOn ;
            #endif
         #endif

         #ifdef E4PARM_LOW
            if ( d4 == 0 || optFlag < -1 || optFlag > 1 )
               return error4( 0, e4parm, E91102 ) ;
         #endif

         rc = file4optimizeWrite( &d4->file, optFlag ) ;
         if ( rc < 0 )
            return error4stack( d4->c4, rc, E91102 ) ;

         #ifndef S4OFF_MEMO
            if ( d4->memoFile.file.hand != INVALID4HANDLE )
            {

               rc = file4optimizeWrite( &d4->memoFile.file, optFlag ) ;
               if ( rc < 0 )
                  return error4stack( d4->c4, rc, E91102 ) ;
            }
         #endif

         #ifndef S4OFF_INDEX
            #ifndef N4OTHER
               indexOn = (INDEX4FILE *) l4first( &d4->indexes ) ;
               if ( indexOn != 0 )
                  do
                  {
                     rc = file4optimizeWrite( &indexOn->file, optFlag ) ;
                     if ( rc < 0 )
                        return error4stack( d4->c4, rc, E91102 ) ;
                     indexOn = (INDEX4FILE *)l4next( &d4->indexes, indexOn ) ;
                  } while ( indexOn != 0 ) ;
            #else
               for( tagOn = 0;;)
               {
                  tagOn = dfile4tagNext( d4, tagOn ) ;
                  if ( tagOn == 0 )
                     break ;
                  rc = file4optimizeWrite( &tagOn->file, optFlag ) ;
                  if ( rc < 0 )
                     return error4stack( d4->c4, rc, E91102 ) ;
               }
            #endif
         #endif
      #endif
   #endif
   return 0 ;
}

#ifndef S4OFF_OPTIMIZE
static void opt4freeAlloc( OPT4 *opt )
{
   OPT4BLOCK *curBlock ;
   int i ;
   #ifdef S4ADVANCE_READ
      FILE4ADVANCE_READ *advanceRead ;
      LINK4 *advanceLink ;
      FILE4 *f4 ;
   #endif

   E4PARMLOW( opt, E92508 ) ;

   #ifdef S4WRITE_DELAY
      if ( opt->delayWriteBuffer != 0 )
      {
         while ( l4numNodes( &opt->delayAvail ) != opt->maxBlocks )  /* wait for delay-write to finish on blocks */
            Sleep( 0 ) ;

         for ( i = 0 ; i < (int)opt->maxBlocks ; i++ )
         {
            curBlock = &opt->blocks[ opt->numBuffers * opt->maxBlocks + i] ;
            l4remove( &opt->delayAvail, &curBlock->lruLink ) ;
         }

         DeleteCriticalSection( &opt->critical4optWrite ) ;
         u4free( opt->delayWriteBuffer ) ;
         opt->delayWriteBuffer = 0 ;
      }
   #endif

   #ifdef S4ADVANCE_READ
      if ( opt->advanceLargeBuffer != 0 )
      {
         /* just cancel it */
         EnterCriticalSection( &opt->critical4optRead ) ;
         f4 = opt->advanceReadFile ;
         if ( f4 != 0 )
            if ( l4numNodes( &f4->advanceReadFileList ) != 0 )
            {
               EnterCriticalSection( &f4->critical4file ) ;
               for ( advanceLink = (LINK4 *)l4first( &f4->advanceReadFileList ) ;; )
               {
                  if ( advanceLink == 0 )
                     break ;
                  advanceRead = (FILE4ADVANCE_READ *)(advanceLink - 1 ) ;
                  advanceLink = (LINK4 *)l4next( &f4->advanceReadFileList, advanceLink ) ;

                  if ( advanceRead->data == opt->advanceLargeBuffer )  /* found the one we want */
                     break ;
               }

               if ( advanceRead != 0 )  /* have the advance-read, just remove it */
               {
                  while ( advanceRead->usageFlag == r4inUse )  /* is being read, so wait */
                     Sleep( 0 ) ;
                  if ( advanceRead->usageFlag == r4queued )  /* remove ourselves */
                  {
                     EnterCriticalSection( &advanceRead->file->codeBase->critical4advanceReadList ) ;
                     l4remove( &advanceRead->file->codeBase->advanceReadList, advanceRead ) ;
                     l4remove( &advanceRead->file->advanceReadFileList, &advanceRead->fileLink ) ;
                     LeaveCriticalSection( &advanceRead->file->codeBase->critical4advanceReadList ) ;
                  }
               }
               LeaveCriticalSection( &f4->critical4file ) ;
            }

         LeaveCriticalSection( &opt->critical4optRead ) ;
         DeleteCriticalSection( &opt->critical4optRead ) ;
         u4free( opt->advanceLargeBuffer ) ;
         opt->advanceLargeBuffer = 0 ;
      }
   #endif

   opt4flushAll( opt, 1 ) ;  /* move all blocks over to avail list */

   if ( opt->buffers )
   {
      for ( --opt->numBuffers; opt->numBuffers >= 0 ; opt->numBuffers-- )
      {
         for ( i = 0 ; i < (int)opt->maxBlocks ; i++ )
         {
            curBlock = &opt->blocks[ opt->numBuffers * opt->maxBlocks + i] ;
            l4remove( &opt->avail, &curBlock->lruLink ) ;
         }
         if ( opt->buffers[opt->numBuffers] != 0 )
            u4free( opt->buffers[opt->numBuffers] ) ;
      }
      u4free( opt->buffers ) ;
      opt->buffers = 0 ;
   }

   opt->writeBuffer = 0 ;
   #ifdef S4WRITE_DELAY
      u4free( opt->delayLargeBuffer ) ;
      opt->delayLargeBuffer = 0 ;
   #endif
   u4free( opt->writeBufferActual ) ;
   opt->writeBufferActual = 0 ;
   u4free( opt->readBuffer ) ;
   opt->readBuffer = 0 ;
   u4free( opt->blocks ) ;
   opt->blocks = 0 ;
   u4free( opt->lists ) ;
   opt->lists = 0 ;
   u4free( opt->lists ) ;
   opt->lists = 0 ;
}

static int opt4initAlloc( CODE4 *c4, int numBuffers )
{
   OPT4BLOCK *curBlock ;
   OPT4 *opt ;
   int numAlloc, i ;

   #ifdef E4PARM_LOW
      if ( c4 == 0 || numBuffers < 0 )
         return error4( c4, e4parm, E92508 ) ;
   #endif

   opt = &c4->opt ;

   opt->buffers = (void **)u4alloc( ( (long)numBuffers ) * sizeof( void * ) ) ;
   if ( opt->buffers == 0 )
   {
      code4optSuspend( c4 ) ;
      return error4stack( c4, e4memory, E92508 ) ;
   }

   opt->lists = (LIST4 *)u4alloc( opt->numLists * sizeof (LIST4) ) ;
   if ( opt->lists == 0 )
   {
      code4optSuspend( c4 ) ;
      return error4stack( c4, e4memory, E92508 ) ;
   }

   #ifdef S4WRITE_DELAY
      #ifdef S4ADVANCE_READ
         opt->blocks = (OPT4BLOCK *)u4alloc( (long)(numBuffers + 2) * opt->maxBlocks * sizeof( OPT4BLOCK ) ) ;
      #else
         opt->blocks = (OPT4BLOCK *)u4alloc( (long)(numBuffers + 1) * opt->maxBlocks * sizeof( OPT4BLOCK ) ) ;
      #endif
   #else
      #ifdef S4ADVANCE_READ
         opt->blocks = (OPT4BLOCK *)u4alloc( (long)(numBuffers + 1) * opt->maxBlocks * sizeof( OPT4BLOCK ) ) ;
      #else
         opt->blocks = (OPT4BLOCK *)u4alloc( (long)numBuffers * opt->maxBlocks * sizeof( OPT4BLOCK ) ) ;
      #endif
   #endif
   if ( opt->blocks == 0 )
   {
      code4optSuspend( c4 ) ;
      return error4stack( c4, e4memory, E92508 ) ;
   }

   opt->writeBufferActual = (char *)u4alloc( opt->bufferSize ) ;
   if( opt->writeBufferActual == 0 )
      return 0 ;
   opt->writeBuffer = opt->writeBufferActual ;

   opt->writeBlockCount = 0 ;
   opt->writeCurPos = 0 ;
   opt->writeStartPos = 0 ;
   opt->writeFile = 0 ;

   opt->readBuffer = (char *)u4alloc( opt->bufferSize ) ;
   if( opt->readBuffer == 0 )
      return 0 ;

   #ifdef S4WRITE_DELAY
      opt->delayWriteBuffer = (char *)u4alloc( opt->bufferSize ) ;
      if( opt->delayWriteBuffer == 0 )
         return 0 ;
      opt->delayLargeBuffer = (char *)u4alloc( opt->bufferSize ) ;
      if( opt->delayLargeBuffer == 0 )
         return 0 ;
      InitializeCriticalSection( &opt->critical4optWrite ) ;
      opt->delayLargeBufferAvail = 1 ;
      opt->writeBufferActualAvail = 1 ;
   #endif
   #ifdef S4ADVANCE_READ
      opt->advanceLargeBuffer = (char *)u4alloc( opt->bufferSize ) ;
      if( opt->advanceLargeBuffer == 0 )
         return 0 ;
      InitializeCriticalSection( &opt->critical4optRead ) ;
      opt->advanceLargeBufferAvail = 1 ;
   #endif

   for ( numAlloc = 0 ; numAlloc < numBuffers ; numAlloc++ )
   {
      opt->buffers[numAlloc] = (void *)u4alloc( opt->bufferSize ) ;
      if( opt->buffers[numAlloc] == 0 )
         break ;
      for ( i = 0 ; i < (int)opt->maxBlocks ; i++ )
      {
         curBlock = &opt->blocks[ numAlloc * opt->maxBlocks + i] ;
         curBlock->data = (OPT4BLOCK *)( (char *)opt->buffers[numAlloc] + i * opt->blockSize ) ;
         l4add( &opt->avail, &curBlock->lruLink ) ;
      }
   }

   #ifdef S4WRITE_DELAY
      /* now set up the delay write buffer. numAlloc already 1 greater than max, so just use */
      for ( i = 0 ; i < (int)opt->maxBlocks ; i++ )
      {
         curBlock = &opt->blocks[ numAlloc * opt->maxBlocks + i] ;
         curBlock->data = (OPT4BLOCK *)( (char *)opt->delayWriteBuffer + i * opt->blockSize ) ;
         l4add( &opt->delayAvail, &curBlock->lruLink ) ;
      }
   #endif

   return numAlloc ;
}

#endif /* S4OFF_OPTIMIZE */
