/*****************************************************************************/
/*                                                                           */
/*   Program    : Test Buttons                     Advanced Machine & Tool   */
/*   Project    :                                  3824 Transportation Dr.   */
/*   Author     : Court Sailor                     Fort Wayne, IN  46818     */
/*   Date       : 05/12/1993                       (219) 489-3572            */
/*                                                                           */
/*   File Name  : BUTTONS.C                                                  */
/*   Header File:                                                            */
/*   Directory  : E:\PROTO\BUTTONS                                           */
/*                                                                           */
/*   Description:                                                            */
/*                                                                           */
/*                                                                           */
/*   Revision # : 000                                                        */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
#define INCL_PM
#define INCL_WIN
#define INCL_GPI
#define INCL_DOSPROCESS
#define INCL_COMMOS2_32BIT
#include <os2.h>
#include <time.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commos2.h>

#include "mmilib.h"
#include "touchscr.h"


HAB   Hab;
HMQ   Hmq;
HWND  HWndSystem, HWndSystemClient;
char  SystemClass[] = "System";


void Thread1( HMQ  MsgQ );


/*****************************************************************************/
/*                                                                           */
/*     Function:  SystemProcess                                              */
/*                                                                           */
/*     This function is designed to be the Main Menu of the MMI.  From here  */
/*     an Operator can perform all the needed functions for this machine.    */
/*                                                                           */
/*****************************************************************************/
MRESULT EXPENTRY SystemProcess( HWND HWnd, USHORT msg, MPARAM mp1, MPARAM mp2 )
{
   int     Choice;
   char    Str[80];
   USHORT  i, KeyFlags;
   HPS     Hps;
   RECTL   Rect;
   HWND    HWndDlg;

   switch ( msg )
      {
      case WM_CREATE      : return( 0 );

      case WM_PAINT       : Hps = WinBeginPaint( HWnd, NULLHANDLE, &Rect );

			    GpiDrawBox( Hps, CLR_DARKGRAY, DRO_FILL, &Rect );
			    GpiSetupRect( 120, 10, 520, 420, &Rect );
			    GpiDrawBDBox( Hps, CLR_PALEGRAY, 3, BUMPED, &Rect );

			    WinEndPaint( Hps );
			    return( 0 );

      case WM_BUTTON1DOWN : DosBeep( 1000, 10 );
			    Hps = WinGetPS( HWnd );
			    WinReleasePS( Hps );
			    return( 0 );

      case WM_BUTTON1UP   : Hps = WinGetPS( HWnd );
			    WinReleasePS( Hps );
			    return( 0 );

      case WM_BUTTON2DOWN : Hps = WinGetPS( HWnd );
			    WinReleasePS( Hps );
			    return( 0 );

      case WM_DESTROY     : return( 0 );

      case WM_TOUCHSCREEN : WinSetPointerPos( HWND_DESKTOP, SHORT1FROMMP( mp1 ), SHORT2FROMMP( mp1 ) );
			    return( 0 );

      case WM_TOUCHBUTTON : WinSendMsg( HWnd, WM_BUTTON1DOWN, mp1, mp2 );
			    return( 0 );
      }

   return WinDefWindowProc( HWnd, msg, mp1, mp2 );
}




