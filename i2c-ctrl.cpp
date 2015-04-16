#include "i2c-ctrl.h"

Userspace_i2c::Userspace_i2c(){}
Userspace_i2c::~Userspace_i2c(){}

Userspace_i2c::Userspace_i2c(char* i2c_dev, unsigned char addr)
{
	//sprintf(I2C_DEV,"%s",i2c_dev);
	I2C_DEV = new char[20];
	sprintf(I2C_DEV,"%s",i2c_dev);
	//I2C_DEV = "i2c_dev";
	ADDR = addr;
}


int Userspace_i2c::i2c_write(unsigned char dev_addr, unsigned char *buf, int length)
{
    int fd = 0;
    fd = open(I2C_DEV, O_RDWR); //open "/dev/i2c-2"
    //printf("command : %x,%x\nstm32 i2c slave address:%x\n",buf[0],buf[1],dev_addr);
    if (fd < 0) {
        printf("Error opening file: %s\r\n", strerror(errno));
        return 1;
    }

    ioctl(fd,I2C_TENBIT,0);
    if (ioctl(fd, I2C_SLAVE, dev_addr) < 0) {
        printf("ioctl error: %s\r\n", strerror(errno));
        return 1;
    }

    if (write(fd, buf, length) != length) {
        printf("Error writing file: %s\r\n", strerror(errno));
        return 1;
    }

    close(fd);
    return 0;
}

int Userspace_i2c::i2c_read( unsigned char dev_addr,unsigned char *buf, int length)
{
    int fd = 0;
    fd = open(I2C_DEV, O_RDWR); //open "/dev/i2c-2"

    if (fd < 0) {
        printf("Error opening file: %s\r\n", strerror(errno));
        return 1;
    }

    ioctl(fd,I2C_TENBIT,0);
    if (ioctl(fd, I2C_SLAVE, dev_addr) < 0) {
        printf("ioctl error: %s\r\n", strerror(errno));
        return 1;
    }
    if (read(fd, buf, length) != length) {
        printf("Error reading file: %s\r\n", strerror(errno));
        return 1;
    }

    close(fd);
    return 0;
}

int Userspace_i2c::transport_read( unsigned char *buf,  int len )
{
    return i2c_read(ADDR,buf,len);
}

int Userspace_i2c::transport_write( unsigned char *buf, int len )
{
    return i2c_write(ADDR,buf,len);
}

int Userspace_i2c::reg_burst_read(unsigned char reg, unsigned char *data, unsigned char len)
{
	int ret = -1;
	unsigned char cmd[256] = {0};
	cmd[0] = (unsigned char)reg;

	ret = transport_write(&reg, 1);
	if (ret) {
		printf("Doing %s in write part, but failed\r\n", __func__);
		return -1;
	}

	ret = transport_read(cmd, (int)len);
	if (ret) {
		printf("Doing %s, but failed\r\n", __func__);
		*data = 0;
		return -1;
	}

	memcpy(data,cmd,len);
	return 0;
}

int Userspace_i2c::reg_read(unsigned char reg, unsigned char *data)
{
	reg_burst_read(reg, data, 1);
	return 0;
}

int Userspace_i2c::reg_write(unsigned char reg, unsigned char data)
{
	int ret = -1;
	unsigned char cmd[2];
	cmd[0] = reg;
	cmd[1] = data;
	ret = transport_write(cmd,2);
	if (ret)
		printf("The i2c fail causes %s failed\r\n", __func__);
	return ret;
}
