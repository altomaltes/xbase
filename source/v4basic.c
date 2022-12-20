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


#include "d4all.h"

#include <math.h>

#ifdef S4WINDOWS
   extern HINSTANCE cb5inst ;
#endif

#define r4check -5



            /* if value is true, return VB True value (-1) */
           /* JACS
short c4setLog( short *logValue, short newValue )
{
   short temp;
   if (newValue == r4check) return (!!(*logValue))*(-1);

   temp = *logValue;
   *logValue = !!newValue;  // any non-zero value becomes one
   return (!!temp)*(-1);

}

*/

int c4setLog( int *logValue, short newValue )
{                             /* if value is true, return VB True value (-1) */
   int temp;
   if (newValue == r4check) return (!!(*logValue))*(-1);

   temp = *logValue;
   *logValue = !!newValue;  /* any non-zero value becomes one */
   return (!!temp)*(-1);
}

char c4setLog( char *logValue, short newValue )
{                             /* if value is true, return VB True value (-1) */
   char temp;
   if (newValue == r4check) return (!!(*logValue))*(-1);

   temp = *logValue;
   *logValue = !!newValue;  /* any non-zero value becomes one */
   return (!!temp)*(-1);
}

#ifdef __cplusplus
   extern "C" {
#endif

/*  CODE4 CLASS TRANSLATIONS  */
short S4FUNCTION code4autoOpen( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40101 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->autoOpen), value ) ;
}

short S4FUNCTION code4codePage( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, 0 ) ) return -1 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->codePage ) ;
   temp = cb->codePage ;
   cb->codePage = value ;

   return( temp ) ;
}

short S4FUNCTION code4collatingSequence( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, 0 ) ) return -1 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->collatingSequence ) ;
   temp = cb->collatingSequence ;
   cb->collatingSequence = value ;

   return( temp ) ;
}

short S4FUNCTION code4createTemp( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, 0 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->createTemp), value ) ;
}

short S4FUNCTION code4errCreate( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40102 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->errCreate), value ) ;
}

#ifndef  S4VB_DOS

const char S4PTR *S4FUNCTION code4dateFormatVB( CODE4 *cb )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40103 ) ) return 0 ;
   #endif

   return code4dateFormat( cb ) ;
 }

#endif

short S4FUNCTION code4errDefaultUnique( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40104 ) ) return -1 ;
   #endif

   if ( value < 0 && value != r4check && value != e4unique ) return( r4check ) ;
   if ( value == r4check ) return( cb->errDefaultUnique ) ;
   temp = cb->errDefaultUnique ;
   cb->errDefaultUnique = value ;
   return( temp ) ;
}

short S4FUNCTION code4errorCode( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40105 ) ) return -1 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( error4code( cb ) ) ;
   temp = error4code( cb ) ;
   error4set( cb, value ) ;
   return( temp ) ;
}

/* Needed for CSA application */
short S4FUNCTION code4errorCode2( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40105 ) ) return -1 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( (short)error4code2( cb ) ) ;
   temp = (short)error4code2( cb ) ;
   error4set2( cb, value ) ;
   return( temp ) ;
}

short S4FUNCTION code4accessMode( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40106 ) ) return -1 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->accessMode ) ;
   temp = cb->accessMode ;
   cb->accessMode = value>0 ;
   return( temp ) ;
}

short S4FUNCTION code4errExpr( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40107 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->errExpr), value ) ;
}


short S4FUNCTION code4errFieldName( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40108 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->errFieldName), value ) ;
}

short S4FUNCTION code4fileFlush( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40152 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->fileFlush), value ) ;
}

short S4FUNCTION code4errGo( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40109 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->errGo), value ) ;
}

long S4FUNCTION code4hInst( CODE4 *cb, long value )
{
   #ifdef __WIN32
      void *temp ;
   #else
      short temp ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40110 ) ) return -1L ;
   #endif

   if ( value < 0 && value != r4check ) return (long) r4check ;
   if ( value == r4check ) return (long) cb->hInst ;
   temp = cb->hInst ;
   #ifdef __WIN32
      cb->hInst = (void*) value ;
   #else
      cb->hInst = (int) value ;
   #endif
   return (long)temp ;
}

