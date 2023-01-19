#ifndef _TMR_SERIAL_TRANSPORT_H
#define _TMR_SERIAL_TRANSPORT_H
/**
 *  @file tmr_serial_transport.h
 *  @brief Mercury API - Serial Transport Interface
 *  @author Nathan Williams
 *  @date 10/20/2009
 */

/*
 * Copyright (c) 2009 ThingMagic, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef TMR_ENABLE_SERIAL_TRANSPORT_LLRP
#include <sys/types.h>
#include <sys/socket.h>
#endif

#if defined(WIN32) || defined(WINCE)
#if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
#include <winsock2.h>
#endif
#endif

#ifdef OSDEP_SERIAL_INCLUDE
#include "osdep_serial_transport.h"
#endif

#ifndef PLATFORM_HANDLE
# ifdef WIN32
#  define PLATFORM_HANDLE HANDLE
# else
#if defined(ARDUINO) || defined(FREERTOS_USED)
#   define PLATFORM_HANDLE void*
# else
#  define PLATFORM_HANDLE int
# endif
# endif
#endif
typedef struct TMR_SR_SerialTransport TMR_SR_SerialTransport;
/**
 * The TMR_SR_SerialTransport structure is the mechanism that the
 * SerialReader layer uses to conduct low-level communications with
 * the device. Users may create their own SerialTransport objects for
 * systems with custom serial communication needs, and then use
 * TMR_SR_SerialReader_init() to complete the construction of the
 * reader object, as an alternative to using TMR_create().
 *
 * @see TMR_SR_SerialPortTransportInit and
 * TMR_SR_LlrpEapiTransportInit for pre-existing
 * TMR_SR_SerialTransport implementations.
 */
struct TMR_SR_SerialTransport
{
  /** Context value made available to callback functions */
  void *cookie;

  /**
   * This callback causes the communication interface to be opened but
   * does not transmit any serial-layer data. This should perform
   * actions such as opening a serial port device or establishing a
   * network connection within a wrapper protocol.
   *
   * @param this The TMR_SR_SerialTransport structure.
   */
  TMR_Status (*open)(TMR_SR_SerialTransport *);

  /**
   * This callback transmits the provided bytes on the serial
   * transport. If the operation takes longer than timeoutMs to
   * complete, TMR_ERROR_TIMEOUT should be returned.
   *
   * @param this The TMR_SR_SerialTransport structure.
   * @param length The number of bytes to send.
   * @param message Pointer to the bytes to send.
   * @param timeoutMs The duration for the operation to complete.
   */
  TMR_Status (*sendBytes)(TMR_SR_SerialTransport *, uint32_t length, 
                          uint8_t* message, const uint32_t timeoutMs);

  /**
   * This callback receives bytes message on the serial transport. The
   * length parameter contains the size of the buffer pointed to by
   * message. If the operation takes longer than timeoutMs to receive
   * length bytes, TMR_ERROR_TIMEOUT should be returned.
   *
   * @param this The TMR_SR_SerialTransport structure.
   * @param length The number of bytes to receive.
   * @param[out] messageLength The number of bytes received.
   * @param[out] message Pointer to the location to store received bytes.
   * @param timeoutMs The duration for the operation to complete.
   */
  TMR_Status (*receiveBytes)(TMR_SR_SerialTransport *, uint32_t length,
                             uint32_t *messageLength, uint8_t* message, const uint32_t timeoutMs);
  /**
   * This callback causes the underlying serial transport connected to
   * the device to the provided baud rate.
   *
   * @param this The TMR_SR_SerialTransport structure.
   * @param rate The baud rate to set.
   */
  TMR_Status (*setBaudRate)(TMR_SR_SerialTransport *, uint32_t rate);

  /**
   * This callback releases any resources allocated by the transport
   * layer and informs the other end, if necessary, that the
   * connection is ending.
   *
   * @param this The TMR_SR_SerialTransport structure.
   */
  TMR_Status (*shutdown)(TMR_SR_SerialTransport *);

