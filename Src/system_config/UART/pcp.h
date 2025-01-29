/******************************************************************************
* File:             pcp.h
*
* Author:           Eric Xu  
* Created:          2025-01-26 20:54
* Description:      Peripheral control protocol (PCP) is a transmission protocol
*                   built above UART that allows the sending and receiving of
*                   packets. Features include
*                   - Buffered send and receive
*                   - Acknowledgement and retransmission
*
*****************************************************************************/


#ifndef SYSTEM_CONFIG_UART_PCP_H_
#define SYSTEM_CONFIG_UART_PCP_H_

#include <globals.h>
#include <stm32l476xx.h>
#include <UART/uart.h>
#include <stdarg.h>
#include <Timers/timers.h>
#include <stdlib.h>

#define E_LOST 1
#define E_INVALID 2
#define E_OVERFLOW 3

#define PAYLOAD_MAXBYTES 125
// PACKET_START, SEQ_NUM, PACKET_END
#define OVERHEAD_MAXBYTES 3
// Without escaping payload
#define PACKET_MAXBYTES PAYLOAD_MAXBYTES + OVERHEAD_MAXBYTES

typedef struct PCPDevice PCPDevice;

bool request(uint8_t payload[], int nbytes);
int getResponse(uint8_t* buf);
void retransmit(PCPDevice* dev);

#endif /* SYSTEM_CONFIG_UART_PCP_H_ */
