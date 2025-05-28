/*
 * encoder_gen.h
 *
 *  Created on: May 28, 2025
 *      Author: VR
 */

#ifndef INC_ENCODER_GEN_HPP_
#define INC_ENCODER_GEN_HPP_

class encoder
{
public:
	encoder();
	float get_angle();
	uint16_t get_ticks();
private:
	uint16_t gear_encoder_CPR {16383};
	float alpha_for_filter;
	uint16_t prev_ticks_from_encoder;
};

#endif /* INC_ENCODER_GEN_HPP_ */
