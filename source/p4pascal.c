/* p4pascal.c  (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved.  */

#include "d4all.h"
#include <windows.h>

#ifdef S4PASCAL

#ifdef S4WINDOWS
   extern HINSTANCE cb5inst ;
#endif

#define r4check    -5
#define E4MESSAGE "Function was passed an invalid structure pointer"


/*
     Since a user in CodePascal cannot create a structure variable (can only
     declare a pointer to one), functions must exist to do the allocations
     and frees.  The following have been done :

     Structure              Allocation               Free
     ------------------------------------------------------------------

     CODE4                  code4init              code4initUndo
*/

/*********************************************************************
  Begin Prototypes
*********************************************************************/

int S4FUNCTION code4accessMode                ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4autoOpen                  ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4codePage                  ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4collatingSequence         ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4createTemp                ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4errDefaultUnique          ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4errCreate                 ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4errExpr                   ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4errFieldName              ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4errGo                     ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4errOff                    ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4errOpen                   ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4errorCode                 ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4errRelate                 ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4errSkip                   ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4errTagName                ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4fileFlush                 ( CODE4 S4PTR *, int ) ;
HANDLE code4hInst                             ( CODE4 S4PTR *, HANDLE ) ;
HANDLE code4hWnd                              ( CODE4 S4PTR *, HANDLE ) ;
int S4FUNCTION code4lockAttempts              ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4lockAttemptsSingle        ( CODE4 S4PTR *, int ) ;
unsigned S4FUNCTION code4lockDelay            ( CODE4 S4PTR *, unsigned ) ;
void S4FUNCTION code4lockClearP               ( CODE4 S4PTR * ) ;
int S4FUNCTION code4lockEnforce               ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4optimize                  ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4optimizeWrite             ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4memExpandBlock            ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4memExpandData             ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4memExpandIndex            ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4memExpandLock             ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4memExpandTag              ( CODE4 S4PTR *, int ) ;
unsigned S4FUNCTION code4memSizeBlock         ( CODE4 S4PTR *, unsigned ) ;
unsigned S4FUNCTION code4memSizeBuffer        ( CODE4 S4PTR *, unsigned ) ;
unsigned S4FUNCTION code4memSizeMemo          ( CODE4 S4PTR *, unsigned ) ;
unsigned S4FUNCTION code4memSizeMemoExpr      ( CODE4 S4PTR *, unsigned ) ;
unsigned S4FUNCTION code4memSizeSortBuffer    ( CODE4 S4PTR *, unsigned ) ;
unsigned S4FUNCTION code4memSizeSortPool      ( CODE4 S4PTR *, unsigned ) ;
unsigned S4FUNCTION code4memStartBlock        ( CODE4 S4PTR *, unsigned ) ;
int S4FUNCTION code4memStartLock              ( CODE4 S4PTR *, int ) ;
unsigned S4FUNCTION code4memStartData         ( CODE4 S4PTR *, unsigned ) ;
unsigned S4FUNCTION code4memStartIndex        ( CODE4 S4PTR *, unsigned ) ;
unsigned long S4FUNCTION code4memStartMax     ( CODE4 S4PTR *, unsigned long ) ;
unsigned S4FUNCTION code4memStartTag          ( CODE4 S4PTR *, unsigned ) ;
int S4FUNCTION code4readLock                  ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4readOnly                  ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4safety                    ( CODE4 S4PTR *, int ) ;
int S4FUNCTION code4singleOpen                ( CODE4 S4PTR *, int ) ;

int S4FUNCTION code4tranStatusP               ( CODE4 S4PTR * ) ;
int S4FUNCTION code4unlockAutoP               ( CODE4 S4PTR * ) ;
void S4FUNCTION code4unlockAutoSetP           ( CODE4 S4PTR *, int ) ;


RELATE4 S4PTR * S4FUNCTION report4relate( REPORT4 S4PTR * ) ;
short S4FUNCTION report4screenBreaks( REPORT4 S4PTR*, short ) ;
short S4FUNCTION report4doP(REPORT4 *r4);

#ifndef S4OFF_REPORT
   void S4FUNCTION report4registerDelClass( REPORT4 *r4 );
   long S4FUNCTION delRepWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
#endif

CODE4 S4PTR * S4FUNCTION code4initP( void ) ;
int S4FUNCTION code4initUndoP ( CODE4 S4PTR * ) ;

