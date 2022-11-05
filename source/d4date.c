/* d4date.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TUROBC__ */
#endif  /* S4UNIX */

#define  JULIAN_ADJUSTMENT    1721425L
#define  S4NULL_DATE          1.0E100   /* may not compile on some Op.Sys. */
                                        /* Set to MAXDOUBLE ?              */
static int monthTot[] =
    { 0, 0,  31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 } ;
         /* Jan Feb Mar  Apr  May  Jun        Jul  Aug  Sep  Oct  Nov  Dec
             31  28  31   30   31   30         31   31   30        31   30   31
         */

typedef struct
{
   char cdow[12] ;
}  DOW ;

typedef struct
{
   char cmonth[10] ;
} MONTH ;

static DOW dayOfWeek[] =
#ifndef S4LANGUAGE
{
   { "\0          " },
   { "Sunday\0    " },
   { "Monday\0    " },
   { "Tuesday\0   " },
   { "Wednesday\0 " },
   { "Thursday\0  " },
   { "Friday\0    " },
   { "Saturday\0  " },
} ;
#else
   #ifdef S4GERMAN
   {
      { "\0          " },
      { "Sonntag\0   " },
      { "Montag\0    " },
      { "Dienstag\0  " },
      { "Mittwoch\0  " },
      { "Donnerstag\0" },
      { "Freitag\0   " },
      { "Samstag\0   " },
   } ;
   #endif
   #ifdef S4FRENCH
   {
      { "\0          " },
      { "Dimanche\0  " },
      { "Lundi\0     " },
      { "Mardi\0     " },
      { "Mercredi\0  " },
      { "Jeudi\0     " },
      { "Vendredi\0  " },
      { "Samedi\0    " },
   } ;
   #endif
   #ifdef S4SWEDISH
   {
      { "\0          " },
      { "M†ndag\0    " },
      { "Tisdag\0    " },
      { "Onsdag\0    " },
      { "Torsdag\0   " },
      { "Fredag\0    " },
      { "L”rdag\0    " },
      { "S”ndag\0    " },
   } ;
   #endif
   #ifdef S4FINNISH
   {
      { "\0          " },
      { "Maanantai\0 " },
      { "Tiistai\0   " },
      { "Keskiviikko " },
      { "Torstai\0   " },
      { "Perjantai\0 " },
      { "Lauantai\0  " },
      { "Suununtai\0 " },
   } ;
   #endif
   #ifdef S4NORWEGIAN
   {
      { "\0          " },
      { "Mandag\0    " },
      { "Tirsdag\0   " },
      { "Onsdag\0    " },
      { "Torsdag\0   " },
      { "Fredag\0    " },
      { "L”rdag\0    " },
      { "S”ndag\0    " },
   } ;
   #endif
#endif

static MONTH monthOfYear[] =
#ifndef S4LANGUAGE
{
   { "\0        " },
   { "January\0 " },
   { "February\0" },
   { "March\0   " },
   { "April\0   " },
   { "May\0     " },
   { "June\0    " },
   { "July\0    " },
   { "August\0  " },
   { "September" },
   { "October\0 " },
   { "November\0" },
   { "December\0" },
} ;
#else
#ifdef S4GERMAN
{
   { "\0        " },
   { "Januar\0  " },
   { "Februar\0 " },
   { "M„rz\0    " },
   { "April\0   " },
   { "Mai\0     " },
   { "Juni\0    " },
   { "Juli\0    " },
   { "August\0  " },
   { "September" },
   { "Oktober\0 " },
   { "November\0" },
   { "Dezember\0" },
} ;
#endif
#ifdef S4FRENCH
{
   { "\0        " },
   { "Janvier\0 " },
   { "F‚vrier\0 " },
   { "Mars\0    " },
   { "Avril\0   " },
   { "Mai\0     " },
   { "Juin\0    " },
   { "Juillet\0 " },
   { "Ao–t\0    " },
   { "Septembre" },
   { "Octobre\0 " },
   { "Novembre\0" },
   { "D‚cembre\0" },
} ;
#endif
#ifdef S4SWEDISH
{
   { "\0        " },
   { "Januari\0 " },
   { "Februari\0" },
   { "Mars\0    " },
   { "April\0   " },
   { "Maj\0     " },
   { "Juni\0    " },
   { "Juli\0    " },
   { "Augusti\0 " },
   { "September" },
   { "Oktober\0 " },
   { "November\0" },
   { "December\0" },
} ;
#endif
#ifdef S4FINNISH
{
   { "\0        " },
   { "Tammikuu\0" },
   { "Helmikuu\0" },
   { "Maaliskuu" },
   { "Huhtikuu\0" },
   { "Toukokuu\0" },
   { "Kes„kuu\0 " },
   { "Hein„kuu\0" },
   { "Elokuu\0  " },
   { "Syyskuu\0 " },
   { "Lokakuu\0 " },
   { "Marraskuu" },
   { "Joulukuu\0" },
} ;
#endif
#ifdef S4NORWEGIAN
{
   { "\0        " },
   { "Januar\0  " },
   { "Februar\0 " },
   { "Mars\0    " },
   { "April\0   " },
   { "Mai\0     " },
   { "Juni\0    " },
   { "Juli\0    " },
   { "August \0 " },
   { "September" },
   { "Oktober\0 " },
   { "November\0" },
   { "Desember\0" },
} ;
#endif
#endif

