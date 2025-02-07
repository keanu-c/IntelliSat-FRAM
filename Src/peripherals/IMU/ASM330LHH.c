/*
 * ASM330LHH.c  (IMU interface)
 *
 *	- Jan 30, 2024
 *		Author 		 : Basti
 *		Log			 : Utilized the SPI function
 *
 *	- Jan  6, 2024
 *		Author		 : Darsh
 *		Log			 : Edited function names to follow consistent naming
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : IMU Control functions written
 */

#include "ASM330LHH.h"

//GLOBAL VARIABLES
#define ACCEL_RATE_REG 0x10
#define GYRO_CTRL_REG  0x11
#define IMU_RESET_REG  0x12

// Sensitivity Values
#define ASM330LHH_ACC_SENSITIVITY_FS_2G   		0.061f
#define ASM330LHH_ACC_SENSITIVITY_FS_4G   		0.122f
#define ASM330LHH_ACC_SENSITIVITY_FS_8G   		0.244f
#define ASM330LHH_ACC_SENSITIVITY_FS_16G  		0.488f
#define ASM330LHH_GYRO_SENSITIVITY_FS_125DPS    4.370f
#define ASM330LHH_GYRO_SENSITIVITY_FS_250DPS    8.750f
#define ASM330LHH_GYRO_SENSITIVITY_FS_500DPS   17.500f
#define ASM330LHH_GYRO_SENSITIVITY_FS_1000DPS  35.000f
#define ASM330LHH_GYRO_SENSITIVITY_FS_2000DPS  70.000f
#define ASM330LHH_GYRO_SENSITIVITY_FS_4000DPS 140.000f

#define IMU_RESET_CMD  0x01

/****************************** IMU Properties ****************************/

float imu_acelFullScaleIMU0 = 0;
float imu_gyroFullScaleIMU0 = 0;

float imu_acelFullScaleIMU1 = 0;
float imu_gyroFullScaleIMU1 = 0;

enum IMU_SELECT IMU_global = IMU0;

/*************************** IMU Helper Functions *************************/

// #define ScaledData(data, scale) ((data * scale) * (uint16_t)(-1))
#define ScaledData(data, scale) ((data * scale))

#if OP_REV == 3

void set_IMU(enum IMU_SELECT select){
	IMU_global = select;
}

#endif

void imu_spiWriteReg(void *address, uint8_t data) {
	uint8_t spiDATA[2];
	spiDATA[0] = (uint8_t)address & 0x7F;
	spiDATA[1] = data;

#if OP_REV == 2
	
	spi_startCommunication(IMU_SPI_CS);
	spi_transmitReceive(IMU_SPI, spiDATA, NULL, 2, false);
	spi_stopCommunication(IMU_SPI_CS);

#elif OP_REV == 3

	if (IMU_global == IMU0) {
		spi_startCommunication(IMU0_SPI_CS);
		spi_transmitReceive(IMU0_SPI, spiDATA, NULL, 2, false);
		spi_stopCommunication(IMU0_SPI_CS);
	} else {
		spi_startCommunication(IMU1_SPI_CS);
		spi_transmitReceive(IMU1_SPI, spiDATA, NULL, 2, false);
		spi_stopCommunication(IMU1_SPI_CS);
	}
#endif

}