S4EXPORT int S4FUNCTION d4logStatusP( DATA4 S4PTR * ) ;
S4EXPORT int S4FUNCTION d4tagSelectP( DATA4 S4PTR *, TAG4 S4PTR * ) ;
int S4FUNCTION d4writeP( DATA4 S4PTR *, long ) ;

int S4FUNCTION expr4lenP                  ( EXPR4 S4PTR * ) ;
DATA4 S4PTR * S4FUNCTION expr4dataP       ( EXPR4 S4PTR * ) ;
EXPR4 S4PTR * S4FUNCTION expr4parseP      ( DATA4 S4PTR * e4, char S4PTR * ) ;
int S4FUNCTION expr4typeP                 ( EXPR4 S4PTR * ) ;

DATA4 S4PTR * S4FUNCTION relate4dataP       ( RELATE4 S4PTR * ) ;
TAG4 S4PTR * S4FUNCTION relate4dataTagP     ( RELATE4 S4PTR * ) ;
RELATE4 S4PTR * S4FUNCTION relate4masterP   ( RELATE4 S4PTR * ) ;
S4EXPORT S4CONST char S4PTR * S4FUNCTION relate4masterExprP ( RELATE4 S4PTR * ) ;

S4EXPORT S4CONST char S4PTR * S4FUNCTION t4exprP    ( TAG4 S4PTR * ) ;
S4EXPORT S4CONST char S4PTR * S4FUNCTION t4filterP  ( TAG4 S4PTR * ) ;
S4EXPORT S4CONST TAG4 *S4FUNCTION t4openP (DATA4 S4PTR *d4, INDEX4 S4PTR *i4ndx, S4CONST char *fileName);

/*********************************************************************
  End Prototypes
*********************************************************************/
void pascal4structCheck( CODE4 * ) ;

void pascal4structCheck( CODE4 *cb )
{
   if ( cb == 0 )
   {
      error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL ) ;
      FatalAppExit( 0, E4_MESSAG_EXI ) ;
   }
}
/*********************************************************************
  CODE4 members
*********************************************************************/
int S4FUNCTION code4accessMode( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->accessMode ;
   if ( value != r4check )
      cb->accessMode = value > 0 ;
   return temp ;
}

int S4FUNCTION code4autoOpen( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->autoOpen ;
   if ( value != r4check )
      cb->autoOpen = value > 0 ;
   return temp ;
}

/* New for CP6 */
#ifndef S4CB51
int S4FUNCTION code4codePage( CODE4 *cb, int value )
{
   int temp ;

      pascal4structCheck( cb );

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->codePage ) ;
   temp = cb->codePage ;
   cb->codePage = value ;

   return( temp ) ;
}

int S4FUNCTION code4collatingSequence( CODE4 *cb, int value )
{
   int temp;

      pascal4structCheck( cb );

   if ( value < 0 && value != r4check ) return( r4check ) ;
   if ( value == r4check ) return( cb->collatingSequence ) ;
   temp = cb->collatingSequence ;
   cb->collatingSequence = value ;

   return( temp ) ;
}
#endif

int S4FUNCTION code4createTemp( CODE4 *cb, int value )
{
   int temp ;

   pascal4structCheck( cb ) ;

   temp = cb->createTemp ;
   if ( value != r4check )
      cb->createTemp = value > 0 ;

   return temp ;
}

int S4FUNCTION code4errDefaultUnique( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->errDefaultUnique ;
   if ( value != r4check )
      cb->errDefaultUnique = value ;
   return temp ;
}

int S4FUNCTION code4errCreate( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->errCreate ;
   if ( value != r4check )
      cb->errCreate = value > 0 ;
   return temp ;
}

int S4FUNCTION code4errExpr( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->errExpr ;
   if ( value != r4check )
      cb->errExpr = value > 0 ;
   return temp ;
}

int S4FUNCTION code4errFieldName( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->errFieldName ;
   if ( value != r4check )
      cb->errFieldName = value > 0 ;
   return temp ;
}

int S4FUNCTION code4errGo( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->errGo ;
   if ( value != r4check )
      cb->errGo = value > 0 ;
   return temp ;
}

int S4FUNCTION code4errOff( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->errOff ;
   if ( value != r4check )
      cb->errOff = value > 0 ;
   return temp ;
}

int S4FUNCTION code4errOpen( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->errOpen ;
   if ( value != r4check )
      cb->errOpen = value > 0 ;
   return temp ;
}

