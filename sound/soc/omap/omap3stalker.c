/*
 * omap3stalker.c  -- ALSA SoC support for OMAP3 Stalker
 *
 * Author: Jason Lam <lzg@ema-tech.com>
 *
 * Based on sound/soc/omap/beagle.c by Steve Sakoman
 *
 * Copyright (C) 2008 EMA-Tech
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <plat/hardware.h>
#include <plat/gpio.h>
#include <plat/mcbsp.h>

#include "omap-mcbsp.h"
#include "omap-pcm.h"
#include "../codecs/twl4030.h"

#if defined(CONFIG_SND_SOC_WL1271BT)
#include <plat/control.h>
#include "../codecs/wl1271bt.h"
#endif
#if defined(CONFIG_SND_SOC_TLV320AIC12K)
#include <plat/control.h>
#include "../codecs/tlv320aic12k.h"
#include <linux/i2c.h>
#endif

static int omap3stalker_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

	//printk("\r\nomap3stalker_hw_params ==>\r\n");
	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "Can't set codec DAI configuration\n");
		return ret;
	}

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "Can't set cpu DAI configuration\n");
		return ret;
	}

	/* Set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
				     SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "Can't set codec system clock\n");
		return ret;
	}

	//printk("omap3stalker_hw_params <==\r\n");
	return 0;
}

static struct snd_soc_ops omap3stalker_ops = {
	.hw_params = omap3stalker_hw_params,
};

#if defined(CONFIG_SND_SOC_WL1271BT)
static int omap3stalker_wl1271bt_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

	/* Set cpu DAI configuration for WL1271 Bluetooth codec */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_DSP_B |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "Can't set cpu DAI configuration for " \
						"WL1271 Bluetooth codec \n");
		return ret;
	}

	return 0;
}

static struct snd_soc_ops omap3stalker_wl1271bt_pcm_ops = {
	.hw_params = omap3stalker_wl1271bt_pcm_hw_params,
};

#endif

#if defined(CONFIG_SND_SOC_TLV320AIC12K)
static int omap3stalker_tlv320aic12k_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

	//printk("\r\n\r\n\r\nomap3stalker_tlv320aic12k_pcm_hw_params ==>\r\n");
	/* Set cpu DAI configuration for TLV320AIC12K codec */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_DSP_B|
				  SND_SOC_DAIFMT_IB_IF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "Can't set cpu DAI configuration for " \
						"TLV320AIC12K codec \n");
		return ret;
	}
/*	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKX_EXT, 0,
				SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set CPU system clock OMAP_MCBSP_SYSCLK_CLKX_EXT\n");
		return ret;
	}*/
	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_CLKR_SRC_CLKX, 0,
				SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set CPU system clock OMAP_MCBSP_CLKR_SRC_CLKX\n");
		return ret;
	}

	snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_FSR_SRC_FSX, 0,
				SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set CPU system clock OMAP_MCBSP_FSR_SRC_FSX\n");
		return ret;
	}

	/*ret = snd_soc_dai_set_sysclk(cpu_dai, 0, 256000,
				     SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "dillon Can't set codec system clock\n");
		return ret;
	}*/
	//printk("omap3stalker_tlv320aic12k_pcm_hw_params <==\r\n");

	return 0;
}
static int omap3stalker_tlv320aic12k_pcm_hw_params2(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

	/* Set cpu DAI configuration for TLV320AIC12K codec */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_DSP_B|
				  SND_SOC_DAIFMT_IB_IF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "Can't set cpu DAI configuration for " \
						"TLV320AIC12K codec \n");
		return ret;
	}

	return 0;
}

static struct snd_soc_ops omap3stalker_tlv320aic12k_pcm_ops = {
	.hw_params = omap3stalker_tlv320aic12k_pcm_hw_params,
};
static struct snd_soc_ops omap3stalker_tlv320aic12k_pcm_ops2 = {
	.hw_params = omap3stalker_tlv320aic12k_pcm_hw_params2,
};

