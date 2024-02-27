/*
 * joint_config.c
 *
 *  Created on: Dec 15, 2023
 *      Author: Sorokonog
 */
#ifdef USE_FREERTOS
#include "cmsis_os.h"
#define os_delay(x) vTaskDelay(x/ portTICK_PERIOD_MS)
#endif
#define os_delay(x) HAL_Delay(x)
#include "joint_config.h"
#include "at24_eeprom.h"
#include "main.h"
#include "tmc5160.h"

void joint_config_init(joint_config * jconf)
{
#ifdef NODIPSWITCH
	jconf->domain_id = DOMAINID;
	jconf->joint_number = JOINTN;
#else
	  joint_number_selector(jconf);
	  domain_id_selector(jconf);
#endif
}

void motor_config_assembler(motor_config * mc)
{
	if (JOINT_N == 1)
	{
		mc->motor_type = 17;
		mc->direction = 1;
	}
	else if (JOINT_N == 2)
	{
		mc->motor_type = 23;
		mc->direction = 1;
	}
	else if (JOINT_N == 3)
	{
		mc->motor_type = 17;
		mc->direction = 1;
	}
	else if (JOINT_N == 4)
	{
		mc->motor_type = 17;
		mc->direction = 1;
	}
	else if (JOINT_N == 5)
	{
		mc->motor_type = 14;
		mc->direction = 1;
	}
	else if (JOINT_N == 6)
	{
		mc->motor_type = 14;
		mc->direction = 1;
	}

	switch(mc->motor_type)
	{
	case 14:
		mc->max_irun_scaler = 10;
		mc->max_effort = 0.5;
		mc->init_irun = 1;
	case 17:
		mc->max_irun_scaler = 12;
		mc->max_effort = 3.9;
		mc->init_irun = 1;
	case 23:
		mc->max_irun_scaler = 31;
		mc->max_effort = 10.2;
		mc->init_irun = 3;
	}
}


void joint_config_assembler(joint_config * jconf)
{
	if (JOINT_N == 1)
	{
		jconf->full_steps = 2560000;
		jconf->gear_ratio = 50;
	}
	else if (JOINT_N == 2)
	{
		jconf->full_steps = 2560000;
		jconf->gear_ratio = 50;
	}
	else if (JOINT_N == 3)
	{
		jconf->full_steps = 2560000;
		jconf->gear_ratio = 50;
	}
	else if (JOINT_N == 4)
	{
		jconf->full_steps = 2560000;
		jconf->gear_ratio = 50;
	}
	else if (JOINT_N == 5)
	{
		jconf->full_steps = 983204;
		jconf->gear_ratio = 19;
	}
	else if (JOINT_N == 6)
	{
		jconf->full_steps = 983204;
		jconf->gear_ratio = 19;
	}

	jconf->domain_id = 0;
	jconf->full_steps = 0;
	jconf->gear_ratio = 0.0;
	jconf->lower_limit_enc = 0.0;
	jconf->lower_limit_ticks = 0;
	jconf->motor_type = 0;
	jconf->upper_limit_enc = 0.0;
	jconf->upper_limit_ticks = 0;
	jconf->zero_enc = 0.0;
	jconf->upper_limit_effort = 0.0;
	jconf->direction = 1;



//#define NEMA14_FULLSTEPS    983204
//#define NEMA17_FULLSTEPS	2560000
//#define NEMA23_FULLSTEPS    2560000
//
//
//
//#define NEMA14_GR 19 //TODO possible to correct ratio to 19.38/187 for more precise velocity calculation
//#define NEMA17_GR 50
//#define NEMA23_GR 50
}



//void joint_number_selector(joint_config * jc)
//{
//	if(HAL_GPIO_ReadPin(DIP_1_GPIO_Port, DIP_1_Pin))
//	{
//		jc->joint_number = 1;
//	}
//	else if(HAL_GPIO_ReadPin(DIP_2_GPIO_Port, DIP_2_Pin))
//	{
//		jc->joint_number = 2;
//	}
//	else if (HAL_GPIO_ReadPin(DIP_3_GPIO_Port, DIP_3_Pin))
//	{
//		jc->joint_number = 3;
//	}
//	else if (HAL_GPIO_ReadPin(DIP_4_GPIO_Port, DIP_4_Pin))
//	{
//		jc->joint_number = 4;
//	}
//	else if (HAL_GPIO_ReadPin(DIP_5_GPIO_Port, DIP_5_Pin))
//	{
//		jc->joint_number = 5;
//	}
//	else if (HAL_GPIO_ReadPin(DIP_6_GPIO_Port, DIP_6_Pin))
//	{
//		jc->joint_number = 6;
//	}
//	else
//	{
//		jc->joint_number = 0;
//	}
//}

//void domain_id_selector(joint_config * jc)
//{
//	if(HAL_GPIO_ReadPin(DIP_8_GPIO_Port, DIP_8_Pin))
//	{
//		jc->domain_id = 0;
//	}
//}