long S4FUNCTION code4hWnd( CODE4 *cb, long value )
{
   #ifdef __WIN32
      void *temp ;
   #else
      short temp ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40111 ) ) return -1L ;
   #endif

   if ( value < 0 && value != r4check ) return (long) r4check ;
   if ( value == r4check ) return (long) cb->hWnd ;
   temp = cb->hWnd ;
   #ifdef __WIN32
      cb->hWnd = (void*) value ;
   #else
      cb->hWnd = (unsigned int) value ;
   #endif
   return (long) temp ;
}

const char S4PTR* S4FUNCTION code4indexExtensionVB( CODE4 *cb )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E91110 ) ) return 0 ;
   #endif

   return code4indexExtension( cb ) ;
}

short S4FUNCTION code4lockAttempts( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40112 ) ) return -1 ;
   #endif

   if ( value < -1 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->lockAttempts ) ;
   temp = cb->lockAttempts ;
   cb->lockAttempts = value ;
   return( temp ) ;
}

short S4FUNCTION code4lockAttemptsSingle( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40112 ) ) return -1 ;
   #endif

   if ( value < -1 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->lockAttemptsSingle ) ;
   temp = cb->lockAttemptsSingle ;
   cb->lockAttemptsSingle = value ;
   return( temp ) ;
}

long S4FUNCTION code4lockDelay( CODE4 *cb, long value )
{
   long temp ;

/*   'change error code */
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40112 ) ) return -1 ;
   #endif

   if (( value < 0 && value != r4check ) || ( value > UINT_MAX ) )
      return( r4check ) ;
   if ( value == r4check ) return( (long)cb->lockDelay ) ;

   temp = (long)cb->lockDelay ;
   cb->lockAttempts = (unsigned int)value ;

   return( temp ) ;
}

short S4FUNCTION code4lockEnforce( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, 0 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->lockEnforce), value ) ;
}

const char S4PTR* S4FUNCTION code4lockFileNameVB( CODE4 *cb )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E91011 ) ) return 0 ;
   #endif

   return code4lockFileName( cb ) ;
}

const char S4PTR* S4FUNCTION code4lockNetworkIdVB( CODE4 *cb )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E91009 ) ) return 0 ;
   #endif

   return code4lockNetworkId( cb ) ;
}

const char S4PTR* S4FUNCTION code4lockUserIdVB( CODE4 *cb )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E91010 ) ) return 0 ;
   #endif

   return code4lockUserId( cb ) ;
}

short S4FUNCTION code4log( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40154 ) ) return -1 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->log ) ;
   temp = cb->log ;
   cb->log = value ;
   return( temp ) ;
}

short S4FUNCTION code4memExpandBlock( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40113 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->memExpandBlock ) ;
   temp = cb->memExpandBlock ;
   cb->memExpandBlock = value ;
   return( temp ) ;
}

short S4FUNCTION code4memExpandData( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40114 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->memExpandData ) ;
   temp = cb->memExpandData ;
   cb->memExpandData = value ;
   return( temp ) ;
}

short S4FUNCTION code4memExpandIndex( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40115 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->memExpandIndex ) ;
   temp = cb->memExpandIndex ;
   cb->memExpandIndex = value ;
   return( temp ) ;
}

short S4FUNCTION code4memExpandLock( CODE4 *cb, short value )
{
   short temp ;

/*   'change error code */
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40115 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->memExpandLock ) ;
   temp = cb->memExpandLock ;
   cb->memExpandLock = value ;
   return( temp ) ;
}

short S4FUNCTION code4memExpandTag( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40116 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->memExpandTag ) ;
   temp = cb->memExpandTag ;
   cb->memExpandTag = value ;
   return( temp ) ;
}

