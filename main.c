/*
 * newtest.c
 *
 * Copyright (c) 2014 Jeremy Garff <jer @ jers.net>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     1.  Redistributions of source code must retain the above copyright notice, this list of
 *         conditions and the following disclaimer.
 *     2.  Redistributions in binary form must reproduce the above copyright notice, this list
 *         of conditions and the following disclaimer in the documentation and/or other materials
 *         provided with the distribution.
 *     3.  Neither the name of the owner nor the names of its contributors may be used to endorse
 *         or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdarg.h>
#include <getopt.h>

#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"
#include "version.h"

#include "ws2811.h"

#define GPIO_PIN 18
#define DMA 10
#define MAX_LEDS 255
#define STRIP_TYPE WS2811_STRIP_GBR

ws2811_t ledstring =
	{
		.freq = WS2811_TARGET_FREQ,
		.dmanum = DMA,
		.channel =
			{
				[0] =
					{
						.gpionum = GPIO_PIN,
						.count = 0,
						.invert = 0,
						.brightness = 255,
						.strip_type = STRIP_TYPE,
					},
				[1] =
					{
						.gpionum = 0,
						.count = 0,
						.invert = 0,
						.brightness = 0,
					},
			},
};

void set_color(const char *leds, size_t leds_count, uint32_t color)
{
	printf("setting color %X to leds ",color);
	for (size_t i = 0; i < leds_count; i++)
	{
		if (leds[i]!='0'){
			printf("1");
			ledstring.channel[0].leds[i] = color;
		} else {
			printf("0");
		}
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("ledstrip %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
		printf("led strip should be connected to GPIO PIN %d. DMA: 0x%X. strip type: 0x%X \n", GPIO_PIN, STRIP_TYPE);
		printf("Usage: [color] [leds]\n");
		printf("Where:\n");
		printf("\tcolor is (0xWWRRGGBB), but in decimal form\n");
		printf("\tleds is a string representing the leds to lit: try: 010101\n");
		return EXIT_SUCCESS;
	}

	uint32_t color = atoi(argv[1]);
	const char *leds = argv[2];

	int leds_count = strnlen(leds, MAX_LEDS);

	printf("color %d, leds: %s\n", color, leds);
	if (leds_count < 1)
	{
		return EXIT_SUCCESS;
	}

	ledstring.channel[0].count = leds_count;

	ws2811_return_t ret = ws2811_init(&ledstring);
	if (ret != WS2811_SUCCESS)
	{
		fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
		return ret;
	}

	set_color(leds, leds_count, color);
	ret = ws2811_render(&ledstring);
	if (ret != WS2811_SUCCESS)
	{
		fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
	}
	ws2811_fini(&ledstring);
	return ret;
}
