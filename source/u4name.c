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

/* u4name.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifdef S4WINTEL
   #ifndef S4WINCE
      #include <direct.h>
   #endif
#endif
#include <ctype.h>

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION u4nameExt( char *name, int lenResult, const char *newExt, const int doReplace )
{
   int fileNameLen, extPos, onPos ;
   char *ptr ;
   #ifdef E4MISC
      int extLen ;
   #endif

   ptr = name + (int)strlen( name ) - 1 ;
   while ( *ptr == ' ' )
   {
      *ptr = '\0' ;
      ptr-- ;
   }

   extPos = fileNameLen = strlen( name ) ;

   if ( extPos != 0 )
      for( onPos = extPos-1 ;; onPos-- )
      {
         if ( name[onPos] == '.' )
         {
            extPos = onPos ;
            break ;
         }
         if ( name[onPos] == S4DIR )
            break ;
         if ( onPos == 0 )
           break ;
      }

   if ( fileNameLen != extPos &&  !doReplace )
   {
      #ifndef S4CASE_SEN
         c4upper( name ) ;
      #endif

      return 0 ;
   }

   if ( *newExt == '.' )
      newExt++ ;
   #ifdef E4MISC
      extLen = strlen( newExt ) ;
      if ( extLen > 3 )
         extLen = 3 ;
      if ( lenResult <= extPos + extLen + 1 )
         return error4( 0, e4result, E94507 ) ;
   #endif

   name[extPos++] = '.' ;
   strcpy( name + extPos, newExt ) ;

   #ifndef S4CASE_SEN
      c4upper( name ) ;
   #endif

   return 0 ;
}

/* takes the input buffer and removes any "..\" or ".\"pieces 
 */
static int u4nameFix( char *buf )
{
   int i, j, len, l2 ;

   len = strlen( buf ) ;

   for( i = 0 ; i < len - 2 ; i++ )
   {
      #ifdef __unix__
         if ( c4memcmp(buf+i, "../", 3 ) == 0 )
      #else
         if ( c4memcmp( buf + i, "..\\", 3 ) == 0 )
      #endif
      {
         len -= 3 ;
         c4memmove( buf+i, buf+i+3, (unsigned int)(len - i) ) ;
         if ( i >= 2 )   /* need to remove previous path part too */
         {
            if ( buf[i-1] != S4DIR )
               return error4( 0, e4name, E94510 ) ;
            for( j = i-2 ; j > 0 ; j-- )
            {
               if ( buf[j] == S4DIR )
               {
                  c4memmove( buf+j+1, buf+i, (unsigned int)(len - i) ) ;
                  l2 = i - j - 1 ;
                  len -= l2 ;
                  i -= l2 ;
                  break ;
         }  }  }
         i-- ;  /* compensate for position */
      }
   }

   for( i = 0 ; i < len - 1 ; i++ )
   {
   #ifdef __unix__
      if ( c4memcmp( buf + i, "./", 2 ) == 0 )
   #else
      if ( c4memcmp( buf + i, ".\\", 2 ) == 0 )
   #endif
      {
         len -= 2 ;
         c4memmove( buf+i, buf+i+2, (unsigned int)(len - i) ) ;
         i-- ;  /* compensate for position */
   }  }

   buf[len] = 0 ;

   #ifndef S4CASE_SEN
      c4upper( buf ) ;
   #endif

   return 0 ;
}

#ifndef S4WINTEL

#ifdef S4MACINTOSH
int S4FUNCTION u4nameCurrent( char *buf, const int bufLen, const char *name )
{
   strcpy(buf, name) ;
   return 0 ;
}
#else

