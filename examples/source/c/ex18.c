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



void openAFile( CODE4 *cb )
{
   DATA4 *d ;

   /* 'd' falls out of scope.  Data file is still open*/
   d = d4open( cb, "info" ) ;
}


void main( void )
{
   CODE4 cb ;
	DATA4 *d ;

   code4init( &cb ) ;
   openAFile( &cb ) ;

   d = code4data( &cb, "info" ) ; /* obtain a new DATA4 structure*/

   if( d != NULL )
   {
      printf("INFO has %d records.\n", d4recCount( d )) ;
      d4top( d ) ;
      d4close( code4data( &cb, "info") ) ; /*an alternative way to close the file*/
   }

   code4initUndo( &cb ) ;
}