void joint_config_write(joint_config * jc)
{
	uint16_t address_of_domain_id = 0x00;
	uint16_t address_of_upper_limit_enc = sizeof(&jc->domain_id);
	uint16_t address_of_lower_limit_enc = address_of_upper_limit_enc + sizeof(&jc->upper_limit_enc);
	uint16_t address_of_upper_limit_ticks = address_of_lower_limit_enc + sizeof(&jc->lower_limit_enc);
	uint16_t address_of_lower_limit_ticks = address_of_upper_limit_ticks + sizeof(&jc->upper_limit_ticks);
	uint16_t address_of_zero_enc = address_of_lower_limit_ticks + sizeof(&jc->lower_limit_ticks);
	uint16_t address_of_joint_number = address_of_zero_enc + sizeof(&jc->zero_enc);
	uint16_t address_of_motor_type = address_of_joint_number + sizeof(&jc->joint_number);
	uint16_t address_of_full_steps = address_of_motor_type + sizeof(&jc->motor_type);
	uint16_t address_of_gear_ratio = address_of_full_steps + sizeof(&jc->full_steps);
	uint16_t address_of_upper_limit_effort = address_of_gear_ratio + sizeof(&jc->gear_ratio);
	uint16_t address_of_direction = address_of_upper_limit_effort + sizeof(&jc->upper_limit_effort);

	int timeout = 100;
	if (at24_isConnected())
	{

		at24_write(address_of_domain_id, &jc->domain_id, sizeof(&jc->domain_id), timeout);
		os_delay(1);
		at24_write(address_of_upper_limit_enc, &jc->upper_limit_enc, sizeof(&jc->upper_limit_enc), timeout);
		os_delay(1);
		at24_write(address_of_lower_limit_enc, &jc->lower_limit_enc, sizeof(&jc->lower_limit_enc), timeout);
		os_delay(1);
		at24_write(address_of_upper_limit_ticks, &jc->upper_limit_ticks, sizeof(&jc->upper_limit_ticks), timeout);
		os_delay(1);
		at24_write(address_of_lower_limit_ticks, &jc->lower_limit_ticks, sizeof(&jc->lower_limit_ticks), timeout);
		os_delay(1);
		at24_write(address_of_zero_enc, &jc->zero_enc, sizeof(&jc->zero_enc), timeout);
		os_delay(1);
		at24_write(address_of_joint_number, &jc->joint_number, sizeof(&jc->joint_number), timeout);
		os_delay(1);
		at24_write(address_of_motor_type, &jc->motor_type, sizeof(&jc->motor_type), timeout);
		os_delay(1);
		at24_write(address_of_full_steps, &jc->full_steps, sizeof(&jc->full_steps), timeout);
		os_delay(1);
		at24_write(address_of_gear_ratio, &jc->gear_ratio, sizeof(&jc->gear_ratio), timeout);
		os_delay(1);
		at24_write(address_of_upper_limit_effort, &jc->upper_limit_effort, sizeof(&jc->upper_limit_effort), timeout);
		os_delay(1);
		at24_write(address_of_direction, &jc->direction, sizeof(&jc->direction), timeout);
		os_delay(1);

	}
}

void joint_config_read(joint_config * jc)
{
	uint16_t address_of_domain_id = 0x00;
	uint16_t address_of_upper_limit_enc = sizeof(&jc->domain_id);
	uint16_t address_of_lower_limit_enc = address_of_upper_limit_enc + sizeof(&jc->upper_limit_enc);
	uint16_t address_of_upper_limit_ticks = address_of_lower_limit_enc + sizeof(&jc->lower_limit_enc);
	uint16_t address_of_lower_limit_ticks = address_of_upper_limit_ticks + sizeof(&jc->upper_limit_ticks);
	uint16_t address_of_zero_enc = address_of_lower_limit_ticks + sizeof(&jc->lower_limit_ticks);
	uint16_t address_of_joint_number = address_of_zero_enc + sizeof(&jc->zero_enc);
	uint16_t address_of_motor_type = address_of_joint_number + sizeof(&jc->joint_number);
	uint16_t address_of_full_steps = address_of_motor_type + sizeof(&jc->motor_type);
	uint16_t address_of_gear_ratio = address_of_full_steps + sizeof(&jc->full_steps);
	uint16_t address_of_upper_limit_effort = address_of_gear_ratio + sizeof(&jc->gear_ratio);
	uint16_t address_of_direction = address_of_upper_limit_effort + sizeof(&jc->upper_limit_effort);

	int timeout = 100;
	if (at24_isConnected())
		{
		at24_read(address_of_domain_id, &jc->domain_id, sizeof(&jc->domain_id), timeout);
		os_delay(1);
		at24_read(address_of_upper_limit_enc, &jc->upper_limit_enc, sizeof(&jc->upper_limit_enc), timeout);
		os_delay(1);
		at24_read(address_of_lower_limit_enc, &jc->lower_limit_enc, sizeof(&jc->lower_limit_enc), timeout);
		os_delay(1);
		at24_read(address_of_upper_limit_ticks, &jc->upper_limit_ticks, sizeof(&jc->upper_limit_ticks), timeout);
		os_delay(1);
		at24_read(address_of_lower_limit_ticks, &jc->lower_limit_ticks, sizeof(&jc->lower_limit_ticks), timeout);
		os_delay(1);
		at24_read(address_of_zero_enc, &jc->zero_enc, sizeof(&jc->zero_enc), timeout);
		os_delay(1);
		//at24_read(address_of_joint_number, &jc->joint_number, sizeof(&jc->joint_number), timeout);
		//vTaskDelay(1000/ portTICK_PERIOD_MS);
		at24_read(address_of_motor_type, &jc->motor_type, sizeof(&jc->motor_type), timeout);
		os_delay(1);
		at24_read(address_of_full_steps, &jc->full_steps, sizeof(&jc->full_steps), timeout);
		os_delay(1);
		at24_read(address_of_gear_ratio, &jc->gear_ratio, sizeof(&jc->gear_ratio), timeout);
		os_delay(1);
		at24_read(address_of_upper_limit_effort, &jc->upper_limit_effort, sizeof(&jc->upper_limit_effort), timeout);
		os_delay(1);
		at24_read(address_of_direction, &jc->direction, sizeof(&jc->direction), timeout);
		os_delay(1);
		}
}