/* take the input file name and add the current drive and path if required */
int S4FUNCTION u4nameCurrent( char *buf, const int bufLen, const char *name )
{
   int namePos, len, len2 ;

   E4PARMLOW( buf && name, E94509 ) ;

   if ( name[0] == S4DIR )  /* full path */
   {
      len = strlen( name ) ;
      if ( len+1 > bufLen )
         return error4( 0, e4parm, E94509 ) ;
      memcpy( buf, name, (unsigned int)len ) ;
      buf[len] = 0 ;
      return u4nameFix( buf ) ;
   }

   namePos = 0 ;
   #ifndef S4MACINTOSH
      if ( getcwd( buf, bufLen ) == 0 )
   #else
      if ( u4getPath( buf, bufLen ) == 0 )
   #endif
      return error4( 0, e4parm, E94509 ) ;

   len2 = strlen( buf ) ;
   len = strlen( name ) ;
   if ( len > 2 )
      for ( ;; )
      {
         #ifndef S4MACINTOSH
            if ( c4memcmp( name + namePos, "../", 3 ) != 0 )
         #else
            if ( c4memcmp( name + namePos, "..:", 3 ) != 0 )
         #endif
            break;
         /* must remove part of the current path */
         if ( len2 > 2 )
            len2-- ;
         for ( ;; )
         {
            if ( len2 == 2 )
               break ;
            len2-- ;
            if ( buf[len2] == S4DIR )
               break ;
         }
         namePos += 3 ;
      }

   if ( buf[len2-1] != S4DIR )  /* need to add the backslash */
   {
      if ( len2 + 1 >= bufLen )
         return error4( 0, e4parm, E94509 ) ;
      buf[len2] = S4DIR ;
      len2++ ;
   }
   len -= namePos ;
   if ( len + 1 > bufLen - len2 )
      return error4( 0, e4parm, E94509 ) ;
   memcpy( buf + len2, name + namePos, (unsigned int)len ) ;
   buf[len2 + len] = 0 ;
   return u4nameFix( buf ) ;
}
#endif /* !S4MACINTOSH*/
#else
#ifdef S4WINCE

int S4FUNCTION u4nameCurrent( char *buf, const int bufLen, const char *name )
{
   strcpy(buf, name) ;
   return 0 ;
}

#else

/* similar to u4nameCurrent, but takes an additional path paramater as well.
   Used for ole-db when we need to combine all of:  current directory, catalog
   path (from properties), and input name */
int S4FUNCTION u4nameCurrentExtended( char *buf, const int bufLen, const char *name, const char *path )
{
   char includePath = 1 ;
   char nameBuf[LEN4PATH] ;

   assert5 ( buf != 0 && name != 0 && path != 0 && strlen( name ) > 0 ) ;

   /* first combine path and name, then just call u4nameCurrent() */
   if ( name[0] == '\\' )  /* if machine name case or root path, don't need to consider path */
      includePath =0 ;
   else
   {
      if ( name[1] != ':' )  /* don't bother if a drive is included (overrides path) */
         includePath =0 ;
   }

   if ( includePath )
   {
      int len ;
      strcpy( nameBuf, path ) ;
      len = strlen( nameBuf ) ;
      if ( len != 0 )
      {
         if ( nameBuf[len-1] != '\\' )  /* ensure the '\' is included on path */
            strcat( nameBuf, "\\" ) ;
      }
      strcat( nameBuf, name ) ;
   }
   else
      strcpy( nameBuf, name ) ;

   return u4nameCurrent( buf, bufLen, nameBuf ) ;
}

