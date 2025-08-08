/*
 * tim_int_hndlr.c
 *
 *  Created on: May 15, 2025
 *      Author: VR
 */

extern "C" {

#include "main.h"
#include "drive_controller.hpp"
#include "tmc5160.h"
#include "joint_config.h"
#include "encoder_gen.hpp"

drive drv;

extern joint_config jc;

static uint16_t D_enc_ticks = 0; //to be able to read this value inside STM32Monitor
static float D_enc_angle = 0.0; //to be able to read this value inside STM32Monitor


//Основной цикл управления 1кГц здесь читается энкодер, к нему применяется фильтр и выдется управление на мотор в зависимости от уставки пришедшей сверху через cyphal
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM7) {
        drv.update();
    }
    if (htim->Instance == TIM6) {
    	D_enc_ticks = drv.e.get_ticks();
    	D_enc_angle = drv.e.get_angle();
    	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
    }
}


drive::drive()
	{

	};

void drive::update()
	{

	};


void drive::js_move(float pos, float w, float w_acc)
{
	if(w_acc != 0.0)
	{
		//tmc5160_velocity(rad_to_steps(w, jc.full_steps));
		tmc5160_move(rad_to_steps(w_acc, jc.full_steps));
	}
	else if(fabs(w) < 0.0001)
	{
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
    	tmc5160_velocity(rad_to_steps(20000, jc.full_steps));
    	tmc5160_position(rad_to_steps(pos, jc.full_steps));
	}
	else
	{
		tmc5160_move(rad_to_steps(w, jc.full_steps));
	}
};

void drive::move(float speed) {
			int32_t val_speed = 0;
			tmc5160_velocity(rad_to_steps(speed, jc.full_steps));
			tmc5160_move(val_speed);
};

float drive::get_position()
{
	return steps_to_rads(tmc5160_position_read(), jc.full_steps);
};

float drive::get_velocity()
{
	return steps_to_rads(tmc5160_velocity_read(), jc.full_steps);
};



void drive::move_to_encoder_position(uint16_t encoder_tics_to_go)
{
		int8_t Kp = 100;
		uint32_t epsilon = 5;
		uint32_t deviation = 0;
		deviation = encoder_tics_to_go - this->e.get_ticks();
		while(deviation > epsilon)
		{
			deviation = jc.direction * (encoder_tics_to_go - this->e.get_ticks());
			tmc5160_move(deviation * Kp);
			HAL_Delay(1);
		}
		tmc5160_stop();
};



void drive::calibrate_encoder()
{
this->move_to_encoder_position(jc.lower_limit_ticks);


//for(int i=0; i < encoders_regions_numbers; i++)
//{
//
//}
};


}
