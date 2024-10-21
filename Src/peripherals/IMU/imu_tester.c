#include "ASM330LHH.h"
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>
#include <inttypes.h>

void testFunction_IMU_AcelGyro_Reads() {
//	printMsg("Brub\r\n");
//	imu_init(IMU_ODR_3333_Hz, IMU_FS_2_g, IMU_ODR_3333_Hz, IMU_FS_1000_dps);

	 volatile int acel_X = 0;
	 int acel_Y = 0;
	 int acel_Z = 0;
	// float gyro_X = 0;
	// float gyro_Y = 0;
	// float gyro_Z = 0;

	while (1) {

		printMsg("AccelX: %" PRId16 ", AccelY: %" PRId16 ", AccelZ: %" PRId16
			", GyroX: %" PRId16 ", GyroY: %" PRId16 ", GyroZ: %" PRId16 "\r\n",
			imu_readAcel_X(), imu_readAcel_Y(), imu_readAcel_Z(),
			imu_readGyro_X(), imu_readGyro_Y(), imu_readGyro_Z());
//
//		acel_X = imu_readAcel_X();
//		acel_X = imu_readAcel_Y();
//		acel_X = imu_readAcel_Z();


	}
}
