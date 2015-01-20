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
#include <linux/spi/spi.h>
#include "cmx865a.h"
#define GPIO_SPI 1
static char cmx865a_output_buffer[32];	/* Stores data to write out of device */
struct spi_device g_spi;
bool g_incoming_call=false;
unsigned char phone_state;
#if GPIO_SPI
#define CLK_PIN 17
#define MOSI_PIN 14
#define MISO_PIN 15
#define CS_PIN 16
void ms_delay(void)
{
	volatile int i,j;
	for(i=0;i<1;i++)
		j=0;
	//udelay(1);
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
unsigned char MISO(void)
{
	return gpio_get_value(MISO_PIN);
}
unsigned char write_spi(unsigned char data)
{
	
	unsigned char i; 
	unsigned char Temp=0x00;
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
#endif
void write_cmx865a(unsigned char addr,unsigned short data,unsigned char len)
{
#if GPIO_SPI
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
#else
	unsigned char tmp[3];
	int count=0;
	if(len==0)
	{
		tmp[0]=addr;
		count=1;
	}
	else
	{
		tmp[0]=addr;
		if(len==2)
		{
			tmp[1]=(data>>8)&0xff;
			tmp[2]=data&0xff;
			count=3;
		}
		else
		{
			tmp[1]=data&0xff;
			count=2;
		}
			
	}
	#if 0
	struct spi_transfer	t = {
			.tx_buf		= tmp,
			.len		= count,
			.cs_change  = 0,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	ssize_t status= spi_sync(&g_spi, &m);
	#else
	ssize_t status=spi_write(&g_spi,tmp,count);
	#endif
#endif
}
void read_cmx865a(unsigned char addr,unsigned char* data,unsigned char len)
{
#if GPIO_SPI
//	unsigned char i=0;
	CS(0);
	write_spi(addr);
	data[0]=write_spi(0);
	if(len==2)
	{	
		data[1]=data[0];
		data[0]=write_spi(0);
	}
	CS(1);
#else
spi_write_then_read(&g_spi,&addr,1,data,len);
#endif
}

static irqreturn_t cmx865a_irq_handler (int irq, void *dev_id)
{
	unsigned int  i,tmp; 
	unsigned char  j; 
	static unsigned char  k=0; 
	static unsigned short  fsk_long=0; 
	static unsigned short CID_RX_count= 0;
	static enum CID_recive_state CID_state=0;
	read_cmx865a(Status_addr,&i,2);	
	//if(!g_incoming_call)
		//return IRQ_HANDLED;
	
	//printk("Status ==>%02x\r\n",i);
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
			read_cmx865a(Receive_Data_addr,(unsigned char *)&tmp,2);
		}
	}
	else
	{
		if(i&0x0040)//FSK
		{
			udelay(1);
			read_cmx865a(Receive_Data_addr,&j,1);
			//printk("%d==> %x\r\n",CID_state,j);
			//if(j>='0'&&j<='9')
				//printk(">>%c\r\n",j);
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
							//printk("==>Recived_55\r\n");
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
					//printk("==>Recived_02 1\r\n");
						CID_state=Recived_02;
					}
					else if(j==0x04)
					{
					//printk("==>Recived_02 2\r\n");
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
					//printk("==>Recived_long\r\n");
					CID_state=Recived_long;
					break;
				}	
				case Recived_long:
				{
					if(CID_RX_count<fsk_long)
					{
						cmx865a_output_buffer[CID_RX_count++]=j;
						/*
						if(CID_RX_count==max_buff)
						{
						CID_RX_count=max_buff-1;
						}
						*/
						//printk("Got FSK Num %d %c\r\n",j,j);
					}
					else
					{
						CID_state=Waite;
						printk("New coming call: ");
						//for(i=0;i<CID_RX_count;i++)
							printk("%s",cmx865a_output_buffer);
						printk("\r\n");
						g_incoming_call=true;
						//return IRQ_HANDLED;
					}
					break;
				}	
				default:
					break;
				}
			}
		}
	return IRQ_HANDLED;
}
static irqreturn_t qcx2101_irq_handler (int irq, void *dev_id)
{
	//printk("New comming call ...\r\n");
	//cmx865a_irq_handler(irq,dev_id);
	//if(!g_incoming_call)
		//g_incoming_call=true;
	return IRQ_HANDLED;
}