/* take the input file name and add the current drive and path if required */
int S4FUNCTION u4nameCurrent( char *buf, const int bufLen, const char *name )
{
   int namePos, len, len2, i, isMachineName, lx ;
   unsigned int driveNo ;
   #ifdef __WATCOMC__
      unsigned origDrive, curDrive, dummy;
   #endif

   E4PARMLOW( buf && name, E94509 ) ;

   /* also must consider machine name accesses (eg. "\\BARNEY\TEST...") */

   if ( name[0] == '\\' && name[1] == '\\' )
   {
      for ( namePos = 2 ;; namePos++ )
         if ( name[namePos] == '\\' || name[namePos] == 0 )
            break ;
      namePos++ ;
      memcpy( buf, name, namePos ) ;
      buf[namePos] = 0 ;
      isMachineName = 1 ;
   }
   else
   {
      isMachineName = 0 ;
      #ifndef S4WINCE
         if ( name[1] != ':' )   /* must get the default drive */
         {
            #ifdef __WIN32
               #ifdef __BORLANDC__
                  driveNo = getdisk() + 1 ;  /* get disk returns one less than get drive */
               #else
                  driveNo = _getdrive() ;
               #endif
            #else
               #ifdef S4OS2
                  driveNo = _getdrive() ;
               #else
                  _dos_getdrive( &driveNo ) ;
               #endif
            #endif
            if ( !driveNo || driveNo > 26 )   /* means the current directory is on a machine name (eg. \\server\c\dbf is current directory) */
            {
               #ifdef S4WINCE
                  if (GetCurrentDirectory(buf + 2, bufLen - 2) == 0)
               #else
                  #ifdef _MSC_VER
                     if ( _getcwd( buf, bufLen) == 0 )
                  #else
                     if ( getcwd( buf, bufLen) == 0 )
                  #endif
               #endif
                     return error4( 0, e4parm, E94509 ) ;
               namePos=0;
               isMachineName = 1 ;
            }
            else
            {
               buf[0] = 'A' + (char)(driveNo - 1) ;
               buf[1] = ':' ;
               if ( name[0] == '\\' )  /* just append the path */
               {
                  len = strlen( name ) ;
                  if ( len + 3 > bufLen )
                     return error4( 0, e4parm, E94509 ) ;
                  memcpy( buf + 2, name, (unsigned int)len ) ;
                  buf[len + 2] = 0 ;
                  return u4nameFix( buf ) ;
               }
               namePos = 0 ;
            }
         }
         else
         {
      #endif
         if ( name[2] == '\\' )  /* have the full path, so done */
         {
            len = strlen( name ) ;
            if ( len + 1 > bufLen )
               return error4( 0, e4parm, E94509 ) ;
            memcpy( buf, name, (unsigned int)len ) ;
            buf[len] = 0 ;
            return u4nameFix( buf ) ;
         }
         #ifndef S4WINCE
            memcpy( buf, name, 2 ) ;  /* get the drive */
            namePos = 2 ;
         }
         #endif

   }

   if ( isMachineName == 0 )
   {
      /* get the current path and add it to buf */
      buf[0] = toupper( buf[0] ) ;

      #ifdef __WATCOMC__
         _dos_getdrive(&origDrive);
         _dos_setdrive(buf[0]-'A'+1, &dummy);
         _dos_getdrive(&curDrive);
         if (curDrive != buf[0]-'A'+1)
         {
            _dos_setdrive(origDrive, &dummy);
            return error4( 0, e4parm, E94509 ) ;
         }
         if (getcwd(buf + 2, bufLen - 2) == 0)
         {
            _dos_setdrive(origDrive, &dummy);
            return error4( 0, e4parm, E94509 ) ;
         }
         _dos_setdrive(origDrive, &dummy);
         _dos_getdrive(&curDrive);
         if (curDrive != origDrive)
            return error4( 0, e4parm, E94509 ) ;
      #else
         #ifdef S4WINCE
            if (GetCurrentDirectory(buf + 2, bufLen - 2) == 0)
         #else
            if ( _getdcwd( buf[0] - 'A' + 1, buf + 2, bufLen - 2 ) == 0 )
         #endif
               return error4( 0, e4parm, E94509 ) ;
      #endif

      lx = strlen( buf ) ;

      if (buf[3] == ':')
         for ( i = 2 ; i <= lx ; i++ )
            buf[i-2] = buf[i];
   }

   len2 = strlen( buf ) ;
   for ( ;; )
   {
      if ( c4memcmp( name + namePos, "..\\", 3 ) != 0 )
         break;
      /* must remove part of the current path */
      if (isMachineName==1)
      {
         for (; ; )
            if ( buf[--len2] == '\\' )
               break ;
      }else
      {
         if ( len2 > 2 )
            len2-- ;
         for ( ;; )
         {
            if ( len2 == 2 )
               break ;
            len2-- ;
            if ( buf[len2] == '\\' )
               break ;
         }
      }
      namePos += 3 ;
   }
   if ( name[0] != '\\' && buf[len2-1] != '\\' )  /* need to add the backslash */
   {
      if ( len2 + 1 >= bufLen )
         return error4( 0, e4parm, E94509 ) ;
      buf[len2] = '\\' ;
      len2++ ;
   }
   len = strlen( name + namePos ) ;
   if ( len + 1 > bufLen - len2 )
      return error4( 0, e4parm, E94509 ) ;
   memcpy( buf + len2, name + namePos, (unsigned int)len ) ;
   buf[len2 + len] = 0 ;
   return u4nameFix( buf ) ;
}
#endif  /*S4WINCE */
#endif  /*S4WINTEL */

void S4FUNCTION u4nameMakeFindDrive( char *buf, const int bufLen, const char *defaultDirectory, const char *fileName )
{
   char drive[2] ;
   char *drivePtr = 0 ;
   if ( strlen( fileName ) > 1 && ( fileName[1] == ':' || (fileName[0] == '\\' && fileName[1] == '\\') ) )  /* full path for file name */
      defaultDirectory = 0 ;
   else
   {
      if ( defaultDirectory[1] == ':' )
      {
         drive[0] = defaultDirectory[0] ;
         drive[1] = 0 ;
         drivePtr = drive ;
         defaultDirectory += 2 ;
      }
   }
   u4nameMake( buf, bufLen, drivePtr, defaultDirectory, fileName ) ;
}

