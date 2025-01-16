/******************************************************************************
* File:             mgt.c
*
* Author:           Eric Xu  
* Created:          11/03/24 
* Description:      Based on ethanese's radio.c
*****************************************************************************/


#include "MGT/mgt.h"

uint64_t lastStateChange = 0;

const char PACKET_START = '{';
const char PACKET_END = '}';
typedef enum : uint8_t {
    SET_PERCENT = 'S',
    GET_CURRENT = 'C',
    ACKNOWLEDGE = 'A',
    SHUTDOWN = 'D',
    STOP_TIMER = 'T',
} mgt_op;

/**
 * Initialize mgt
 */
void mgt_init() {
	usart_init(MGT_USART, MGT_BAUDRATE);
}

/**
 * Transmit `nbytes` from `message` to MGT. References usart_transmitBytes
 */
void mgt_transmitBytes(uint8_t message[], int nbytes) {
	// Enable UART3 and Transmitter
	MGT_USART->CR1 |= USART_CR1_UE | USART_CR1_TE;

	// Transfer each character one at a time
	for (int i = 0; i < nbytes; i++){
		// wait until Data register is empty
		while (!(MGT_USART->ISR & USART_ISR_TXE));
		// Place the character in the Data Register
		MGT_USART->TDR = message[i];
	}

	// Wait for the Transfer to be completed by monitoring the TC flag
	while(!(MGT_USART->ISR & USART_ISR_TC));
}

/**
 * TODO
 * Pauses the system to wait for acknowledgement byte from other device
 *
 * @return false if time out, true if didn't received acknowledgement
 */
bool mgt_waitForAcknowledgement(uint64_t initialTime) {
	while (mgt_readOneByte() != 'A') {
		if (getSysTime() - initialTime > MGT_ACKNOWLEDGEMENT_TIMEOUT) {
			return false;
		}
	}
	return true;
}

/**
 * Set percent to Coil X PWM 0/1
 *
 * @param coilNumber - A number between 0 and 2 (inclusive)
 * @param pwm - 0 or 1
 * @param percentage - A number between 0 and 100 (inclusive)
 *
 * @return bytes sent if message is acknowledged, -E_MGT_LOST if message is sent but not
 * acknowledged, -E_MGT_INVALID if message is invalid and not sent
 */
int mgt_setCoilPercent(uint8_t coilNumber, uint8_t pwm, uint8_t percentage) {
    uint8_t message[3+3];
    message[0] = PACKET_START;
    message[1] = SET_PERCENT;
    message[2] = coilNumber;
    message[3] = pwm;
    message[4] = percentage;
    message[5] = PACKET_END;
	mgt_transmitBytes(message, 6);
    for (int i = 1; i < 5; i++) {
        if (message[i] == '{' || message[i] == '}')
            return -E_MGT_INVALID;
    }
	bool acknowledged = mgt_waitForAcknowledgement(getSysTime());
    return acknowledged? 6 : -E_MGT_LOST;
}

/**
 * Send a request to transfer data to the ground station
 *
 * @param numBytesToSend: The number of bytes to send to the ground station
 * @param numTimesToSend: Number of times to resend the data to ground station
 *
 * @return true if it succeeded and false if it failed
 */
//bool mgt_transferToGroundStationRequest(uint8_t numBytesToSend, int numTimesToSend) {
//	mgt_sendByte('t');
//	mgt_sendByte(numBytesToSend);
//	mgt_sendByte(numTimesToSend);
//
//	return mgt_waitForAcknowledgement(getSysTime());
//}

/**
 * Gets the current state of the mgt as r = RX_ACTIVE, t = TX_ACTIVE, o = OFF
 * If timesout will return -1 as an error
 *
 * @return the state of the mgt as a character
 */
//char mgt_receiveStateRequest(void){
//	mgt_sendByte('R');
//
//	char byteRead = -1;
//
//	uint64_t initRequestTime = getSysTime();
//	while (byteRead != 'r' && byteRead != 't' && byteRead != 'o') {
//
//		if (getSysTime() - initRequestTime > MGT_ACKNOWLEDGEMENT_TIMEOUT) {
//			return -1;
//		}
//
//		byteRead = mgt_readOneByte();
//	}
//
//	mgt_sendByte('A');
//
//	return byteRead;
//}

/**
 * Send a stream of bytes
 *
 * @param numberOfBytes: Number of bytes that is being sent in the buffer
 * @param buffer: the data to send
 *
 */
//void mgt_sendByteStream(int numberOfBytes, uint8_t buffer[]) {
//	uint64_t startingStreamTime = getSysTime();
//	for (int i = 0; i < numberOfBytes; i++) {
//			//If byte per second past threshold then halt byte sending
//			if (i / ((startingStreamTime - getSysTime()) / 1000) > BYTE_PER_SECOND_LIMIT) {
//				i--;
//			} else {
//				mgt_sendByte(buffer[i]);
//			}
//	}
//}


/**
 * Sends a stream of bytes
 *
 * @param numberOfBytes: Number of bytes to send
 * @param buffer: The buffer with the bytes to send
 *
 * @return False if sending failed and true otherwise
 */
//bool mgt_sendByteStreamToMemRequest(int numberOfBytes, uint8_t buffer[]) {
//	uint64_t startingStreamTime = getSysTime();
//	if (!mgt_sendTransferToMemRequest(numberOfBytes)) {
//		return false;
//	}
//	mgt_sendByteStream(numberOfBytes, buffer);
//	return true;
//}
//
//void mgt_sendState(enum MgtState state) {
//	if (getSysTime() - lastStateChange > MGT_MAX_STATE_CHANGE_TIME_LIMIT) {
//		switch (state) {
//			case MGT_OFF:
//				mgt_sendByte('o');
//				break;
//			case MGT_TX_ACTIVE:
//				mgt_sendByte('t');
//				break;
//			case MGT_RX_ACTIVE:
//				mgt_sendByte('r');
//				break;
//		}
//		lastStateChange = getSysTime();
//	}
//
//}


/**
 * Reads bytes from the mgt into a buffer
 *
 * @param buffer: the buffer that will be filled by the mgt's data
 * @param receive_buffer_size: the number of bytes the buffer will take
 */
int mgt_readBytes(uint8_t buffer[], int receive_buffer_size) {
	uint16_t amount_to_recieve = receive_buffer_size;
	uint16_t amount_read = 0;

	uint64_t initTime  = getSysTime();
	while (amount_read < amount_to_recieve) {

			if (getSysTime() - initTime > 10) {
                break;
			}

			amount_read += usart_recieveBytes(MGT_USART,
					buffer,
					(receive_buffer_size - amount_read));
			buffer += amount_read;	// pointer arithmetic :)

            // usart_recieverTimedOut not implemented
			//if (usart_recieverTimedOut(MGT_USART)) {
			//	break;
			//}
	}
    // Not sure what this line does
	//amount_read = usart_recieveBytes(MGT_USART, buffer, receive_buffer_size);
    return amount_read;
}

/**
 * Reads one byte from the buffer and returns it
 */
//int mgt_readOneByte() {
//	uint8_t buffer[1];
//	mgt_readBytes(buffer, 1);
//	return buffer[0];
//
//}