#ifndef S4SERVER
const char *S4FUNCTION code4dateFormat( CODE4 *c4 )
{
   #ifdef E4PARM_HIGH
      if ( c4 == 0 )
      {
         error4( c4, e4parm_null, E96303 ) ;
         return 0 ;
      }
   #endif

   #ifdef S4SERVER
      return c4->currentClient->trans.dateFormat ;
   #else
      return c4->c4trans.trans.dateFormat ;
   #endif
}

int S4FUNCTION code4dateFormatSet( CODE4 *c4, const char *str )
{
   #ifdef S4CLIENT
      CONNECTION4 *connection ;
      CONNECTION4DATE_FORMAT_SET_INFO_IN *infoIn ;
      int rc ;
   #endif

   #ifdef E4PARM_HIGH
      if ( c4 == 0 || str == 0 )
         return error4( c4, e4parm_null, E96302 ) ;
      if ( strlen( str ) >= sizeof( c4->c4trans.trans.dateFormat ) )
         return error4( c4, e4parm, E96302 ) ;
   #endif

   #ifdef S4SERVER
      strcpy( c4->currentClient->trans.dateFormat, str ) ;
   #else
      strcpy( c4->c4trans.trans.dateFormat, str ) ;
      #ifdef S4CLIENT
         if ( c4->defaultServer.connected )
         {
            connection = &c4->defaultServer ;
            #ifdef E4ANALYZE
               if ( connection == 0 )
                  return error4( c4, e4struct, E96302 ) ;
            #endif
            connection4assign( connection, CON4DATE_FORMAT, 0L, 0L ) ;
            connection4addData( connection, NULL, sizeof(CONNECTION4DATE_FORMAT_SET_INFO_IN), (void **)&infoIn ) ;
            memcpy( infoIn->dateFormat, &c4->c4trans.trans.dateFormat, sizeof( c4->c4trans.trans.dateFormat ) ) ;
            connection4sendMessage( connection ) ;
            rc = connection4receiveMessage( connection ) ;
            if ( rc < 0 )
               return error4stack( c4, rc, E96302 ) ;
            rc = connection4status( connection ) ;
            if ( rc < 0 )
               connection4error( connection, c4, rc, E96302 ) ;
            return rc ;
         }
      #endif
   #endif

   return 0 ;
}
#endif

int S4FUNCTION date4isLeap( const char *date )
{
   int year ;

   year = date4year( date ) ;
   return ( ( ((year%4 == 0) && (year%100 != 0)) || (year%400 == 0 )) ?  1 : 0 ) ;
}