long S4FUNCTION code4memSizeBlock( CODE4 *cb, long value )
{
   long temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40117 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return (long)  ( cb->memSizeBlock ) ;
   temp = (long) cb->memSizeBlock ;
   cb->memSizeBlock = (unsigned) value ;
   return( temp ) ;
}

long S4FUNCTION code4memSizeBuffer( CODE4 *cb, long value )
{
   long temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40118 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return (long) cb->memSizeBuffer ;
   temp = (long) cb->memSizeBuffer ;
   cb->memSizeBuffer = (unsigned) value ;
   return  ( temp ) ;
}

short S4FUNCTION code4memSizeMemo( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40119 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return (int) ( cb->memSizeMemo ) ;
   temp = (int) cb->memSizeMemo ;
   cb->memSizeMemo = (unsigned) value ;
   return( temp ) ;
}

long S4FUNCTION code4memSizeMemoExpr( CODE4 *cb, long value )
{
   long temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40120 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return (long) cb->memSizeMemoExpr ;
   temp = (long) cb->memSizeMemoExpr ;
   cb->memSizeMemoExpr = (unsigned) value ;
   return  ( temp ) ;
}

long S4FUNCTION code4memSizeSortBuffer( CODE4 *cb, long value )
{
   long temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40121 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return (long) cb->memSizeSortBuffer ;
   temp = (long) cb->memSizeSortBuffer ;
   cb->memSizeSortBuffer = (unsigned) value ;
   return  ( temp ) ;
}

long S4FUNCTION code4memSizeSortPool( CODE4 *cb, long value )
{
   long temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40122 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return (long) cb->memSizeSortPool ;
   temp = (long) cb->memSizeSortPool ;
   cb->memSizeSortPool = (unsigned) value ;
   return( temp ) ;
}

short S4FUNCTION code4memStartBlock( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40123 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->memStartBlock ) ;
   temp = cb->memStartBlock ;
   cb->memStartBlock = value ;
   return( temp ) ;
}

long S4FUNCTION code4memStartMax( CODE4 *cb, long value )
{
   long temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40124 ) ) return 0 ;
   #endif

   if( value < 0 && value != r4check )
      return( r4check ) ;
   if ( value == r4check ) return( cb->memStartMax ) ;
   temp = cb->memStartMax ;
   cb->memStartMax = value ;
   return( temp ) ;
}


short S4FUNCTION code4memStartData( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40125 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->memStartData ) ;
   temp = cb->memStartData ;
   cb->memStartData = value ;
   return( temp ) ;
}

short S4FUNCTION code4memStartIndex( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40126 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->memStartIndex ) ;
   temp = cb->memStartIndex ;
   cb->memStartIndex = value ;
   return( temp ) ;
}

short S4FUNCTION code4memStartLock( CODE4 *cb, short value )
{
   short temp ;

/*   'change error code */
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40115 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->memStartLock ) ;
   temp = cb->memStartLock ;
   cb->memStartLock = value ;
   return( temp ) ;
}

short S4FUNCTION code4memStartTag( CODE4 *cb, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40127 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->memStartTag ) ;
   temp = cb->memStartTag ;
   cb->memStartTag = value ;
   return( temp ) ;
}

short S4FUNCTION code4errOff( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40128 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->errOff), value ) ;
}

short S4FUNCTION code4errOpen( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40129 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->errOpen), value ) ;
}


short S4FUNCTION code4optimize( CODE4 *cb, short value )
{
   #ifndef S4OPTIMIZE_OFF
      short temp ;

      #ifdef S4VBASIC
         if ( c4parm_check( cb, 1, E40130 ) ) return -1 ;
      #endif

      if ( value < -1 && value != r4check ) return( r4check ) ;
      if ( value == r4check ) return( cb->optimize ) ;
      temp = cb->optimize ;
      cb->optimize = value ;
      return( temp ) ;
   #else
      return 0 ;
   #endif
}

