/*
 * tim_int_hndlr.c
 *
 *  Created on: May 15, 2025
 *      Author: VR
 */

extern "C" {

#include "main.h"
#include "motor_controller.hpp"
#include "tmc5160.h"
#include "joint_config.h"
#include "as50xx.h"

motor m;

extern joint_config jc;

static float angle_enc_read_for_debug = 0.0; //to be able to read this value inside STM32Monitor
static uint16_t enc_ticks_read_for_debug = 0; //to be able to read this value inside STM32Monitor

//Основной цикл управления 1кГц здесь читается энкодер, к нему применяется фильтр и выдется управление на мотор в зависимости от уставки пришедшей сверху через cyphal
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM7) {
        m.update();
    }
    if (htim->Instance == TIM6) {
    	angle_enc_read_for_debug = m.read_encoder();
    	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
    }
}


motor::motor()
	{
	as50_readAngle(&this->prev_ticks_from_encoder, 100); //encoder init
	};

void motor::update()
	{

	};


void motor::set_position_to_go(float rad)
{

};

void motor::_apply_speed(float speed) {
			int32_t val_speed = 0;
			tmc5160_velocity(rad_to_steps(speed, jc.full_steps));
			tmc5160_move(val_speed);
};

float motor::get_position()
{
	return steps_to_rads(tmc5160_position_read(), jc.full_steps);
};

float motor::get_velocity()
{
	return steps_to_rads(tmc5160_velocity_read(), jc.full_steps);
};

float motor::read_encoder()
{
	uint16_t raw_ticks_from_encoder;
	uint16_t filtered_ticks_from_encoder;
	as50_readAngle(&raw_ticks_from_encoder, 100);
	filtered_ticks_from_encoder = simple_filter(raw_ticks_from_encoder);
	enc_ticks_read_for_debug = filtered_ticks_from_encoder; //to be able to read this value inside STM32Monitor
	raw_gear_angle_from_encoder = static_cast<float>(filtered_ticks_from_encoder) * ((M_PI * 2) / gear_encoder_CPR);
	return raw_gear_angle_from_encoder;
};

void motor::move_to_encoder_position(uint16_t encoder_tics_to_go)
{
		int8_t Kp = 100;
		uint32_t epsilon = 5;
		uint32_t deviation = 0;
		uint16_t raw_ticks_from_encoder;
		as50_readAngle(&raw_ticks_from_encoder, 100);
		deviation = encoder_tics_to_go - simple_filter(raw_ticks_from_encoder);
		while(deviation > epsilon)
		{
			as50_readAngle(&raw_ticks_from_encoder, 100);
			deviation = jc.direction * (encoder_tics_to_go - raw_ticks_from_encoder);
			tmc5160_move(deviation * Kp);
			HAL_Delay(1);
		}
		tmc5160_stop();
		//tmc5160_set_zero();
};

uint16_t motor::simple_filter(uint16_t encoder_tics)
{
	uint16_t filtered_encoder_value_tics = 0;
	filtered_encoder_value_tics = (encoder_tics + this->prev_ticks_from_encoder)/2;
	this->prev_ticks_from_encoder = filtered_encoder_value_tics;
	return filtered_encoder_value_tics;
};

}