#endif
/* Digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link omap3stalker_dai[] = {
	{
		.name 		= "TWL4030",
		.stream_name 	= "TWL4030",
		.cpu_dai 	= &omap_mcbsp_dai[0],
		.codec_dai 	= &twl4030_dai[TWL4030_DAI_HIFI],
		.ops 		= &omap3stalker_ops,
	},
#if defined(CONFIG_SND_SOC_WL1271BT)
	/* Connects WL1271 Bluetooth codec <--> CPU */
	{
		.name = "WL1271BTPCM",
		.stream_name = "WL1271 BT PCM",
		.cpu_dai = &omap_mcbsp_dai[1],
		.codec_dai = &wl1271bt_dai,
		.ops = &omap3stalker_wl1271bt_pcm_ops,
	}
#endif
#if defined(CONFIG_SND_SOC_TLV320AIC12K)
	/* Connects TLV320AIC12K codec <--> CPU */
	{
		.name = "TLV320AIC12KPCM",
		.stream_name = "TLV320AIC12K PCM 1",
		.cpu_dai = &omap_mcbsp_dai[1],
		.codec_dai = &tlv320aic12k_dai,
		.ops = &omap3stalker_tlv320aic12k_pcm_ops,
	},
		/* Connects TLV320AIC12K codec <--> CPU */
	{
		.name = "TLV320AIC12KPCM",
		.stream_name = "TLV320AIC12K PCM 2",
		.cpu_dai = &omap_mcbsp_dai[2],
		.codec_dai = &tlv320aic12k_dai,
		.ops = &omap3stalker_tlv320aic12k_pcm_ops2,
	},
		/* Connects TLV320AIC12K codec <--> CPU */
	{
		.name = "TLV320AIC12KPCM",
		.stream_name = "TLV320AIC12K PCM 3",
		.cpu_dai = &omap_mcbsp_dai[3],
		.codec_dai = &tlv320aic12k_dai,
		.ops = &omap3stalker_tlv320aic12k_pcm_ops2,
	}
#endif
};

/* Audio machine driver */
static struct snd_soc_card snd_soc_omap3stalker = {
	.name = "omap3stalker",
	.platform = &omap_soc_platform,
	.dai_link = &omap3stalker_dai[0],
	.num_links = ARRAY_SIZE(omap3stalker_dai),
};

/* twl4030 setup */
static struct twl4030_setup_data twl4030_setup = {
	.ramp_delay_value = 4,
	.sysclk = 26000,
};

/* Audio subsystem */
static struct snd_soc_device omap3stalker_snd_devdata = {
	.card = &snd_soc_omap3stalker,
	.codec_dev = &soc_codec_dev_twl4030,
	.codec_data = &twl4030_setup,
};

static struct platform_device *omap3stalker_snd_device;

