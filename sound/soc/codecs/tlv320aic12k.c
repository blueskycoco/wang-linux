/*
 * tlv320aic12k.c  --  ALSA SoC TLV320AIC12K Bluetooth codec driver for omap3evm board
 *
 * Author: Sinoj M. Issac, <sinoj at mistralsolutions.com>
 *
 * Based on sound/soc/codecs/twl4030.c by Steve Sakoman
 *
 * This file provides stub codec that can be used on OMAP3530 evm to
 * send/receive voice samples to/from TLV320AIC12K Bluetooth chip over PCM interface.
 * The Bluetoothchip codec interface is configured by HCI commands. ALSA is
 * configured and aligned to the codec interface.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <sound/soc.h>
#include <sound/pcm.h>

/*
 * Since TLV320AIC12K PCM interface is intended for Voice,
 * Support sampling rate 8K only
 */
#define AIC12K_RATES		SNDRV_PCM_RATE_8000
#define AIC12K_FORMATS	SNDRV_PCM_FMTBIT_S16_LE

struct snd_soc_dai tlv320aic12k_dai = {
	.name = "tlv320aic12k",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 1,
		.rates = AIC12K_RATES,
		.formats = AIC12K_FORMATS,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 1,
		.rates = AIC12K_RATES,
		.formats = AIC12K_FORMATS,},
};

static int __init tlv320aic12k_modinit(void)
{
	/* Register number of DAIs (tlv320aic12k_dai) with the ASoC core */
	return snd_soc_register_dais(&tlv320aic12k_dai, 1);
}

static void __exit tlv320aic12k_modexit(void)
{
	/* Unregister number of DAIs (tlv320aic12k_dai) from the ASoC core */
	snd_soc_unregister_dais(&tlv320aic12k_dai, 1);
}

module_init(tlv320aic12k_modinit);
module_exit(tlv320aic12k_modexit);