int16_t imu_spiReadHighLow(void *low_address) {
	uint8_t instruction = (uint8_t)low_address | 0x80;
	uint8_t datah = 0;
	uint8_t datal = 0;

#if OP_REV == 2

	spi_startCommunication(IMU_SPI_CS);
	spi_transmitReceive(IMU_SPI, &instruction, NULL, 1, false);
	spi_transmitReceive(IMU_SPI, NULL, &datal, 1, false);
	spi_transmitReceive(IMU_SPI, NULL, &datah, 1, false);
	spi_stopCommunication(IMU_SPI_CS);

#elif OP_REV == 3

	if (IMU_global == IMU0) {
		spi_startCommunication(IMU0_SPI_CS);
		spi_transmitReceive(IMU0_SPI, &instruction, NULL, 1, false);
		spi_transmitReceive(IMU0_SPI, NULL, &datal, 1, false);
		spi_transmitReceive(IMU0_SPI, NULL, &datah, 1, false);
		spi_stopCommunication(IMU0_SPI_CS);
	} else {
		spi_startCommunication(IMU1_SPI_CS);
		spi_transmitReceive(IMU1_SPI, &instruction, NULL, 1, false);
		spi_transmitReceive(IMU1_SPI, NULL, &datal, 1, false);
		spi_transmitReceive(IMU1_SPI, NULL, &datah, 1, false);
		spi_stopCommunication(IMU1_SPI_CS);
	}

#endif

	nop(10);

	return (datah << 8) | datal;
}

/**
 * Configures the accelerometer control register of an IMU device.
 *
 * @param acel_rate The rate of the accelerometer.
 * @param acel_scale The scale of the accelerometer.
 * @param digital_filter_on Whether the digital filter is enabled or not.
 *
 * @returns None
 */
void imu_acelCtrl(int acel_rate, int acel_scale, int digital_filter_on) {

	// local variables
	acel_rate &= 0xFF;
	acel_scale &= 0xF;
	digital_filter_on &= 1;
	int data = acel_rate << 4 | acel_scale << 2 | digital_filter_on << 1;

#if OP_REV == 1

	softi2c_writeReg(IMU_I2C, IMU_ADDR, ACCEL_RATE_REG, data);

#elif OP_REV == 2 || OP_REV == 3

	imu_spiWriteReg(ACCEL_RATE_REG, data);

#endif
	if (IMU_global == IMU1) { 
		switch (acel_scale) {
			case IMU_FS_2_g:
				imu_acelFullScaleIMU1 = ASM330LHH_ACC_SENSITIVITY_FS_2G / 1000;
				break;
			case IMU_FS_4_g:
				imu_acelFullScaleIMU1 = ASM330LHH_ACC_SENSITIVITY_FS_4G / 1000;
				break;
			case IMU_FS_8_g:
				imu_acelFullScaleIMU1 = ASM330LHH_ACC_SENSITIVITY_FS_8G / 1000;
				break;
			case IMU_FS_16_g:
				imu_acelFullScaleIMU1 = ASM330LHH_ACC_SENSITIVITY_FS_16G / 1000;
				break;
		}
	} else {
		switch (acel_scale) {
			case IMU_FS_2_g:
				imu_acelFullScaleIMU0 = ASM330LHH_ACC_SENSITIVITY_FS_2G / 1000;
				break;
			case IMU_FS_4_g:
				imu_acelFullScaleIMU0 = ASM330LHH_ACC_SENSITIVITY_FS_4G / 1000;
				break;
			case IMU_FS_8_g:
				imu_acelFullScaleIMU0 = ASM330LHH_ACC_SENSITIVITY_FS_8G / 1000;
				break;
			case IMU_FS_16_g:
				imu_acelFullScaleIMU0 = ASM330LHH_ACC_SENSITIVITY_FS_16G / 1000;
				break;
		}
	}

}

/**
 * Configures the gyroscope rate and scale for the IMU sensor.
 *
 * @param gyro_rate The rate of the gyroscope.
 * @param gyro_scale The scale of the gyroscope.
 *
 * @returns None
 */
