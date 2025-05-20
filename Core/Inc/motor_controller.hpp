/*
 * motor_controller.h
 *
 *  Created on: May 15, 2025
 *      Author: VR
 */

#ifndef INC_MOTOR_CONTROLLER_HPP_
#define INC_MOTOR_CONTROLLER_HPP_

class motor {
public:
	float filtered_angle;
	float raw_gear_angle_from_encoder;
	motor();
	void update();
	float read_encoder();
	void set_position_to_go(float rad);
	float get_position();
	float get_velocity();
private:
	void _apply_speed(float speed);
	uint16_t gear_encoder_CPR {16383};
};



#endif /* INC_MOTOR_CONTROLLER_HPP_ */
