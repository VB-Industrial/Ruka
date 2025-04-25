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

extern joint_config jc;
extern joint_config_address jc_address;

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

void motor_config_assembler(motor_config * mc, joint_config * jc)
{
	if (JOINT_N == 1)
	{
		mc->motor_type = 17;
	}
	else if (JOINT_N == 2)
	{
		mc->motor_type = 23;
	}
	else if (JOINT_N == 3)
	{
		mc->motor_type = 17;
	}
	else if (JOINT_N == 4)
	{
		mc->motor_type = 14;
	}
	else if (JOINT_N == 5)
	{
		mc->motor_type = 14;
	}
	else if (JOINT_N == 6)
	{
		mc->motor_type = 14;
	}

	switch(mc->motor_type)
	{
	case 14:
		mc->max_irun_scaler = 8;
		mc->max_effort = 0.5;
		mc->init_irun = 4;
		break;
	case 17:
		mc->max_irun_scaler = 12;
		mc->max_effort = 3.9;
		mc->init_irun = 8;
		break;
	case 23:
		mc->max_irun_scaler = 31;
		mc->max_effort = 10.2;
		mc->init_irun = 18;
		break;
	}
	mc->direction = jc->direction;
}


void joint_config_assembler(joint_config * jconf, joint_config_address * jc_a)
{
	if (JOINT_N == 1)
	{
		jconf->motor_gear_ratio = 50;
		jconf->joint_gear_ratio = 3;
		jconf->full_steps = (uint32_t)(256 * 200* jconf->motor_gear_ratio * jconf->joint_gear_ratio);
		jconf->direction = -1;
		jconf->lower_limit_ticks = 0;
		jconf->upper_limit_ticks = 0;
	}
	else if (JOINT_N == 2)
	{
		jconf->motor_gear_ratio = 50;
		jconf->joint_gear_ratio = 2;
		jconf->full_steps = (uint32_t)(256 * 200* jconf->motor_gear_ratio * jconf->joint_gear_ratio);
		jconf->direction = 1;
		jconf->lower_limit_ticks = 0;
		jconf->upper_limit_ticks = 0;
	}
	else if (JOINT_N == 3)
	{
		jconf->motor_gear_ratio = 50;
		jconf->joint_gear_ratio = 2;
		jconf->full_steps = (uint32_t)(256 * 200* jconf->motor_gear_ratio * jconf->joint_gear_ratio);
		jconf->direction = 1;
		jconf->lower_limit_ticks = 0;
		jconf->upper_limit_ticks = 0;
	}
	else if (JOINT_N == 4)
	{
		jconf->motor_gear_ratio = 19.203208;
		jconf->joint_gear_ratio = 2.5;
		jconf->full_steps = (uint32_t)(256 * 200* jconf->motor_gear_ratio * jconf->joint_gear_ratio);
		jconf->direction = 1;
		jconf->lower_limit_ticks = 0;
		jconf->upper_limit_ticks = 0;
	}
	else if (JOINT_N == 5)
	{
		jconf->motor_gear_ratio = 19.203208;
		jconf->joint_gear_ratio = 2.5;
		jconf->full_steps = (uint32_t)(256 * 200* jconf->motor_gear_ratio * jconf->joint_gear_ratio);
		jconf->direction = 1;
		jconf->lower_limit_ticks = 0;
		jconf->upper_limit_ticks = 0;
	}
	else if (JOINT_N == 6)
	{
		jconf->motor_gear_ratio = 19.203208;
		jconf->joint_gear_ratio = 1;
		jconf->full_steps = (uint32_t)(256 * 200* jconf->motor_gear_ratio * jconf->joint_gear_ratio);
		jconf->direction = -1;
		jconf->lower_limit_ticks = 0;
		jconf->upper_limit_ticks = 0;
	}

	//TODO!!!!!!!!!!!!!!!!!!!!!!!!! from config read
	jconf->lower_limit_enc = 0.0;
	jconf->upper_limit_enc = 0.0;
	jconf->zero_enc = 0;
	//TODO!!!!!!!!!!!!!!!!!!!!!!!!!

	jc_a->address_of_upper_limit_enc = 0x00;
	jc_a->address_of_lower_limit_enc = jc_a->address_of_upper_limit_enc + sizeof(jconf->upper_limit_enc);
	jc_a->address_of_zero_enc = jc_a->address_of_lower_limit_enc + sizeof(jconf->lower_limit_enc);

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





void joint_config_write(joint_config * jc, joint_config_address * jc_a)
{
	int timeout = 100;
	if (at24_isConnected())
	{
		at24_write(&jc_a->address_of_upper_limit_enc, &jc->upper_limit_enc, sizeof(&jc->upper_limit_enc), timeout);
		os_delay(1);
		at24_write(&jc_a->address_of_lower_limit_enc, &jc->lower_limit_enc, sizeof(&jc->lower_limit_enc), timeout);
		os_delay(1);
		at24_write(&jc_a->address_of_zero_enc, &jc->zero_enc, sizeof(&jc->zero_enc), timeout);
		os_delay(1);
	}
}

void joint_config_read(joint_config * jc, joint_config_address * jc_a)
{
	int timeout = 100;
	if (at24_isConnected())
		{
		at24_read(&jc_a->address_of_upper_limit_enc, &jc->upper_limit_enc, sizeof(&jc->upper_limit_enc), timeout);
		os_delay(1);
		at24_read(&jc_a->address_of_lower_limit_enc, &jc->lower_limit_enc, sizeof(&jc->lower_limit_enc), timeout);
		os_delay(1);
		at24_read(&jc_a->address_of_zero_enc, &jc->zero_enc, sizeof(&jc->zero_enc), timeout);
		os_delay(1);
		}
}

