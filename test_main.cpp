#include "i2c-ctrl.h"


int main(int argc, char **argv)
{
    unsigned int fd;
    unsigned int slave_address, reg_address;
    unsigned r_w;
    unsigned w_val;
    unsigned char rw_val;

    if (argc < 5) {
        printf("Usage:\n%s /dev/i2c-x start_addr reg_addr rw[0|1] [write_val]\n", argv[0]);
        return 0;
    }

    fd = open(argv[1], O_RDWR);

    if (!fd) {
        printf("Error on opening the device file %s\n", argv[1]);
        return 0;
    }

    sscanf(argv[2], "%x", &slave_address);
    sscanf(argv[3], "%x", &reg_address);
    sscanf(argv[4], "%d", &r_w);

    Userspace_i2c i2c_ctrl(argv[1], slave_address); 


    if (r_w == 0) {
	i2c_ctrl.reg_read( reg_address, &rw_val);
        //i2c_read_reg(argv[1], &rw_val, slave_address, reg_address, 1);
        printf("Read %s-%x reg %x, read value:%x\n", argv[1], slave_address, reg_address, rw_val);
    } else {
        if (argc < 6) {
            printf("Usage:\n%s /dev/i2c-x start_addr reg_addr r|w[0|1] [write_val]\n", argv[0]);
            return 0;
        }
        sscanf(argv[5], "%d", &w_val);
        if ((w_val & ~0xff) != 0)
            printf("Error on written value %s\n", argv[5]);

        rw_val = (unsigned char)w_val;
	i2c_ctrl.reg_write( reg_address, rw_val);
        //i2c_write_reg(argv[1], &rw_val, slave_address, reg_address, 1);
    }

    return 0;
}