static int c4Julian( const int year, const int month, const int day )
{
   /*  Returns */
   /*     >0   The day of the year starting from 1 */
   /*          Ex.    Jan 1, returns  1 */
   /*     -1   Illegal Date */
   int isLeap, monthDays ;

   isLeap =  ( ((year%4 == 0) && (year%100 != 0)) || (year%400 == 0) ) ?  1 : 0 ;

   monthDays = monthTot[ month+1 ] -  monthTot[ month] ;
   if ( month == 2 )  monthDays += isLeap ;

   if ( year  < 0  ||
        month < 1  ||  month > 12  ||
        day   < 1  ||  day   > monthDays )
        return( -1 ) ;        /* Illegal Date */

   if ( month <= 2 )  isLeap = 0 ;

   return(  monthTot[month] + day + isLeap ) ;
}

static int c4monDy( const int year, const int days,  int *monthPtr,  int *dayPtr )
{
   /*  Given the year and the day of the year, returns the month and day of month. */
   int isLeap, i ;

   isLeap = ( ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0) ) ?  1 : 0 ;
   if ( days <= 59 )
      isLeap = 0 ;

   for( i = 2; i <= 13; i++)
   {
      if ( days <= monthTot[i] + isLeap )
      {
         *monthPtr = --i ;
         if ( i <= 2)
            isLeap = 0 ;

         *dayPtr = days - monthTot[ i] - isLeap ;
         return 0 ;
      }
   }
   *dayPtr = 0 ;
   *monthPtr = 0 ;

   return -1 ;
}

static long c4ytoj( const int y )
{
   int yr ;
   /*  Calculates the number of days to the year */
   /*  This calculation takes into account the fact that */
   /*     1)  Years divisible by 400 are always leap years. */
   /*     2)  Years divisible by 100 but not 400 are not leap years. */
   /*     3)  Otherwise, years divisible by four are leap years. */
   /*  Since we do not want to consider the current year, we will */
   /*  subtract the year by 1 before doing the calculation. */
   yr = y - 1 ;
   return( yr*365L +  yr/4L - yr/100L + yr/400L ) ;
}

int S4FUNCTION date4assign( char *datePtr, const long ldate )
{
   /* Converts from a Julian day to the dbf file date format. */
   long totDays ;
   int  iTemp, year, nDays, maxDays, month, day ;

   if ( ldate <= 0 )
   {
      memset( datePtr, ' ',  8 ) ;
      return 0L ;
   }

   totDays = ldate - JULIAN_ADJUSTMENT ;
   iTemp = (int)( (double)totDays / 365.2425 ) ;
   year = iTemp + 1 ;
   nDays = (int)( totDays - c4ytoj( year ) ) ;
   if ( nDays <= 0 )
   {
      year-- ;
      nDays = (int)( totDays - c4ytoj( year ) ) ;
   }

   if ( (( year % 4 == 0 ) && ( year % 100 )) || ( year % 400 == 0 ) )
      maxDays = 366 ;
   else
      maxDays = 365 ;

   if ( nDays > maxDays )
   {
      year++ ;
      nDays -= maxDays ;
   }

   #ifdef E4MISC
      if ( c4monDy( year, nDays, &month, &day ) < 0 )
         return error4( 0, e4result, E96301 ) ;
   #else
      c4monDy( year, nDays, &month, &day ) ;
   #endif

   c4ltoa45( (long)year, datePtr, -4 ) ;
   c4ltoa45( (long)month, datePtr + 4, -2 ) ;
   c4ltoa45( (long)day, datePtr + 6, -2 ) ;

   return 0 ;
}

S4CONST char *S4FUNCTION date4cdow( const char *datePtr )
{
   return dayOfWeek[date4dow(datePtr)].cdow ;
}

S4CONST char *S4FUNCTION date4cmonth( const char *datePtr )
{
   return monthOfYear[date4month( datePtr )].cmonth ;
}

/* inlined*/
/*int S4FUNCTION date4day( const char *datePtr )*/
/*{*/
/*   return (int)c4atol( datePtr + 6, 2 ) ;*/
/*}*/

int S4FUNCTION date4dow( const char *datePtr )
{
   long date ;
   date = date4long(datePtr) ;
   if ( date < 0 )
      return 0 ;
   return (int)( ( date + 1 ) % 7 ) + 1 ;
}

