#ifdef WIN32
    #include <windows.h>
    #include <winsock.h>
#else
    #define closesocket close
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

/*!------------------------------------------------------------------------
 * Program:   wall client
 *
 * Purpose:   allocate a socket, connect to a server, and print all output
 *
 * Syntax:    client <host> <port>
 *
 *               host  - name of a computer on which server is executing
 *               port  - protocol port number server is using
 *------------------------------------------------------------------------
 */

#define ERROR(ptr,msg,params)                \
    do {                                     \
        if (asprintf(&ptr,msg,params) > 0) { \
            error(ptr);                      \
            free(ptr);                       \
            ptr=NULL;                        \
        }                                    \
        else                                 \
            error("asprintf error\n");       \
    } while(1)                               \

#ifdef WIN32

int vasprintf( char **sptr, char *fmt, va_list argv ) {
    int wanted = vsnprintf( *sptr = NULL, 0, fmt, argv );
    if( (wanted > 0) && ((*sptr = malloc( 1 + wanted )) != NULL) )
        return vsprintf( *sptr, fmt, argv );
    return wanted;
}

int asprintf( char **sptr, char *fmt, ... ) {
    int retval;
    va_list argv;
    va_start( argv, fmt );
    retval = vasprintf( sptr, fmt, argv );
    va_end( argv );
    return retval;
}
#endif

void usage(char *prg) {
    printf("usage:%s <server> <port>",prg);
    exit(EXIT_FAILURE);
}

void error(char *msg) {
    printf (msg);
#ifdef WIN32
    WSACleanup();
#endif
    exit(EXIT_FAILURE);
}

int openConnect(char *host, int port) {
    struct  hostent  *ptrh;  /* pointer to a host table entry       */
    struct  protoent *ptrp;  /* pointer to a protocol table entry   */
    struct  sockaddr_in sad; /* structure to hold an IP address     */
    int     sd;              /* socket descriptor                   */
    char *errorMsgs;         /* error messages */

    if (!host || !*host)
        error("host is empty\n");

    if (port <= PUBLIC_PORT)
        ERROR(errorMsgs, "You cannot use ports under %d\n",PUBLIC_PORT);

    memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET;         /* set family to Internet     */
    sad.sin_port = htons((u_short)port);

    ptrh = gethostbyname(host);
    if ( !ptrh )
        ERROR(errorMsgs, "Invalid host: %s\n", host);

    memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);
    /* Map TCP transport protocol name to protocol number. */
    if ( !(ptrp = getprotobyname("tcp")) )
        error("Cannot map \"tcp\" to protocol number");

    /* Create a socket. */
    sd = socket(AF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd < 0)
        error("Socket creation failed\n");

    /* Connect the socket to the specified server. */
    if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0)
        error("Connect failed\n");

    return sd;
}

int main(int argc, char *argv[]) {
        int     sd;              /* socket descriptor                   */
        int     port;            /* protocol port number                */
        int     n;               /* number of characters read           */
        char    buf[BUFFER_SIZE];/* buffer for data from the server     */
        char    *text;           /* pointer to user's line of text      */
#ifdef WIN32
        WSADATA wsaData;
        WSAStartup(0x0101, &wsaData);
#endif
        if (argc != 3)
            usage(argv[0]);

        port = atoi(argv[2]);
        do {
            text = fgets(buf, sizeof(buf), stdin);
            if (*text == QUIT_CHAR)
                break;
            sd = openConnect(argv[1], port);
            if (sd > 0) {
                send(sd, buf, strlen(buf), 0);
                n = recv(sd, buf, sizeof(buf), 0);
                closesocket(sd);
                write(stdout, buf, n);
            }
        } while(1);

        /* Terminate the client program gracefully. */
#ifdef WIN32
        WSACleanup();
#endif
        exit(EXIT_SUCCESS);
}












