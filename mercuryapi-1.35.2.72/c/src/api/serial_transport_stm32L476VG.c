
/**
 *  @file serial_transport_stm32L476VG.c
 *  @brief Mercury API - Serial transport functions for STM32F103RB board
 *  @author Pallav Joshi
 *  @date 5/16/2016
 */


/*
 * Copyright (c) 2016 ThingMagic, Inc.
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

#include "tm_reader.h"
#include "main.h"

/*** MAX_TX_BUFFER_LENGTH Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
#define MAX_TX_BUFFER_LENGTH   256
#define TBUFLEN ((uint32_t)((tbuf.head < tbuf.tail) ? ((MAX_TX_BUFFER_LENGTH- tbuf.tail)+tbuf.head):(tbuf.head - tbuf.tail)))

/*** MAX_RX_BUFFER_LENGTH Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
#define MAX_RX_BUFFER_LENGTH   256
#define RBUFLEN ((uint32_t)((rbuf.head < rbuf.tail) ? ((MAX_RX_BUFFER_LENGTH- rbuf.tail)+rbuf.head):(rbuf.head - rbuf.tail)))

/* Circular Buffer Structure */
typedef struct buf_st
{
  volatile uint32_t head;                      /* Next In Index  */
  uint32_t tail;                               /* Next Out Index */
  volatile char buffer [MAX_RX_BUFFER_LENGTH]; /* Buffer         */
}circBuf_t;

circBuf_t rbuf,tbuf;

/************************Circular Buffer length validation*********************************************/  

#if MAX_TX_BUFFER_LENGTH < 2
#error MAX_TX_BUFFER_LENGTH is too small.  It must be larger than 1.
#elif ((MAX_TX_BUFFER_LENGTH & (MAX_TX_BUFFER_LENGTH-1)) != 0)
#error MAX_TX_BUFFER_LENGTH must be a power of 2.
#endif

#if MAX_RX_BUFFER_LENGTH < 2
#error MAX_RX_BUFFER_LENGTH is too small.  It must be larger than 1.
#elif ((MAX_RX_BUFFER_LENGTH & (MAX_RX_BUFFER_LENGTH-1)) != 0)
#error MAX_RX_BUFFER_LENGTH must be a power of 2.
#endif
/*********************************************************************/  

static unsigned int tx_restart = 1;             /* NZ if TX restart is required     */
bool inIsrTX = false;

/********** USART1_IRQHandler Handles USART1 global interrupt request. ***********/
void USART1_IRQHandler (void) 
{
  volatile unsigned int IIR;
  uint32_t next;
  uint8_t receiveByte;
  uint32_t sendByte;

  IIR = USART1->ISR;
  if (IIR & USART_ISR_RXNE)                      /* read interrupt  */
  {
    USART1->ISR &= ~USART_ISR_RXNE;               /* clear interrupt */

    receiveByte = (USART1->RDR & 0x1FF);
    next= rbuf.head + 1;
    next &= (MAX_RX_BUFFER_LENGTH-1);

    if (next != rbuf.tail)
    {
      rbuf.buffer[rbuf.head] = receiveByte;
      rbuf.head = next;
    }
   }
  
  if (IIR & USART_ISR_TXE)                       /* read interrupt  */
  {
    USART1->ISR &= ~USART_ISR_TXE;               /* clear interrupt  */
    if (tbuf.head != tbuf.tail)
    {
      sendByte =(tbuf.buffer[tbuf.tail] & 0x1FF);
      USART1->TDR  = sendByte ;
      inIsrTX = true;
      next = tbuf.tail + 1;
      next &= (MAX_TX_BUFFER_LENGTH-1);
      tbuf.tail = next;
    }
    else
    {
      tx_restart = 1;
      USART1->CR1 &= ~USART_ISR_TXE;           /* disable TX IRQ if nothing to send */
    }
  }
}