void S4FUNCTION date4format( const char *datePtr, char *result, char *picture )
{
   int rest, mNum ;
   unsigned int resultLen, length ;
   char *ptrEnd, *monthPtr, tChar ;

   resultLen = strlen( picture ) ;
   memset( result, ' ', resultLen ) ;

   c4upper( picture ) ;
   c4encode( result, datePtr, picture, "CCYYMMDD" ) ;

   ptrEnd = strchr( picture, 'M' ) ;
   if ( ptrEnd )
   {
      monthPtr = result+ (int)( ptrEnd - picture ) ;
      length = 0 ;
      while ( *(ptrEnd++) == 'M' )
         length++ ;

      if ( length > 2)
      {
         /* Convert from a numeric form to character format for month */
         if (!c4memcmp( datePtr+4, "  ", 2 ))   /* if blank month */
         {
            memset( monthPtr, ' ', length ) ;
            return ;
         }

         mNum = c4atoi( datePtr+4, 2) ;

         if ( mNum < 1)
            mNum = 1 ;
         if ( mNum > 12)
            mNum = 12 ;

         rest = length - 9 ;
         if (length > 9)
            length = 9 ;

         memcpy( monthPtr, monthOfYear[mNum].cmonth, length ) ;
         if (rest > 0)
            memset( monthPtr+length, (int) ' ', (size_t)rest ) ;

         tChar = monthOfYear[mNum].cmonth[length] ;
         if( tChar == '\0' || tChar == ' ' )
         {
            mNum = strlen(monthOfYear[mNum].cmonth) ;
            if ( (unsigned)mNum != length )
               monthPtr[mNum] = ' ' ;
         }
         #ifdef S4WINDOWS
            #ifndef S4WINCE
               OemToAnsi( result, result ) ;
            #endif
         #endif
      }
   }
}

double S4FUNCTION date4formatMdx( const char *datePtr )
{
   long ldate ;
   ldate = date4long(datePtr) ;
   if ( ldate == 0 )
      return (double) S4NULL_DATE ;  /* Blank or Null date */
   return (double)ldate ;
}

int S4FUNCTION date4formatMdx2( const char *datePtr, double *doubPtr )
{
   long ldate ;

   ldate = date4long(datePtr) ;
   if ( ldate == 0 )
      *doubPtr = (double)S4NULL_DATE ;  /* Blank or Null date */
   else
      *doubPtr = (double)ldate ;
   #ifdef S4BYTE_SWAP
      *doubPtr = x4reverseDouble(doubPtr) ;
   #endif

   return 0 ;
}

