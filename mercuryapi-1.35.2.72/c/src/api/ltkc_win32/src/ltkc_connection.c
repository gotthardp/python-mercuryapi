
/*
 ***************************************************************************
 *  Copyright 2007,2008 Impinj, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ***************************************************************************
 */

/**
 *****************************************************************************
 **
 ** @file  ltkc_connection.c
 **
 ** @brief Functions to communicate using LLRP messages over a socket
 **
 ** This provides a simple interface to send and receive LLRP messages
 ** using a network socket.
 **
 ** This is simplified. It is possible, even likely, that some
 ** applications could require much more sophistication. To that
 ** end this source file serves as an example to illustrate how
 ** framing, encoding, decoding, and response matching are done.
 **
 *****************************************************************************/


#include <assert.h>
#include "ltkc_platform.h"
#include "ltkc_base.h"
#include "ltkc_frame.h"
#include "ltkc_connection.h"

#ifdef linux
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#endif
#if defined(WIN32)|| defined(WINCE)
#include <ws2tcpip.h>
#include <time.h>
#endif

#define LLRP1_TCP_PORT   (5084u)


/* forward declaration of private routine. */
static LLRP_tResultCode
recvAdvance (
  LLRP_tSConnection *           pConn,
  int                           nMaxMS,
  time_t                        timeLimit);

static time_t
calculateTimeLimit (
  int                           nMaxMS);



/**
 *****************************************************************************
 **
 ** @brief  Construct a new LLRP connection instance
 **
 ** @param[in]  pTypeRegistry   The LLRP registry of known message/parameter
 **                             types. Includes standard and custom.
 **                             Used during decode.
 ** @param[in]  nBufferSize     Size of each the receive and send buffers.
 **                             Use size larger than the largest frame
 **                             you expect. 0 selects a default value.
 **
 ** @return     !=NULL          Pointer to connection instance
 **             ==NULL          Error, always an allocation failure
 **                             most likely nBufferSize is weird
 **
 *****************************************************************************/

LLRP_tSConnection *
LLRP_Conn_construct (
  const LLRP_tSTypeRegistry *   pTypeRegistry,
  unsigned int                  nBufferSize)
{
    LLRP_tSConnection *         pConn;

    /*
     * Apply default buffer size
     */
    if(0 == nBufferSize)
    {
        nBufferSize = 128u*1024u;
    }

    /*
     * Make sure the buffer size is sane. This is arbitrary.
     * The smallest message is 10 bytes, but it ain't anything
     * useful. 1024u covers a surprising number of messages.
     */
    if(1024u > nBufferSize || 1u*1024u*1024u < nBufferSize)
    {
        /* Insane buffer size */
        return NULL;
    }

    /*
     * Allocate, check, and zero-fill connection instance.
     */
    pConn = (LLRP_tSConnection *) malloc(sizeof *pConn);
    if(NULL == pConn)
    {
        return pConn;
    }
    memset(pConn, 0, sizeof *pConn);

    /*
     * Capture variables. fd=-1 indicates there
     * is no connection yet.
     */
#if defined(WIN32)|| defined(WINCE)
	pConn->fd = INVALID_SOCKET;
#else
    pConn->fd = -1;
#endif
    pConn->pTypeRegistry = pTypeRegistry;
    pConn->nBufferSize = nBufferSize;

    /*
     * Allocate and check each the recv and send buffers.
     */
    pConn->Recv.pBuffer = (unsigned char *) malloc(nBufferSize);
    pConn->Send.pBuffer = (unsigned char *) malloc(nBufferSize);

    if(NULL == pConn->Recv.pBuffer || NULL == pConn->Send.pBuffer)
    {
        LLRP_Conn_destruct(pConn);
        return NULL;
    }

    /*
     * Zero-fill buffers just so debugger printing is reasonable
     */
    memset(pConn->Recv.pBuffer, 0, nBufferSize);
    memset(pConn->Send.pBuffer, 0, nBufferSize);

    /*
     * Victory
     */
    return pConn;
}


/**
 *****************************************************************************
 **
 ** @brief  Destruct a LLRP connection instance
 **
 ** @param[in]  pConn           Pointer to the connection instance.
 **
 ** @return     void
 **
 *****************************************************************************/