int main()
{
   QMSG   Qmsg;
   HPS    Hps;
   TID    Thread1ID;
   CHAR   Str[80];
   ULONG  Err,
	  FrameFlags = FCF_TASKLIST | FCF_SYSMENU | FCF_MINMAX | FCF_TITLEBAR;

   /************************************************************************/
   /*  Obtain a PM Window Handle from the O/S and create a Qmsg Queue.  */
   /************************************************************************/
   Hab = WinInitialize( 0 );
   Hmq = WinCreateMsgQueue( Hab, 0 );

   /*****************************************************************************/
   /*  Register the System Process with the O/S then create a Standard Window.  */
   /*****************************************************************************/
   WinRegisterClass( Hab,
		     SystemClass,
		     (PFNWP) SystemProcess,
		     CS_SIZEREDRAW,
		     0 );

   HWndSystem = WinCreateStdWindow( HWND_DESKTOP,
				    WS_VISIBLE,
				    &FrameFlags,
				    SystemClass,
				    "Touch Screen Dialog Button Tester",
				    0L,
				    NULLHANDLE,
				    0,
				    &HWndSystemClient );

   WinSetWindowPos( HWndSystem, HWND_BOTTOM,
		    0,                                               /* x pos */
		    0,                                               /* y pos */
		    640,                                             /* x size */
		    480,                                             /* y size */
		    SWP_ACTIVATE | SWP_MOVE | SWP_SIZE | SWP_SHOW);  /* flags  */

   /**********************************/
   /*  Start Up the COMM Subsystem.  */
   /**********************************/
   Err = CommStartSystem( 3 );
   if ( Err != COMM_ERROR_NOERROR )
      {
      sprintf( Str, "COMM Subsystem Start Error => %ld", Err );
      WinMessageBox( HWndSystem, HWndSystem,
		     Str,
		     "TOUCH SCREEN INITIALIZATION",
		     1,
		     MB_CANCEL | MB_ERROR |
		     MB_DEFBUTTON1 | MB_SYSTEMMODAL );
      }

   /**********************************/
   /*  Initialize the Touch Screen.  */
   /**********************************/
   Err= InitializeTouchScreen( 1 );
   if ( Err )
      {
      sprintf( Str, "Touch Screen Init Error => %ld", Err );
      WinMessageBox( HWndSystem, HWndSystem,
		     Str,
		     "TOUCH SCREEN INITIALIZATION",
		     1,
		     MB_CANCEL | MB_ERROR |
		     MB_DEFBUTTON1 | MB_SYSTEMMODAL );
      }

   DosCreateThread( &Thread1ID, (PFNTHREAD) Thread1, Hmq, 0L, 8192 );
/*   DosSetPriority( 2, 2, 0, Thread1ID );*/

   /*******************************************************/
   /*  Dispatch all messages to the SystemProc Function.  */
   /*******************************************************/
   while ( WinGetMsg( Hab, &Qmsg, 0, 0, 0 ) ) WinDispatchMsg( Hab, &Qmsg );

   /*****************************************************************/
   /*  Distroy the System Process Main Window and close things up.  */
   /*****************************************************************/
   WinDestroyWindow( HWndSystem );
   WinDestroyMsgQueue( Hmq );
   WinTerminate( Hab );

   /********************************/
   /*  Close Up the Touch Screen.  */
   /********************************/
   DosKillThread( Thread1ID );
   TurnOffTouchScreen();

   return( 0 );
}




void Thread1( HMQ  MsgQ )
{
   int     X, Y, Action;
   MPARAM  mp1 = 0,
	   mp2 = 0;

   while ( 1 == 1 )
      {
      /******************************************************************/
      /*  Check the Touch Screen to see if any action has taken place.  */
      /******************************************************************/
      Action = GetTouchScreenSelection( &X, &Y );
      X *= 8;  Y = 480 - (Y * 10);
      mp1 = MPFROMLONG( X + ((ULONG) Y << 16 ) );

      /*************************************************************************/
      /*  Post a message in the Message Queue that the Touch Screen was used.  */
      /*************************************************************************/
      if ( Action == TOUCH_TRACKING )
	 {
	 WinPostQueueMsg( MsgQ, WM_TOUCHSCREEN, mp1, mp2 );
	 }
      else if ( Action == TOUCH_EXIT )
	 {
	 /***********************/
	 /*  This Works Great!  */
	 /***********************/
/*	 SystemProcess( HWndSystem, WM_BUTTON1DOWN, mp1, mp2 );
*/

	 /***********************/
	 /*  This Works Great!  */
	 /***********************/
/*	 WinPostQueueMsg( MsgQ, WM_TOUCHBUTTON, mp1, mp2 );
*/

	 /********************************/
	 /*  This DOES NOT Work At All.  */
	 /********************************/
	 WinSendMsg( HWndSystem, WM_BUTTON1DOWN, mp1, mp2 );
	 }
      }

}