short S4FUNCTION code4optimizeWrite( CODE4 *cb, short value )
{
   #ifndef S4OPTIMIZE_OFF
      short temp ;

      #ifdef S4VBASIC
         if ( c4parm_check( cb, 1, E40131 ) ) return -1 ;
      #endif

      if ( value < -1 && value != r4check ) return( r4check ) ;
      if ( value == r4check ) return( cb->S4CONV( optimizeWrite, optimize_write ) ) ;
      temp = cb->S4CONV( optimizeWrite, optimize_write ) ;
      cb->S4CONV( optimizeWrite, optimize_write )= value ;
      return( temp ) ;
   #else
      return 0 ;
   #endif
}

short S4FUNCTION code4readLock( CODE4 *cb, short value )
{
   short temp;
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40132 ) ) return -1 ;
   #endif

   if (value == r4check) return (!!(cb->readLock))*(-1);

   temp = cb->readLock;
   cb->readLock = !!cb->readLock;  /* any non-zero value becomes one */
   return (!!temp)*(-1);
}

short S4FUNCTION code4readOnly( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40133 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->readOnly), value ) ;
}

short S4FUNCTION code4errRelate( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40101 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->errRelate), value ) ;
}


short S4FUNCTION code4safety( CODE4 *cb, short value )
{
   short temp;
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40135 ) ) return -1 ;
   #endif

   if (value == r4check) return (!!(cb->safety))*(-1);

   return c4setLog( &(cb->safety), value ) ;
}

short S4FUNCTION code4singleOpen( CODE4 *cb, short value )
{
   short temp;
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, 0 ) ) return -1 ;
   #endif

   if (value == r4check) return (!!(cb->singleOpen))*(-1);

   return c4setLog( &(cb->singleOpen), value ) ;
}

short S4FUNCTION code4errSkip( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40136 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->errSkip), value ) ;
}

short S4FUNCTION code4errTagName( CODE4 *cb, short value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40137 ) ) return -1 ;
   #endif

   return c4setLog( &(cb->errTagName), value ) ;
}

short S4FUNCTION code4tranStatusVB( CODE4 *cb )
{

/*   'change error code */
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40133 ) ) return -1 ;
   #endif

   return code4tranStatus( cb ) ;
}

short S4FUNCTION code4unlockAutoVB( CODE4 *cb )
{

/*   'change error code */
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40133 ) ) return -1 ;
   #endif

   return code4unlockAuto( cb ) ;
}

short S4FUNCTION code4unlockAutoSetVB( CODE4 *cb, short value )
{

/*   'change error code */
   #ifdef S4VBASIC
      if ( c4parm_check( cb, 1, E40133 ) ) return -1 ;
   #endif

   return code4unlockAutoSet( cb, value ) ;
}

/*
   DATA4 CLASS TRANSLATIONS
   A lot of these functions required because VB-32 still uses 2 byte ints
   Others are required because of source code macros. A few are actually
   needed to make specific enhancements for CodeBase from VB (ie d4init)
*/


short S4FUNCTION d4appendStartVB( DATA4 *d4, short useMemoEntries )
{
   return (short) d4appendStart( d4, (int)useMemoEntries ) ;
}

short S4FUNCTION d4changedVB( DATA4 *d4, short flag )
{
   return (short) d4changed( d4, (int)flag ) ;
}

short S4FUNCTION d4fieldJVB( DATA4 *d4, short jField )
{
   return (short) d4fieldJ( d4, (int)jField ) ;
}

const char S4PTR* S4FUNCTION d4fileNameVB( DATA4 *d4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E40139 ) ) return 0 ;
   #endif

      return d4->dataFile->file.name ;

}

#ifndef S4VB_DOS
CODE4 S4PTR* S4FUNCTION d4init_v( void )
{
   CODE4 *c4 ;

   c4 = code4alloc( 1 ) ;
   if (c4==NULL)
      error4( 0, e4memory, 0 );
   c4->hWnd = 0 ;
   c4->hInst = cb5inst ;
   return( c4 ) ;
}