int S4FUNCTION code4errorCode( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = error4code( cb ) ;
   if ( value != r4check )
      error4set( cb, value ) ;
   return temp ;
}

int S4FUNCTION code4errRelate( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->errRelate ;
   if ( value != r4check )
      cb->errRelate = value > 0 ;
   return temp ;
}

int S4FUNCTION code4errSkip( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->errSkip ;
   if ( value != r4check )
      cb->errSkip = value > 0 ;
   return temp ;
}

int S4FUNCTION code4errTagName( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->errTagName ;
   if ( value != r4check )
      cb->errTagName = value > 0 ;
   return temp ;
}

int S4FUNCTION code4fileFlush ( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   if ( value != r4check )
           temp = cb->fileFlush ;
   cb->fileFlush = value > 0 ;
   return temp ;
}

HANDLE code4hInst ( CODE4 *cb, HANDLE h )
{
   HANDLE hTemp ;

        pascal4structCheck( cb ) ;

   hTemp = cb->hInst ;
   if ( (int) h != r4check )
      cb->hInst = h ;
   return hTemp ;
}


HANDLE code4hWnd ( CODE4 *cb, HANDLE h )
{
   HANDLE hTemp ;

        pascal4structCheck( cb ) ;

   hTemp = cb->hWnd ;
   cb->hWnd = h ;
   return hTemp ;
}

int S4FUNCTION code4lockAttempts( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->lockAttempts ;
   if ( value != r4check )
      cb->lockAttempts = value ;
   return temp ;
}

int S4FUNCTION code4lockAttemptsSingle( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->lockAttemptsSingle ;
   if ( value != r4check )
      cb->lockAttemptsSingle = value ;
   return temp ;
}

void S4FUNCTION code4lockClearP( CODE4 S4PTR *cb )
{
   code4lockClear( cb ) ;
}

unsigned S4FUNCTION code4lockDelay( CODE4 *cb, unsigned value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->lockDelay ;
   if ( (int) value != r4check )
      cb->lockDelay = value ;
   return temp ;
}

int S4FUNCTION code4lockEnforce( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->lockEnforce ;
   if ( value != r4check )
      cb->lockEnforce = value ;
   return temp ;
}

int S4FUNCTION code4log( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->log ;
   if ( value != r4check )
      cb->log = value ;
   return temp ;
}

int S4FUNCTION code4optimize( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->optimize ;
   if ( value != r4check )
      cb->optimize = value ;
   return temp ;
}

int S4FUNCTION code4memExpandBlock( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memExpandBlock ;
   if ( value != r4check )
      cb->memExpandBlock = value ;
   return temp ;
}

int S4FUNCTION code4memExpandData( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memExpandData ;
   if ( value != r4check )
      cb->memExpandData = value ;
   return temp ;
}

int S4FUNCTION code4memExpandIndex( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memExpandIndex ;
   if ( value != r4check )
      cb->memExpandIndex = value ;
   return temp ;
}

int S4FUNCTION code4memExpandLock( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memExpandLock ;
   if ( value != r4check )
      cb->memExpandLock = value ;
   return temp ;
}

int S4FUNCTION code4memExpandTag( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memExpandTag ;
   if ( value != r4check )
      cb->memExpandTag = value ;
   return temp ;
}

unsigned S4FUNCTION code4memSizeBlock( CODE4 *cb, unsigned value )
{
   unsigned temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memSizeBlock ;
   if ( (int) value != r4check )
      cb->memSizeBlock = value ;
   return temp ;
}

unsigned S4FUNCTION code4memSizeBuffer( CODE4 *cb, unsigned value )
{
   unsigned temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memSizeBuffer ;
   if ( (int) value != r4check )
      cb->memSizeBuffer = value ;
   return temp ;
}

unsigned S4FUNCTION code4memSizeMemo( CODE4 *cb, unsigned value )
{
   unsigned temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memSizeMemo ;
   if ( (int) value != r4check )
      cb->memSizeMemo = value ;
   return temp ;
}

unsigned S4FUNCTION code4memSizeMemoExpr( CODE4 *cb, unsigned value )
{
   unsigned temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memSizeMemoExpr ;
   if ( (int) value != r4check )
      cb->memSizeMemoExpr = value ;
   return temp ;
}

unsigned S4FUNCTION code4memSizeSortBuffer( CODE4 *cb, unsigned value )
{
   unsigned temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memSizeSortBuffer ;
   if ( (int) value != r4check )
      cb->memSizeSortBuffer = value ;
   return temp ;
}

