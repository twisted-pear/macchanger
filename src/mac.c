/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* MAC Changer
 *
 * Authors:
 *      Alvaro Lopez Ortega <alvaro@alobbs.com>
 *
 * Copyright (C) 2002,2013 Alvaro Lopez Ortega
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mac.h"
#include "common.h"


mac_t *
mc_mac_dup (const mac_t *mac)
{
	mac_t *new;

	new = (mac_t *)xmalloc(sizeof(mac_t));
	memcpy (new, mac, sizeof(mac_t));
	return new;
}


void
mc_mac_free (mac_t *mac)
{
	free (mac);
}


void
mc_mac_into_string (const mac_t *mac, char *s)
{
	int i;

	for (i=0; i<6; i++) {
		sprintf (&s[i*3], "%02x%s", mac->byte[i], i<5?":":"");
	}
}


void
mc_mac_random (mac_t *mac, unsigned char last_n_bytes, char set_bia)
{
	/* The LSB of first octet can not be set.  Those are musticast
	 * MAC addresses and not allowed for network device:
	 * x1:, x3:, x5:, x7:, x9:, xB:, xD: and xF:
	 */
	unsigned char random_data[6];
	if (strong_random_get(random_data, sizeof(random_data)) != 0) {
		fatal("Failed to get random MAC.");
	}

	switch (last_n_bytes) {
	case 6:
		/* 8th bit: Unicast / Multicast address
		 * 7th bit: BIA (burned-in-address) / locally-administered
		 */
		mac->byte[0] = random_data[0] & 0xFC;
		mac->byte[1] = random_data[1];
		mac->byte[2] = random_data[2];
	case 3:
		mac->byte[3] = random_data[3];
		mac->byte[4] = random_data[4];
		mac->byte[5] = random_data[5];
	}

	/* Handle the burned-in-address bit
	 */
	if (set_bia) {
		mac->byte[0] &= ~2;
	} else {
		mac->byte[0] |= 2;
	}
}


int
mc_mac_equal (const mac_t *mac1, const mac_t *mac2)
{
	int i;

	for (i=0; i<6; i++) {
		if (mac1->byte[i] != mac2->byte[i]) {
			return 0;
		}
	}
	return 1;
}


int
mc_mac_read_string (mac_t *mac, char *string)
{
	int nbyte = 5;

	/* Check the format */
	if (strlen(string) != 17) {
		error ("Incorrect format: MAC length should be 17. %s(%lu)", string, strlen(string));
		return -1;
	}

	for (nbyte=2; nbyte<16; nbyte+=3) {
		if (string[nbyte] != ':') {
			error ("Incorrect format: %s", string);
			return -1;
		}
	}

	/* Read the values */
	for (nbyte=0; nbyte<6; nbyte++) {
		mac->byte[nbyte] = (char) (strtoul(string+nbyte*3, 0, 16) & 0xFF);
	}

	return 0;
}
