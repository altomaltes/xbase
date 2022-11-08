/* f4temp.c (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifdef S4TEMP
   #include "t4test.h"
#endif

#ifdef S4WINTEL
   #ifndef S4OFF_MULTI
      #ifndef S4IBMOS2
         #ifndef __TURBOC__
            #include <sys\locking.h>
            #define S4LOCKING
         #endif
         #ifdef _MSC_VER
            #include <sys\types.h>
            #include <sys\locking.h>
         #endif
      #endif
   #endif
#endif

int file4tempLow( FILE4 *file, CODE4 *c4, const int autoRemove, int useTempDirectory, const char *ext )
{
   int i, saveFlag, rc ;
   #ifndef S4OFF_MULTI
      int oldExcl ;
   #endif
   #ifndef S4OPTIMIZE_OFF
      int oldOptWr ;
      int tfDoBuffer = 0 ;
      long tfHashInit = -1L ;
      char tfType = (char)-1 ;
      char tfBufferWrites = (char)-1 ;
      char tfWriteBuffer = (char)-1 ;
      long tfLen = -1L ;
      LINK4 tfLink ;
   #endif
   #ifdef S4WINCE
      SYSTEMTIME st ;
      WORD t ;
   #else
      time_t t ;
   #endif
   char name[255], *env ;
   char fileName[13] ;
   char drive[3] ;

   #ifdef E4PARM_HIGH
      if ( file == 0 || c4 == 0  )
         return error4( c4, e4parmNull, E90618 ) ;
   #endif

   #ifndef S4OPTIMIZE_OFF
      code4memStartMaxSet( c4, c4->memMaxPercent ) ;  /* start optimization if not enabled and not suspended */

      memset( &tfLink, 0, sizeof( tfLink ) ) ;

      if ( autoRemove && c4->opt.numBuffers != 0 )
      {
         if ( c4->opt.forceCurrent == 0 )
         {
            memset( (void *)file, 0, sizeof( FILE4 ) ) ;
            file->isTemp = 1 ;
            file->codeBase = c4 ;
            file->fileCreated = 0 ;
            #ifndef S4OFF_MULTI
               file->lowAccessMode = OPEN4DENY_RW ;
            #endif
            oldOptWr = c4->optimizeWrite ;
            c4->optimizeWrite = 1 ;
            file4optimize( file, 1, OPT4OTHER ) ;
            c4->optimizeWrite = oldOptWr ;
            return 0 ;
         }
         else
         {
            tfHashInit = file->hashInit ;
            tfLen = file4longGetLo( file->len ) ;
            tfType = file->type ;
            tfBufferWrites = file->bufferWrites ;
            tfDoBuffer = file->doBuffer ;
            tfWriteBuffer = file->writeBuffer ;
            memcpy( (void *)&tfLink, (void *)&file->link, sizeof( LINK4 ) ) ;
            #ifdef E4ANALYZE
               if ( file->name != 0 )
                  return error4( c4, e4struct, E90618 ) ;
            #endif
         }
      }
   #endif

   saveFlag = c4getErrCreate( c4 ) ;
   c4setErrCreate( c4, 0 ) ;
   #ifndef S4OFF_MULTI
      oldExcl = c4->accessMode ;
      c4->accessMode = OPEN4DENY_RW ; /* all temporary files are for exclusive access only */
   #endif

   #ifndef S4WINCE
      if ( useTempDirectory )
      {
         #ifdef __unix__
            env = u4environ( "TMPDIR", 0 ) ;
         #else
            env = u4environ( "TEMP", 0 ) ;
            if ( env == 0 )
               env = u4environ( "TMP", 0 ) ;
         #endif
      }
      else
   #endif
      env = 0 ;
   if ( env == 0 )
   {
      drive[0] = 0 ;
      env = "" ;
   }
   else
   {
      if ( env[1] == ':' )
      {
         drive[0] = env[0] ;
         drive[1] = env[1] ;
         drive[2] = 0 ;
      }
      else
         drive[0] = 0 ;
   }

   strcpy( fileName, "TEMP" ) ;
   if ( ext == NULL )
   {
      strcpy( fileName + 8, ".TMP" ) ;
      fileName[12] = 0 ;
   }
   else
   {
      fileName[8] = '.' ;
      strcpy( fileName + 9, ext ) ;
      fileName[9+strlen(ext)] = 0 ;
   }

   for ( i = 0 ;; )
   {
      if ( i >= 100 )
      {
         rc = error4( c4, e4create, E80605 ) ;
         break ;
      }
      i++ ;
      #ifndef S4WINCE
         time( &t );                    /* changed back from time from clock due to size and linking considerations*/
      #else
         GetLocalTime(&st) ;
         t = st.wMilliseconds ;
      #endif
      t %= 10000L ;
      c4ltoa45( t, fileName + 4, -4 ) ;
      u4nameMake( name, sizeof( name ), drive, env + strlen( drive ), fileName ) ;

      rc = file4create( file, c4, name, 1 ) ;
      if ( rc <= 0 )
         break ;

      u4delayHundredth( 50 ) ;
   }

   #ifndef S4OFF_MULTI
      c4->accessMode = oldExcl ;
   #endif
   c4setErrCreate( c4, saveFlag ) ;

   if ( rc < 0 )
      return error4stack( c4, (short)rc, E90618 ) ;
   if ( autoRemove )
      file->isTemp = 1 ;
   #ifndef S4OPTIMIZE_OFF
      if ( autoRemove && c4->opt.numBuffers != 0 )
      {
         if ( c4->opt.forceCurrent == 1 )
         {
            file->hashInit = tfHashInit ;
            file4longAssign( file->len, tfLen, 0 ) ;
            file->type = tfType ;
            file->bufferWrites = tfBufferWrites ;
            file->doBuffer = tfDoBuffer ;
            file->writeBuffer = tfWriteBuffer ;
            memcpy( (void *)&file->link, (void *)&tfLink, sizeof( LINK4 ) ) ;
         }
         else
         {
            oldOptWr = c4->optimizeWrite ;
            c4->optimizeWrite = 1 ;
            file4optimize( file, 1, OPT4OTHER ) ;
            c4->optimizeWrite = oldOptWr ;
         }
      }
   #endif
   return 0 ;
}