short S4FUNCTION d4initUndo( CODE4 *c4 )
{
   short rc ;

   #ifdef S4VBASIC
      if ( c4parm_check( c4, 1, E40138 ) ) return -1 ;
   #endif

   rc = code4initUndo( c4 ) ;

         /* The next line is commented out due to a change made to code4alloc function
   which will cause code4initUndo to deallocate the CODE4 structure.
   u4free(c4) ; */

   return rc ;
}
#endif /* ifndef S4VB_DOS */

short S4FUNCTION d4logVB( DATA4 *d4, short logFlag )
{
   return (short) d4log( d4, (int)logFlag ) ;
}

short S4FUNCTION d4optimizeVB( DATA4 *d4, short flag )
{
   return (short) d4optimize( d4, (int)flag ) ;
}

short S4FUNCTION d4optimizeWriteVB( DATA4 *d4, short flag )
{
   return (short) d4optimizeWrite( d4, (int)flag ) ;
}

long S4FUNCTION d4recWidth_v ( DATA4 *data )
{
   return (long)d4recWidth( data ) ;
}

short S4FUNCTION d4seekNVB( DATA4 *d4, const char* ptr, const short len )
{
   return (short) d4seekN( d4, ptr, len ) ;
}

short S4FUNCTION d4seekNextNVB( DATA4 *d4, const char* ptr, const short len )
{
   return (short) d4seekNextN( d4, ptr, len ) ;
}

short S4FUNCTION d4writeVB( DATA4 *data, const long recno )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E92601 ) ) return 0 ;
   #endif

   return d4write( data, recno ) ;
}


/* DATE4 CLASS TRANSLATIONS */

/* These functions needed because some date4 functions macroed to c4atol */

short S4FUNCTION date4day_v( char *dateString )
{
   return date4day( dateString ) ;
}

short S4FUNCTION date4month_v( char *dateString )
{
   return date4month( dateString ) ;
}

short S4FUNCTION date4year_v( char *dateString )
{
   return date4year( dateString ) ;
}


/* ERROR4 Class translations */

short S4FUNCTION error4VB( CODE4 *c4, short errCode, long extraInfo )
{
   return (short) error4( c4, (int)errCode, extraInfo ) ;
}

short S4FUNCTION error4describeVB( CODE4 *c4, short errCode, long extraInfo, char* desc1, char* desc2, char* desc3 )
{
   return (short) error4describe( c4, (int)errCode, extraInfo, desc1, desc2, desc3 ) ;
}

short S4FUNCTION error4fileVB( CODE4 *c4, char* fileName, short overWrite )
{
   return (short) error4file( c4, fileName, (int)overWrite ) ;
}

short S4FUNCTION error4setVB( CODE4 *c4, short errCode )
{
   return (short) error4set( c4, (int)errCode ) ;
}

/* EXPR4 CLASS TRANSLATIONS */

DATA4 S4PTR* S4FUNCTION expr4dataVB( EXPR4* expr )
{
   return expr4data( expr ) ;
}

void S4FUNCTION expr4freeVB( EXPR4 *expr )
{
   expr4free( expr ) ;
}

short S4FUNCTION expr4lenVB( EXPR4* expr )
{
   return expr4len( expr ) ;
}

short S4FUNCTION expr4typeVB( EXPR4* expr )
{
   return expr4type( expr ) ;
}

EXPR4 S4PTR* S4FUNCTION expr4parseVB( DATA4 *data, char S4PTR *string )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E90904 ) ) return 0 ;
   #endif

   return expr4parse( data, string ) ;

}


/* FIELD4 Class Translations */

void S4FUNCTION f4assignCharVB( FIELD4 *f4, short character )
{
   f4assignChar( f4, (int)character ) ;
}

void S4FUNCTION f4assignIntVB( FIELD4 *field, const int iValue )
{
   f4assignInt( field, (const short)iValue ) ;
}

void S4FUNCTION f4assignNVB( FIELD4 *field, const char* data, short dataLen )
{
   f4assignN( field, data, (int)dataLen ) ;
}

