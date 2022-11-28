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

    code4init( &cb ) ;
    data = d4open( &cb, "database") ;

    for( d4top( data ) ; !d4eof( data ) ; d4skip( data, 1L ) )
       d4blank( data )  ;
  /* blank out all records in the data file*/
    code4initUndo( &cb ) ;  /* close all files and release any memory */
}