void imu_gyroCtrl(int gyro_rate, int gyro_scale) {

	// local variables
	gyro_rate &= 0xFF;
	gyro_scale &= 0xFF;
	int data = gyro_rate << 4 | gyro_scale;

#if OP_REV == 1

	softi2c_writeReg(IMU_I2C, IMU_ADDR, GYRO_CTRL_REG, data);

#elif OP_REV == 2 || OP_REV == 3


	imu_spiWriteReg(GYRO_CTRL_REG, data);

#endif
	if (IMU_global == IMU0) {
		switch (gyro_scale) {
			case IMU_FS_125_dps:
				imu_gyroFullScaleIMU0 = ASM330LHH_GYRO_SENSITIVITY_FS_125DPS / 1000;
				break;
			case IMU_FS_250_dps:
				imu_gyroFullScaleIMU0 =  ASM330LHH_GYRO_SENSITIVITY_FS_250DPS / 1000;
				break;
			case IMU_FS_500_dps:
				imu_gyroFullScaleIMU0 =  ASM330LHH_GYRO_SENSITIVITY_FS_500DPS / 1000;
				break;
			case IMU_FS_1000_dps:
				imu_gyroFullScaleIMU0 =  ASM330LHH_GYRO_SENSITIVITY_FS_1000DPS / 1000;
				break;
			case IMU_FS_2000_dps:
				imu_gyroFullScaleIMU0 =  ASM330LHH_GYRO_SENSITIVITY_FS_2000DPS / 1000;
				break;
			case IMU_FS_4000_dps:
				imu_gyroFullScaleIMU0 =  ASM330LHH_GYRO_SENSITIVITY_FS_4000DPS / 1000;
				break;
		}
	} else {
		switch (gyro_scale) {
			case IMU_FS_125_dps:
				imu_gyroFullScaleIMU1 = ASM330LHH_GYRO_SENSITIVITY_FS_125DPS / 1000;
				break;
			case IMU_FS_250_dps:
				imu_gyroFullScaleIMU1 =  ASM330LHH_GYRO_SENSITIVITY_FS_250DPS / 1000;
				break;
			case IMU_FS_500_dps:
				imu_gyroFullScaleIMU1 =  ASM330LHH_GYRO_SENSITIVITY_FS_500DPS / 1000;
				break;
			case IMU_FS_1000_dps:
				imu_gyroFullScaleIMU1 =  ASM330LHH_GYRO_SENSITIVITY_FS_1000DPS / 1000;
				break;
			case IMU_FS_2000_dps:
				imu_gyroFullScaleIMU1 =  ASM330LHH_GYRO_SENSITIVITY_FS_2000DPS / 1000;
				break;
			case IMU_FS_4000_dps:
				imu_gyroFullScaleIMU1 =  ASM330LHH_GYRO_SENSITIVITY_FS_4000DPS / 1000;
				break;
		}
	}
}

/*************************** IMU Interface Functions *************************/

void imu_init(int acel_rate, int acel_scale, int gyro_rate, int gyro_scale) {

#if OP_REV == 1

	softi2c_init(IMU_I2C);
	softi2c_writeReg(IMU_I2C, IMU_ADDR, IMU_RESET_REG, IMU_RESET_CMD); // soft reset imu

#elif OP_REV == 2

	spi_config(IMU_SPI);
	imu_spiWriteReg(IMU_RESET_REG, IMU_RESET_CMD);

#elif OP_REV == 3

	if (IMU_global == IMU1) {
		spi_config(IMU1_SPI);
		imu_spiWriteReg(IMU_RESET_REG, IMU_RESET_CMD);
	} else {
		spi_config(IMU0_SPI);
		imu_spiWriteReg(IMU_RESET_REG, IMU_RESET_CMD);
	}

#endif

	//initialize accelerometer and gyroscope
	imu_acelCtrl(acel_rate, acel_scale, 0);
	imu_gyroCtrl(gyro_rate, gyro_scale);


}

float imu_readAcel_X() {

	uint8_t instructionHi = 0x29 | 0x80 ;	//Where we send Hi instruction
	uint8_t instructionLow = 0x28 | 0x80;	//Where we send Low instruction

    int16_t data = 0;

#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data =  imu_spiReadHighLow((uint8_t*)0x28);

#endif

    return IMU_global == IMU0 ? ScaledData(data, imu_acelFullScaleIMU0) : ScaledData(data, imu_acelFullScaleIMU1);

}

