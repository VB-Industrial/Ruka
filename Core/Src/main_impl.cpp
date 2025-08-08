/*
 * main_cpp.cpp
 *
 *  Created on: May 16, 2025
 *      Author: VR
 */

extern "C" {
#include "communication.h"
#include "main_impl.h"
#include "IMU.h"

extern I2C_HandleTypeDef hi2c4;
extern FDCAN_HandleTypeDef hfdcan1;
motor_config mc;
joint_config jc;
joint_config_address jc_a;
uint16_t enc_angle = 0;

void main_cpp()
{
	//cyphal_can_starter(&hfdcan1);
	//setup_cyphal(&hfdcan1);
	HAL_Delay(10);
	joint_config_assembler(&jc, &jc_a);
	motor_config_assembler(&mc, &jc);
	//joint_config_read(&jc, &jc_a);
	HAL_Delay(10);
	tmc5160_init(&mc);
	//js_init();
	HAL_Delay(10);


	//uint8_t msg[10];
	//int i = 0;

	HAL_StatusTypeDef rv;

	uint32_t last_hbeat = HAL_GetTick();
	uint32_t last_js = HAL_GetTick();
	uint32_t last_AR = HAL_GetTick();

	vec_4ax linear = {0};
	vec_4ax quat = {0};
	vec_4ax gyro = {0};

	//tmc5160_move(50000);

	//rv = HAL_I2C_IsDeviceReady(&hi2c4, 0x29, 1, 10);
	//IMU_setup();
	while(1)
	{
	      uint32_t now = HAL_GetTick();
	      if ( (now - last_hbeat) >= 1000) {
	          last_hbeat = now;
	          heartbeat();
	      	  //imu_get_quat(&quat);
	      	  //imu_get_linear(&linear);
	      	  //imu_get_gyro(&gyro);
	          //sprintf(msg,"%d\n\0", q[1]);
	          //HAL_UART_Transmit_IT(&huart2, msg, sizeof(msg));
	          //send_IMU(&quat.w, &quat.x, &quat.y, &quat.z, &linear.x, &linear.y, &linear.z, &gyro.x, &gyro.y, &gyro.z);
	          //HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
	      }
	      if ( (now - last_js) >= 100) {
	    	  last_js = now;
	    	  send_JS(&jc);
	    	  //as50_readAngle(&enc_angle, 100);
	    	  //HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
	      }
	      cyphal_loop();
	}
};

} //extern C end