void cmx865a_hw_init(void)
{
	unsigned short data;
	write_cmx865a(G_Reset_Command_addr,0,0);
	msleep(5);
	write_cmx865a(G_Control_Command_addr, Reset_CMX865|PowerUp,2);
	msleep(50);
	write_cmx865a(G_Control_Command_addr, NORMAL,2);
	msleep(50);
	
	read_cmx865a(Status_addr,(unsigned char *)&data,2);
	printk("cmx865a_hw_init %x\r\n",data);

	if(data&0x00ff)
	{
		printk("init cmx865a failed");
		return ;

	}
	else
	{	
		unsigned short temp_int=6<<9;
		if (DTMF_MODE)
		{
			write_cmx865a(Receive_Mode_addr, Received_DTMF|temp_int,2);//????
			printk("DTMF Re\r\n");
		}
		else
		{
			write_cmx865a(Receive_Mode_addr, Received_FSK|temp_int,2);//????
			printk("FSK Re\r\n");
		}
	}	
	
	return ;
}

static int cmx865a_read (struct file *filp, char __user *buffer,
			size_t count, loff_t *ppos)
{
	int len=strlen(cmx865a_output_buffer);
	//printk("cmx865a_read len %d\r\n",len);
	if(len!=0&&g_incoming_call)
	{
		copy_to_user(buffer,cmx865a_output_buffer,len);
		printk("Len %d Phone:%s\r\n",len,cmx865a_output_buffer);
		memset(cmx865a_output_buffer,'\0',32);
		g_incoming_call=false;
		return len;
	}
	else
		return 0;
	//return (copy_to_user (buffer, &cmx865a_output_buffer, 32))
		// ? -EFAULT : 32;
}
static int qcx2101_lcs_ctl (struct file *filp, char __user *buffer,
			size_t count, loff_t *ppos)
{
	char accept=0;
	static ctl=0;
	if(copy_from_user(&accept,buffer,sizeof(char)))
		return -EFAULT;
	if(accept==1)
		gpio_direction_output(104,1);
	else
	{
		if(ctl==0)
		{
			gpio_direction_output(104,1);
			
			msleep(100);
			gpio_direction_output(104,0);
			ctl=1;
		}
		else
		{
			gpio_direction_output(104,0);
			ctl=0;
		}
	}
	printk("qcx2101_lcs_ctl==> %s the pstn call\r\n",accept?"accept":"reject");
	return 0;
}

static const struct file_operations cmx865a_fops = {
	.owner		= THIS_MODULE,
	.read		= cmx865a_read,
	.write		= qcx2101_lcs_ctl,
};

static struct miscdevice cmx865a_misc_device = {
	CMX865A_MINOR,
	"cmx865a",
	&cmx865a_fops,
};
static int cmx865a_probe(struct spi_device *spi)   
{   
    int ret = 0;  
    spi->bits_per_word = 8;
	spi->mode = SPI_MODE_0;
	ret = spi_setup(spi);
	if (ret < 0)
	{
		printk("cmx865a_probe failed %d\r\n",ret);
		return ret;
	}
	g_spi=*spi;
	printk("cmx865a_probe ok\r\n");
    return 0;   
}   

static int __devexit cmx865a_remove(struct spi_device *spi)
{
	return 0;
}

static struct spi_driver   cmx865a_driver = { 
.driver = {
        .name   ="cmx865a",
        .owner  = THIS_MODULE,
    },
    .probe  = cmx865a_probe,
    .remove =__devexit_p(cmx865a_remove),
};

static int __init cmx865a_init(void)
{

	printk (KERN_INFO "cmx865a_init %s\n", VERSION);


	if (misc_register (&cmx865a_misc_device)) {
		printk (KERN_WARNING "cmx865a: Couldn't register device 10, %d.\n", CMX865A_MINOR);
		return -EBUSY;
	}
	#if !GPIO_SPI
	spi_register_driver(&cmx865a_driver);
	#endif
	if (request_irq (OMAP_GPIO_IRQ(103), cmx865a_irq_handler, IRQF_TRIGGER_FALLING,"cmx865a", NULL)) 
	{
		printk (KERN_WARNING "cmx865a: IRQ %d is not free.\n",OMAP_GPIO_IRQ(103));
		misc_deregister (&cmx865a_misc_device);
		return -EIO;
	}
	memset(cmx865a_output_buffer,'\0',32);
	/*if (request_irq (OMAP_GPIO_IRQ(100), qcx2101_irq_handler, IRQF_TRIGGER_FALLING,"qcx2101", NULL)) 
	{
		printk (KERN_WARNING "qcx2101: IRQ %d is not free.\n",OMAP_GPIO_IRQ(100));
		misc_deregister (&cmx865a_misc_device);
		return -EIO;
	}*/
	cmx865a_hw_init();
	return 0;
}

static void __exit cmx865a_exit (void) 
{
	free_irq (OMAP_GPIO_IRQ(103), NULL);
	//free_irq (OMAP_GPIO_IRQ(100), NULL);
	misc_deregister (&cmx865a_misc_device);
	#if !GPIO_SPI
	spi_unregister_driver(&cmx865a_driver);
	#endif
}


MODULE_AUTHOR("Alex Holden");
MODULE_LICENSE("GPL");

module_init(cmx865a_init);
module_exit(cmx865a_exit);
