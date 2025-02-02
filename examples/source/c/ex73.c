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

/*ex73.c*/
#include "xbase/d4all.h"


void main( void )
{
   CODE4 cb ;
   DATA4 *data;
   long numRecs ;

   code4init( &cb ) ;
   cb.accessMode = OPEN4DENY_RW ; /* open exclusively to avoid corruption*/
   data = d4open( &cb, "info1" ) ;
   d4top( data ) ;

   /* Make all of the records in the data file the same as the first record*/

   for( numRecs = d4recCount( data ) ; numRecs > 1 ; numRecs-- )
      if( d4write( data, numRecs ) != 0 )
         break ;

   d4close( data ) ;
   code4initUndo( &cb ) ;
}
