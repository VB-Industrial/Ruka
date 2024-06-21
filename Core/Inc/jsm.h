/*
 * jsm.h
 *
 *  Created on: Jun 21, 2024
 *      Author: VR
 */

#ifndef INC_JSM_H_
#define INC_JSM_H_

#include <stdint.h>

struct joint_state
{
	int8_t stop;
	int8_t pos;
	int8_t move;
	int8_t fail;
	int8_t in_upper_limit;
	int8_t in_lower_limit;
};
typedef struct joint_state joint_state;

void js_init();

#endif /* INC_JSM_H_ */