void
LLRP_Conn_destruct (
  LLRP_tSConnection *           pConn)
{
    if(NULL != pConn)
    {
        /*
         * Close the connection, if one
         */
        LLRP_Conn_closeConnectionToReader(pConn);

        /*
         * Destruct any message in the input queue
         */
        while(NULL != pConn->pInputQueue)
        {
            LLRP_tSMessage *    pMessage;

            pMessage = pConn->pInputQueue;
            pConn->pInputQueue = pMessage->pQueueNext;

            LLRP_Element_destruct((LLRP_tSElement *)pMessage);
        }

        /*
         * free each the receive and send bufers
         */
        if(NULL != pConn->Recv.pBuffer)
        {
            free(pConn->Recv.pBuffer);
        }
        if(NULL != pConn->Send.pBuffer)
        {
            free(pConn->Send.pBuffer);
        }

        /*
         * Wipe it out so any stale uses are likely to crash
         * on a NULL pointer.
         */
        memset(pConn, 0, sizeof *pConn);

        /*
         * Finally, free the connection data structure itself.
         */
        free(pConn);

#if defined(WIN32)|| defined(WINCE)
    /*
     * On Windows have to disable (dereference) the WinSock library
     */
    {
        WSACleanup();
    }
#endif
    }
}


/**
 *****************************************************************************
 **
 ** @brief  Open the connection to the reader
 **
 ** The steps:
 **     - Look up the host name
 **     - Create a socket
 **     - Connect to the host address
 **     - Condition the socket (set options)
 **
 ** @param[in]  pConn           Pointer to the connection instance.
 ** @param[in]  pReaderHostName String containing the reader host name
 **
 ** @return     ==0             Connected OK, ready for business
 **             !=0             Error, check LLRP_Conn_getConnectError()
 **                             for reason
 **
 *****************************************************************************/

