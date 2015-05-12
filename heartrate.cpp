#include "i2c-ctrl.h"
#include <pixart_8001_1000.h>
#include "led_ctrl.h"
#include "sys/time.h"
extern "C" {
#include "pxialg.h"
}

#define I2C_DEV "/dev/i2c-2"
#define HRM_ADDR 0x33
#define INERT_ADDR 0x68
#define FIFO_SIZE 8
#define FIFO_SIZE_M1 (FIFO_SIZE-1)

typedef unsigned char u8;
typedef unsigned int u32;

struct timeval tv;

Userspace_i2c i2c_hrm((char*)I2C_DEV, HRM_ADDR);
Userspace_i2c i2c_iner((char*)I2C_DEV, INERT_ADDR);

typedef struct {
        u8 HRD_Data[13] ;
        float MEMS_Data[3] ;
}ppg_mems_data_t;

static ppg_mems_data_t _ppg_mems_data[FIFO_SIZE] ;
static ppg_mems_data_t test_ppg_mems_data;

static void bank_select(bank_e bank);

void pah8001_power_down(u8 yes)
{
	u8 data = 0 ;
	//Power Down
	//Bank0
	i2c_hrm.reg_write(0x7f, 0);
	//ADDR6, Bit3 = 1
	i2c_hrm.reg_read(0x06, &data);
	if(!!yes)
	        data |= 0x08 ;
	else
	        data &= (~0x08);
	i2c_hrm.reg_write(0x06, data);
}

static void bank_select(bank_e bank)
{
	unsigned char command;
	switch(bank) {
		case BANK0:
			command = OFN_BANK0;
			break;
		case BANK1:
			command = OFN_BANK1;
			break;
	}
	i2c_hrm.reg_write(OFN_REGITER_BANK_SEL, command);
}

int pixart_init()
{
	unsigned char command[10];
	unsigned char id[2];
	int i=0;
	int bank = 0;
	int ret = -1;
	u8 data = 0 ;

	bank_select(BANK0);
	ret = i2c_hrm.reg_read(0x00, &data);
	if(ret) {
		printf("init failed\r\n");
		return ret;
	}
	else
		id[0] = data;

	ret = i2c_hrm.reg_read(0x01, &data);
	if(ret) {
		printf("init failed\r\n");
		return ret;
	}
	else
		id[1] = data;

	for (i = 0; (unsigned int)i < INIT_PPG_REG_ARRAY_SIZE_1000;i++) {
	        if(init_ppg_register_array_1000[i][0] == 0x7F)
	                bank = init_ppg_register_array_1000[i][1];

	        if((bank == 0) && (init_ppg_register_array_1000[i][0] == 0x17) )
	        {
	                //read and write bit7=1
	                ret = i2c_hrm.reg_read(0x17, &data);
	                if(ret == 0)
	                {
	                        data |= 0x80 ;
	                        ret = i2c_hrm.reg_write(0x17,data);
	                }
	        }
	        else
	        {
	                ret = i2c_hrm.reg_write(init_ppg_register_array_1000[i][0],init_ppg_register_array_1000[i][1]);
	        }
	        if(ret)
	                break;
	}
	pah8001_power_down(1);

	return ret;
}

static u8 Frame_Count = 0 ;
static void ofn_ppg(void)
{
	u8 touch_flag = 0 ;
	u8 data;
	u32 mtime_p = 0;
	u32 mtime_n = 0;
	int _write_index = 0; //use the fix buffer
	int ret = 0;
	float myHR = 0.0;
	unsigned char ready_flag = 0;
	unsigned char motion_flag = 0;

	if(1)
	{
		bank_select(BANK0);
		i2c_hrm.reg_read(0x59, &touch_flag);

		touch_flag &= 0x80  ;
		led_ctrl(touch_flag);

		bank_select(BANK1);
		i2c_hrm.reg_read(0x68, &data);
		test_ppg_mems_data.HRD_Data[0] = data & 0x0f ;

		if(test_ppg_mems_data.HRD_Data[0] == 0)
		{
			printf("no touch");
			bank_select(BANK0);
		}
		else
		{
			printf("touched");
			int tmp = 0;
			i2c_hrm.reg_burst_read(0x64, &(test_ppg_mems_data.HRD_Data[1]), 4);
			i2c_hrm.reg_burst_read(0x1A, &(test_ppg_mems_data.HRD_Data[5]), 3);
			test_ppg_mems_data.HRD_Data[8] = Frame_Count++;
			gettimeofday(&tv, NULL);
			mtime_n = 1000 * tv.tv_sec + tv.tv_usec/1000;
			test_ppg_mems_data.HRD_Data[9] = mtime_n - mtime_p;
			mtime_p = mtime_n;
			test_ppg_mems_data.HRD_Data[10] = get_led_current_change_flag();

			test_ppg_mems_data.HRD_Data[11] = touch_flag ;
			test_ppg_mems_data.HRD_Data[12] = test_ppg_mems_data.HRD_Data[6];
		}
	}
	test_ppg_mems_data.MEMS_Data[0] = 0;
	test_ppg_mems_data.MEMS_Data[1] = 0;
	test_ppg_mems_data.MEMS_Data[2] = 0;
	ret = PxiAlg_Process(test_ppg_mems_data.HRD_Data, test_ppg_mems_data.MEMS_Data);
	if(ret==FLAG_DATA_READY)
	{
		PxiAlg_HrGet(&myHR);
		ready_flag = PxiAlg_GetReadyFlag();
		motion_flag = PxiAlg_GetMotionFlag();
	}
	if(myHR!=0.0)
		printf(" -- %d||HR rt is %f\r\n", mtime_n, myHR);
	else
		printf("\r\n");
}

static void pixart_work()
{
	pah8001_power_down(0);

	PxiAlg_SetMemsScale(0);
	PxiAlg_EnableFastOutput(0);
	PxiAlg_EnableAutoMode(1);
	PxiAlg_EnableMotionMode(0);
	while(1)
	{
		ofn_ppg();
		usleep(40000);
	}
	pah8001_power_down(1);
}

int main()
{

	//init
	int ret;
	ret = pixart_init();
	if (ret) {
		printf("hrm module init fail\r\n");
		return ret;
	}
	//execute in infinit loop
	pixart_work();
	return 0;
}