short S4FUNCTION f4memoAssignNVB( FIELD4 *field, char* data, short dataLen )
{
   return (short)f4memoAssignN( field, data, (int)dataLen ) ;
}


/* RELATE4 CLASS TRANSLATIONS */

DATA4 S4PTR* S4FUNCTION relate4data_v ( RELATE4 *r4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( r4, 5, E40140 ) ) return 0 ;
   #endif

   return relate4data( r4 ) ;
}

TAG4 S4PTR* S4FUNCTION relate4dataTag_v( RELATE4 *r4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( r4, 5, E40141 ) ) return 0 ;
   #endif

   return relate4dataTag( r4 ) ;
}


short S4FUNCTION relate4errorActionVB( RELATE4* r4, short action )
{
   return (short)relate4errorAction( r4, (int)action ) ;
}

short S4FUNCTION relate4freeVB( RELATE4* r4, short closeFiles )
{
   return (short)relate4free( r4, (int)closeFiles ) ;
}

RELATE4 S4PTR* S4FUNCTION relate4masterVB( RELATE4 *r4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( r4, 5, E40142 ) ) return 0 ;
   #endif

   return relate4master( r4 ) ;
}

#ifndef S4VB_DOS
const char S4PTR* S4FUNCTION relate4masterExprVB( RELATE4 *r4 )

{
   #ifdef S4VBASIC
      if ( c4parm_check( r4, 5, E40144 ) ) return 0 ;
   #endif

   return relate4masterExpr( r4 ) ;

}
#endif /* !S4VB_DOS */

short S4FUNCTION relate4matchLenVB( RELATE4* r4, short len )
{
   return (short)relate4matchLen( r4, (int)len ) ;
}

short S4FUNCTION relate4skipEnableVB( RELATE4* r4, short doEnable )
{
   return (short)relate4skipEnable( r4, (int)doEnable ) ;
}

RELATE4 S4PTR* S4FUNCTION relate4topMaster( RELATE4 *r4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( r4, 5, E40143 ) ) return 0 ;
   #endif

      return relate4masterVB( r4 ) ;
}

short S4FUNCTION relate4typeVB( RELATE4* r4, short type )
{
   return (short)relate4type( r4, (int)type ) ;
}


#ifndef S4OFF_REPORT

short S4FUNCTION report4screenBreaks( REPORT4* r4, short value )
{
   short temp ;

   #ifdef S4VBASIC
      if ( c4parm_check( r4, 6, E40155 ) ) return 0 ;
   #endif

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( r4->screen_breaks ) ;

   temp = r4->screen_breaks ;
   r4->screen_breaks = ( value > 0 ) ;
   return temp ;

}

RELATE4 S4PTR* S4FUNCTION report4relate( REPORT4* r4 )
{
   if ( c4parm_check( r4, 6, E40145 ) ) return 0 ;

   return ( r4->relate ) ;
}

#ifndef S4VB_DOS

short S4FUNCTION report4do_v( REPORT4 *r4 )
{
   HWND hWnd, userhWnd ;
   MSG msg ;
   LPCSTR class_name = "vbRepClass" ;
   LPCSTR window_name = "vbRepWindow" ;
   short rc ;

   if ( c4parm_check( r4, 6, E40201 ) ) return -1 ;

   if ( ! r4->hWnd ) return - 1 ;

   userhWnd = r4->hWnd ;

   /* if report is going to printer don't need a window */
   if ( r4->output_code == 0 ) return report4do(r4) ;

   report4registerVBClass( r4 ) ;

        hWnd = CreateWindow( "vbRepClass", "vbRepWindow",
           WS_OVERLAPPED, 0, 0, 0, 0, (HWND) NULL, (HMENU) NULL,
      cb5inst, (void FAR*) NULL) ;

/*      r4->codeBase->hInst, (void FAR*) NULL) ; */

   if ( ! hWnd ) return -1 ;

   r4->hWnd = hWnd ;

   rc = (int) SendMessage( hWnd, WM_USER, (WPARAM) userhWnd, (LPARAM) r4  ) ;

   if ( r4->output_code )
   {
      while ( GetMessage ( &msg, (HWND) NULL, (UINT) 0 , (UINT) 0 ) )
      {
         TranslateMessage( &msg ) ;
         DispatchMessage( &msg ) ;
      }
   }

   r4->hWnd = userhWnd ;

   return rc ;

}