unsigned S4FUNCTION code4memSizeSortPool( CODE4 *cb, unsigned value )
{
   unsigned temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memSizeSortPool ;
   if ( (int) value != r4check )
      cb->memSizeSortPool = value ;
   return temp ;
}

unsigned S4FUNCTION code4memStartBlock( CODE4 *cb, unsigned value )
{
   unsigned temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memStartBlock ;
   if ( (int) value != r4check )
      cb->memStartBlock = value ;
   return temp ;
}

int S4FUNCTION code4memStartLock (CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memStartLock ;
   if ( value != r4check )
      cb->memStartLock = value ;

   return temp ;
}

unsigned S4FUNCTION code4memStartData( CODE4 *cb, unsigned value )
{
   unsigned temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memStartData ;
   if ( (int) value != r4check )
      cb->memStartData = value ;
   return temp ;
}

unsigned S4FUNCTION code4memStartIndex( CODE4 *cb, unsigned value )
{
   unsigned temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memStartIndex ;
   if ( (int) value != r4check )
      cb->memStartIndex = value ;
   return temp ;
}

unsigned long S4FUNCTION code4memStartMax( CODE4 *cb, unsigned long value )
{
   unsigned long temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memStartMax ;
   if ( (int) value != r4check )
      cb->memStartMax = value ;
   return temp ;
}

unsigned S4FUNCTION code4memStartTag( CODE4 *cb, unsigned value )
{
   unsigned temp ;

        pascal4structCheck( cb ) ;

   temp = cb->memStartTag ;
   if ( (int) value != r4check )
      cb->memStartTag = value ;
   return temp ;
}

int S4FUNCTION code4readLock( CODE4 *cb, int value )
{
   int temp ;

   pascal4structCheck( cb ) ;

   temp = c4getReadLock( cb ) ;
   if ( value != r4check )
      c4setReadLock( cb, ( value > 0 ) ) ;
   return temp ;
}

int S4FUNCTION code4readOnly( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->readOnly ;
   if ( value != r4check )
      cb->readOnly = value > 0 ;
   return temp ;
}

int S4FUNCTION code4safety( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->safety ;
   if ( value != r4check )
      cb->safety = value > 0 ;
   return temp ;
}

int S4FUNCTION code4singleOpen( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->singleOpen ;
   if ( value != r4check )
      cb->singleOpen = value > 0 ;
   return temp ;
}

int S4FUNCTION code4tranStatusP( CODE4 S4PTR * c4 )
{
           int rc ;

      pascal4structCheck( c4 ) ;
           rc = c4->c4trans.trans.currentTranStatus ;

           return rc ;
}


int S4FUNCTION code4optimizeWrite( CODE4 *cb, int value )
{
   int temp ;

        pascal4structCheck( cb ) ;

   temp = cb->optimizeWrite ;
   if ( value != r4check )
      cb->optimizeWrite = value ;
   return temp ;
}

#ifdef S4SINGLE
int S4FUNCTION code4unlockAutoP( CODE4 *c4 )
{
        return 0 ;
}
void S4FUNCTION code4unlockAutoSetP( CODE4 *c4, int val )
{
}

#else
int S4FUNCTION code4unlockAutoP( CODE4 S4PTR *c4 )
{
   pascal4structCheck( c4 ) ;

   return c4->c4trans.trans.unlockAuto ;
}
void S4FUNCTION code4unlockAutoSetP( CODE4 S4PTR *c4, int val )
{
   pascal4structCheck( c4 ) ;

   c4->c4trans.trans.unlockAuto = val ;
}
#endif

/*********************************************************************
  REPORT4 members
*********************************************************************/

RELATE4 S4PTR * S4FUNCTION report4relate( REPORT4 *r )
{
   #ifdef S4OFF_REPORT
      error4( r->codeBase, e4notSupported, E40145 ) ;
      return 0;
   #else
      if ( r == 0 )
      {
         error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL ) ;
         FatalAppExit( 0, E4_MESSAG_EXI ) ;
      }

      return r->relate ;
   #endif
}

