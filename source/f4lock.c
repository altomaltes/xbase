/* f4lock.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifdef S4IBMOS2
   #define INCL_DOSFILEMGR
   #include <os2.h>
   #ifndef NO_ERROR
      #define NO_ERROR 0
      #define ERROR_LOCK_VIOLATION 33
      #define S4BYTE_DEFINED
   #endif
#endif

#ifdef S4TEMP
   #include "t4test.h"
#endif

#ifndef S4SINGLE

#ifndef S4WINTEL
   #include <unistd.h>
   #ifdef S4NO_LOCKF
      #include <fcntl.h>
   #endif
#else
   #ifndef __WIN32
   #ifndef __TURBOC__
      #ifndef S4IBMOS2
         #include <sys\locking.h>
         #define S4LOCKING
      #endif
   #endif
   #ifdef __ZTC__
      extern int  errno ;
   #endif
   #ifdef _MSC_VER
      #include <sys\types.h>
   #endif
   #ifdef __TURBOC__
      #ifndef S4OS2
        #ifdef __BORLANDC__
           #include <sys\locking.h> /* Borland C++ compilers */
        #else
           #include <locking.h>             /* Turbo C++ for DOS compilers */
           /* #include <sys\locking.h> */   /* Turbo C++ for Win compilers */
        #endif
   /*   extern int cdecl errno ; */
      #endif
   #endif
/*   #include <sys\stat.h>*/
/*   #include <share.h>*/
   #endif
#endif

#endif

#ifndef __WIN32
/*   #include <fcntl.h>*/
   #include <errno.h>
#endif

#ifdef S4ERRNO
   extern int errno ;
#endif