/****************** SendChar:Sends a character ****************/  
int SendChar (int c) 
{
  if (TBUFLEN >= MAX_TX_BUFFER_LENGTH)          /* If the buffer is full            */
  {
    return (-1);                                /* return an error value            */
  }

  tbuf.buffer[tbuf.head] = c;
  inIsrTX = false;
  tbuf.head +=1;

  if(tbuf.head == MAX_TX_BUFFER_LENGTH)
  {
    tbuf.head = 0;
  }

  if(tx_restart)                                /* If TX interrupt is disabled   */
  {
    tx_restart = 0;                             /* enable it                     */
    USART1->CR1 |= USART_ISR_TXE;               /* enable TX interrupt           */
  }

  return (0);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
  {
    Error_Handler();
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  LL_RCC_MSI_Enable();

  /* Wait till MSI is ready */
  while(LL_RCC_MSI_IsReady() != 1)
  {
  }
  LL_RCC_MSI_EnableRangeSelection();
  LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_6);
  LL_RCC_MSI_SetCalibTrimming(0);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_MSI);

  /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_MSI)
  {
  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  LL_Init1msTick(4000000);

  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
  LL_SetSystemCoreClock(4000000);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
  /* USER CODE BEGIN USART1_Init 0 */
  /* USER CODE END USART1_Init 0 */
  LL_USART_InitTypeDef USART_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);

  /**USART1 GPIO Configuration
   * PB6   ------> USART1_TX
   * PB7   ------> USART1_RX 
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6|LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USART1 interrupt Init */
  NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(USART1_IRQn);

  /* USER CODE BEGIN USART1_Init 1 */
  /* USER CODE END USART1_Init 1 */
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_EnableIT_RXNE(USART1);
  LL_USART_EnableIT_TXE(USART1);
  LL_USART_Enable(USART1);
  /* USER CODE BEGIN USART1_Init 2 */
  /* USER CODE END USART1_Init 2 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


/****************** Stub implementation of serial transport layer routines. ****************/  
static TMR_Status
s_open(TMR_SR_SerialTransport *this)
{
  int i;

  /* Initialize the Tx circular buffer structure */
  tbuf.head = 0;
  tbuf.tail = 0;
  for(i = 0; i< MAX_RX_BUFFER_LENGTH; i++)
  {
    tbuf.buffer[i] = 0;
  }

  /* Initialize the Rx circular buffer structure */
  rbuf.head = 0;
  rbuf.tail = 0;
  for(i = 0; i< MAX_RX_BUFFER_LENGTH; i++)
  {
    rbuf.buffer[i] = 0;
  }
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  NVIC_EnableIRQ(USART1_IRQn);

  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  //MX_GPIO_Init();
  MX_USART1_UART_Init();

  return TMR_SUCCESS;
}

  
static TMR_Status
s_sendBytes(TMR_SR_SerialTransport *this, uint32_t length, 
                uint8_t* message, const uint32_t timeoutMs)
{
  /* This routine should send length bytes, pointed to by message on
  * the serial connection. If the transmission does not complete in
  * timeoutMs milliseconds, it should return TMR_ERROR_TIMEOUT.
  */
  uint32_t i = 0;
  tbuf.head = 0;                                /* clear com buffer indexes */
  tbuf.tail = 0;

  for (i = 0; i<length; i++)
  {
    SendChar(message[i]);
  }
  return TMR_SUCCESS;
}

static TMR_Status
s_receiveBytes(TMR_SR_SerialTransport *this, uint32_t length,
uint32_t* messageLength, uint8_t* message, const uint32_t timeoutMs)
{
  /* This routine should receive exactly length bytes on the serial
  * connection and store them into the memory pointed to by
  * message. If the required number of bytes are note received in
  * timeoutMs milliseconds, it should return TMR_ERROR_TIMEOUT.
  */
  uint32_t index = 0;
  *messageLength = 0;

  while (index < length )
  {
    uint32_t next;

    while (rbuf.head == rbuf.tail);             /*If no data in circular buffer then wait for data*/
    if (rbuf.head != rbuf.tail)
    {
      message[index] = rbuf.buffer[rbuf.tail];
      next = rbuf.tail + 1;
      next &= (MAX_RX_BUFFER_LENGTH-1);
      rbuf.tail = next;
    }
    index++;
  }
  *messageLength = index;

  return TMR_SUCCESS;
}

static TMR_Status
s_setBaudRate(TMR_SR_SerialTransport *this, uint32_t rate)
{
  /* This routine should change the baud rate of the serial connection
  * to the specified rate, or return TMR_ERROR_INVALID if the rate is
  * not supported.
  */
  USART1->BRR   = 4000000 / (long) rate;       /* 115200 baud @ PCLK2 72MHz */

  return TMR_SUCCESS;
}

static TMR_Status
s_shutdown(TMR_SR_SerialTransport *this)
{
  /* This routine should close the serial connection and release any
  * acquired resources.
  */
  USART1->CR1 &= ~USART_ISR_TXE;     /* disable TX IRQ if nothing to send */
  return TMR_SUCCESS;
}

static TMR_Status
s_flush(TMR_SR_SerialTransport *this)
{
  /* This routine should empty any input or output buffers in the
  * communication channel. If there are no such buffers, it may do
  * nothing.
  */
  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_SR_SerialTransport structure with a given serial device.
 *
 * @param transport The TMR_SR_SerialTransport structure to initialize.
 * @param context A TMR_SR_SerialPortNativeContext structure for the callbacks to use.
 * @param device The path or name of the serial device (@c /dev/ttyS0, @c COM1)
 */
TMR_Status
TMR_SR_SerialTransportNativeInit(TMR_SR_SerialTransport *transport,
                                 TMR_SR_SerialPortNativeContext *context,
                                 const char *device)
{
  /* Each of the callback functions will be passed the transport
   * pointer, and they can use the "cookie" member of the transport
   * structure to store the information specific to the transport,
   * such as a file handle or the memory address of the FIFO.
   */

  transport->cookie = context;
  transport->open = s_open;
  transport->sendBytes = s_sendBytes;
  transport->receiveBytes = s_receiveBytes;
  transport->setBaudRate = s_setBaudRate;
  transport->shutdown = s_shutdown;
  transport->flush = s_flush;

#if TMR_MAX_SERIAL_DEVICE_NAME_LENGTH > 0
  if (strlen(device) + 1 > TMR_MAX_SERIAL_DEVICE_NAME_LENGTH)
  {
    return TMR_ERROR_INVALID;
  }
  strcpy(context->devicename, device);
  return TMR_SUCCESS;
#else
  /* No space to store the device name, so open it now */
  return s_open(transport);
#endif
}
