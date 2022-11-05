#ifndef S4STAND_ALONE

#define ver4NT 1895
#define ver431 1931
#define ver495 1995
#define ver4Unix 1000

#ifdef S4BYTEORDER_3210
   #define htond(a) (a)
   #define ntohd(a) (a)
#else
   double htond(double) ;
   #define ntohd(a) htond((a))
#endif

#ifdef __cplusplus
   extern "C" {
#endif

int blast4testWrite( CODE4 *, long  ) ;
int blast4testRead( CODE4 *, long  ) ;

#ifndef S4OFF_THREAD
   void code4freeEvent(CODE4 *, EVENT4 *) ;
   EVENT4 *code4getEvent(CODE4 * ) ;
        EVENT4 *code4allocateEvent(CODE4 *) ;
   void code4clearEvents(CODE4 *c4) ;
#endif
S4EXPORT int S4FUNCTION code4osVersion ( void ) ;

int connect4connect(CONNECT4 *, CODE4 *, unsigned short, const char * ) ;
void connect4disconnect(CONNECT4 * ) ;
int connect4init(CONNECT4 *, CONNECT4LOW *, CODE4 *) ;
void connect4initUndo(CONNECT4 *) ;
#define connect4receive(a,b,c,d) connect4bufferReceive(&(a)->connectBuffer,(b),(c),(d))
#define connect4send(a,b,c) connect4bufferSend(&(a)->connectBuffer,(b),(c))
#define connect4sendLong(a,b) connect4bufferSendLong(&(a)->connectBuffer, (b))
#define connect4receiveChar(a) connect4bufferReceiveChar(&(a)->connectBuffer, code4timeout( (a)->cb ) )
#define connect4receiveShort(a) connect4bufferReceiveShort(&(a)->connectBuffer, code4timeout( (a)->cb ) )
#define connect4receiveString(a) connect4bufferReceiveString(&(a)->connectBuffer, code4timeout( (a)->cb ) )
#define connect4receiveLong(a) connect4bufferReceiveLong(&(a)->connectBuffer, code4timeout( (a)->cb ) )
#define connect4sendChar(a,b) connect4bufferSendChar(&(a)->connectBuffer, (b))
#define connect4sendShort(a,b) connect4bufferSendShort(&(a)->connectBuffer, (b))
#define connect4sendString(a,b) connect4bufferSendString(&(a)->connectBuffer, (b))
#define connect4sendFlush(a) connect4bufferSendFlush(&(a)->connectBuffer)
int connect4workContinue( CONNECT4 * ) ;

TCP4ADDRESS connect4peerTcpAddress( CONNECT4 * ) ;
TCP4ADDRESS connect4lowPeerTcpAddress( CONNECT4LOW * ) ;
TCP4ADDRESS connect4localTcpAddress( CONNECT4 * ) ;
TCP4ADDRESS connect4lowLocalTcpAddress( CONNECT4LOW * ) ;

CONNECT4BUFFER *connect4bufferAuxConnection(CONNECT4 *, int , int , int  ) ;
CONNECT4BUFFER *connect4bufferAuxConnectionGet(CONNECT4 *, int , int , int  ) ;
void connect4bufferAuxConnectionPut(CONNECT4BUFFER *, CONNECT4 * ) ;
#ifdef S4SERVER
   CONNECT4BUFFER *connect4bufferAuxConnectionSpecific(CONNECT4 *, short  ) ;
#endif
int connect4bufferConnect( CONNECT4BUFFER *, CONNECT4 *) ;
void connect4bufferDisconnect(CONNECT4BUFFER * ) ;
#ifndef S4OFF_THREAD
   NET4MESSAGE *connect4bufferGetWriteBuffer(CONNECT4BUFFER * ) ;
   NET4MESSAGE *connect4bufferReadRetrieve( CONNECT4BUFFER *, int ) ;
   NET4MESSAGE *connect4bufferWriteRetrieve(CONNECT4BUFFER *, int, int ) ;
   int connect4bufferInitRead(CONNECT4BUFFER *, int , int ) ;
#endif
int connect4bufferInit(CONNECT4BUFFER *, CONNECT4 *, CONNECT4LOW *, int, int, int ) ;
void connect4bufferInitUndo(CONNECT4BUFFER * ) ;
int connect4bufferReceive(CONNECT4BUFFER *, void *, long , int ) ;
int connect4bufferSend(CONNECT4BUFFER *, const void *, long ) ;
int connect4bufferSendLong(CONNECT4BUFFER *, const long ) ;
short connect4bufferReceiveShort(CONNECT4BUFFER *, int ) ;
long connect4bufferReceiveLong(CONNECT4BUFFER *, int ) ;
char *connect4bufferReceiveString(CONNECT4BUFFER *, int ) ;
int connect4bufferSendString(CONNECT4BUFFER *, const char *string ) ;
int connect4bufferSendShort(CONNECT4BUFFER *, const short ) ;
int connect4bufferSendFlush(CONNECT4BUFFER *) ;
int connect4bufferSendChar(CONNECT4BUFFER *, const char ) ;
char connect4bufferReceiveChar(CONNECT4BUFFER *, int ) ;

int address4getLocalNetID(C4NETID *, short *, CONNECT4LOW * ) ;
int address4makeName(C4ADDRESS *, short *, unsigned short , const char *) ;
int address4makeNetID(C4ADDRESS *, short *, C4NETID , unsigned short  ) ;

/* void address4changePort(C4ADDRESS *, short *, unsigned short );*/
/* int address4getLocalAddress(C4ADDRESS *, short *, CONNECT4LOW * );*/
/* int address4make(C4ADDRESS *, short *, unsigned short , const char *);*/

int connect4lowAccept(CONNECT4LOW *, CONNECT4LOW *) ;
int connect4lowAcceptWait(CONNECT4LOW *, CONNECT4LOW *, int) ;
void connect4lowBlast(CONNECT4LOW *) ;
int connect4lowCheck( CONNECT4LOW * ) ;
int connect4lowClose(CONNECT4LOW * ) ;
int connect4lowConnect(CONNECT4LOW *, void *, int ) ;
int connect4lowConnectConnect(CONNECT4LOW *, void *, int ) ;
#define connect4lowError( a ) ( connect4lowClose((a)))
int connect4lowGetPersonalAddress(CONNECT4LOW *, void *, int *) ;
int connect4lowListen(CONNECT4LOW *, CODE4 *, short *, int ) ;
int connect4lowMakeAddress(void *, int *, int , const char *) ;
int connect4lowRead(CONNECT4LOW *conlow, char *buffer, int len ) ;
int connect4lowWrite(CONNECT4LOW *conlow, char *buffer, int len ) ;
#ifndef S4OFF_THREAD
   int connect4lowReadAsynchronous(CONNECT4LOW *, NET4MESSAGE *, EVENT4 *) ;
   int connect4lowWriteAsynchronous(CONNECT4LOW *, NET4MESSAGE *, EVENT4 *) ;
   int connect4lowWriteSynchronous( CONNECT4LOW *, NET4MESSAGE * ) ;
#endif

#ifndef S4OFF_THREAD
   void connect4threadCompleteRequest(CONNECT4THREAD *) ;
   int connect4threadInit(CONNECT4THREAD *, CODE4 *, CONNECT4BUFFER * ) ;
   void connect4threadInitUndo(CONNECT4THREAD *) ;
   void connect4threadReadCompleted( CONNECT4THREAD *, SIGNAL4ROUTINE *,  NET4MESSAGE *) ;
   int connect4threadReadRequest(CONNECT4THREAD *, NET4MESSAGE * ) ;
   SIGNAL4ROUTINE *connect4threadReadSignalRetrieve(CONNECT4THREAD *, SIGNAL4ROUTINE *, int ) ;
   void connect4threadReadStore( CONNECT4THREAD *, NET4MESSAGE * ) ;
   #ifdef S4SERVER
      void connect4threadRegisterShutdown(CONNECT4THREAD *) ;
   #endif
   int connect4threadWorkContinue(CONNECT4THREAD * ) ;
   void connect4threadWriteCompleted(CONNECT4THREAD *, NET4MESSAGE *, SIGNAL4ROUTINE * ) ;
   void connect4threadWriteRequest(CONNECT4THREAD *, NET4MESSAGE *) ;
   SIGNAL4ROUTINE *connect4threadWriteSignalRetrieve(CONNECT4THREAD *, SIGNAL4ROUTINE *, int ) ;

   int event4init( EVENT4 * ) ;
   int event4initUndo( EVENT4 * ) ;
   int event4reset(EVENT4 *) ;
   /* int event4set( EVENT4 * ); */
   #define event4set( event ) ( SetEvent( (event)->handle ) )
   int event4wait( EVENT4 *, int ) ;

   void inter4( void * ) ;
   void inter4completeRequested(INTER4 *, CONNECT4THREAD * ) ;
   void inter4completeRequired(INTER4 *, SIGNAL4ROUTINE *) ;
   void inter4connectionCheck(INTER4 * ) ;
   void inter4error(INTER4 *, CONNECT4THREAD *) ;
   int inter4init(INTER4 *, CODE4 * ) ;
   void inter4initUndo(INTER4 *) ;
   void inter4readCompleted(INTER4 *, SIGNAL4ROUTINE * ) ;
   void inter4readRequested(INTER4 *, NET4MESSAGE * ) ;
   void inter4readRequired(INTER4 *, SIGNAL4ROUTINE * ) ;
   void inter4shutdown(INTER4 *, SIGNAL4ROUTINE *) ;
   void inter4shutdownRequest(INTER4 * ) ;
   void inter4write(INTER4 *, NET4MESSAGE *, SIGNAL4ROUTINE * ) ;
   void inter4writeCompleted(INTER4 *, SIGNAL4ROUTINE * ) ;
   void inter4writeRequested(INTER4 *, NET4MESSAGE * ) ;
   void inter4writeRequired(INTER4 *, SIGNAL4ROUTINE * ) ;

   void list4mutexAdd( LIST4MUTEX *, void * ) ;
   void list4mutexInit( LIST4MUTEX * ) ;
   void list4mutexInitUndo( LIST4MUTEX * ) ;
   S4EXPORT void list4mutexRelease( LIST4MUTEX * ) ;
   void *list4mutexRemove( LIST4MUTEX * ) ;
   void list4mutexRemoveLink( LIST4MUTEX *, LINK4 * ) ;
   S4EXPORT void list4mutexWait( LIST4MUTEX * ) ;

   /* int overlap4amountTransferred( OVERLAPPED *) */
   #define overlap4amountTransferred( a ) ( (a).InternalHigh )

   int signal4routineInit( SIGNAL4ROUTINE *, HANDLE, COMPLETION4ROUTINE *, void *, void *) ;
   int signal4arrayInit( SIGNAL4ROUTINE_ARRAY *, CODE4 *, int ) ;
   void signal4arrayInitUndo(SIGNAL4ROUTINE_ARRAY * ) ;
   int signal4arrayAdd(SIGNAL4ROUTINE_ARRAY *, SIGNAL4ROUTINE *, int ) ;
   void signal4arrayRemoveSignal( SIGNAL4ROUTINE_ARRAY *,  SIGNAL4ROUTINE * ) ;
   SIGNAL4ROUTINE *signal4arrayGet( SIGNAL4ROUTINE_ARRAY *, int ) ;
   /* SIGNAL4ROUTINE *signal4arrayWait( SIGNAL4ROUTINE_ARRAY *, unsigned long ) ; */
   int signal4arrayWait(SIGNAL4ROUTINE_ARRAY *, SIGNAL4ROUTINE **, unsigned long ) ;
   int signal4arrayWaitHere(SIGNAL4ROUTINE_ARRAY *, unsigned long, int, int, SIGNAL4ROUTINE ** ) ;
#endif /*!S4OFF_THREAD */
#endif

#ifdef __cplusplus
   }
#endif