void S4FUNCTION date4init( char *datePtr, const char *dateData, char *picture )
{
   char *monthStart, monthData[10], buf[2] ;
   int yearCount, monthCount, dayCount, centuryCount, i, length ;
   long currentCentury ;
   #ifdef S4WINCE
      SYSTEMTIME st ;
   #else
      time_t currentTime ;
      struct tm *brokenTime ;
   #endif
   dayCount = 5 ;
   monthCount = 3 ;
   yearCount = 1 ;
   centuryCount= -1 ;

   memset( datePtr, ' ', 8 ) ;

   c4upper( picture ) ;
   for ( i=0; picture[i] != '\0'; i++ )
   {
      switch( picture[i] )
      {
         case 'D':
            if ( ++dayCount >= 8 )
               break ;
            datePtr[dayCount] = dateData[i] ;
            break ;
         case 'M':
            if ( ++monthCount >=6 )
               break ;
            datePtr[monthCount] = dateData[i] ;
            break ;
         case 'Y':
            if ( ++yearCount >= 4 )
               break ;
            datePtr[yearCount] = dateData[i] ;
            break ;
         case 'C':
            if ( ++centuryCount >= 2 )
               break ;
            datePtr[centuryCount] = dateData[i] ;
            break ;
         default:
            break ;
      }
   }

   if ( strcmp( datePtr, "        " ) == 0 )
      return ;

   if ( centuryCount == -1 )
   {
      /* check the century on the system clock if possible */
      #ifndef S4WINCE
         time( &currentTime ) ;
         brokenTime = localtime( &currentTime ) ;
         currentCentury = (1900 + brokenTime->tm_year) / 100 ;
      #else
         GetLocalTime(&st) ;
         currentCentury = st.wYear / 100 ;
      #endif
      c4ltoa45( currentCentury, datePtr, 2 ) ;
      /* memcpy( datePtr, "19", (size_t)2 ) ; */
   }
   if ( yearCount ==  1 )
      memcpy( datePtr + 2, "01", (size_t)2 ) ;
   if ( monthCount == 3 )
      memcpy( datePtr + 4, "01", (size_t)2 ) ;
   if ( dayCount == 5 )
      memcpy( datePtr + 6, "01", (size_t)2 ) ;

   if ( monthCount >= 6 )
   {
      /* Convert the Month from Character Form to Date Format */
      monthStart = strchr( picture, 'M' ) ;

      length = monthCount - 3 ;        /* Number of 'M' characters in picture */

      memcpy( datePtr+4, "  ", (size_t)2 ) ;

      if ( length > 3 )
         length = 3 ;
      memcpy( monthData, dateData + (int)( monthStart - picture ), (size_t)length) ;
      while ( length > 0 )
         if ( monthData[length-1] == ' ' )
            length-- ;
         else
            break ;

      monthData[length] = '\0' ;

      c4lower( monthData ) ;
      buf[0] = monthData[0] ;
      buf[1] = 0 ;
      c4upper(buf) ;
      monthData[0] = buf[0] ;

      if ( length > 0 )
         for( i = 1 ; i <= 12; i++ )
         {
            if ( c4memcmp( monthOfYear[i].cmonth, monthData, (size_t)length ) == 0 )
            {
               c4ltoa45( (long) i, datePtr+4, 2 ) ;  /* Found Month Match */
               break ;
            }
         }
   }

   for ( i = 0 ; i < 8 ; i++ )
      if ( datePtr[i] == ' ' )
         datePtr[i] = '0' ;
}

long S4FUNCTION date4long( const char *datePtr )
{
   /*  Returns: */
   /*    >0  -  Julian day */
   /*           That is the number of days since the date  Jan 1, 4713 BC */
   /*           Ex.  Jan 1, 1981 is  2444606 */
   /*     0  -  NULL Date (dbfDate is all blank) */
   /*    -1  -  Illegal Date */
   int  year, month, day, dayYear, i ;

   /* first verify that the input date is valid --> which returns a -1 */
   for ( i = 0 ; i < 8 ; i++ )
      if ( ( datePtr[i] < '0' || datePtr[i] > '9' ) && datePtr[i] != ' ' )
         return -1 ;

   year = c4atoi( datePtr, 4 ) ;
   if ( year == 0)
      if ( c4memcmp( datePtr, "        ", 8 ) == 0)
         return  0 ;

   month = c4atoi( datePtr + 4, 2 ) ;
   day = c4atoi( datePtr + 6, 2 ) ;
   dayYear = c4Julian( year, month, day ) ;
   if ( dayYear < 1 )    /* Illegal Date */
      return -1L ;

   return ( c4ytoj( year ) + dayYear + JULIAN_ADJUSTMENT ) ;
}

void S4FUNCTION date4timeNow( char *timeData )
{
   #ifdef S4WINCE
      SYSTEMTIME st;
      GetLocalTime( &st );
      c4ltoa45( (long)st.wHour, timeData, -2) ;
      c4ltoa45( (long)st.wMinute, timeData + 3, -2) ;
      c4ltoa45( (long)st.wSecond, timeData + 6, -2) ;
   #else
      #ifdef S4UNIX_THREADS
         long timeVal ;
         struct tm result ;

         time( (time_t *)&timeVal) ;
         localtime_r( (time_t *)&timeVal, &result) ;

         c4ltoa45( (long)result.tm_hour, timeData, -2) ;
         c4ltoa45( (long)result.tm_min, timeData + 3, -2) ;
         c4ltoa45( (long)result.tm_sec, timeData + 6, -2) ;
      #else
         time_t timeVal ;
         struct tm *tmPtr ;

         time( &timeVal) ;
         tmPtr = localtime( &timeVal) ;

         c4ltoa45( (long)tmPtr->tm_hour, timeData, -2) ;
         c4ltoa45( (long)tmPtr->tm_min, timeData + 3, -2) ;
         c4ltoa45( (long)tmPtr->tm_sec, timeData + 6, -2) ;
      #endif
   #endif
   timeData[2] = ':' ;
   timeData[5] = ':' ;
}