short S4FUNCTION report4screenBreaks( REPORT4* r4, short value )
{
   #ifdef S4OFF_REPORT
      return error4( r4->codeBase, e4notSupported, E40155 ) ;
   #else
      short temp ;

      if ( r4 == 0 )
      {
         error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL ) ;
         FatalAppExit( 0, E4_MESSAG_EXI ) ;
      }

      if ( value < 0 && value != r4check ) return( r4check ) ;
      if ( value == r4check ) return( r4->screen_breaks ) ;

      temp = r4->screen_breaks ;

      if( value != r4check )
         r4->screen_breaks = ( value > 0 ) ;

      return temp ;
   #endif
}

short S4FUNCTION report4doP( REPORT4 *r4 )
{
   #ifdef S4OFF_REPORT
      return error4( r4->codeBase, e4notSupported, E40201 ) ;
   #else
      HWND hWnd, userhWnd ;
      MSG msg ;
      LPCSTR class_name = "delRepClass" ;
      LPCSTR window_name = "delRepWindow" ;
      short rc ;

      if ( r4 == 0 )
      {
         error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL );
         FatalAppExit( 0, E4_MESSAG_EXI ) ;
      }

      if ( ! r4->hWnd ) return - 1 ;

      userhWnd = r4->hWnd ;

      /* if report is going to printer don't need a window */
      if ( r4->output_code == 0 ) return report4do(r4) ;

      report4registerDelClass( r4 )  ;

           hWnd = CreateWindow( "delRepClass", "delRepWindow",
              WS_OVERLAPPED, 0, 0, 0, 0, (HWND) NULL, (HMENU) NULL,
         cb5inst, (void FAR*) NULL);

   /*      r4->codeBase->hInst, (void FAR*) NULL); */

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
   #endif
}

#ifndef S4OFF_REPORT
void S4FUNCTION report4registerDelClass( REPORT4 *r4 )
{
   WNDCLASS wndclass ;

        wndclass.style = CS_GLOBALCLASS ;
        wndclass.lpfnWndProc = delRepWndProc;
        wndclass.cbClsExtra = 0;
        wndclass.cbWndExtra = 0 ;
        wndclass.hInstance = cb5inst ;
/*      wndclass.hInstance = GetWindowWord( r4->hWnd, GWW_HINSTANCE ) ; */
        wndclass.hIcon = LoadIcon( (HINSTANCE) NULL, IDI_APPLICATION) ;
        wndclass.hCursor = LoadCursor( (HINSTANCE) NULL,IDC_ARROW);
        wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
        wndclass.lpszMenuName = NULL;
        wndclass.lpszClassName = "delRepClass";

   RegisterClass(&wndclass) ;
}

long S4FUNCTION delRepWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
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
      SendMessage( hWnd, WM_DESTROY, (WPARAM) NULL, (LPARAM) NULL ) ;
      return 0;

   case WM_DESTROY :
      PostQuitMessage(0) ;
      return 0 ;
   }

   return DefWindowProc( hWnd, message, wParam, lParam ) ;
}

#endif /* S4OFF_REPORT */






/*********************************************************************
  CODE4 translations
*********************************************************************/

CODE4 * S4FUNCTION code4initP( void )
{
   CODE4 *c4 ;

   #ifndef __WIN32
      SetHandleCount( 100 );
   #endif

         c4 = code4alloc( 1 ) ;
         if (c4==NULL)
   {
        error4( 0, e4memory, 0 );
   }
   c4->hWnd = 0 ;

   #ifndef S4PASCAL_DOS
      #ifdef __WIN32
         c4->hInst = (void S4PTR *)GetWindowLong( 0, GWL_HINSTANCE ) ;
      #else
         c4->hInst = GetWindowWord( 0, GWW_HINSTANCE ) ;
      #endif
   #endif /* S4PASCAL_DOS */

   return c4 ;
}

int S4FUNCTION code4initUndoP( CODE4 *c4 )
{
   int rc ;

   pascal4structCheck( c4 );

   rc = code4initUndo( c4 ) ;

   /* not needed because code4initUndo has already unallocated the CODE4
   structure
   u4free( c4 ) ;*/

   return rc ;
}

int S4FUNCTION d4tagSelectP( DATA4 S4PTR * d4, TAG4 S4PTR * tag )
{
   #ifdef E4PARM_NULL
      if ( d4 == 0 || tag == 0 )
         return error4( 0, e4parmNull, E92409 ) ;
   #endif

   d4tagSelect( d4, tag ) ;
   return d4->codeBase->errorCode ;
}


