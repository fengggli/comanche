/*
 * Copyright (c) 2005 Topspin Communications.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __RDMA_HELPER_H__
#define __RDMA_HELPER_H__


#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <vector>
#include <infiniband/verbs.h>
#include <infiniband/verbs_exp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <malloc.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <time.h>

  
enum {
	CHANNEL_RECV_WRID = 1,
	CHANNEL_SEND_WRID = 2,
};

struct buffer_def
{
  struct ibv_mr *mr;
  void *addr;
  size_t len;
};
  
struct channel_context {
	struct ibv_context	*context;
	struct ibv_comp_channel *channel;
  enum ibv_qp_type qtype;
	struct ibv_pd		*pd;
	struct ibv_cq		*cq;
	struct ibv_qp		*qp;
  //  struct ibv_mr * mr; /**< primary memory region */
  //  void * buf;
  size_t size;
	int			 rx_depth;
	int			 pending;
	struct ibv_port_attr	 portinfo;
	int			 inlr_recv;
};

struct channel_dest {
	int lid;
	int qpn;
	int psn;
	union ibv_gid gid;
};

int channel_connect_ctx(struct channel_context *ctx,
                        int port,
                        int my_psn,
                        enum ibv_mtu mtu,
                        int sl,
                        struct channel_dest *dest,
                        int sgid_idx);

struct channel_context *channel_init_ctx(struct ibv_device *ib_dev,
                                         enum ibv_qp_type qtype,
                                         unsigned long long size,
                                         int rx_depth,
                                         int tx_depth,
                                         int port,
                                         int use_event,
                                         int inlr_recv);

struct channel_dest *channel_server_exch_dest(struct channel_context *ctx,
                                              int ib_port, enum ibv_mtu mtu,
                                              int port, int sl,
                                              const struct channel_dest *my_dest,
                                              int sgid_idx);

struct channel_dest *channel_client_exch_dest(const char *servername, int port,
                                              const struct channel_dest *my_dest);
  

int channel_close_ctx(struct channel_context *ctx);

int channel_post_recv(struct channel_context *ctx,
                      uint64_t issue_id,
                      struct ibv_mr * mr);

// int channel_post_recv(struct channel_context *ctx,
//                       uint64_t issue_id,
//                       struct ibv_mr * mr);

// int channel_post_send_trim(struct channel_context *ctx,
// 			   uint64_t issue_id,
// 			   size_t first_sge_len,
// 			   struct ibv_mr * extra_mr,
// 			   size_t offet = 0,
// 			   size_t length = 0);

int channel_post_send(struct channel_context *ctx,
                      uint64_t issue_id,
                      struct ibv_mr * mr0,
                      struct ibv_mr * extra_mr);


struct ibv_mr * channel_register_memory(struct channel_context *ctx, void * contig_addr, size_t len);

enum ibv_mtu rdma_mtu_to_enum(int mtu);
uint16_t rdma_get_local_lid(struct ibv_context *context, int port);
int rdma_get_port_info(struct ibv_context *context, int port,
		     struct ibv_port_attr *attr);
void wire_gid_to_gid(const char *wgid, union ibv_gid *gid);
void gid_to_wire_gid(const union ibv_gid *gid, char wgid[]);


#endif