void S4FUNCTION date4today( char *datePtr )
{
   #ifdef S4WINCE
      SYSTEMTIME st;
      GetLocalTime( &st );

      c4ltoa45( st.wYear, datePtr, -4 ) ;
      c4ltoa45( st.wMonth, datePtr + 4, -2 ) ;
      c4ltoa45( st.wDay, datePtr + 6, -2 ) ;
   #else
      #ifdef S4UNIX_THREADS
         time_t timeVal ;
         struct tm result ;

         time( &timeVal ) ;
         localtime_r( &timeVal, &result ) ;

         c4ltoa45( 1900L + result.tm_year, datePtr, -4 ) ;
         c4ltoa45( (long)result.tm_mon + 1, datePtr + 4, -2 ) ;
         c4ltoa45( (long)result.tm_mday, datePtr + 6, -2 ) ;
      #else
         time_t timeVal ;
         struct tm *tmPtr ;

         time( &timeVal ) ;
         tmPtr = localtime( &timeVal ) ;

         c4ltoa45( 1900L + tmPtr->tm_year, datePtr, -4 ) ;
         c4ltoa45( (long)tmPtr->tm_mon + 1, datePtr + 4, -2 ) ;
         c4ltoa45( (long)tmPtr->tm_mday, datePtr + 6, -2 ) ;
      #endif
   #endif
}

/* inlined*/
/*int S4FUNCTION date4month( const char *datePtr )*/
/*{*/
/*   return (int)c4atol( datePtr + 4, 2 ) ;*/
/*}*/
/*int S4FUNCTION date4year( const char *yearPtr )*/
/*{*/
/*   return (int)c4atol( yearPtr, 4 ) ;*/
/*}*/


#ifdef S4VB_DOS


/* DATE Functions */

void date4assign_v( char *date, long julianDay )
{
  date4assign( c4buf, julianDay ) ;
  c4buf[8] = '\0' ;
  u4ctov( date, c4buf ) ;
}

char * date4cdow_v( char *date )
{
   return v4str( date4cdow( c4str(date) ) ) ;
}

char * date4cmonth_v( char *date )
{
   return v4str( date4cmonth( c4str(date) ) ) ;
}

int date4day_v( char *date )
{
  return date4day( c4str(date) ) ;
}

int date4dow_v( char *date )
{
  return date4dow( c4str(date) ) ;
}

void date4format_v( char *vDate, char *vRes, char *vPic )
{
 char cDate[9], cRes[255], cPic[255] ;

 u4vtoc( cDate, sizeof(cDate), vDate ) ;
 u4vtoc( cPic, sizeof(cPic), vPic ) ;

 date4format( cDate, cRes, cPic ) ;
 u4ctov( vRes, cRes ) ;

}

double date4formatMdx ( char *date )
{
 return date4formatMdx( c4str(date) ) ;
}

void date4init_v( char *vRes, char *vDate, char *vPic )
{
 char cRes[9],cDate[255],cPic[255] ;

 u4vtoc( cDate, sizeof(cDate), vDate ) ;
 u4vtoc( cPic, sizeof(cPic), vPic ) ;

 date4init( cRes, cDate, cPic ) ;

 cRes[8] = '\0' ;

 u4ctov( vRes, cRes ) ;
}

long date4long_v( char *date )
{
 return date4long( c4str(date) ) ;
}

int date4month_v( char *date )
{
 return date4month( c4str(date) ) ;
}

void date4today_v( char *vDate )
{
 date4today( c4buf ) ;
 u4ctov( vDate, c4buf ) ;
}

int date4year_v( char *date )
{
 return date4year( c4str(date) ) ;
}

#endif /* S4VB_DOS */