float imu_readAcel_Y() {

	uint8_t instructionHi = 0x2B | 0x80;	//Where we send Hi instruction
	uint8_t instructionLow = 0x2A | 0x80;	//Where we send Low instruction

    int16_t data = 0;

#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data = imu_spiReadHighLow((uint8_t*)0x2A);

#endif

    return IMU_global == IMU0 ? ScaledData(data, imu_acelFullScaleIMU0) : ScaledData(data, imu_acelFullScaleIMU1);

}

float imu_readAcel_Z() {

	uint8_t instructionHi = 0x2D | 0x80;	//Where we send Hi instruction
	uint8_t instructionLow = 0x2C | 0x80;	//Where we send Low instruction

    int16_t data = 0;

#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data = imu_spiReadHighLow((uint8_t*)0x2C);

#endif

    return IMU_global == IMU0 ? ScaledData(data, imu_acelFullScaleIMU0) : ScaledData(data, imu_acelFullScaleIMU1);
}

float imu_readGyro_X() {

	uint8_t instructionHi = 0x23 | 0x80;	//Where we send Hi instruction
	uint8_t instructionLow = 0x22 | 0x80;	//Where we send Low instruction

    int16_t data = 0;

	#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data = imu_spiReadHighLow((uint8_t*)0x22);

#endif

    return IMU_global == IMU0 ? ScaledData(data, imu_gyroFullScaleIMU0) : ScaledData(data, imu_gyroFullScaleIMU1);
}

float imu_readGyro_Y() {

	uint8_t instructionHi = 0x25 | 0x80;	//Where we send Hi instruction
	uint8_t instructionLow = 0x24 | 0x80;	//Where we send Low instruction

    int16_t data = 0;

#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data = imu_spiReadHighLow((uint8_t*)0x24);

#endif

    return IMU_global == IMU0 ? ScaledData(data, imu_gyroFullScaleIMU0) : ScaledData(data, imu_gyroFullScaleIMU1);
}

float imu_readGyro_Z() {
	uint8_t instructionHi = 0x27 | 0x80;	//Where we send Hi instruction
	uint8_t instructionLow = 0x26 | 0x80;	//Where we send Low instruction

    int16_t data = 0;

#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data = imu_spiReadHighLow((uint8_t*)0x26);

#endif

    return IMU_global == IMU0 ? ScaledData(data, imu_gyroFullScaleIMU0) : ScaledData(data, imu_gyroFullScaleIMU1);
}

float imu_readTemp() {

	uint8_t instructionHi = 0x21 | 0x80;	//Where we send Hi instruction
	uint8_t instructionLow = 0x20 | 0x80;	//Where we send Low instruction

    int16_t data = 0;

#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data = imu_spiReadHighLow((uint8_t*)0x20);

#endif

    return (data / 256.0) + 25;
}

bool imu_isCommunicationWorking() {

	uint8_t value;
	uint8_t instruction = 0x0F | 0x80;
	
	if (IMU_global == IMU0) {
		spi_startCommunication(IMU0_SPI_CS);
		spi_transmitReceive(IMU0_SPI, &instruction, NULL, 1, false);
		spi_transmitReceive(IMU0_SPI, NULL, &value, 1, false);
		spi_stopCommunication(IMU0_SPI_CS);
	} else {
		spi_startCommunication(IMU1_SPI_CS);
		spi_transmitReceive(IMU1_SPI, &instruction, NULL, 1, false);
		spi_transmitReceive(IMU1_SPI, NULL, &value, 1, false);
		spi_stopCommunication(IMU1_SPI_CS);
	}

	return value == 0x6B;
}

void imu_printAllValues() {
	printMsg("Gyro\r\n");
	printMsg("X: %f\r\n", imu_readGyro_X());
	printMsg("Y: %f\r\n", imu_readGyro_Y());
	printMsg("Z: %f\r\n", imu_readGyro_Z());
	printMsg("Accel:\r\n");
	printMsg("X: %f\r\n", imu_readAcel_X());
	printMsg("Y: %f\r\n", imu_readAcel_Y());
	printMsg("Z: %f\r\n", imu_readAcel_Z());
	printMsg("%f\r\n", imu_readTemp());
}
