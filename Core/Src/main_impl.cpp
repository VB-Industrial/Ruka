#include "main.h"
#include "ruka_joints.h"



#include <memory>
#include <cstdio>


#include "cyphal/cyphal.h"
#include "cyphal/providers/G4CAN.h"
#include "cyphal/allocators/sys/sys_allocator.h"
#include "cyphal/subscriptions/subscription.h"

#include "uavcan/node/Heartbeat_1_0.h"
#include "uavcan/primitive/scalar/Integer32_1_0.h"
#include "reg/udral/physics/kinematics/rotation/Planar_0_1.h"
#include "reg/udral/physics/kinematics/cartesian/Twist_0_1.h"
#include "reg/udral/physics/kinematics/cartesian/State_0_1.h"

#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>

#include <uavcan/node/GetInfo_1_0.h>


//#include "tmc5160.h"



extern "C" {
#include "utility.h"
#include "mainimpl.h"//TMC5160.h HERE

extern motor_config mc;
extern joint_config jc;

TYPE_ALIAS(HBeat, uavcan_node_Heartbeat_1_0)
TYPE_ALIAS(JS_msg, reg_udral_physics_kinematics_rotation_Planar_0_1)
TYPE_ALIAS(State, reg_udral_physics_kinematics_cartesian_State_0_1)


TYPE_ALIAS(RegisterListRequest, uavcan_register_List_Request_1_0)
TYPE_ALIAS(RegisterListResponse, uavcan_register_List_Response_1_0)

TYPE_ALIAS(RegisterAccessRequest, uavcan_register_Access_Request_1_0)
TYPE_ALIAS(RegisterAccessResponse, uavcan_register_Access_Response_1_0)

TYPE_ALIAS(NodeInfoRequest, uavcan_node_GetInfo_Request_1_0)
TYPE_ALIAS(NodeInfoResponse, uavcan_node_GetInfo_Response_1_0)

static float pos_in = 0;
static float vel_in = 0;
static float eff_in = 0;

std::byte buffer[sizeof(CyphalInterface) + sizeof(G4CAN) + sizeof(SystemAllocator)];
std::shared_ptr<CyphalInterface> interface;


void error_handler() { Error_Handler(); }
// Тут не нужен точный таймер, поэтому так
uint64_t micros_64() { return HAL_GetTick() * 1000; }
UtilityConfig utilities(micros_64, error_handler);

class HBeatReader: public AbstractSubscription<HBeat> {
public:
    HBeatReader(InterfacePtr interface): AbstractSubscription<HBeat>(interface,
        // Тут параметры - port_id, transfer kind или только port_id
        uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_
    ) {};
    void handler(const uavcan_node_Heartbeat_1_0& hbeat, CanardRxTransfer* transfer) override {}
};


HBeatReader* h_reader;

class JSReader: public AbstractSubscription<JS_msg> {
public:
	JSReader(InterfacePtr interface): AbstractSubscription<JS_msg>(interface,
        // Тут параметры - port_id, transfer kind или только port_id
		JS_SUB_PORT_ID
    ) {};
    void handler(const reg_udral_physics_kinematics_rotation_Planar_0_1& js_in, CanardRxTransfer* transfer) override
    {
    	if(js_in.angular_velocity.radian_per_second)
    	{
    		tmc5160_move(rad_to_steps(js_in.angular_velocity.radian_per_second, jc.full_steps));
    	}
    	else
    	{
    		js_in.angular_position.radian != steps_to_rads(tmc5160_position_read(), jc.full_steps);
    		tmc5160_position(rad_to_steps(js_in.angular_position.radian, jc.full_steps));
    	}
//    	if (js_in.angular_acceleration.radian_per_second_per_second != eff_in)
//    	{
//    		eff_in = js_in.angular_acceleration.radian_per_second_per_second;
//    		tmc5160_effort(js_in.angular_acceleration.radian_per_second_per_second, &mc);
//    	}
//    	if (js_in.angular_position.radian != pos_in)
//    	{
//    		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
//    		pos_in = js_in.angular_position.radian;
//    		tmc5160_position(rad_to_steps(js_in.angular_position.radian, 2560000)); //jc.full_steps
//    	}
    }
};

JSReader* js_reader;


class RegisterListReader : public AbstractSubscription<RegisterListRequest> {
public:
    RegisterListReader(InterfacePtr interface): AbstractSubscription<RegisterListRequest>(
        interface,
        uavcan_register_List_1_0_FIXED_PORT_ID_,
        CanardTransferKindRequest
    ) {};
    void handler(const RegisterListRequest::Type&, CanardRxTransfer*) override;
};

RegisterListReader* reg_list_service;


class NodeInfoReader : public AbstractSubscription<NodeInfoRequest> {
public:
    NodeInfoReader(InterfacePtr interface): AbstractSubscription<NodeInfoRequest>(
        interface,
        uavcan_node_GetInfo_1_0_FIXED_PORT_ID_,
        CanardTransferKindRequest
    ) {};
    void handler(const NodeInfoRequest::Type&, CanardRxTransfer*) override;
};

NodeInfoReader* nireader;

void NodeInfoReader::handler(
    const uavcan_node_GetInfo_Request_1_0& object,
    CanardRxTransfer* transfer
) {
    static uint8_t node_info_buf[NodeInfoResponse::buffer_size];

    NodeInfoResponse::Type node_info_response = {
        .protocol_version =
            {
                1,
                0,
            },
        .hardware_version =
            {
                1,
                0,
            },
        .software_version = {0, 1},
        .software_vcs_revision_id = 0};
    node_info_response.certificate_of_authenticity.count = 0;
    node_info_response.software_image_crc.count = 0;

    size_t name_len = 8;
    char joint_name[name_len];
    std::sprintf(joint_name,"joint_%d", JOINT_N);
    memcpy(node_info_response.name.elements, joint_name, name_len);
    node_info_response.name.count = name_len;

    uint32_t word0 = 1;
    uint32_t word1 = 2;
    uint32_t word2 = 3;
    memcpy(node_info_response.unique_id, &word0, 4);
    memcpy(node_info_response.unique_id + 4, &word1, 4);
    memcpy(node_info_response.unique_id + 8, &word2, 4);

    node_info_response.unique_id[0] = JOINT_N;

    interface->send_response<NodeInfoResponse>(
        &node_info_response,
        node_info_buf,
        transfer,
        uavcan_node_GetInfo_1_0_FIXED_PORT_ID_
    );
}



class RegisterAccessReader : public AbstractSubscription<RegisterAccessRequest> {
public:
    RegisterAccessReader(InterfacePtr interface): AbstractSubscription<RegisterAccessRequest>(
        interface,
        uavcan_register_Access_1_0_FIXED_PORT_ID_,
        CanardTransferKindRequest
    ) {};
    void handler(const RegisterAccessRequest::Type&, CanardRxTransfer*) override;
};

RegisterAccessReader* reg_access_service;

#define TEST_REG_NAME_LEN 4
#define MOVE_REG_NAME_LEN 4
#define POS_REG_NAME_LEN 3
#define DIR_REG_NAME_LEN 3
#define GET_POS_REG_NAME_LEN 7
#define CALIB_REG_NAME_LEN 5
#define UPPER_LIM_REG_NAME_LEN 9
#define LOWER_LIM_REG_NAME_LEN 9
#define SET_ZERO_REG_NAME_LEN 8
#define SET_ENC_ZERO_REG_NAME_LEN 12
#define NAME_REG_NAME_LEN 4
#define TYPE_REG_NAME_LEN 4

uint8_t test_reg_name[TEST_REG_NAME_LEN + 1] = "test"; //for test purpose TODO REMOVE
uint8_t move_reg_name[MOVE_REG_NAME_LEN + 1] = "move"; //INT32 _tag_ == 9
uint8_t pos_reg_name[POS_REG_NAME_LEN + 1] = "pos"; //INT32 _tag_ == 9
uint8_t dir_reg_name[DIR_REG_NAME_LEN + 1] = "dir"; //UINT8 _tag_ == 11
uint8_t get_pos_reg_name[GET_POS_REG_NAME_LEN + 1] = "get_pos"; //INT32 _tag_ == 9
uint8_t calib_reg_name[CALIB_REG_NAME_LEN + 1] = "calib"; //UINT8 _tag_ == 11
uint8_t upper_lim_reg_name[UPPER_LIM_REG_NAME_LEN + 1] = "upper_lim"; //INT32 _tag_ == 9
uint8_t lower_lim_reg_name[LOWER_LIM_REG_NAME_LEN + 1] = "lower_lim"; //INT32 _tag_ == 9
uint8_t set_enc_zero_reg_name[SET_ENC_ZERO_REG_NAME_LEN + 1] = "set_enc_zero"; //FLOAT64  _tag_ == 12
uint8_t set_zero_reg_name[SET_ZERO_REG_NAME_LEN + 1] = "set_zero"; //UINT8 _tag_ == 11
uint8_t name_reg_name[NAME_REG_NAME_LEN + 1] = "name"; //STRING _tag_ == 1
uint8_t type_reg_name[TYPE_REG_NAME_LEN + 1] = "type"; //UINT8 _tag_ == 11

void RegisterAccessReader::handler(
    const uavcan_register_Access_Request_1_0& register_access_request,
    CanardRxTransfer* transfer
) {
    static uint8_t register_access_response_buf[RegisterAccessResponse::buffer_size];
    RegisterAccessResponse::Type register_access_response = {};

    register_access_response.timestamp.microsecond = micros_64();
    uavcan_register_Value_1_0 value = {};
    uint64_t tv = 0; //PZDC!!!! temp value...
    int32_t js_pos_v = 0;
    if (memcmp(register_access_request.name.name.elements, test_reg_name, TEST_REG_NAME_LEN) == 0)
    {
        if (register_access_request.value._tag_ == 4) {
        	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
//            if (register_access_request.value.integer64.value.elements[0] != 0)
//            {
            	tv = register_access_request.value.integer64.value.elements[0];  //PZDC!!!
            	tmc5160_move(tv);
//            }
//            else
//            {
//            	tv = register_access_request.value.integer64.value.elements[0];  //PZDC!!!
//            	tmc5160_move(0);
//            }
        }

        register_access_response.persistent = true;
        register_access_response._mutable = true;
        value._tag_ = 4;
        uavcan_primitive_array_Integer64_1_0 result = {};
        result.value.elements[0] = tv; //PZDC!!!
        result.value.count = 1;
        value.integer64 = result;
    }
    else if (memcmp(register_access_request.name.name.elements, move_reg_name, MOVE_REG_NAME_LEN) == 0) {
        if (register_access_request.value._tag_ == 9) {
            //MOVE
        	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
        	tv = 0;
        }
        register_access_response.persistent = true;
        register_access_response._mutable = true;
        value._tag_ = 9;
        uavcan_primitive_array_Integer32_1_0 result = {};
        result.value.elements[0] = register_access_request.value._tag_;
        result.value.count = 1;
        value.integer32 = result;
    }
    else if (memcmp(register_access_request.name.name.elements, pos_reg_name, POS_REG_NAME_LEN) == 0) {

        if (register_access_request.value._tag_ == 9) {
            tmc5160_position(register_access_request.value.integer32.value.elements[0]);
        	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
        	js_pos_v = tmc5160_position_read();
        }
        register_access_response.persistent = true;
        register_access_response._mutable = true;
        value._tag_ = 9;
        uavcan_primitive_array_Integer32_1_0 result = {};
        result.value.elements[0] = js_pos_v;
        result.value.count = 1;
        value.integer32 = result;
    }
    else if (memcmp(register_access_request.name.name.elements, get_pos_reg_name, GET_POS_REG_NAME_LEN) == 0) {
        if (register_access_request.value._tag_ == 9) {
        	js_pos_v = tmc5160_position_read();
        	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
        	tv = 0;
        }
        register_access_response.persistent = true;
        register_access_response._mutable = true;
        value._tag_ = 9;
        uavcan_primitive_array_Integer32_1_0 result = {};
        result.value.elements[0] = js_pos_v;
        result.value.count = 1;
        value.integer32 = result;
    }
    else if (memcmp(register_access_request.name.name.elements, dir_reg_name, DIR_REG_NAME_LEN) == 0) {
        if (register_access_request.value._tag_ == 7) {
            tmc5160_set_motor_direction(register_access_request.value.integer8.value.elements[0]);
        	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
        	tv = 0;
        }
        register_access_response.persistent = true;
        register_access_response._mutable = true;
        value._tag_ = 11;
        uavcan_primitive_array_Integer8_1_0 result = {};
        result.value.elements[0] = register_access_request.value.integer8.value.elements[0];
        result.value.count = 1;
        value.integer8 = result;
    	}
    else if (memcmp(register_access_request.name.name.elements, calib_reg_name, CALIB_REG_NAME_LEN) == 0) {
        if (register_access_request.value._tag_ == 11) {
            //ENABLE CALIB
        	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
        	tv = 0;
        }
        register_access_response.persistent = true;
        register_access_response._mutable = true;
        value._tag_ = 11;
        uavcan_primitive_array_Natural8_1_0 result = {};
        result.value.elements[0] = register_access_request.value._tag_;
        result.value.count = 1;
        value.natural8 = result;
    	}
	else if (memcmp(register_access_request.name.name.elements, upper_lim_reg_name, UPPER_LIM_REG_NAME_LEN) == 0) {
		if (register_access_request.value._tag_ == 9) {
			//SET UPPER LIMIT FOR JOINT
			HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
			tv = 0;
		}
		register_access_response.persistent = true;
		register_access_response._mutable = true;
		value._tag_ = 9;
		uavcan_primitive_array_Integer32_1_0 result = {};
		result.value.elements[0] = register_access_request.value._tag_;
		result.value.count = 1;
		value.integer32 = result;
	}
	else if (memcmp(register_access_request.name.name.elements, lower_lim_reg_name, LOWER_LIM_REG_NAME_LEN) == 0) {
		if (register_access_request.value._tag_ == 9) {
			//SET UPPER LIMIT FOR JOINT
			HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
			tv = 0;
		}
		register_access_response.persistent = true;
		register_access_response._mutable = true;
		value._tag_ = 9;
		uavcan_primitive_array_Integer32_1_0 result = {};
		result.value.elements[0] = register_access_request.value._tag_;
		result.value.count = 1;
		value.integer32 = result;
	}
	else if (memcmp(register_access_request.name.name.elements, set_zero_reg_name, SET_ZERO_REG_NAME_LEN) == 0) {
		if (register_access_request.value._tag_ == 12) {
			tmc5160_set_zero();
			HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
			tv = 0;
		}
		register_access_response.persistent = true;
		register_access_response._mutable = true;
		value._tag_ = 12;
		uavcan_primitive_array_Real64_1_0 result = {};
		result.value.elements[0] = register_access_request.value._tag_;
		result.value.count = 1;
		value.real64 = result;
	}
    else if (memcmp(register_access_request.name.name.elements, set_enc_zero_reg_name, SET_ENC_ZERO_REG_NAME_LEN) == 0) {
        if (register_access_request.value._tag_ == 11) {
            //SET TYPE OF MOTOR FOR JOINT
        	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
        	tv = 0;
        }
        register_access_response.persistent = true;
        register_access_response._mutable = true;
        value._tag_ = 11;
        uavcan_primitive_array_Natural8_1_0 result = {};
        result.value.elements[0] = register_access_request.value._tag_;
        result.value.count = 1;
        value.natural8 = result;
    }
    else if (memcmp(register_access_request.name.name.elements, name_reg_name, NAME_REG_NAME_LEN) == 0) {
        if (register_access_request.value._tag_ == 1) {
            //SET NAME FOR JOINT
        	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
        	tv = 0;
        }
        register_access_response.persistent = true;
        register_access_response._mutable = true;
        value._tag_ = 1;
        uavcan_primitive_String_1_0 result = {};

        char mystring[4] = "xyz";
        memcpy(result.value.elements, mystring, 4);
        result.value.count = 3;
        value._string = result;
    }
    else if (memcmp(register_access_request.name.name.elements, type_reg_name, TYPE_REG_NAME_LEN) == 0) {
        if (register_access_request.value._tag_ == 11) {
            //SET TYPE OF MOTOR FOR JOINT
        	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
        	tv = 0;
        }
        register_access_response.persistent = true;
        register_access_response._mutable = true;
        value._tag_ = 11;
        uavcan_primitive_array_Natural8_1_0 result = {};
        result.value.elements[0] = register_access_request.value._tag_;
        result.value.count = 1;
        value.natural8 = result;
    }
    else {
        value._tag_ = 0;
        value.empty = (uavcan_primitive_Empty_1_0){};
    }
    register_access_response.value = value;

    interface->send_response<RegisterAccessResponse>(
        &register_access_response,
        register_access_response_buf,
        transfer,
        uavcan_register_Access_1_0_FIXED_PORT_ID_
    );
}

//TODO reg list

//void RegisterListReader::handler(
//    const uavcan_register_List_Request_1_0& register_list_request,
//    CanardRxTransfer* transfer
//) {
//    static uint8_t register_list_response_buf[RegisterListResponse::buffer_size];
//    RegisterListResponse::Type register_list_response = {};
//
//    uavcan_register_Name_1_0 name = {};
//    uint8_t* reg_name = nullptr;
//    size_t reg_name_len = 0;
//    switch (register_list_request.index) {
//        case 0:
//            reg_name = test_reg_name;
//            reg_name_len = TEST_REG_NAME_LEN;
//            break;
//        case 1:
//            reg_name = motor_speed_reg_name;
//            reg_name_len = MOTOR_SPEED_REG_NAME_LEN;
//            break;
//        case 2:
//            reg_name = motor_current_lim_reg_name;
//            reg_name_len = MOTOR_CURRENT_LIM_REG_NAME_LEN;
//            break;
//    }
//    if (reg_name != nullptr) {
//        memcpy(name.name.elements, reg_name, reg_name_len);
//        name.name.count = reg_name_len;
//    }
//    register_list_response.name = name;
//
//    interface->send_response<RegisterListResponse>(
//        &register_list_response,
//        register_list_response_buf,
//        transfer,
//        uavcan_register_List_1_0_FIXED_PORT_ID_
//    );
//}




void send_JS(joint_config * jc) {             //float* pos, float* vel, float* eff
	static uint8_t js_buffer[JS_msg::buffer_size];
	static CanardTransferID int_transfer_id = 0;
	reg_udral_physics_kinematics_rotation_Planar_0_1 js_msg =
	{
			.angular_position = steps_to_rads(tmc5160_position_read(), jc->full_steps),
			.angular_velocity = steps_to_rads(tmc5160_velocity_read(), jc->full_steps),
			.angular_acceleration = eff_in
	};
    interface->send_msg<JS_msg>(
		&js_msg,
		js_buffer,
		AGENT_JS_SUB_PORT,
		&int_transfer_id
	);
}

void send_IMU(float* qw, float* qx, float* qy, float* qz, float* ax, float* ay, float* az, float* gx, float* gy, float* gz)
{
	static uint8_t state_buffer[State::buffer_size];
	static CanardTransferID int_transfer_id = 0;

	//HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);

	//uavcan_si_unit_angle_Quaternion_1_0 q_orient = {*av_1, *av_2, *av_3, *av_3};
	reg_udral_physics_kinematics_cartesian_Pose_0_1 imu_pose;
	imu_pose.orientation = {*qw, *qx, *qy, *qz};
	//imu_pose.position =

	reg_udral_physics_kinematics_cartesian_Twist_0_1 imu_twist;
	imu_twist.angular = {*ax, *ay, *az};
	imu_twist.linear = {*gx, *gy, *gz};

	reg_udral_physics_kinematics_cartesian_State_0_1 state_msg =
	{
			.pose = imu_pose,
			.twist = imu_twist
	};
    interface->send_msg<State>(
		&state_msg,
		state_buffer,
		AGENT_IMU_PORT,
		&int_transfer_id
	);
}

void heartbeat() {
	static uint8_t hbeat_buffer[HBeat::buffer_size];
	static CanardTransferID hbeat_transfer_id = 0;
	static uint32_t uptime = 0;
    uavcan_node_Heartbeat_1_0 heartbeat_msg = {
        .uptime = uptime,
        .health = {uavcan_node_Health_1_0_NOMINAL},
        .mode = {uavcan_node_Mode_1_0_OPERATIONAL}
    };
    interface->send_msg<HBeat>(
		&heartbeat_msg,
		hbeat_buffer,
		uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
		&hbeat_transfer_id
	);
    uptime += 1;
}

void setup_cyphal(FDCAN_HandleTypeDef* handler) {
	interface = std::shared_ptr<CyphalInterface>(
		         // memory location, node_id, fdcan handler, messages memory pool, utils ref
		CyphalInterface::create_bss<G4CAN, SystemAllocator>(buffer, JOINT_N, handler, 400, utilities)
	);
	h_reader = new HBeatReader(interface);
	js_reader = new JSReader(interface);
	reg_access_service = new RegisterAccessReader(interface);
	nireader = new NodeInfoReader(interface);
}

void cyphal_loop() {
    interface->loop();
}

void cyphal_can_starter(FDCAN_HandleTypeDef* hfdcan)
{

	CanardFilter cyphal_filter_for_node_id = canardMakeFilterForServices(JOINT_N);
	CanardFilter cyphal_filter_for_JS = canardMakeFilterForSubject(JS_SUB_PORT_ID);//
	CanardFilter cyphal_filter_for_HB = canardMakeFilterForSubject(7509);//JS_SUB_PORT_ID
	CanardFilter cyphal_filter_consolidated = canardConsolidateFilters(&cyphal_filter_for_node_id, &cyphal_filter_for_JS);

	static FDCAN_FilterTypeDef sFilterConfig;
	static FDCAN_FilterTypeDef hbFilterConfig;
	static FDCAN_FilterTypeDef niFilterConfig;

	niFilterConfig.IdType = FDCAN_EXTENDED_ID;
	niFilterConfig.FilterIndex = 0;
	niFilterConfig.FilterType = FDCAN_FILTER_MASK;
	niFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	niFilterConfig.FilterID1 =  cyphal_filter_for_node_id.extended_can_id;
	niFilterConfig.FilterID2 =  cyphal_filter_for_node_id.extended_mask;

	sFilterConfig.IdType = FDCAN_EXTENDED_ID;
	sFilterConfig.FilterIndex = 1;
	sFilterConfig.FilterType = FDCAN_FILTER_MASK;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 =  cyphal_filter_for_JS.extended_can_id;
	sFilterConfig.FilterID2 =  cyphal_filter_for_JS.extended_mask;

	hbFilterConfig.IdType = FDCAN_EXTENDED_ID;
	hbFilterConfig.FilterIndex = 2;
	hbFilterConfig.FilterType = FDCAN_FILTER_MASK;
	hbFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	hbFilterConfig.FilterID1 =  cyphal_filter_for_HB.extended_can_id;
	hbFilterConfig.FilterID2 =  cyphal_filter_for_HB.extended_mask;



	if (HAL_FDCAN_ConfigGlobalFilter(hfdcan, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT) != HAL_OK)
	{
	  Error_Handler();
	}
	if (HAL_FDCAN_ConfigFilter(hfdcan, &niFilterConfig) != HAL_OK) {
	  Error_Handler();
	}
	if (HAL_FDCAN_ConfigFilter(hfdcan, &sFilterConfig) != HAL_OK) {
	  Error_Handler();
	}
	if (HAL_FDCAN_ConfigFilter(hfdcan, &hbFilterConfig) != HAL_OK) {
	  Error_Handler();
	}

	if (HAL_FDCAN_ConfigTxDelayCompensation(hfdcan, 5, 0) != HAL_OK) {
	  Error_Handler();
	}
	if (HAL_FDCAN_EnableTxDelayCompensation(hfdcan) != HAL_OK) {
	  Error_Handler();
	}
//	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
//	{
//	  Error_Handler();
//	}

	HAL_FDCAN_Start(hfdcan);
}




}

