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
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/ethtool.h>
#include <linux/sockios.h>

#include "netinfo.h"
#include "common.h"


net_info_t *
mc_net_info_new (const char *device)
{
	net_info_t *new = (net_info_t *) xmalloc (sizeof(net_info_t));

	new->sock = socket (AF_INET, SOCK_DGRAM, 0);
	if (new->sock<0) {
		perror ("[ERROR] Socket");
		free(new);
		return NULL;
	}

	strncpy (new->dev.ifr_name, device, sizeof(new->dev.ifr_name));
	new->dev.ifr_name[sizeof(new->dev.ifr_name)-1] = '\0';
	if (ioctl(new->sock, SIOCGIFHWADDR, &new->dev) < 0) {
		perror ("[ERROR] Set device name");
		free(new);
		return NULL;
	}

	return new;
}


void
mc_net_info_free (net_info_t *net)
{
	close(net->sock);
	free(net);
}


mac_t *
mc_net_info_get_mac (const net_info_t *net)
{
	int    i;
	mac_t *new = (mac_t *) xmalloc (sizeof(mac_t));

	for (i=0; i<6; i++) {
		new->byte[i] = net->dev.ifr_hwaddr.sa_data[i] & 0xFF;
	}

	return new;
}


int
mc_net_info_set_mac (net_info_t *net, const mac_t *mac)
{
	int i;

	for (i=0; i<6; i++) {
		net->dev.ifr_hwaddr.sa_data[i] = mac->byte[i];
	}

	if (ioctl(net->sock, SIOCSIFHWADDR, &net->dev) < 0) {
		perror ("[ERROR] Could not change MAC: interface up or insufficient permissions");
		return -1;
	}

	return 0;
}

mac_t *
mc_net_info_get_permanent_mac (const net_info_t *net)
{
	int                       i;
	struct ifreq              req;
	struct ethtool_perm_addr *epa;
	mac_t                    *newmac;

	newmac = (mac_t *) xcalloc (1, sizeof(mac_t));

	epa = (struct ethtool_perm_addr*) xmalloc(sizeof(struct ethtool_perm_addr) + IFHWADDRLEN);
	epa->cmd = ETHTOOL_GPERMADDR;
	epa->size = IFHWADDRLEN;

	memcpy(&req, &(net->dev), sizeof(struct ifreq));
	req.ifr_data = (caddr_t)epa;

	if (ioctl(net->sock, SIOCETHTOOL, &req) < 0) {
		perror ("[ERROR] Could not read permanent MAC");
	} else {
		for (i=0; i<6; i++) {
			newmac->byte[i] = epa->data[i];
		}
	}

	free(epa);
	return newmac;
}