static int __init omap3stalker_soc_init(void)
{
	int ret;
#if defined(CONFIG_SND_SOC_WL1271BT) || defined(CONFIG_SND_SOC_TLV320AIC12K)
	u16 reg;
	u32 val;
#endif

	if (!machine_is_sbc3530()) {
		pr_err("Not OMAP3 STALKER!\n");
		return -ENODEV;
	}
	pr_info("OMAP3 STALKER SoC init\n");

	omap3stalker_snd_device = platform_device_alloc("soc-audio", -1);
	if (!omap3stalker_snd_device) {
		printk(KERN_ERR "Platform device allocation failed\n");
		return -ENOMEM;
	}

#if defined(CONFIG_SND_SOC_WL1271BT) || defined(CONFIG_SND_SOC_TLV320AIC12K)
/*
 * Set DEVCONF0 register to connect
 * MCBSP1_CLKR -> MCBSP1_CLKX & MCBSP1_FSR -> MCBSP1_FSX
 */
	reg = OMAP2_CONTROL_DEVCONF0;
	val = omap_ctrl_readl(reg);
	val = val | 0x18;
	omap_ctrl_writel(val, reg);
#endif

	platform_set_drvdata(omap3stalker_snd_device, &omap3stalker_snd_devdata);
	omap3stalker_snd_devdata.dev = &omap3stalker_snd_device->dev;
	*(unsigned int *)omap3stalker_dai[0].cpu_dai->private_data = 1;
#if defined(CONFIG_SND_SOC_WL1271BT)
	*(unsigned int *)omap3stalker_dai[1].cpu_dai->private_data = 0; /* McBSP1 */
#endif
#if defined(CONFIG_SND_SOC_TLV320AIC12K)
	*(unsigned int *)omap3stalker_dai[1].cpu_dai->private_data = 0; /* McBSP1 */
	*(unsigned int *)omap3stalker_dai[2].cpu_dai->private_data = 3; /* McBSP4 */
	*(unsigned int *)omap3stalker_dai[3].cpu_dai->private_data = 4; /* McBSP5 */
	u8 data[2];
	struct i2c_client  *i2c_device;
	struct i2c_board_info tlv320aic12k_i2c_info = {
	I2C_BOARD_INFO("tlv320aic12k", 0x40),
	};
	if(!i2c_get_adapter(2))	
		printk("get i2c adapter failed\r\n");
	//else
	//	printk("get i2c adapter ok\r\n");

	i2c_device = i2c_new_device(i2c_get_adapter(2),&tlv320aic12k_i2c_info);	
	data[0]=0x04;
	data[1]=0x8A;//write M
	i2c_master_send(i2c_device,data,2);
	//printk("sent 0x%x,0x%x, %d\r\n",data[0],data[1],i2c_master_send(i2c_device,data,2));
	data[1]=0x1;//write N,P
	i2c_master_send(i2c_device,data,2);
	//printk("sent 0x%x,0x%x, %d\r\n",data[0],data[1],i2c_master_send(i2c_device,data,2));
	data[0]=0x05;
	data[1]=0x3E;
	i2c_master_send(i2c_device,data,2);
	//printk("sent 0x%x,0x%x, %d\r\n",data[0],data[1],i2c_master_send(i2c_device,data,2));
	data[1]=0x7e;//0x56
	i2c_master_send(i2c_device,data,2);
	//printk("sent 0x%x,0x%x, %d\r\n",data[0],data[1],i2c_master_send(i2c_device,data,2));
	//data[1]=0x83;//0xbb
	//i2c_master_send(i2c_device,data,2);
	//printk("sent 0x%x,0x%x, %d\r\n",data[0],data[1],i2c_master_send(i2c_device,data,2));

	i2c_unregister_device(i2c_device);
	if(!i2c_get_adapter(3))	
		printk("get i2c adapter failed\r\n");
	//else
	//	printk("get i2c adapter ok\r\n");

	i2c_device = i2c_new_device(i2c_get_adapter(3),&tlv320aic12k_i2c_info);	
	data[0]=0x04;
	data[1]=0x8A;//write M
	i2c_master_send(i2c_device,data,2);
	//printk("sent 0x%x,0x%x, %d\r\n",data[0],data[1],i2c_master_send(i2c_device,data,2));
	data[1]=0x1;//write N,P
	i2c_master_send(i2c_device,data,2);
	//printk("sent 0x%x,0x%x, %d\r\n",data[0],data[1],i2c_master_send(i2c_device,data,2));
	//data[0]=0x05;
	//data[1]=0x30;
	//i2c_master_send(i2c_device,data,2);
	//printk("sent 0x%x,0x%x, %d\r\n",data[0],data[1],i2c_master_send(i2c_device,data,2));
	//data[1]=0x7e;//0x56
	//i2c_master_send(i2c_device,data,2);
	//printk("sent 0x%x,0x%x, %d\r\n",data[0],data[1],i2c_master_send(i2c_device,data,2));
	//data[1]=0xbb;
	//printk("sent 0x%x,0x%x, %d\r\n",data[0],data[1],i2c_master_send(i2c_device,data,2));

	i2c_unregister_device(i2c_device);
	/* one gpio emulator to set codec register */

	/* one gpio emulator to set codec register */
#endif

	ret = platform_device_add(omap3stalker_snd_device);
	if (ret)
		goto err1;

	return 0;

err1:
	printk(KERN_ERR "Unable to add platform device\n");
	platform_device_put(omap3stalker_snd_device);

	return ret;
}

static void __exit omap3stalker_soc_exit(void)
{
	platform_device_unregister(omap3stalker_snd_device);
}

module_init(omap3stalker_soc_init);
module_exit(omap3stalker_soc_exit);

MODULE_AUTHOR("Jason Lam <lzg@ema-tech.com>");
MODULE_DESCRIPTION("ALSA SoC OMAP3 STALKER");
MODULE_LICENSE("GPLv2");
