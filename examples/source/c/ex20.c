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
   DATA4  *data1, *data2;
   long numRec;

   code4init( &cb ) ;
   data1 = d4open( &cb, "database" ) ;
   data2 = d4open( &cb, "data2" ) ;

   d4top( data1 ) ;
   d4top( data2 ) ;

   d4lockAddFile( data1 ) ;
   d4lockAddAppend( data2 ) ;

   numRec = d4recCount( data2 ) ;
   d4lockAdd( data2, numRec ) ;
   d4lockAdd( data2, numRec-1 ) ;

   if( code4lock( &cb ) == r4success )
      printf( "All locks were successfully performed\n") ;

   code4initUndo( &cb ) ;
}