#ifndef S4SINGLE
int dfile4registerLocked( DATA4FILE *d4, const long lockId, int doExtra )
{
   DATA4 *lockedData ;
   TRAN4 *trans ;
   CODE4 *c4 ;
   long clientId, serverId ;

   if ( d4 == 0 )
      return 0 ;

   c4 = d4->c4 ;

   #ifdef E4PARM_LOW
      if ( lockId < -2L )
         return error4( c4, e4parm, E96701 ) ;
   #endif

      c4->lockedLockItem = lockId ;

   if ( doExtra )
   {
      switch( lockId )
      {
         case -1:  /* file */
            clientId = d4->fileClientLock ;
            serverId = d4->fileServerLock ;
            break ;
         case 0:  /* append lock */
            clientId = d4->appendClientLock ;
            serverId = d4->appendServerLock ;
            if ( clientId == 0 || serverId == 0 )  /* probably due to a file lock */
            {
               clientId = d4->fileClientLock ;
               serverId = d4->fileServerLock ;
            }
            if ( clientId == 0 || serverId == 0 )  /* probably due to an exclusive open */
            {
               clientId = d4->tempClientLock ;
               serverId = d4->tempServerLock ;
            }
            break ;
         default:  /* record number */
            clientId = d4->tempClientLock ;
            serverId = d4->tempServerLock ;
            if ( clientId == 0 || serverId == 0 )  /* probably due to a file lock */
            {
               clientId = d4->fileClientLock ;
               serverId = d4->fileServerLock ;
            }
            if ( clientId == 0 || serverId == 0 )  /* probably due to an exclusive open */
            {
               clientId = d4->tempClientLock ;
               serverId = d4->tempServerLock ;
            }
            break ;
      }

      if ( clientId == 0 || serverId == 0 )  /* means system has the lock... */
      {
         return 0 ;
      }
      lockedData = code4idData( c4, serverId, clientId ) ;
      if ( lockedData == 0 )
         return 0 ;
      trans = lockedData->trans ;

         c4->lockedFileName = lockedData->alias ;
         c4->lockedUserName = trans->userId ;
         c4->lockedNetName = trans->netId ;
   }

   return 0 ;
}
#endif /* S4SINGLE */

   const char *S4FUNCTION code4lockNetworkId( CODE4 *c4 )
   {
      #ifdef E4PARM_HIGH
         if ( c4 == 0 )
         {
            error4( 0, e4parmNull, E91009 ) ;
            return 0 ;
         }
      #endif
      return c4->lockedNetName ;
   }

   const char *S4FUNCTION code4lockUserId( CODE4 *c4 )
   {
      #ifdef E4PARM_HIGH
         if ( c4 == 0 )
         {
            error4( 0, e4parmNull, E91010 ) ;
            return 0 ;
         }
      #endif
      return c4->lockedUserName ;
   }

   const char *S4FUNCTION code4lockFileName( CODE4 *c4 )
   {
      #ifdef E4PARM_HIGH
         if ( c4 == 0 )
         {
            error4( 0, e4parmNull, E91011 ) ;
            return 0 ;
         }
      #endif
      return c4->lockedFileName ;
   }

   long S4FUNCTION code4lockItem( CODE4 *c4 )
   {
      #ifdef E4PARM_HIGH
         if ( c4 == 0 )
         {
            error4( 0, e4parmNull, E91012 ) ;
            return 0 ;
         }
      #endif
      return c4->lockedLockItem ;
   }

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION file4lockInternal( FILE4 *file, unsigned long posStart, long posStartHi, unsigned long numBytes, long numBytesHi )
{
   #ifdef S4SINGLE
      return 0 ;
   #else
      int rc, numAttempts ;
      CODE4 *c4 ;
      #ifdef __unix__
         #ifdef S4MULTIC4
            int pid, status;
         #endif
         #ifdef S4NO_LOCKF
            struct flock fstruct ;
         #endif
      #endif
      #ifdef S4IBMOS2
         struct LockStrc
         {
            long FileOffset ;
            long RangeLength ;
         } L4RANGE ;
      #endif
      #ifdef S4MACINTOSH
         ParamBlockRec MACParmBlk ;
      #endif

      #ifdef E4PARM_HIGH
         if ( file == 0 )
            return error4( 0, e4parmNull, E90613 ) ;
         #ifndef S4MDX
            if ( numBytes < 0 || posStart < 0 )
               return error4( 0, e4parm, E90613 ) ;
         #endif
      #endif

      if ( numBytes == 0 || file->isReadOnly || file->lowAccessMode != OPEN4DENY_NONE )  /* don't check error code */
         return 0 ;

      #ifdef E4PARM_HIGH
         /* check before c4 since it will be invalid if file handle invalid */
         if ( file->hand == INVALID4HANDLE )
            return error4( 0, e4parm, E80608 ) ;
      #endif

      c4 = file->codeBase ;
      if ( error4code( c4 ) < 0 )
         return e4codeBase ;

      #ifdef S4LOCK_HOOK
         numAttempts = 0 ;
      #else
         numAttempts = c4->lockAttempts ;
         if ( numAttempts == 0 )
            numAttempts = 1 ;
      #endif

      #ifndef __WIN32
         errno = 0 ;
      #endif

      for( ;; )
      {
         #ifdef S4LOCKING
            #ifdef S4WINDOWS
               _llseek( file->hand, posStart, 0 ) ;
            #else
               lseek( file->hand, posStart, 0 ) ;
            #endif
            rc = locking( file->hand, LK_NBLCK, numBytes ) ;
         #else
            #ifdef __unix__
               #ifndef S4NO_LOCKF
                  lseek( file->hand, posStart, 0 ) ;
               #else
                  fstruct.l_type = F_WRLCK ;
                  fstruct.l_whence = SEEK_SET ;
                  fstruct.l_start = posStart ;
                  fstruct.l_len = numBytes ;
               #endif
               #ifdef S4MULTIC4
                  if ((pid = fork())<0)
                     return error4(c4, e4lock, E80609);
                  if (pid ==0)               /* Child fork() */
                  {
                     #ifndef S4NO_LOCKF
                        if ( numAttempts == WAIT4EVER )    /* sleep until lock */
                           rc = lockf( file->hand, F_LOCK, numBytes ) ;
                        else
                           rc = lockf( file->hand, F_TLOCK, numBytes ) ;
                     #else
                        if ( numAttempts == WAIT4EVER )    /* sleep until lock */
                           rc = fcntl( file->hand, F_SETLKW, &fstruct ) ;
                        else
                           rc = fcntl( file->hand, F_SETLK, &fstruct ) ;
                     #endif
                     exit(rc);
                  }
                  while (wait(&status)!=pid)    /* Parent fork() */
                     ;
                  if ((WIFEXITED(status)==0))   /* If it didn't exit properly */
                     return error4(c4, e4lock, E80609);
                  if ((rc=WEXITSTATUS(status)) == 0)   /*IF it was able to lock*/
                  {
                  #ifndef S4NO_LOCKF
                     if ( numAttempts == WAIT4EVER )    /* sleep until lock */
                        rc = lockf( file->hand, F_LOCK, numBytes ) ;
                     else
                        rc = lockf( file->hand, F_TLOCK, numBytes ) ;
                  #else
                     if ( numAttempts == WAIT4EVER )    /* sleep until lock */
                        rc = fcntl( file->hand, F_SETLKW, &fstruct ) ;
                     else
                        rc = fcntl( file->hand, F_SETLK, &fstruct ) ;
                  #endif
                  }
               #else
                  #ifndef S4NO_LOCKF
                     if ( numAttempts == WAIT4EVER )    /* sleep until lock */
                        rc = lockf( file->hand, F_LOCK, numBytes ) ;
                     else
                        rc = lockf( file->hand, F_TLOCK, numBytes ) ;
                  #else
                     if ( numAttempts == WAIT4EVER )    /* sleep until lock */
                        rc = fcntl( file->hand, F_SETLKW, &fstruct ) ;
                     else
                        rc = fcntl( file->hand, F_SETLK, &fstruct ) ;
                  #endif
               #endif  /*  !S4MULTIC4  */
            #endif  /*  __unix__  */

            #ifdef S4IBMOS2
               L4RANGE.FileOffset = posStart ;
               L4RANGE.RangeLength = numBytes ;

               rc = DosSetFileLocks( (HFILE) file->hand, 0L,(PFILELOCK) &L4RANGE, 100L, 0L ) ;
            #endif

            #ifdef S4MACINTOSH
               MACParmBlk.ioParam.ioRefNum = file->hand ;
               MACParmBlk.ioParam.ioReqCount = numBytes ;
               MACParmBlk.ioParam.ioPosMode = fsFromStart ;
               MACParmBlk.ioParam.ioPosOffset = posStart ;

               PBLockRangeSync( &MACParmBlk ) ;
               rc = MACParmBlk.ioParam.ioResult ;
            #endif

            #ifdef __WIN32
               if ( LockFile( (HANDLE)file->hand, posStart, posStartHi, numBytes, numBytesHi ) )
                  rc = NO_ERROR ;
               else
                  rc = GetLastError() ;
            #else
               #ifdef __TURBOC__
                  rc = lock( file->hand, posStart, numBytes ) ;
               #endif
            #endif
         #endif

         #ifdef __ZTC__
            if (rc == 0 || errno == 1)
         #else
            #ifdef S4IBMOS2
               if (rc == NO_ERROR )
            #else
               #ifdef S4MACINTOSH
                  if (rc == 0 )
               #else
                  #ifdef __WIN32
                     if (rc == NO_ERROR )
                  #else
                     if (rc == 0 || errno == EINVAL)
                  #endif
               #endif
            #endif
         #endif
            {
               #ifdef S4LOCK_CHECK
                  l4lock_save( file->hand, posStart, numBytes ) ;
               #endif
               return 0 ;
            }

         if ( rc == 0 )
         {
            #ifdef S4LOCK_CHECK
               l4lock_save( file->hand, posStart, numBytes ) ;
            #endif
            #ifndef S4OPTIMIZE_OFF
               file4setWriteOpt( file, 1 ) ;
            #endif
            return 0 ;
         }

         #ifdef __WIN32
            if ( rc != ERROR_LOCK_VIOLATION )
         #else
            #ifdef _MSC_VER
               #if _MSC_VER == 600
                  #ifdef S4WINDOWS
                     if (errno != -1 )       /* Microsoft 6.00a does not return */
                  #else                      /* valid 'errno' under Windows,    */
                     if (errno != EACCES)    /* but performs locking correctly  */
                  #endif
               #else
                  if (errno != EACCES)
               #endif
            #else
               #ifdef __ZTC__
                  if (errno != 33)
               #else
                  #ifdef S4IBMOS2
                     if ( rc != ERROR_LOCK_VIOLATION )
                  #else
                     #ifdef S4MACINTOSH
                        if ( (rc != fLckdErr ) && (rc != afpRangeOverlap ) )
                     #else
                        if ((errno != EACCES) && (errno != EAGAIN) && (errno != 0) )
                     #endif
                  #endif
               #endif
            #endif
         #endif
            return error4describe( c4, e4lock,E90613,  file->name, (char *)0, (char *)0 ) ;

         #ifdef S4LOCK_HOOK
            rc = code4lockHook( c4, file->name, 0, 0, -2, ++numAttempts ) ;
            if( rc != 0 )
               return rc ;
         #else
            if ( numAttempts == 1 )
               return r4locked ;
            if ( numAttempts > 1 )
               numAttempts-- ;
         #endif
         #ifdef S4TEMP
            if ( d4display_quit( &display ) )
               return error4( c4, e4result, E80604 ) ;
         #endif

         u4delayHundredth( (unsigned int)c4->lockDelay ) ;   /* delay & try lock again */
      }
   #endif
}

