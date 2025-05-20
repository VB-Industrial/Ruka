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

motor motor;

extern joint_config jc;

static float angle_enc_read_for_debug = 0.0;
static uint16_t enc_ticks_read_for_debug = 0;

//Основной цикл управления 1кГц здесь читается энкодер, к нему применяется фильтр и выдется управление на мотор в зависимости от уставки пришедшей сверху через cyphal
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM7) {
        motor.update();
    }
    if (htim->Instance == TIM6) {
    	angle_enc_read_for_debug = motor.read_encoder();
    	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
    }
}


motor::motor()
	{

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
	as50_readAngle(&raw_ticks_from_encoder, 100);
	enc_ticks_read_for_debug = raw_ticks_from_encoder;
	raw_gear_angle_from_encoder = static_cast<float>(raw_ticks_from_encoder) * ((M_PI * 2) / gear_encoder_CPR);
	return raw_gear_angle_from_encoder;
};

}
