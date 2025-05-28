/*
 * motor_controller.h
 *
 *  Created on: May 15, 2025
 *      Author: VR
 */

#ifndef INC_DRIVE_CONTROLLER_HPP_
#define INC_DRIVE_CONTROLLER_HPP_

#include <cstdint>
#include "encoder_gen.hpp"

class drive {
public:
	float filtered_angle;
	float raw_gear_angle_from_encoder;
	drive();
	encoder e;
	void update();
	void set_position_to_go(float rad);
	float get_position();
	float get_velocity();
	void move_to_encoder_position(uint16_t encoder_tics);
	void calibrate_encoder();
private:
	void move(float speed);


};



#endif /* INC_DRIVE_CONTROLLER_HPP_ */
