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



static FIELD4INFO fieldArray[ ] =
{
    { "NAME_FIELD", 'C', 20, 0 },
    { "AGE_FIELD",  'N',  3, 0 },
    { "BIRTH_DATE", 'D',  8, 0 },
    { 0,0,0,0 }
};


void main( void )
{
    CODE4 cb ;
    DATA4 *data ;

	 code4init( &cb ) ;
    cb.safety = 0 ;       /* overwrite the file if it exists*/
    data = d4create( &cb, "NEWDBF", fieldArray, 0 ) ;
    error4exitTest( &cb ) ;

    if( cb.errorCode )
       printf( "An error occurred, NEWDBF not created\n") ;
    else
       printf( "Created successfully!\n") ;

    code4close( &cb ) ;
    code4initUndo( &cb ) ;
 }