int
LLRP_Conn_openConnectionToReader (
  LLRP_tSConnection *           pConn,
  const char *                  pReaderHostName)
{
#ifdef linux
    int                         Sock;
#endif
#if defined(WIN32)|| defined(WINCE)
    SOCKET                      Sock;
#endif
    static const struct addrinfo AddrInfoMask =
    {
        0,
        AF_INET,
        SOCK_STREAM,
        0,
        0,
        NULL,
        NULL,
        NULL
    };
    struct addrinfo *           HostAddress;
    int                         Flag;
    struct sockaddr_in          Sin;
    int                         Rc;

    /*
     * Clear the connect error string
     */
    pConn->pConnectErrorStr = NULL;

    /*
     * Make sure there isn't already a connection.
     */
#if defined(WIN32)|| defined(WINCE)
	if(INVALID_SOCKET != pConn->fd)
#else
    if(0 <= pConn->fd)
#endif
    {
        pConn->pConnectErrorStr = "already connected";
        return -1;
    }

#if defined(WIN32)|| defined(WINCE)
    /*
     * On Windows have to enable the WinSock library
     */
    {
        WSADATA SocketLibraryInitData;
        WSAStartup(0xFFFF, &SocketLibraryInitData);
    }
#endif

    /*
     * Look up host using getaddrinfo().
     * Gethostbyname() could be configured a lot of
     * different ways. There is /etc/hosts, DNS, NIS, etc, etc.
     * Suffice to say it is big, bulky, and susceptible to stall.
     */
    if(0 != getaddrinfo(pReaderHostName, NULL, &AddrInfoMask, &HostAddress))
    {
        pConn->pConnectErrorStr = "host lookup failed";
        return -1;
    }

    /*
     * Convert the address to sockaddr_in format
     */
    memset(&Sin, 0, sizeof Sin);
    Sin.sin_family = AF_INET;
    Sin.sin_addr = ((struct sockaddr_in *)(HostAddress->ai_addr))->sin_addr;
    Sin.sin_port = htons(LLRP1_TCP_PORT);

    /*
     * Done withe the host addrinfo
     */
    freeaddrinfo(HostAddress);

    /*
     * Create the socket.
     */
    Sock = socket(AF_INET, SOCK_STREAM, 0);
#ifdef linux
    if(0 > Sock)
#endif /* linux */
#if defined(WIN32)|| defined(WINCE)
    if(INVALID_SOCKET == Sock)
#endif /* WIN32 */
    {
        pConn->pConnectErrorStr = "socket() failed";
        return -3;
    }

    /*
     * Connect the socket to reader. This can stall.
     */
    Rc = connect(Sock, (struct sockaddr *)&Sin, sizeof Sin);
    if(0 > Rc)
    {
        /* Connect failed */
        pConn->pConnectErrorStr = "connect() failed";
#ifdef linux
        close(Sock);
#endif
#if defined(WIN32)|| defined(WINCE)
        closesocket(Sock);
#endif
        return -4;
    }

    /*
     * Best effort to set no delay. If this doesn't work
     * (no reason it shouldn't) we do not declare defeat.
     */
    Flag = 1;
#ifdef linux
    setsockopt(Sock, IPPROTO_TCP, TCP_NODELAY, (void*)&Flag, sizeof Flag);
#endif
#if defined(WIN32)|| defined(WINCE)
    setsockopt(Sock, IPPROTO_TCP, TCP_NODELAY, (char*)&Flag, sizeof Flag);
#endif

    /*
     * Record the socket in the connection instance
     */
    pConn->fd = Sock;

    /*
     * Victory
     */
    return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Get the string that explains
 **         LLRP_Conn_openReaderConnection() error
 **
 ** @param[in]  pConn           Pointer to the connection instance.
 **
 ** @return     ==NULL          No error
 **             !=NULL          Short string description of error
 **
 *****************************************************************************/

const char *
LLRP_Conn_getConnectError (
  LLRP_tSConnection *           pConn)
{
    return pConn->pConnectErrorStr;
}


/**
 *****************************************************************************
 **
 ** @brief  Close connection to reader, allow reuse of instance
 **
 ** @param[in]  pConn           Pointer to the connection instance.
 **
 ** @return     ==0             Connected OK, ready for business
 **             !=0             Error, check getConnectError() for reason
 **
 *****************************************************************************/

int
LLRP_Conn_closeConnectionToReader (
  LLRP_tSConnection *           pConn)
{
    if(0 > pConn->fd)
    {
        pConn->pConnectErrorStr = "not connected";
        return -1;
    }

#ifdef linux
    close(pConn->fd);
#endif
#if defined(WIN32)|| defined(WINCE)
    closesocket(pConn->fd);
#endif
    pConn->fd = -1;

    return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Transact a LLRP request and response to a connection
 **
 ** This is a combination of LLRP_Conn_sendMessage() and
 ** LLRP_Conn_recvResponse(). The MessageID is taken from
 ** the outgoing messages. It's best to not use MessageID 0.
 ** The expected response type is also taken from the outgoing
 ** message. ERROR_MESSAGE is also deemed a response;
 **
 ** @param[in]  pConn           Pointer to the connection instance.
 ** @param[in]  pSendMessage    Pointer to the LLRP message to send.
 ** @param[in]  nMaxMS          -1 => block indefinitely
 **                              0 => just peek at input queue and
 **                                   socket queue, return immediately
 **                                   no matter what
 **                             >0 => ms to await complete frame
 **
 ** @return     ==NULL          Something failed. Use
 **                             LLRP_Conn_getTransactError() for best
 **                             guess at what went wrong.
 **             !=NULL          Response message
 **
 *****************************************************************************/

LLRP_tSMessage *
LLRP_Conn_transact (
  LLRP_tSConnection *           pConn,
  LLRP_tSMessage *              pSendMessage,
  int                           nMaxMS)
{
    const LLRP_tSTypeDescriptor * pResponseType;
    LLRP_tResultCode            lrc;
    LLRP_tSMessage *            pResponseMessage;

    /*
     * Determine the response type. The type descriptor
     * of the outgoing request message points to the
     * type descriptor of the response. Since we are
     * totally dependent upon it, fail if there
     * is no response type pointer value.
     */
    pResponseType = pSendMessage->elementHdr.pType->pResponseType;
    if(NULL == pResponseType)
    {
        LLRP_tSErrorDetails *   pError = &pConn->Send.ErrorDetails;

        LLRP_Error_clear(pError);
        LLRP_Error_resultCodeAndWhatStr(pError,
                LLRP_RC_MissingResponseType,
                "send message has no response type");
        return NULL;
    }

    /*
     * Send the request
     */
    lrc = LLRP_Conn_sendMessage(pConn, pSendMessage);
    if(LLRP_RC_OK != lrc)
    {
        return NULL;
    }

    /*
     * Receive the response subject to timeout
     */
    pResponseMessage = LLRP_Conn_recvResponse(pConn,  nMaxMS,
                            pResponseType, pSendMessage->MessageID);

    /*
     * Whatever recvResponse() returned is the result.
     */
    return pResponseMessage;
}


/**
 *****************************************************************************
 **
 ** @brief  Get the details that explains LLRP_Conn_transact() error
 **
 ** This is not 100% reliable. It looks at the send error and
 ** if that seems OK it uses the recv error. Uses of
 ** LLRP_Conn_sendMessage(), LLRP_Conn_recvMessage() or
 ** LLRP_CONN_recvResponse() since LLRP_Conn_transact() will
 ** distort the error.
 **
 ** @return                     Pointer to const error details
 **
 *****************************************************************************/

const LLRP_tSErrorDetails *
LLRP_Conn_getTransactError (
  LLRP_tSConnection *           pConn)
{
    const LLRP_tSErrorDetails * pError;

    pError = LLRP_Conn_getSendError(pConn);
    if(LLRP_RC_OK == pError->eResultCode)
    {
        pError = LLRP_Conn_getRecvError(pConn);
    }

    return pError;
}


/**
 *****************************************************************************
 **
 ** @brief  Send a LLRP message to a connection
 **
 ** @param[in]  pConn           Pointer to the connection instance.
 ** @param[in]  pMessage        Pointer to the LLRP message to send.
 **
 ** @return     LLRP_RC_OK          Frame sent
 **             LLRP_RC_SendIOError I/O error in write().
 **                                 Probably means fd is bad.
 **             LLRP_RC_...         Encoder error.
 **                                 Check LLRP_Conn_getSendError() for why.
 **
 *****************************************************************************/

LLRP_tResultCode
LLRP_Conn_sendMessage (
  LLRP_tSConnection *           pConn,
  LLRP_tSMessage *              pMessage)
{
    LLRP_tSErrorDetails *       pError = &pConn->Send.ErrorDetails;
    LLRP_tSFrameEncoder *       pEncoder;

    /*
     * Clear the error details in the send state.
     */
    LLRP_Error_clear(pError);

    /*
     * Make sure the socket is open.
     */
    if(0 > pConn->fd)
    {
        LLRP_Error_resultCodeAndWhatStr(pError,
            LLRP_RC_MiscError, "not connected");
        return pError->eResultCode;
    }

    /*
     * Construct a frame encoder. It needs to know the buffer
     * base and maximum size.
     */

    pEncoder = LLRP_FrameEncoder_construct(pConn->Send.pBuffer,
                                                pConn->nBufferSize);

    /*
     * Check that the encoder actually got created.
     */
    if(NULL == pEncoder)
    {
        LLRP_Error_resultCodeAndWhatStr(pError,
            LLRP_RC_MiscError, "encoder constructor failed");
        return pError->eResultCode;
    }

    /*
     * Encode the message. Return value is ignored.
     * We check the encoder's ErrorDetails for results.
     * The &...encoderHdr is in lieu of type casting since
     * the generic LLRP_Encoder_encodeElement() takes the
     * generic LLRP_tSEncoder.
     */
    LLRP_Encoder_encodeElement(&pEncoder->encoderHdr, &pMessage->elementHdr);

    /*
     * Regardless of what happened capture the error details
     * and the number of bytes placed in the buffer.
     */
    pConn->Send.ErrorDetails = pEncoder->encoderHdr.ErrorDetails;
    pConn->Send.nBuffer = pEncoder->iNext;

    /*
     * Bye bye li'l encoder.
     */
    LLRP_Encoder_destruct(&pEncoder->encoderHdr);

    /*
     * If the encoding appears complete write the frame
     * to the connection. NB: this is not ready for
     * non-blocking I/O (EWOULDBLOCK).
     */
    if(LLRP_RC_OK == pError->eResultCode)
    {
        int             rc;

#if defined(WIN32)|| defined(WINCE)
        rc = send(pConn->fd, (char*)pConn->Send.pBuffer, pConn->Send.nBuffer, 0);
		pError->OtherDetail = rc;
        if (-1 == rc)
        {
          //pError->tcpErrno = errno;
        }
#else
		rc = write(pConn->fd, pConn->Send.pBuffer, pConn->Send.nBuffer);
    	pError->OtherDetail = rc;
        if (-1 == rc)
        {
          pError->tcpErrno = errno;
        }
#endif
        if(rc != pConn->Send.nBuffer)
        {
            /* Yikes! */
            LLRP_Error_resultCodeAndWhatStr(pError,
                LLRP_RC_SendIOError, "send IO error");
        }
    }

    /*
     * Done.
     */
    return pError->eResultCode;
}


/**
 *****************************************************************************
 **
 ** @brief  Get the details that explains LLRP_Conn_sendMessage() error
 **
 ** @param[in]  pConn           Pointer to the connection instance.
 **
 ** @return                     Pointer to const error details
 **
 *****************************************************************************/

extern const LLRP_tSErrorDetails *
LLRP_Conn_getSendError (
  LLRP_tSConnection *           pConn)
{
    return &pConn->Send.ErrorDetails;
}


/**
 *****************************************************************************
 **
 ** @brief  Receive a message from a connection
 **
 ** The message could have already arrived and be pending
 ** on the input queue.
 **
 ** @param[in]  pConn           Pointer to the connection instance.
 ** @param[in]  nMaxMS          -1 => block indefinitely
 **                              0 => just peek at input queue and
 **                                   socket queue, return immediately
 **                                   no matter what
 **                             >0 => ms to await complete frame
 **
 ** @return     ==NULL          No message available per parameters.
 **                             Check LLRP_Conn_getRecvErrot() for why
 **             !=NULL          Input message
 **
 *****************************************************************************/

LLRP_tSMessage *
LLRP_Conn_recvMessage (
  LLRP_tSConnection *           pConn,
  int                           nMaxMS)
{
    time_t                      timeLimit = calculateTimeLimit(nMaxMS);
    LLRP_tResultCode            lrc;
    LLRP_tSMessage *            pMessage;

    /*
     * Make sure the socket is open.
     */
    if(0 > pConn->fd)
    {
        LLRP_tSErrorDetails *   pError = &pConn->Recv.ErrorDetails;

        LLRP_Error_resultCodeAndWhatStr(pError,
            LLRP_RC_MiscError, "not connected");
        return NULL;
    }

    /*
     * Loop until victory or some sort of exception happens
     */
    for(;;)
    {
        /*
         * Check the input queue to see if there is already
         * a message pending.
         */
        pMessage = pConn->pInputQueue;
        if(NULL != pMessage)
        {
            pConn->pInputQueue = pMessage->pQueueNext;
            return pMessage;
        }

        /*
         * No message available. Advance the receiver state
         * and see if a message is produced.
         */
        lrc = recvAdvance(pConn, nMaxMS, timeLimit);
        if(lrc != LLRP_RC_OK)
        {
            return NULL;
        }
    }
}


/**
 *****************************************************************************
 **
 ** @brief  Get the details that explains LLRP_Conn_recvMessage()
 **         or LLRP_Conn_recvResponse() error
 **
 ** @param[in]  pConn           Pointer to the connection instance.
 **
 ** @return                     Pointer to const error details
 **
 *****************************************************************************/

const LLRP_tSErrorDetails *
LLRP_Conn_getRecvError (
  LLRP_tSConnection *           pConn)
{
    return &pConn->Recv.ErrorDetails;
}


/**
 *****************************************************************************
 **
 ** @brief  Receive a specific message from a connection
 **
 ** The message is identified by type and message ID.
 **
 ** This is used to receive a response to a request, but
 ** could be used for other things, too. A message matches
 ** and is returned if the type matches AND if the message
 ** ID matches. Either or both can be wildcards.
 **
 ** The sought message could have already arrived and be pending
 ** on the input queue. While we are looking for the response
 ** notifications might arrive. They are held in the input
 ** queue while we continue to look for the sought message.
 **
 ** About timeLimit....
 ** The timeLimit is the last time() we'll try to receive
 ** the sought message and prevents "spinning".
 ** It is conceivable that a steady stream of messages
 ** other than the one sought could arrive, and the time
 ** between those messages could be smaller the nMaxMS.
 ** When that happens, recvAdvance() would not time out
 ** during its poll/select. It could "spin". The time out
 ** must be detected by other means and that's the
 ** purpose of timeLimit.
 **
 ** @param[in]  pConn           Pointer to the connection instance.
 ** @param[in]  nMaxMS          -1 => block indefinitely
 **                              0 => just peek at input queue and
 **                                   socket queue, return immediately
 **                                   no matter what
 **                             >0 => ms to await complete frame
 ** @param[in]  pResponseType   The type descriptor of the sought
 **                             or NULL to match all messages.
 **                             If not NULL, ERROR_MESSAGE will
 **                             also match.
 ** @param[in]  ResponseMessageID The MessageID of sought message
 **                             or 0 to match all messages.
 **
 ** @return     ==NULL          No message available per parameters.
 **                             Check LLRP_Conn_getRecvErrot() for why
 **             !=NULL          Input message
 **
 *****************************************************************************/

LLRP_tSMessage *
LLRP_Conn_recvResponse (
  LLRP_tSConnection *           pConn,
  int                           nMaxMS,
  const LLRP_tSTypeDescriptor * pResponseType,
  llrp_u32_t                    ResponseMessageID)
{
    time_t                      timeLimit = calculateTimeLimit(nMaxMS);
    const LLRP_tSTypeDescriptor *pErrorMsgType;
    LLRP_tResultCode            lrc;
    LLRP_tSMessage *            pMessage;
    LLRP_tSMessage **           ppMessage;

    /*
     * Make sure the socket is open.
     */
    if(0 > pConn->fd)
    {
        LLRP_tSErrorDetails *   pError = &pConn->Recv.ErrorDetails;

        LLRP_Error_resultCodeAndWhatStr(pError,
            LLRP_RC_MiscError, "not connected");
        return NULL;
    }

    /*
     * Look up the ERROR_MESSAGE type descriptor now.
     */
    pErrorMsgType = LLRP_TypeRegistry_lookupMessage(
                    pConn->pTypeRegistry, 100u);

    /*
     * Loop until victory or some sort of exception happens
     */
    for(;;)
    {
        /*
         * Check the input queue to see if the sought
         * message is present.
         */
        for(
            ppMessage = &pConn->pInputQueue;
            NULL != (pMessage = *ppMessage);
            ppMessage = &pMessage->pQueueNext)
        {
            /*
             * Are we looking for a particular message type?
             */
            if(NULL != pResponseType)
            {
                /*
                 * See if it is the sought response type or
                 * an ERROR_MESSAGE.
                 */
                if(pMessage->elementHdr.pType != pResponseType &&
                   pMessage->elementHdr.pType != pErrorMsgType)
                {
                    /* Type does not match. Keep looking. */
                    continue;
                }
            }

            /*
             * Are we looking for a particular message ID?
             */
            if(0 != ResponseMessageID)
            {
                if(pMessage->MessageID != ResponseMessageID)
                {
                    /* Message ID does not match. Keep looking. */
                    continue;
                }
            }

            /* Found it */
            break;
        }

        /*
         * If we found it unlink it from the queue and return it.
         */
        if(NULL != pMessage)
        {
            *ppMessage = pMessage->pQueueNext;
            pMessage->pQueueNext = NULL;
            return pMessage;
        }

        /*
         * Sought message is not in the queue. Advance the
         * receiver state and see if the message is produced.
         */
        lrc = recvAdvance(pConn, nMaxMS, timeLimit);
        if(lrc != LLRP_RC_OK)
        {
            return NULL;
        }

        /*
         * Loop to the top and try again.
         */
    }
}


/**
 *****************************************************************************
 **
 ** @brief  Internal routine to advance receiver
 **
 ** @param[in]  pConn           Pointer to the connection instance.
 ** @param[in]  nMaxMS          -1 => block indefinitely
 **                              0 => just peek at input queue and
 **                                   socket queue, return immediately
 **                                   no matter what
 **                             >0 => ms to await complete frame
 **
 ** @return     LLRP_RC_OK          Frame received
 **             LLRP_RC_RecvEOF     End-of-file condition on fd
 **             LLRP_RC_RecvIOError I/O error in poll() or read().
 **                                 Probably means fd is bad.
 **             LLRP_RC_RecvFramingError
 **                                 LLRP_FrameExtract() detected an
 **                                 impossible situation. Recovery unlikely.
 **             LLRP_RC_RecvTimeout Frame didn't complete within allowed time
 **             LLRP_RC_...         Decoder error.
 **
 *****************************************************************************/

static LLRP_tResultCode
recvAdvance (
  LLRP_tSConnection *           pConn,
  int                           nMaxMS,
  time_t                        timeLimit)
{
    LLRP_tSErrorDetails *       pError = &pConn->Recv.ErrorDetails;

    /*
     * Clear the error details in the receiver state.
     */
    LLRP_Error_clear(pError);

    /*
     * Loop until victory or some sort of exception happens
     */
    for(;;)
    {
        int                     rc;

        /*
         * Note that the frame is in progress.
         * Existing buffer content, if any, is deemed
         * invalid or incomplete.
         */
        pConn->Recv.bFrameValid = FALSE;

        /*
         * Check to see if we have a frame in the buffer.
         * If not, how many more bytes do we need?
         *
         * LLRP_FrameExtract() status
         *
         * LLRP_FRAME_ERROR     Impossible situation, like message
         *                      length too small or the like.
         *                      Recovery in this situation is
         *                      unlikely and probably the app
         *                      should drop the connection.
         *
         * LLRP_FRAME_READY     Frame is complete. Details are
         *                      available for pre-decode decisions.
         *
         * LLRP_FRAME_NEED_MORE Need more input bytes to finish the frame.
         *                      The nBytesNeeded field is how many more.
         */
        pConn->Recv.FrameExtract =
            LLRP_FrameExtract(pConn->Recv.pBuffer, pConn->Recv.nBuffer);

        /*
         * Framing error?
         */
#ifdef __cplusplus
        if(pConn->Recv.FrameExtract.LLRP_FRAME_ERROR == pConn->Recv.FrameExtract.eStatus)
#else
		if(LLRP_FRAME_ERROR == pConn->Recv.FrameExtract.eStatus)
#endif
        {
            LLRP_Error_resultCodeAndWhatStr(pError,
            LLRP_RC_RecvFramingError, "framing error in message stream");
            break;
        }

        /*
         * Need more bytes? extractRc>0 means we do and extractRc is the
         * number of bytes immediately required.
         */
#ifdef __cplusplus
        if(pConn->Recv.FrameExtract.LLRP_FRAME_NEED_MORE == pConn->Recv.FrameExtract.eStatus)
#else
		if(LLRP_FRAME_NEED_MORE == pConn->Recv.FrameExtract.eStatus)
#endif
        {
            unsigned int        nRead = pConn->Recv.FrameExtract.nBytesNeeded;
            unsigned char *     pBufPos =
                                  &pConn->Recv.pBuffer[pConn->Recv.nBuffer];

            /*
             * Before we do anything that might block,
             * check to see if the time limit is exceeded.
             */
            if(0 != timeLimit)
            {
#ifndef WINCE
                if(time(NULL) > timeLimit)
                {
                    /* Timeout */
                    LLRP_Error_resultCodeAndWhatStr(pError,
                        LLRP_RC_RecvTimeout, "timeout");
                    break;
                }
#else
				// Need to check on this
#endif

            }

            /*
             * If this is not a block indefinitely request use poll()
             * to see if there is data in time.
             */
            if(nMaxMS >= 0)
            {
#ifdef linux
                struct pollfd   pfd;

                pfd.fd = pConn->fd;
                pfd.events = POLLIN;
                pfd.revents = 0;

                rc = poll(&pfd, 1, nMaxMS);
#endif /* linux */
#if defined(WIN32)|| defined(WINCE)
                fd_set          readfds;
                struct timeval  timeout;

                timeout.tv_sec = nMaxMS / 1000u;
                timeout.tv_usec = (nMaxMS % 1000u) * 1000u;

                FD_ZERO(&readfds);
                FD_SET(pConn->fd, &readfds);
                rc = select(-1, &readfds, NULL, NULL, &timeout);

#endif /* WIN32 */

                if(0 > rc)
                {
                    /* Error */
                    LLRP_Error_resultCodeAndWhatStr(pError,
                        LLRP_RC_RecvIOError, "poll failed");
                    break;
                }
                if(0 == rc)
                {
                    /* Timeout */
                    LLRP_Error_resultCodeAndWhatStr(pError,
                        LLRP_RC_RecvTimeout, "timeout");
                    break;
                }
            }

            /*
             * Read some number of bytes from the socket.
             */
#if defined(WIN32)|| defined(WINCE)
			rc = recv(pConn->fd, (char*)pBufPos, nRead, 0);
#else
            rc = read(pConn->fd, pBufPos, nRead);
#endif
            if(0 > rc)
            {
                /*
                 * Error. Note this could be EWOULDBLOCK if the
                 * file descriptor is using non-blocking I/O.
                 * So we return the error but do not tear-up
                 * the receiver state.
                 */
                LLRP_Error_resultCodeAndWhatStr(pError,
                    LLRP_RC_RecvIOError, "recv IO error");
                break;
            }

            if(0 == rc)
            {
                /* EOF */
                LLRP_Error_resultCodeAndWhatStr(pError,
                    LLRP_RC_RecvEOF, "recv end-of-file");
                break;
            }

            /*
             * When we get here, rc>0 meaning some bytes were read.
             * Update the number of bytes present.
             * Then loop to the top and retry the FrameExtract().
             */
            pConn->Recv.nBuffer += rc;

            continue;
        }

        /*
         * Is the frame ready?
         * If a valid frame is present, decode and enqueue it.
         */
#ifdef __cplusplus
        if(pConn->Recv.FrameExtract.LLRP_FRAME_READY == pConn->Recv.FrameExtract.eStatus)
#else
		if(LLRP_FRAME_READY == pConn->Recv.FrameExtract.eStatus)
#endif
        {
            /*
             * Frame appears complete. Time to try to decode it.
             */
            LLRP_tSFrameDecoder *   pDecoder;
            LLRP_tSMessage *        pMessage;
            LLRP_tSMessage **       ppMessageTail;

            /*
             * Construct a new frame decoder. It needs the registry
             * to facilitate decoding.
             */
            pDecoder = LLRP_FrameDecoder_construct(pConn->pTypeRegistry,
                    pConn->Recv.pBuffer, pConn->Recv.nBuffer);

            /*
             * Make sure we really got one. If not, weird problem.
             */
            if(pDecoder == NULL)
            {
                /* All we can do is discard the frame. */
                pConn->Recv.nBuffer = 0;
                pConn->Recv.bFrameValid = FALSE;
                LLRP_Error_resultCodeAndWhatStr(pError,
                    LLRP_RC_MiscError, "decoder constructor failed");
                break;
            }

            /*
             * Now ask the nice, brand new decoder to decode the frame.
             * It returns NULL for some kind of error.
             * The &...decoderHdr is in lieu of type casting since
             * the generic LLRP_Decoder_decodeMessage() takes the
             * generic LLRP_tSDecoder.
             */
            pMessage = LLRP_Decoder_decodeMessage(&pDecoder->decoderHdr);

            /*
             * Always capture the error details even when it works.
             * Whatever happened, we are done with the decoder.
             */
            pConn->Recv.ErrorDetails = pDecoder->decoderHdr.ErrorDetails;

            /*
             * Bye bye and thank you li'l decoder.
             */
            LLRP_Decoder_destruct(&pDecoder->decoderHdr);

            /*
             * If NULL there was an error. Clean up the
             * receive state. Return the error.
             */
            if(NULL == pMessage)
            {
                /*
                 * Make sure the return is not LLRP_RC_OK
                 */
                if(LLRP_RC_OK == pError->eResultCode)
                {
                    LLRP_Error_resultCodeAndWhatStr(pError,
                        LLRP_RC_MiscError, "NULL message but no error");
                }

                /*
                 * All we can do is discard the frame.
                 */
                pConn->Recv.nBuffer = 0;
                pConn->Recv.bFrameValid = FALSE;

                break;
            }

            /*
             * Yay! It worked. Enqueue the message.
             */
            ppMessageTail = &pConn->pInputQueue;
            while(NULL != *ppMessageTail)
            {
                ppMessageTail = &(*ppMessageTail)->pQueueNext;
            }

            pMessage->pQueueNext = NULL;
            *ppMessageTail = pMessage;

            /*
             * Note that the frame is valid. Consult
             * Recv.FrameExtract.MessageLength.
             * Clear the buffer count to be ready for next time.
             */
            pConn->Recv.bFrameValid = TRUE;
            pConn->Recv.nBuffer = 0;

            break;
        }

        /*
         * If we get here there was an FrameExtract status
         * we didn't expect.
         */

        /*NOTREACHED*/
        assert(0);
    }

    return pError->eResultCode;
}


/**
 *****************************************************************************
 **
 ** @brief  Internal routine to calculate time limit
 **
 ** Based on nMaxMS, the subscriber specified max time to
 ** await receipt of a (specific) message, determine the
 ** last time() to try.
 **
 ** The timeLimit prevents "spinning".
 ** See LLRP_Conn_recvResponse() above.
 **
 ** @param[in]  nMaxMS          -1 => block indefinitely
 **                              0 => just peek at input queue and
 **                                   socket queue, return immediately
 **                                   no matter what
 **                             >0 => ms to await complete frame
 **
 ** @return     timeLimit        0 => never stop
 **                             >0 => latest time() to try
 **
 *****************************************************************************/

static time_t
calculateTimeLimit (
  int                           nMaxMS)
{
    if(0 == nMaxMS)
    {
        /* When just peeking, try for at most one second */
#ifndef WINCE
        return time(NULL) + 1;
#else
		return  1;
#endif
    }
    else if(0 < nMaxMS)
    {
        /*
         * Try for a at most a certain period of time.
         *
         * timeLimit = now + ceil(nMaxMS/1000) + 1
         *
         * The last +1 compensates for not knowing
         * when the next time() tick will happen.
         *
         * For example, if now is SECONDS.999 seconds
         * the next tick will happen in 1 millisecond.
         * Suppose nMaxMS is 500ms (1/2 second).
         * Even rounding 500ms up to 1 second, the
         * time limit without the +1 would be
         * SECONDS+1 -- 1ms away. That's too soon.
         *
         * The extra +1 makes the maximum timeout
         * longer than required. But the timeLimit
         * is a safeguard anyway and usually the
         * timeout will occur when the user wants.
         */
#ifndef WINCE
        return time(NULL) + ((nMaxMS + 1999u) / 1000u);
#else
		return ((nMaxMS + 1999u) / 1000u);
#endif
    }
    else
    {
        /* Try indefinitely */
        return 0;
    }
}
