/*
 * 	NetWinder Button Driver-
 *	Copyright (C) Alex Holden <alex@linuxhacker.org> 1998, 1999.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/gpio.h>

#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include "cmx865a.h"

static char cmx865a_output_buffer[32];	/* Stores data to write out of device */

rt_uint8_t phone_state;
#define CLK_PIN 141
#define MOSI_PIN 142
#define MISO_PIN 143
#define CS_PIN 144
void ms_delay()
{
	volatile int i,j;
	for(i=0;i<10;i++)
		j=0;
}
void CLK(bool ctl)
{
	if(ctl)
		gpio_direction_output(CLK_PIN, 1);
	else
		gpio_direction_output(CLK_PIN, 0);
}
void MOSI(bool ctl)
{
	if(ctl)
		gpio_direction_output(MOSI_PIN, 1);
	else
		gpio_direction_output(MOSI_PIN, 0);
}
void CS(bool ctl)
{
	if(ctl)
		gpio_direction_output(CS_PIN, 1);
	else
		gpio_direction_output(CS_PIN, 0);
}
unsigned char MISO()
{
	return gpio_get_value(MISO_PIN);
}
rt_uint8_t write_spi(rt_uint8_t data)
{
	
	rt_uint8_t i; 
	rt_uint8_t Temp=0x00;
	unsigned char SDI; 
	for (i = 0; i < 8; i++)
	{
		CLK(1);
		ms_delay();
		if (data&0x80)      
		{
			MOSI(1);
		}
		else
		{
			MOSI(0);
		}
		data <<= 1;  
		CLK(0);
		ms_delay();
		SDI = MISO();
		Temp<<=1;

		if(SDI)
		{
			Temp++;
		}
		CLK(1);
	}
	
	return Temp; 
}
void write_cmx865a(rt_uint8_t addr,unsigned short data,rt_uint8_t len)
{
	CS(0);
	if(len==0)
		write_spi(addr);
	else
		{
		write_spi(addr);
		if(len==2)
			write_spi((data>>8) & 0xff);
		write_spi(data&0xff);
		}
	CS(1);
}
void read_cmx865a(rt_uint8_t addr,rt_uint8_t* data,rt_uint8_t len)
{

	rt_uint8_t i=0;
	CS(0);
	write_spi(addr);
	data[0]=write_spi(0);
	if(len==2)
	{	
		data[1]=data[0];
		data[0]=write_spi(0);
	}
	CS(1);
}
static irqreturn_t cmx865a_irq_handler (int irq, void *dev_id)
{
	unsigned int  i,tmp; 
	unsigned char  j; 
	static unsigned char  k=0; 
	static unsigned char  fsk_long=0; 
	read_cmx865a(Status_addr,&i,2);
	
	if(DTMF_MODE)
	{
		if(i&0x0020)//DTMF
		{
			j=i&0x000f;
			/*if((j==M_or_P_key_value)||(j==Permit_Applay))
			{
				Rx_P_or_M=j;
			}
			else
			{*/
				if(CID_RX_count<max_buff)
				{
					if(j==key_0)
					{
						cmx865a_output_buffer[CID_RX_count++]=0;
					}
					else if((j>0)&&(j<10))
					{
						cmx865a_output_buffer[CID_RX_count++]=j;
						printk("Got DTMF Num %d %c\r\n",j,j);
					}
				}
			//}
		}
		else
		{
			read_cmx865a(Receive_Data_addr,&tmp,2);
		}
	}
	else
	{
		if(i&0x0040)//FSK
		{
			read_cmx865a(Receive_Data_addr,&j,2);
			
		//	rt_kprintf("==> %d %x\r\n",j,j);
			if(j>='0'&&j<='9')
				printk(">>%c\r\n",j);
		switch(CID_state)
		{
			case Waite:
			{
				if(j==0x55)
				{
					k++;
					if(k>2)
					{
						k=0;
						CID_state=Recived_55;
					}
				}
				else
				{
					k=0;
				}
				break;
			}
			case Recived_55:
			{
				if(j==0x02)
				{
					CID_state=Recived_02;
				}
				else if(j==0x04)
				{
					CID_state=Recived_02;
				}
				break;
			}
			case Recived_02:
			{
				if(j<0x10)
				{
					fsk_long=j;
				}
				else
				{
					fsk_long=max_buff;
				}
				CID_RX_count=0;
				CID_state=Recived_long;
				break;
			}	
			case Recived_long:
			{
				if(CID_RX_count<fsk_long)
				{
					cmx865a_output_buffer[CID_RX_count++]=j-'0';
					/*
					if(CID_RX_count==max_buff)
					{
					CID_RX_count=max_buff-1;
					}
					*/
					printk("Got FSK Num %d %c\r\n",j,j);
				}
				else
				{
					CID_state=Waite;
					printk("finish receive phone num\r\n");
				}
				break;
			}	
			default:
				break;
			}
		}
	}
}
void cmx865a_hw_init(void)
{
	unsigned short data;
	write_cmx865a(G_Reset_Command_addr,0,0);
	msleep(5);
	write_cmx865a(G_Control_Command_addr, Reset_CMX865|PowerUp,2);
	msleep(50);
	write_cmx865a(G_Control_Command_addr, NORMAL,2);

	read_cmx865a(Status_addr,&data,2);
	if(data&0x00ff)
	{
		printk("init cmx865a failed");
		return ;

	}
	else
	{	
		temp_int=temp_int<<9;
		if (DTMF_MODE)
		{
			write_cmx865a(Receive_Mode_addr, Received_DTMF|temp_int,2);//????
			printk("DTMF Re");
		}
		else
		{
			write_cmx865a(Receive_Mode_addr, Received_FSK|temp_int,2);//????
			printk("FSK Re");
		}
	}	
	return ;
}

static irqreturn_t cmx865a_irq_handler (int irq, void *dev_id)
{
	return IRQ_HANDLED;
}
static int cmx865a_read (struct file *filp, char __user *buffer,
			size_t count, loff_t *ppos)
{
	return (copy_to_user (buffer, &cmx865a_output_buffer, 32))
		 ? -EFAULT : 32;
}

static const struct file_operations cmx865a_fops = {
	.owner		= THIS_MODULE,
	.read		= cmx865a_read,
};

static struct miscdevice cmx865a_misc_device = {
	CMX865A_MINOR,
	"cmx865a",
	&cmx865a_fops,
};

static int __init cmx865a_init(void)
{

	printk (KERN_INFO "cmx865a_init \n", VERSION);

	if (misc_register (&cmx865a_misc_device)) {
		printk (KERN_WARNING "nwcmx865a: Couldn't register device 10, "
				"%d.\n", CMX865A_MINOR);
		return -EBUSY;
	}

	if (request_irq (OMAP_GPIO_IRQ(103), cmx865a_irq_handler, IRQF_TRIGGER_FALLING,
			"cmx865a", NULL)) {
		printk (KERN_WARNING "cmx865a: IRQ %d is not free.\n",
				IRQ_NETWINDER_BUTTON);
		misc_deregister (&cmx865a_misc_device);
		return -EIO;
	}
	cmx865a_hw_init();
	return 0;
}

static void __exit cmx865a_exit (void) 
{
	free_irq (OMAP_GPIO_IRQ(103), NULL);
	misc_deregister (&cmx865a_misc_device);
}


MODULE_AUTHOR("Alex Holden");
MODULE_LICENSE("GPL");

module_init(cmx865a_init);
module_exit(cmx865a_exit);
