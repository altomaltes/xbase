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

#include "xbase/d4all.h"


void main( void )
{
   CODE4 cb ;
   DATA4 *infoFile ;

   code4init( &cb ) ;
   infoFile = d4open( &cb, "info" ) ;

   /* Go to the end of file and set the End of file flag*/
   d4goEof( infoFile ) ;

   /* Check to see if the end of file flag is set*/
   if( d4eof( infoFile )  )
   {
      printf( "This is always true\n") ;
      d4bottom( infoFile ) ; /* reset the eof flag*/
   }

   d4close( infoFile ) ;
   code4initUndo( &cb ) ;
}
