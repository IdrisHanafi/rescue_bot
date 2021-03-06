
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void set_pwm(int fd,float duty_cycle,char lower_byte_addr,char higher_byte_addr){

	unsigned char buf[10];	
	unsigned int on_time = (duty_cycle*4095);
	usleep(10000);

	//leave default delay time to be zero
	//set the duty cyce of pwm
	unsigned char lower_val = on_time & 0b11111111;
	buf[0] = lower_byte_addr; 
	buf[1] = lower_val; 
	if((write(fd,buf,2)) != 2){
		printf("Error writing to i2c slave\n");
		exit(1);
	}

	unsigned char higher_val = on_time >> 8; 
	buf[0] = higher_byte_addr; 
	buf[1] = higher_val; 
	if((write(fd,buf,2)) != 2){
		printf("Error writing to i2c slave\n");
		exit(1);
	}
}

void reset(int fd) {
	unsigned char buf[10];
	buf[0] = 0x0; 
	buf[1] = 0x0; 
	if((write(fd,buf,2)) != 2){
		printf("Error writing to i2c slave\n");
		exit(1);

	}
}
int main(int argc, char **argv)
{
	
	int motorNumber, angle;
	sscanf(argv[1],"%d",&motorNumber); 
	sscanf(argv[2],"%d",&angle); 
	
	float duty_cycle = 0;
	if (motorNumber == 0 || motorNumber == 4) {
		duty_cycle = angle/180.0;   // can vary this quantity 
	}
	else if (motorNumber == 1 || motorNumber == 3) {
		duty_cycle = (0.055555555*angle + 2.5)/100;   // can vary this quantity 
	}
	else if (motorNumber == 2) {
		if (angle < 50) angle = 50;
		else if (angle > 120) angle = 120;
		duty_cycle = (0.055555555*angle + 2.5)/100;   // can vary this quantity 
	}
	//printf("duty cycle is %f\n", duty_cycle);
	
	int fd;														// File descrition
	// For older raspberry pi modules use "/dev/i2c-0" instead of "/dev/i2c-1" for the i2c port
	char *fileName = "/dev/i2c-1";								// Name of the port we will be using
	int  address = 0x40; //0x70; try 0x70 if 0x40 does not work									
	unsigned char buf[10];										// Buffer for data being read/ written on the i2c bus
	
	if ((fd = open(fileName, O_RDWR)) < 0) {					// Open port for reading and writing
		printf("Failed to open i2c port\n");
		exit(1);
	}
	
	if (ioctl(fd, I2C_SLAVE, address) < 0) {					// Set the port options and set the address of the device we wish to speak to
		printf("Unable to get bus access to talk to slave\n");
		exit(1);
	}

	//reset(fd);
	
	//usleep(100000);

	buf[0] = 0; //read from mode register		
	
	if ((write(fd, buf, 1)) != 1) {
		printf("Error writing to i2c slave\n");
		exit(1);
	}

	
	if ((read(fd, buf, 1)) != 1) { // Read back data into buf[]
		printf("Unable to read from slave\n");
		exit(1);
	}

	usleep(100000);
	

        int pwm_freq = 0x79;
        char freq_address = 0xFE;
        buf[0] = freq_address;
	//printf("write %i to pwm_freq\n", pwm_freq); 
        buf[1] = pwm_freq;


        if ((write(fd, buf, 2)) != 2) { // setting the frequency to 50Hz or 500Hz
                printf("Error writing to i2c slave\n");
                exit(1);
        }
	
	//check whether pwm signal set successfully
	if (write(fd, buf, 1) != 1) {
		printf("nonsense");
		exit(1);
	}
	if (read(fd, buf, 1) != 1) {
		printf("nonsense");
		exit(1);
	}

        //turn the device on set the sleep to 0 in mode register 1
	buf[0] = 0;
	buf[1] = 0x81;
	
	if ((write(fd, buf, 2)) != 2) {
		printf("nonsense");
		exit(1);
	}

        usleep(100000);

	if(motorNumber == 0) {
		set_pwm(fd,duty_cycle,0x8,0x9);//pwm0
	}
	else if(motorNumber == 1) {
		set_pwm(fd,duty_cycle,0xC,0xD);//pwm1
	}
	else if(motorNumber == 2) {
		set_pwm(fd,duty_cycle,0x10,0x11);//pwm2
	}
	else if(motorNumber == 3) {
		set_pwm(fd,duty_cycle,0x14,0x15);//pwm3
	}
	else if(motorNumber == 4) {
		set_pwm(fd,duty_cycle,0x18,0x19);//pwm4
	}


	
	close(fd);	
	
	return 0;
}
