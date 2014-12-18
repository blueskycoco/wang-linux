#ifndef __CMX865A_H
#define __CMX865A_H

#define VERSION "0.3"		/* Driver version number */
#define CMX865A_MINOR 190	/* Major 10, Minor 158, /dev/nwbutton */
#define DTMF_MODE 0
#define key_0 'D'
#define CID_Received 1
#define max_buff 256

enum CID_recive_state
{
	 Waite,
       Recived_55,
       Recived_02,
       Recived_long,
       Recived_num,
}CID_state=0;
#define	G_Reset_Command_addr	0x01
#define	G_Control_Command_addr	0xe0
#define	Transmit_Mode_addr	0xe1
#define	Receive_Mode_addr		0xe2
#define	Transmit_Data_addr	0xe3
#define	Receive_Data_addr		0xe5
#define	Status_addr			0xe6
#define	Programming_addr		0xe8

#define	NORMAL			0x8141//8141/815f
#define	TXAOutDisable		0x4000
#define     AnalogueLoopbackTest  	0x0800
#define     equalizer     		0x0400
#define	PowerUp			0x0100//NORMAL&0xfeff
#define	Reset_CMX865		0x0080
#define	IRQ_EN			0x0040

#define	Transmit_disable		0x0000//0 001 111  0  000 10110 /(00000)
#define	Transmit_DTMF		0x1810// 1e10
#define	Received_DTMF		0x1001//0001 
#define	Received_FSK		0x5035//5e37
#define	DTMF_0			0x1a
#define	DTMF_1			0x11
#define	DTMF_2			0x12
#define	DTMF_3			0x13
#define	DTMF_4			0x14
#define	DTMF_5			0x15
#define	DTMF_6			0x16
#define	DTMF_7			0x17
#define	DTMF_8			0x18
#define	DTMF_9			0x19
#define	DTMF_D			0x00
#define	DTMF_X			0x1b
#define	DTMF_J			0x1c
#define	DTMF_A			0x1d
#define	DTMF_B			0x1e
#define	DTMF_C			0x1f

static irqreturn_t cmx865a_irq_handler (int irq, void *dev_id);
void cmx865a_hw_init (void);
#endif /* __CMX865A_H */