void S4FUNCTION report4registerVBClass( REPORT4 *r4 )
{
   WNDCLASS wndclass ;

        wndclass.style = CS_GLOBALCLASS ;
        wndclass.lpfnWndProc = vbRepWndProc ;
        wndclass.cbClsExtra = 0 ;
        wndclass.cbWndExtra = 0 ;
        wndclass.hInstance = cb5inst ;
/*      wndclass.hInstance = GetWindowWord( r4->hWnd, GWW_HINSTANCE ) ; */
        wndclass.hIcon = LoadIcon( (HINSTANCE) NULL, IDI_APPLICATION) ;
        wndclass.hCursor = LoadCursor( (HINSTANCE) NULL,IDC_ARROW) ;
        wndclass.hbrBackground = GetStockObject(WHITE_BRUSH) ;
        wndclass.lpszMenuName = NULL ;
        wndclass.lpszClassName = "vbRepClass" ;

   RegisterClass(&wndclass) ;
}

long S4FUNCTION vbRepWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
   static HWND userhWnd ;

   switch ( message )
   {
/*
   case WM_CREATE :
      break ;
*/
   case WM_USER :
      userhWnd = (HWND) wParam ;
      EnableWindow( userhWnd, FALSE ) ;
      return (long) report4do( (REPORT4 *) lParam ) ;

   case CRM_REPORTCLOSED :
      EnableWindow( userhWnd, TRUE ) ;
      SetFocus( userhWnd ) ;
      /* SendMessage( hWnd, WM_DESTROY, (WPARAM) NULL, (LPARAM) NULL ) ; */
      SendMessage( hWnd, WM_CLOSE, (WPARAM) NULL, (LPARAM) NULL ) ;
      return 0 ;

   case WM_DESTROY :
      PostQuitMessage(0) ;
      return 0 ;
   }

   return DefWindowProc( hWnd, message, wParam, lParam ) ;
}

#endif   /* ifndef S4VB_DOS */
#endif   /* ifndef S4OFF_REPORT */


/* TAG4 CLASS TRANSLATIONS */

signed short S4FUNCTION t4descendingVB ( TAG4 *tag )
{
   /* returns :
      -1 error
      0  non-descending tag
      1  descending tag
   */

   char tagName[11] ;
   TAG4INFO *tagInfo ;
   short i = 0 ;
   short rc = -1 ;

   #ifdef S4VBASIC
      if ( c4parm_check( tag, 4, E40153 ) ) return rc ;
   #endif

   if ( tag == 0 ) return rc ;

   u4ncpy( tagName, tag->tagFile->alias, sizeof(tagName)) ;

   tagInfo = i4tagInfo(tag->index) ;

   if ( ! tagInfo ) return rc ;

   while ( tagInfo[i].name )
   {
      #ifdef S4NO_STRNICMP
         if ( strnicmp( tagInfo[i].name, tagName, strlen(tagInfo[i].name)) == 0 )
      #else
         if ( strcmpi( tagInfo[i].name, tagName ) == 0 )
      #endif
      {
         if ( tagInfo[i].descending == r4descending )
            rc = 1 ;
         else
            rc = 0 ;

         break ;
      }
      i++ ;
   }

   u4free( tagInfo ) ;
   return rc ;
}

const char S4PTR *S4FUNCTION t4exprVB( TAG4 *tag )
{
   return t4expr( tag ) ;
}

const char S4PTR *S4FUNCTION t4filterVB( TAG4 *tag )
{
   return t4filter( tag ) ;
}

