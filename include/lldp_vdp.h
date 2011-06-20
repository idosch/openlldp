/*******************************************************************************

  implementation of according to IEEE 802.1Qbg
  (c) Copyright IBM Corp. 2010

  Author(s): Jens Osterkamp <jens at linux.vnet.ibm.com>

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

*******************************************************************************/

#ifndef _LLDP_VDP_H
#define _LLDP_VDP_H

#include "lldp_mod.h"
#include "ecp/ecp.h"

#define LLDP_MOD_VDP		(OUI_IEEE_8021Qbg + 1)

#define VDP_MODE_PREASSOCIATE		0x0
#define VDP_MODE_PREASSOCIATE_WITH_RR	0x1
#define VDP_MODE_ASSOCIATE		0x2
#define VDP_MODE_DEASSOCIATE		0x3

#define VDP_RESPONSE_SUCCESS		0x0
#define VDP_RESPONSE_INVALID_FORMAT	0x1
#define VDP_RESPONSE_INSUFF_RESOURCES	0x2
#define VDP_RESPONSE_UNUSED_VTID	0x3
#define VDP_RESPONSE_VTID_VIOLATION	0x4
#define VDP_RESPONSE_VTID_VER_VIOLATION	0x5
#define VDP_RESPONSE_OUT_OF_SYNC	0x6
#define VDP_RESPONSE_NO_RESPONSE	0xff

extern const char * const vsi_responses[];
extern const char * const vsi_states[];

#define VDP_MACVLAN_FORMAT_1	1

#define VDP_TIMER_GRANULARITY		100*MSECS /* 100 ms */
#define VDP_KEEPALIVE_TIMER_DEFAULT	10*SECS  /* 10s */
#define VDP_ACK_TIMER_DEFAULT		(2*ECP_ACK_TIMER_DEFAULT*ECP_MAX_RETRIES)
#define VDP_KEEPALIVE_TIMER_STOPPED	-1
#define VDP_ACK_TIMER_STOPPED		-1
#define VDP_LOCALCHANGE_TIMEOUT		1*MSECS /* 1 ms */

#define VDP_ROLE_STATION		0
#define VDP_ROLE_BRIDGE			1

enum {
	VSI_UNASSOCIATED = 0,
	VSI_ASSOC_PROCESSING,
	VSI_ASSOCIATED,
	VSI_PREASSOC_PROCESSING,
	VSI_PREASSOCIATED,
	VSI_DEASSOC_PROCESSING,
	VSI_EXIT,
};

struct mac_vlan {
	u8 mac[6];
	u16 vlan;
} __attribute__ ((__packed__));

struct tlv_info_vdp {
	u8 oui[3];
	u8 sub;
	u8 mode;
	u8 response;
	u8 mgrid;
	u8 id[3];
	u8 version;
	u8 instance[16];
	u8 format;
	u16 entries;
	struct mac_vlan mac_vlan;
} __attribute__ ((__packed__));

struct vsi_profile {
	int mode;
	int response;
	u8 mgrid;
	int id;
	u8 version;
	u8 instance[16];
	u8 mac[6]; /* TODO: currently only one MAC/VLAN pair supported, more later */
	u16 vlan;
	struct port *port;
	int ackTimer;
	int ackReceived;
	int keepaliveTimer;
	int state;
	int seqnr;
	bool localChange;
	bool remoteChange;
	LIST_ENTRY(vsi_profile) profile;
};

struct vdp_data {
	char ifname[IFNAMSIZ];
	struct ecp ecp;
	struct unpacked_tlv *vdp;
	int role;
	int keepaliveTimer;
	int ackTimer;
	int nroftimers;
	LIST_HEAD(profile_head, vsi_profile) profile_head;
	LIST_ENTRY(vdp_data) entry;
};

struct vdp_user_data {
	LIST_HEAD(vdp_head, vdp_data) head;
};

struct lldp_module *vdp_register(void);
void vdp_unregister(struct lldp_module *mod);
struct vdp_data *vdp_data(char *ifname);
struct packed_tlv *vdp_gettlv(struct vdp_data *vd, struct vsi_profile *profile);
void vdp_vsi_sm_station(struct vsi_profile *profile);
struct vsi_profile *vdp_add_profile(struct vsi_profile *profile);
void vdp_somethingChangedLocal(struct vsi_profile *profile, bool mode);

void vdp_ack_profiles(struct vdp_data *vd, int seqnr);
int vdp_indicate(struct vdp_data *vd, struct unpacked_tlv *tlv, int ecp_mode);
int vdp_vsis_pending(struct vdp_data *vd);
int vdp_vsis(char *ifname);
void vdp_print_profile(struct vsi_profile *);
void ecp_somethingChangedLocal(struct vdp_data *vd, bool flag);
void ecp_rx_send_ack_frame(struct vdp_data *vd);
int instance2str(const u8 *p, char *dst, size_t size);

#define MAC_ADDR_STRLEN		18
#define INSTANCE_STRLEN		36

#endif /* _LLDP_VDP_H */
