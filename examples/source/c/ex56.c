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
   DATA4 *data ;
   TAG4 *tag ;

   code4init( &cb ) ;
   data = d4open( &cb, "info" ) ;
   tag = d4tagDefault( data ) ;

   error4exitTest( &cb ) ;
   d4tagSelect( data, tag ) ; /* select the default tag. */
   d4positionSet( data, .25 ) ; /* move one quarter through the index file.*/
   printf( "Record number: %d \n", d4recNo( data )) ;

   printf( "The current position is: %f \n", d4position( data )) ;

   code4initUndo( &cb ) ;
}
