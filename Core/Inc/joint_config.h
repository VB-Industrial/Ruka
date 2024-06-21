/*
 * joint_config.h
 *
 *  Created on: Nov 22, 2023
 *      Author: VR
 */
#pragma once
#ifndef INC_JOINT_CONFIG_H_
#define INC_JOINT_CONFIG_H_

#include "tmc5160.h"


#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct joint_config
{
	int8_t first_run;
	int8_t direction;
	uint32_t full_steps;
	float joint_gear_ratio;
	float motor_gear_ratio;
	int32_t upper_limit_ticks;
	int32_t lower_limit_ticks;
	float upper_limit_enc;
	float lower_limit_enc;
	uint32_t zero_enc;
};
typedef struct joint_config joint_config;


struct joint_config_address
{

	uint16_t address_of_upper_limit_enc;
	uint16_t address_of_lower_limit_enc;
	uint16_t address_of_zero_enc;
};
typedef struct joint_config_address joint_config_address;


void joint_config_init(joint_config * jc);
void motor_config_assembler(motor_config * mc, joint_config *jc);
void joint_config_assembler(joint_config * jconf, joint_config_address * jc_a);
void joint_number_selector(joint_config * jc);
void domain_id_selector(joint_config * jc);
void joint_config_write(joint_config * jc, joint_config_address * jc_a);
void joint_config_read(joint_config * jc, joint_config_address * jc_a);

#endif /* INC_JOINT_CONFIG_H_ */
