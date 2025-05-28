/*
 * encoder_gen.cpp
 *
 *  Created on: May 28, 2025
 *      Author: VR
 */

extern "C"{

#include "utility.h"
#include "as50xx.h"
#include "encoder_gen.hpp"




encoder::encoder()
{
	as50_readAngle(&this->prev_ticks_from_encoder, 100); //encoder init
	this->alpha_for_filter = 0.5;
}

uint16_t encoder::get_ticks()
{
	uint16_t raw_ticks_from_encoder;
	uint16_t filtered_ticks_from_encoder;
	as50_readAngle(&raw_ticks_from_encoder, 100); //реализация для конкретного энкодера
	filtered_ticks_from_encoder = simple_EMA_filter(raw_ticks_from_encoder, this->prev_ticks_from_encoder, this->alpha_for_filter);
	this->prev_ticks_from_encoder = filtered_ticks_from_encoder;
	return filtered_ticks_from_encoder;
}

float encoder::get_angle()
{
	uint16_t raw_ticks_from_encoder;
	uint16_t filtered_ticks_from_encoder;
	float gear_angle_from_encoder;
	as50_readAngle(&raw_ticks_from_encoder, 100); //реализация для конкретного энкодера
	filtered_ticks_from_encoder = simple_EMA_filter(raw_ticks_from_encoder, this->prev_ticks_from_encoder, this->alpha_for_filter);
	gear_angle_from_encoder = static_cast<float>(filtered_ticks_from_encoder) * ((M_PI * 2) / gear_encoder_CPR);
	return gear_angle_from_encoder;
};


}// extern C
