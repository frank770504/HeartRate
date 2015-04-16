#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>


class Userspace_i2c {

public:
	Userspace_i2c();
	Userspace_i2c(char* i2c_dev, unsigned char addr);
	~Userspace_i2c();
	int transport_read( unsigned char *buf, int len );
	int transport_write( unsigned char *buf, int len );
	int reg_burst_read(unsigned char reg, unsigned char *data, unsigned char len);
	int reg_read(unsigned char reg, unsigned char *data);
	int reg_write(unsigned char reg, unsigned char data);
	//unsigned char cmd[MAX_BUF];
	
private:
	char* I2C_DEV;
	unsigned char ADDR;
	const static int MAX_BUF = 100;
	const static int FIFO_SIZE = 8;
	const static int FIFO_SIZE_M1 = FIFO_SIZE - 1;
	int i2c_write(unsigned char dev_addr, unsigned char *buf, int length);
	int i2c_read(unsigned char dev_addr, unsigned char *buf, int length);
};