#ifdef S4LOCK_MODE_REQD
/* tests for workaround to Novell same process lock inconsistency/bug */
/* returns '1' if overlapping locks are allowed by same process on file */
int file4lockMode( FILE4 *file )
{
   int rc, lockMode, oldLockAttempts ;
   CODE4 *c4 ;

   if ( file->hand == INVALID4HANDLE )
      return -1 ;

   c4 = file->codeBase ;

   lockMode = 0 ;
   oldLockAttempts = c4->lockAttempts ;
   c4->lockAttempts = 1 ;

   if ( file4lock( file, 0x50000000L, 2L ) != 0 )
   {
      c4->lockAttempts = oldLockAttempts ;
      return -1 ;
   }
   rc = file4lock( file, 0x50000000L, 1L ) ;
   if ( rc == 0 )
   {
      lockMode = 1 ;
      file4unlock( file, 0x50000000L, 1L ) ;
   }
   if ( rc < 0 )
      lockMode = rc ;

   file4unlock( file, 0x50000000L, 2L ) ;

   c4->lockAttempts = oldLockAttempts ;
   return lockMode ;
}
#endif

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION file4unlockInternal( FILE4 *file, unsigned long posStart, long posStartHi, unsigned long numBytes, long numBytesHi )
{
   #ifndef S4SINGLE
      int rc ;

      #ifdef S4IBMOS2
         struct LockStrc
         {
            long FileOffset ;
            long RangeLength ;
         } L4RANGE ;
      #endif

      #ifdef S4MACINTOSH
         ParamBlockRec MACParmBlk ;
      #endif
      #ifdef S4NO_LOCKF
         struct flock fstruct ;
      #endif

      #ifdef E4PARM_HIGH
         if ( file == 0 )
            return error4( 0, e4parmNull, E90614 ) ;
         #ifndef S4MDX
            if ( numBytes < 0 || posStart < 0 )
               return error4( file->codeBase, e4parm, E90614 ) ;
         #endif
      #endif

      if ( numBytes == 0 || file->isReadOnly || file->lowAccessMode != OPEN4DENY_NONE )
         return 0 ;

      #ifndef S4OPTIMIZE_OFF
         file4setWriteOpt( file, 0 ) ;
      #endif

      #ifdef S4LOCK_CHECK
         l4lockRemove( file->hand, posStart, numBytes ) ;
      #endif

      #ifndef __WIN32
         errno = 0 ;
      #endif

      #ifdef S4LOCKING
         #ifdef S4WINDOWS
            _llseek( file->hand, posStart, 0 ) ;
         #else
            lseek( file->hand, posStart, 0 ) ;
         #endif
         rc = locking( file->hand, LK_UNLCK, numBytes ) ;
      #else
         #ifdef __unix__
            #ifndef S4NO_LOCKF /* lockf() replaces locking() for SUN OS, AT&T */
               lseek( file->hand, posStart, 0 ) ;
               rc = lockf( file->hand, F_ULOCK, numBytes ) ;
            #else
               fstruct.l_type = F_UNLCK ;
               fstruct.l_whence = SEEK_SET ;
               fstruct.l_start = posStart ;
               fstruct.l_len = numBytes ;

               rc = fcntl( file->hand, F_SETLK, &fstruct ) ;
            #endif
         #endif

         #ifdef S4IBMOS2
            L4RANGE.FileOffset = posStart ;
            L4RANGE.RangeLength = numBytes ;

            rc = DosSetFileLocks( (HFILE)file->hand, (PFILELOCK)&L4RANGE, 0L, 100L , 0L ) ;
         #endif

         #ifdef S4MACINTOSH
            MACParmBlk.ioParam.ioRefNum = file->hand ;
            MACParmBlk.ioParam.ioReqCount = numBytes ;
            MACParmBlk.ioParam.ioPosMode = fsFromStart ;
            MACParmBlk.ioParam.ioPosOffset = posStart ;

            PBUnlockRangeSync( &MACParmBlk ) ;
            rc = MACParmBlk.ioParam.ioResult ;
         #endif

         #ifdef __WIN32
            if ( UnlockFile( (HANDLE)file->hand, posStart, posStartHi, numBytes, numBytesHi ) )
               rc = NO_ERROR ;
            else
               rc = -1 ;
         #else
            #ifdef __TURBOC__
               rc = unlock( file->hand, posStart, numBytes ) ;
            #endif
         #endif
      #endif

      #ifdef __ZTC__
         if ( rc < 0  && errno != 1 )
      #else
         #ifdef S4IBMOS2
            if ( rc != NO_ERROR )
         #else
            #ifdef S4MACINTOSH
               if ( rc != 0 )
            #else
               #ifdef __WIN32
                  if (rc != NO_ERROR )
               #else
                  if ( rc < 0  && errno != EINVAL )
               #endif
            #endif
         #endif
      #endif
         return error4describe( file->codeBase, e4unlock, E90614, file->name, (char *)0, (char *)0 ) ;
   #endif

   return 0 ;
}
