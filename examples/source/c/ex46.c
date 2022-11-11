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

#include "../../../source/d4all.h"


#ifdef __TURBOC__
   extern unsigned _stklen = 10000;
#endif

void main( void )
{
   CODE4 cb ;
   DATA4 *data ;
   INDEX4 *index ;

   code4init( &cb ) ;
   data = d4open( &cb, "INFO" ) ;

   /* Since CODE4.autoOpen is by default true (non-zero),
      the INFO index file should have been opened.*/
   index = d4index( data, "INFO" ) ;
   if ( index != NULL )
      printf( "INDEX: INFO has been opened\n") ;

   index = d4index( data, "JUNK" ) ;
   if( index == NULL )
      printf( "INDEX: JUNK has not been opened\n") ;

   d4close( data ) ;
   code4initUndo( &cb ) ;
}