void u4nameMake( char *buf, const int bufLen, const char *defaultDrive, const char *defaultDirectory, const char *fileName )
{
   int defaultDirectoryLen, pos = 0 ;
   int needsDrive ;

   if ( strlen( fileName ) < 2 )
      needsDrive = 1 ;
   else
   {
      if ( fileName[1] != ':' )
      {
         if ( fileName [0] == '\\' && fileName [1] == '\\' )
            needsDrive = 0 ;
         else
            needsDrive = 1 ;
      }
      else
         needsDrive = 0 ;
   }

   if ( needsDrive )
      if ( defaultDrive != 0 )
         if ( strlen( defaultDrive ) == 2 )
         {
            memcpy( buf, defaultDrive, 2 ) ;
            pos += 2 ;
         }

   if ( defaultDirectory != 0 )
      defaultDirectoryLen = strlen( defaultDirectory ) ;
   else
      defaultDirectoryLen = 0 ;

   if ( fileName[0] != S4DIR  &&  defaultDirectoryLen > 0 )
   {
      if ( pos+2 >= bufLen )
         return ;
      buf[pos++] = S4DIR ;
      if ( defaultDirectory[0] == S4DIR )
         defaultDirectory++ ;

      defaultDirectoryLen = strlen(defaultDirectory) ;

      u4ncpy( buf+pos, defaultDirectory, (unsigned int)(bufLen - pos) ) ;
      pos += defaultDirectoryLen ;
   }

   if ( pos >= bufLen )
      return ;

   if ( pos > 0 )
   {
      if ( buf[pos-1] != S4DIR )
         buf[pos++] = S4DIR ;
      if ( fileName[0] == S4DIR  )
         fileName++ ;
   }

   u4ncpy( buf+pos, fileName, (unsigned int)(bufLen - pos ) ) ;
}

#ifdef P4ARGSUSED
   #pragma argsused
#endif
int S4FUNCTION u4namePiece( char *result, const unsigned int lenResult, const char *from, const int givePath, const int giveExt )
{
   unsigned namePos, extPos, onPos, pos, newLen, fromLen ;
   int arePastExt ;

   namePos = 0 ;
   arePastExt = 0 ;
   extPos = fromLen = strlen(from) ;
   if ( extPos == 0 )
   {
      *result = 0 ;
      return 0 ;
   }

   for( onPos = extPos-1;; onPos-- )
   {
      switch ( from[onPos] )
      {
      #ifndef S4MACINTOSH
         case S4DIR:
      #endif
         case ':':
            if (namePos == 0)  namePos = onPos + 1 ;
            arePastExt = 1 ;
            break ;
         case '.':
            if ( ! arePastExt )
            {
               extPos = onPos ;
               arePastExt = 1 ;
            }
            break ;
         default:
            break ;
      }

      if ( onPos == 0 )
         break ;
   }

   pos = 0 ;
   newLen = fromLen ;
   if ( !givePath )
   {
      pos = namePos ;
      newLen -= namePos ;
   }

   if ( !giveExt )
      newLen -= fromLen - extPos ;

   if ( newLen >= (unsigned) lenResult )
      newLen = lenResult - 1 ;

   memcpy( result, from+ pos, newLen ) ;
   result[newLen] = 0 ;

   #ifndef S4CASE_SEN
      c4upper( result );
   #endif

   return 0 ;
}

