/*
 * mainimpl.h
 *
 *  Created on: Feb 28, 2024
 *      Author: VR
 */

#ifndef INC_MAINIMPL_H_
#define INC_MAINIMPL_H_

#include "joint_config.h"

void cyphal_loop();
void heartbeat();
void send_JS(joint_config * jc); //float*, float*, float*
void send_IMU(float* qw, float* qx, float* qy, float* qz, float* ax, float* ay, float* az, float* gx, float* gy, float* gz);
void setup_cyphal(FDCAN_HandleTypeDef*);
void cyphal_can_starter(FDCAN_HandleTypeDef*);
void calib_move(joint_config * jc);



#endif /* INC_MAINIMPL_H_ */