  /**
   * This callback takes any actions necessary (possibly none) to
   * remove unsent data from the output path.
   *
   * @param this The TMR_SR_SerialTransport structure.
   */
  TMR_Status (*flush)(TMR_SR_SerialTransport *);
};

#ifdef TMR_ENABLE_SERIAL_TRANSPORT_NATIVE
/**
 * The context structure used by the provided serial port transport interface.
 */
typedef struct TMR_SR_SerialPortNativeContext
{
  /** @privatesection */
  /** The file handle for the serial device */
  PLATFORM_HANDLE handle;
  /** The filesystem name of the serial device */
  char devicename[TMR_MAX_READER_NAME_LENGTH];
} TMR_SR_SerialPortNativeContext;
#endif

#ifdef TMR_ENABLE_SERIAL_TRANSPORT_LLRP
/**
 * The context structure used by the provided
 * serial-encapsulated-in-LLRP transport interface.
 */
typedef struct TMR_SR_LlrpEapiTransportContext
{
  /** @privatesection */
  /** The network address of the remote device */
  struct sockaddr_storage addr;
  /** The length of the contents of addr */
  socklen_t addrlen;
  /** Whether or not to power-cycle the reader upon opening a connection */
  bool powerCycleAtOpen;
  /** The file handle for the network socket */
  int socket;
  /** The current LLRP message ID */
  int sequenceId;
  /** Temporary buffer for receiving and processing a LLRP packet */
  uint8_t buf[270];
  /** Pointers into the buffer */
  uint16_t bufstart, buflen;
} TMR_SR_LlrpEapiTransportContext;
#endif

#ifdef TMR_ENABLE_SERIAL_TRANSPORT_NATIVE
/**
 * Initialize a TMR_SR_SerialTransport structure with a given serial device.
 *
 * @param transport The TMR_SR_SerialTransport structure to initialize.
 * @param context A TMR_SR_SerialPortNativeContext structure for the callbacks to use.
 * @param device The path or name of the serial device (@c /dev/ttyS0, @c COM1)
 */
#if defined(WIN32) || defined(WINCE)
__declspec(dllexport)
#endif
TMR_Status TMR_SR_SerialTransportNativeInit(TMR_SR_SerialTransport *transport,
                                            TMR_SR_SerialPortNativeContext *context,
                                            const char *device);
#if defined(WIN32) || defined(WINCE)
__declspec(dllexport)
#endif
TMR_Status TMR_SR_SerialTransportTcpNativeInit(TMR_SR_SerialTransport *transport,
                                            TMR_SR_SerialPortNativeContext *context,
                                            const char *device);
#endif /* TMR_ENABLE_SERIAL_TRANSPORT_NATIVE */

#ifdef TMR_ENABLE_SERIAL_TRANSPORT_LLRP
/**
 * Initialize a TMR_SR_SerialTransport structure with a LLRP+EAPI
 * connection to the given host and port.
 *
 * @param transport The TMR_SR_SerialTransport structure to initialize.
 * @param context A TMR_SR_LlrpEapiTransportContext structure for the callbacks to use.
 * @param host The name or address of the network device implementing LLRP+EAPI.
 * @param port The TCP port to use for the connection.
 * @param powerCycleAtOpen Whether or not to power-cycle the reader upon opening a connection.
 */
TMR_Status TMR_SR_LlrpEapiTransportInit(TMR_SR_SerialTransport *transport,
                                        TMR_SR_LlrpEapiTransportContext *context,
                                        const char *host, int port,
                                        bool powerCycleAtOpen);

/**
 * Power-cycle a device attached via LLRP transport
 * @param transport The TMR_SR_SerialTransport connected to the device
 */
TMR_Status TMR_LlrpEapiPowerCycle(TMR_SR_SerialTransport *this);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _TMR_SERIAL_TRANSPORT_H */