/* u4nameChar  Returns TRUE iff it is a valid dBase field or function name character */
int S4FUNCTION u4nameChar( unsigned char ch)
{
   return ( ((ch>='a') && (ch<='z'))  || ((ch>='A') && (ch<='Z'))  ||
      #ifdef S4MDX
         ((ch>='0') && (ch<='9'))  || ch=='&' || ch=='@' ||
      #else
         ch>='0' && ch<='9'  ||
      #endif
         ch=='_'
/*            ch=='\\'  ||  ch=='.'  || ch=='_'  ||  ch==':' */
   #ifdef S4GERMAN
      #ifdef S4ANSI
         || ch== 196  ||  ch== 214  || ch== 220  ||  ch== 223
         || ch== 228  ||  ch== 246  || ch== 252
      #else
         || ch== 129  ||  ch== 132  || ch== 142  ||  ch== 148
         || ch== 153  ||  ch== 154  || ch== 225
      #endif
   #endif
   #ifdef S4FRENCH
      #ifdef S4ANSI
         || ch== 192 || ch== 194 || ch== 206 || ch== 207
         || ch== 212 || ch== 219 || ch== 224 || ch== 226
         || ch== 238 || ch== 239 || ch== 244 || ch== 251
         || (ch>= 199 && ch <= 203) || (ch >= 231 && ch <= 235)
      #else
         || ch== 128 || ch== 130 || ch== 131 || ch== 133
         || ch== 144 || ch== 147 || ch== 150 || (ch>= 135 && ch <= 140)
      #endif
   #endif
   #ifdef S4SWEDISH
      #ifdef S4ANSI
         || ch== 196 || ch== 197 || ch== 198 || ch== 201
         || ch== 214 || ch== 220 || ch== 228 || ch== 229
         || ch== 230 || ch== 233 || ch== 246 || ch== 252 )
      #else
         || ch== 129 || ch== 130 || ch== 132 || ch== 134
         || ch== 148 || ch== 153 || ch== 154 || (ch>= 142 && ch <= 146)
      #endif
   #endif
   #ifdef S4FINNISH
      #ifdef S4ANSI
         || ch== 196 || ch== 197 || ch== 198 || ch== 201
         || ch== 214 || ch== 220 || ch== 228 || ch== 229
         || ch== 230 || ch== 233 || ch== 246 || ch== 252 )
      #else
         || ch== 129 || ch== 130 || ch== 132 || ch== 134
         || ch== 148 || ch== 153 || ch== 154 || (ch>= 142 && ch <= 146)
      #endif
   #endif
   #ifdef S4NORWEGIAN
      #ifdef S4ANSI
         || ch== 196 || ch== 197 || ch== 198 || ch== 201
         || ch== 214 || ch== 220 || ch== 228 || ch== 229
         || ch== 230 || ch== 233 || ch== 246 || ch== 252 )
      #else
         || ch== 129 || ch== 130 || ch== 132 || ch== 134
         || ch== 148 || ch== 153 || ch== 154 || (ch>= 142 && ch <= 146)
      #endif
   #endif
         ) ;
}

/* returns the length of the path in 'from', and copies the path in 'from' to result */
int S4FUNCTION u4namePath( char *result, const unsigned int lenResult, const char *from )
{
   long onPos ;

   u4namePiece( result, lenResult, from, 1, 0 ) ;
   for( onPos = 0 ; result[onPos] != 0 ; onPos++ ) ;

   for( ; onPos >= 0 ; onPos-- )
      if( result[onPos] == S4DIR || result[onPos] == ':' ) break ;   /* end of path */

   if( onPos < (long)lenResult )
      result[++onPos] = '\0' ;
   return (int)onPos ;
}

/* returns the length of the extension in 'from', and copies the extension in 'from' to 'result' */
int u4nameRetExt( char *result, const int lenIn, const char *from )
{
   char len, name[LEN4PATH+1] ;
   int lenResult, onPos ;

   lenResult = lenIn ;
   memset( result, 0, lenIn ) ;

   #ifdef E4PARM_HIGH
      if ( result == 0 || lenResult < 3 || from == 0 )
         return error4( 0, e4parm, E94506 ) ;
   #endif

   u4namePiece( name, LEN4PATH, from, 0, 1 ) ;

   len = 0 ;
   for( onPos = 0 ; ( name[onPos] != 0 && onPos < sizeof( name ) ) ; onPos++ )
      if ( name[onPos] == '.' )
      {
         for ( onPos++ ; name[onPos] != 0 && lenResult-- > 0 ; onPos++, len++ )
            result[len] = name[onPos] ;
         break ;
      }

   return len ;
}

#ifdef S4MACINTOSH
char *u4getMacPath(CODE4 *c4, char *buf, int buflen )
{
   int count;
   CInfoPBRec  myPB ;
   Str32 dirName ;
   char *dirNameC;
   char temp[258] ;

   buf[0] = '\0' ;
   count = 0 ;
   myPB.dirInfo.ioNamePtr = (StringPtr)&dirName ;
   myPB.dirInfo.ioVRefNum = c4->macVol ;
   myPB.dirInfo.ioDrParID = c4->macDir ;
   myPB.dirInfo.ioFDirIndex = -1 ;
   do
   {
      myPB.dirInfo.ioDrDirID = myPB.dirInfo.ioDrParID ;
      PBGetCatInfoSync(&myPB) ;
      dirNameC = p2cstr(dirName ) ;
      strcat(dirNameC, ":");
      count += strlen(dirNameC);
      if (count > buflen)
         return 0 ;
      strcpy(temp, dirNameC ) ;
      strcat(temp, buf ) ;
      strcpy(buf, temp ) ;
    }
    while  (myPB.dirInfo.ioDrDirID != fsRtDirID);
    return buf ;
}
#endif
