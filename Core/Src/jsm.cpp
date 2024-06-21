/*
 * jsm.cpp
 *
 *  Created on: Jun 21, 2024
 *      Author: VR
 */
extern "C" {
#include <jsm.h>

extern joint_state js;

void js_init(){
	js.fail = 0;
	js.move = 0;
	js.pos = 0;
	js.stop = 0;
	js.in_lower_limit = 0;
	js.in_upper_limit = 0;
}
}
