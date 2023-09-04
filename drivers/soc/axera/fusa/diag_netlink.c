/*
 * SPDX-License-Identifier: GPL-2.0+
 * 
 * Diagnosis netlink
 * 
 * Copyright (C) 2023 Charleye <wangkart@aliyun.com>
 * 
 */
#define pr_fmt(fmt) "diag-netlink: " fmt

#include <linux/types.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/crc32.h>
#include <uapi/linux/diag_netlink.h>

#include "diag_core.h"

static atomic64_t diag_portid;

static int diag_netlink_notify(struct notifier_block *nb,
						unsigned long state,
						void *_notify)
{
	struct netlink_notify *n = _notify;

	if (state != NETLINK_URELEASE)
		return NOTIFY_DONE;

	if (n->protocol == NETLINK_SOC_DIAG)
		atomic64_set(&diag_portid, 0);

	return NOTIFY_DONE;
}

static struct notifier_block diag_netlink_notifier = {
	.notifier_call = diag_netlink_notify,
};

static inline uint32_t calc_crc(struct diag_msg *msg)
{
	struct diag_msg_data *data = &msg->data;
	struct diag_msg_header *head = &msg->head;

	head->checksum = crc32_le(~0, (const char *)data, sizeof(*data));

	return 0;
}

static void diag_netlink_recv_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = NULL;
	struct diag_msg *umsg;

	atomic64_set(&diag_portid, NETLINK_CB(skb).portid);

	if (skb->len >= nlmsg_total_size(0))
	{
		nlh = nlmsg_hdr(skb);
		umsg = NLMSG_DATA(nlh);

		if (umsg)
			pr_info("err ID: %d module ID: %d\n", umsg->data.err.eid, umsg->data.err.mid);
	}
}

int diag_netlink_send_msg(struct sock *sk, struct diag_error *err)
{
	struct diag_msg msg;
	struct diag_msg_header *h = &msg.head;
	struct diag_msg_data *md = &msg.data;
	struct diag_error_mgt *dem;
	struct sk_buff *nl_skb;
	struct nlmsghdr *nlh;
	int ret;

	dem = diagnosis_lookup_error(err->mid, err->eid);
	if (!dem) {
		pr_err("The error %d has been unregistered.\n", err->eid);
		return -EINVAL;
	}

	memset(&msg, 0, sizeof(msg));
	h->version = DIAG_MSG_VER;

	md->prio = dem->handle.attr.prio;
	strncpy(md->name, dem->handle.name, sizeof(md->name)-1);
	memcpy(&md->err, err, sizeof(*err));
	calc_crc(&msg);

	/* Create sk_buff space */
	nl_skb = nlmsg_new(sizeof(msg), GFP_ATOMIC);
	if (!nl_skb)
		return -ENOMEM;

	nlh = nlmsg_put(nl_skb, 0, 0, NETLINK_SOC_DIAG, sizeof(msg), 0);
	if (nlh == NULL) {
		ret = -EMSGSIZE;
		goto out;
	}

	memcpy(nlmsg_data(nlh), &msg, sizeof(msg));
	ret = netlink_unicast(sk, nl_skb, atomic64_read(&diag_portid) ? : 0, MSG_DONTWAIT);
	if (ret < 0)
		goto send_fail;

	return 0;

send_fail:
	nlmsg_cancel(nl_skb, nlh);
out:
	nlmsg_free(nl_skb);
	return ret;
}

static struct netlink_kernel_cfg cfg = {
	.input = diag_netlink_recv_msg,
};

int diag_netlink_init(struct diag_core *dc)
{
	int rc;

	dc->sk= netlink_kernel_create(&init_net, NETLINK_SOC_DIAG, &cfg);
	if (!dc->sk)
		return -EIO;

	rc = netlink_register_notifier(&diag_netlink_notifier);
	if (rc) {
		pr_err("Failed to register diag netlink notifier\n");
		return rc;
	}

	return 0;
}