short S4FUNCTION t4uniqueSetVB( TAG4* t4, const short uniqueCode )
{
   return (short)t4uniqueSet( t4, uniqueCode ) ;
}


#ifndef S4VB_DOS
TAG4 *S4FUNCTION t4open_v(DATA4 *d4, char *name)
{
   return t4open( d4, (INDEX4 *)0, name ) ;
}

void S4FUNCTION u4memCpy( char *dest, char *source, long len)
{
   memcpy(dest, source, (unsigned int)len) ;
}

short S4FUNCTION u4strLen(char *s)
{
   return (int) strlen(s) ;
}

/* Needed for CSA application */

long S4FUNCTION u4ptrPtr2Long( void **ptr, int index)
{
   return *(long *) ptr[index];
}

#endif   /* Not S4VB_DOS */

short S4FUNCTION v4vary ( EXPR4 *e4, char *temp )
{
   char *result ;
   short len, e_type;
   int dec, sign ;

   if( !( (temp) && (*temp) ) )
   return -1 ;

   if ( (len = expr4vary( e4, &result ) ) > 0 )
   {
      e_type = expr4type(e4) ;

      switch ( e_type )
      {
         case r4log:
            #ifdef S4WINCE
               _itoa( *(short *)result, temp, 10 ) ;
            #else
               itoa( *(short *)result, temp, 10 ) ;
            #endif
            break ;

         case r4date_doub:
         case r4num_doub:
            memcpy(temp, _ecvt( *(double *)result, strlen(result),&dec, &sign), strlen(temp)) ;
            break ;

         case r4date:
         case r4str:
         case r4num:
            if ( len > (int)strlen( temp ) )
               memcpy( temp, result, strlen(temp) ) ;
            else
               memcpy( temp, result, len ) ;
            break ;

         default:
            return -1 ;
      }
   }
   return len ;
}




/*
   Dec 6/95 - 'nulls' member added to FIELD4INFO struct, creating a
   naturally four byte aligned structured. Therefore this structure
   and functions are no longer required. However, it could be used
   again if the FIELD4INFO struct ever changes.


#ifdef __WIN32

   This function is required for VB 32-bit because it uses 4 byte alignment,
   which adds an extra two bytes to the FIELD4INFO structure, which is only
   10 bytes long.


typedef struct
{
   char S4PTR *name ;
   short int type ;
   unsigned short int len ;
   unsigned short int dec ;
   short int dummy ;
} FIELD4INFOVB32 ;

PUBLIC DATA4 * S4FUNCTION d4createVB( CODE4 *cb, char *name, FIELD4INFOVB32* fldInfo, TAG4INFO *tagInfo)
{
   int i = 0, numFlds = 0, numTags = 0 ;
   FIELD4INFO *fldInfoC ;
   DATA4 *data ;

   while ( fldInfo[i].name )
   {
      numFlds++ ;
      i++ ;
   }

   if ( ! (fldInfoC = (FIELD4INFO *)u4alloc( sizeof(FIELD4INFO) * (numFlds + 1) ) ) )
      error4describe( cb, e4create, E91401, name, "Out of Memory", 0 ) ;

   for( i = 0 ; i < numFlds ; i++ )
   {
      fldInfoC[i].name = fldInfo[i].name ;
      fldInfoC[i].type = fldInfo[i].type ;
      fldInfoC[i].len  = fldInfo[i].len ;
      fldInfoC[i].dec  = fldInfo[i].dec ;
      fldInfoC[i].nulls = fldInfo[i].nulls ;
   }

   data = d4create( cb, name, fldInfoC, tagInfo ) ;

   u4free( (void *)fldInfoC ) ;

   return data ;
}

#else

PUBLIC DATA4 * S4FUNCTION d4createVB( CODE4 *cb, char *name, FIELD4INFO* fldInfo, TAG4INFO *tagInfo)
{
   return d4create( cb, name, fldInfo, tagInfo ) ;
}

#endif

*/

#ifdef __cplusplus
   }
#endif

