#include "i2c-ctrl.h"
#include <pixart_8001_1000.h>
#include "led_ctrl.h"

#define I2C_DEV "/dev/i2c-2"
#define HRM_ADDR 0x33
#define INERT_ADDR 0x68
#define FIFO_SIZE 8
#define FIFO_SIZE_M1 (FIFO_SIZE-1)

typedef unsigned char u8;
typedef unsigned int u32;

Userspace_i2c i2c_hrm(I2C_DEV, HRM_ADDR);
Userspace_i2c i2c_iner(I2C_DEV, INERT_ADDR);

typedef struct {
        u8 HRD_Data[13] ;
        u32 MEMS_Data[3] ;
}ppg_mems_data_t;

static ppg_mems_data_t _ppg_mems_data[FIFO_SIZE] ;

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

static void ofn_ppg(void)
{
	static u8 Frame_Count = 0 ;
	u8 touch_flag = 0 ;
	u8 data ;
	int _write_index = 0; //use the fix buffer

	if(1)
	{
		bank_select(BANK0);
		i2c_hrm.reg_read(0x59, &touch_flag);

		touch_flag &= 0x80  ;
		led_ctrl(touch_flag);
		
		bank_select(BANK1);
		i2c_hrm.reg_read(0x68, &data);
		_ppg_mems_data[_write_index].HRD_Data[0] = data & 0x0f ;

		if(_ppg_mems_data[_write_index].HRD_Data[0] == 0)
		{
			printf("11111111111\r\n");
			bank_select(BANK0);
			usleep(10000);
		}
		else
		{
			printf("2222222222\r\n");
			int tmp = 0;
			i2c_hrm.reg_burst_read(0x64, &(_ppg_mems_data[_write_index].HRD_Data[1]), 4);
			i2c_hrm.reg_burst_read(0x1A, &(_ppg_mems_data[_write_index].HRD_Data[5]), 3);
			_ppg_mems_data[_write_index].HRD_Data[8] = Frame_Count++;
			_ppg_mems_data[_write_index].HRD_Data[9] = (unsigned)time(NULL);
			_ppg_mems_data[_write_index].HRD_Data[10] = get_led_current_change_flag() ;

			_ppg_mems_data[_write_index].HRD_Data[11] = touch_flag ;
			_ppg_mems_data[_write_index].HRD_Data[12] = _ppg_mems_data[_write_index].HRD_Data[6] ;
		}
	}
	//print result
	printf("%u, raw (%d)(%d)(%d)(%d)\r\n", _ppg_mems_data[_write_index].HRD_Data[9]
						, _ppg_mems_data[_write_index].HRD_Data[1]
						, _ppg_mems_data[_write_index].HRD_Data[2]
						, _ppg_mems_data[_write_index].HRD_Data[3]
						, _ppg_mems_data[_write_index].HRD_Data[4]);
	printf("%u, alg (%d)(%d)(%d)\r\n\r\n", _ppg_mems_data[_write_index].HRD_Data[9]
						, _ppg_mems_data[_write_index].HRD_Data[5]
						, _ppg_mems_data[_write_index].HRD_Data[6]
						, _ppg_mems_data[_write_index].HRD_Data[7]);
}

static void pixart_work()
{
	pah8001_power_down(0);

//	printk(">>>%s (%d)\n", __func__, __LINE__);
	while(1)
	{
		ofn_ppg();
		//printf("%d %u\r\n", time(NULL), time(NULL));
		usleep(100000);
	}
	pah8001_power_down(1);
//	printk("<<< %s (%d)\n", __func__, __LINE__);
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