/******************************************************************************
   Non-user functions.  Used by CodePascal to access internal values of
   some structure, and to access functions that are only #defined in CodeBase,
   since these cannot be exported to Pascal.

        d4write, t4filter, are here because of conflicts in Delphi accessing
        MicroSoft compiled DLLs.

   The 'P' is added to avoid conflicts.
******************************************************************************/

S4EXPORT int S4FUNCTION d4logStatusP( DATA4 S4PTR *d4 )
{
   if ( d4 == 0 )
   {
      error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL ) ;
      FatalAppExit( 0, E4_MESSAG_EXI ) ;
   }

   return d4logStatus( d4 ) ;
}

int S4FUNCTION d4writeP( DATA4 S4PTR * d4, long rec )
{
        return d4writeLow( d4, rec, 0 ) ;
}

DATA4 S4PTR * S4FUNCTION expr4dataP( EXPR4 S4PTR *e4 )
{
   if ( e4 == 0 )
   {
        error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL ) ;
      FatalAppExit( 0, E4_MESSAG_EXI ) ;
    }

        return e4->data ;
}

int S4FUNCTION expr4lenP( EXPR4 *e4 )
{
        if ( e4 == 0 )
        {
           error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL ) ;
           FatalAppExit( 0, E4_MESSAG_EXI ) ;
        }

        return e4->len ;
}

EXPR4 S4PTR * S4FUNCTION expr4parseP( DATA4 S4PTR * d4, char S4PTR * e )
{
        EXPR4 * temp ;

        if ( d4 == 0 )
        {
           error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL ) ;
           FatalAppExit( 0, E4_MESSAG_EXI ) ;
        }

        temp = expr4parse( d4, e ) ;
        return temp ;
}

int S4FUNCTION expr4typeP( EXPR4 S4PTR * e4)
{
        if ( e4 == 0 )
        {
           error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL ) ;
           FatalAppExit( 0, E4_MESSAG_EXI ) ;
        }

        return e4->type ;
}

DATA4 S4PTR * S4FUNCTION relate4dataP( RELATE4 *r4 )
{
        if ( r4 == 0 )
        {
           error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL ) ;
           FatalAppExit( 0, E4_MESSAG_EXI ) ;
        }

        return r4->data ;
}

TAG4 S4PTR * S4FUNCTION relate4dataTagP       ( RELATE4 *r4 )
{
        if ( r4 == 0 )
        {
           error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL ) ;
           FatalAppExit( 0, E4_MESSAG_EXI ) ;
        }

        return r4->dataTag ;
}

RELATE4 S4PTR * S4FUNCTION relate4masterP   ( RELATE4 *r4 )
{
        if ( r4 == 0 )
        {
           error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL ) ;
           FatalAppExit( 0, E4_MESSAG_EXI ) ;
        }

        return r4->master ;
}

S4EXPORT S4CONST char S4PTR * S4FUNCTION relate4masterExprP ( RELATE4 *r4 )
{
        if ( r4 == 0 )
        {
           error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL ) ;
           FatalAppExit( 0, E4_MESSAG_EXI ) ;
        }

        return relate4masterExpr( r4 ) ; /* (r4)->masterExpr->source ; */
}

S4EXPORT S4CONST char S4PTR * S4FUNCTION t4exprP( TAG4 S4PTR *t4 )
{
        if ( t4 == 0 )
        {
           error4out( 0, e4info, 0, E4MESSAGE, NULL , NULL ) ;
           FatalAppExit( 0, E4_MESSAG_EXI ) ;
        }

        #ifdef S4CLIENT
        return t4exprLow( t4 ); /* (t4)->tagFile->expr->source ; */
        #else
        if ( (t4)->tagFile->expr )
           return (t4)->tagFile->expr->source ;
        return NULL ;
        #endif /* S4CLIENT */
}

S4EXPORT S4CONST char S4PTR * S4FUNCTION t4filterP  ( TAG4 S4PTR *t4 )
{
        #ifdef S4CLIENT
                return t4filterLow( t4 ) ;
        #else
        if ( (t4)->tagFile->filter )
           return (t4)->tagFile->filter->source ;
        return NULL ;
        #endif ;
}

S4EXPORT S4CONST TAG4 *S4FUNCTION t4openP (DATA4 S4PTR *d4, INDEX4 S4PTR *i4ndx, S4CONST char *fileName)
{
   return t4openLow(d4,i4ndx,fileName,0);
}
#endif  /* S4PASCAL */
