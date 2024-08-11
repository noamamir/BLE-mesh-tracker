/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/mesh.h>
#include "chat_cli.h"
#include "mesh/net.h"
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include <inttypes.h>
LOG_MODULE_DECLARE(chat);


//#if defined(CONFIG_COUNTER_NRF_RTC)
#define TIMER DT_ALIAS(rtc2)
// #else
// #error "Unsupported board: This sample requires a counter device."
// #endif

// static const struct device *uart_dev;


static void pass_tag_msg_to_pc(const char *uuid, const bt_addr_t *addr, int8_t rssi, uint64_t time_sent);
static void pass_heartbeat_msg_to_pc(const char *uuid ,const uint16_t device_id, const uint64_t time_sent);

bool is_master_device(void);
static void uart_send_string(const char *str);
static void init_counter(struct bt_mesh_chat_cli *chat);
uint64_t get_current_time(struct device *counter_dev);

BUILD_ASSERT(BT_MESH_MODEL_BUF_LEN(BT_MESH_CHAT_CLI_OP_MESSAGE,
				   BT_MESH_CHAT_CLI_MSG_MAXLEN_MESSAGE) <=
		    BT_MESH_RX_SDU_MAX,
	     "The message must fit inside an application SDU.");
BUILD_ASSERT(BT_MESH_MODEL_BUF_LEN(BT_MESH_CHAT_CLI_OP_MESSAGE,
				   BT_MESH_CHAT_CLI_MSG_MAXLEN_MESSAGE) <=
		    BT_MESH_TX_SDU_MAX,
	     "The message must fit inside an application SDU.");

static void encode_presence(struct net_buf_simple *buf,
			    enum bt_mesh_chat_cli_presence presence)
{
	bt_mesh_model_msg_init(buf, BT_MESH_CHAT_CLI_OP_PRESENCE);
	net_buf_simple_add_u8(buf, presence);
}

static const uint8_t *extract_msg(struct net_buf_simple *buf)
{
	buf->data[buf->len - 1] = '\0';
	return net_buf_simple_pull_mem(buf, buf->len);
}

bool is_master_device(void)
{
#if defined(CONFIG_DEVICE_IS_MASTER)
    return true;
#else
    return false;
#endif
}

static void pass_tag_msg_to_pc(const char *uuid, const bt_addr_t *addr, int8_t rssi, uint64_t time_sent)
{
    char json_buffer[180];  // Increased buffer size to accommodate larger time value
    snprintf(json_buffer, sizeof(json_buffer),
             "{\"uuid\":\"%s\",\"addr\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"rssi\":%d,\"time_sent\":%" PRIu64 "}\n",
             uuid,
             addr->val[5], addr->val[4], addr->val[3],
             addr->val[2], addr->val[1], addr->val[0],
             rssi,
             time_sent);

    printk("TAG: %s", json_buffer);
}

// Function to pass hearbeat msg to a computer service
static void pass_heartbeat_msg_to_pc(const char *uuid ,const uint16_t device_id, const uint64_t time_sent)
{
    // Implement the logic to pass the data to your computer service
    // This could involve sending data over UART, USB, or another interface

	char json_buffer[100];
    int written = snprintf(json_buffer, sizeof(json_buffer), 
             "{\"uuid\":\"%s\",\"device_id\":%u,\"time_sent\":%" PRIu64 "}\n",
             uuid,
             device_id,
             time_sent);
    
    
	printk("HEARTBEAT: %s", json_buffer);


}


static void uart_send_string(const char *str)
{
    printk("To-Service: %s", str);
}


static int handle_message(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx, struct net_buf_simple *buf)
{
	struct bt_mesh_chat_cli *chat = model->rt->user_data;
	const uint8_t *msg;

	msg = extract_msg(buf);
	if (chat->handlers->message) {
		chat->handlers->message(chat, ctx, msg);
	}

	return 0;
}

int handle_scan_info(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                     struct net_buf_simple *buf)
{
    bt_addr_t addr;
    int8_t rssi;
    char uuid_str[UUID_STR_LEN];
    uint64_t time_sent;

    // Extract address from the message
    memcpy(&addr, net_buf_simple_pull_mem(buf, BT_ADDR_SIZE), BT_ADDR_SIZE);

    // Extract RSSI from the message
    rssi = net_buf_simple_pull_u8(buf);

    // Extract time sent from the message
    time_sent = net_buf_simple_pull_le64(buf);

    // Convert address to string for logging
    char addr_str[BT_ADDR_STR_LEN];
    bt_addr_to_str(&addr, addr_str, sizeof(addr_str));

    // Convert the source address (UUID) to string
    snprintf(uuid_str, sizeof(uuid_str), "%04x", ctx->addr);

    // Check if this device is the master device
    if (is_master_device()) {
        pass_tag_msg_to_pc(uuid_str, &addr, rssi, time_sent);
    }

    return 0;
}

static int handle_heartbeat(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                     struct net_buf_simple *buf)
{
	uint16_t device_id;
	uint64_t time_sent;
	char uuid_str[UUID_STR_LEN];

  // Extract time_sent from the message (8 bytes)
    time_sent = net_buf_simple_pull_le64(buf);

    // Extract device_id from the message (2 bytes)
    device_id = net_buf_simple_pull_le16(buf);

    // Convert the source address (UUID) to string
    snprintf(uuid_str, sizeof(uuid_str), "%04x", ctx->addr);

	// Check if this device is the master device
    if (is_master_device()) {
       	pass_heartbeat_msg_to_pc(uuid_str, device_id, time_sent);
    }

	return 0;
}


static int handle_time_sync(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                            struct net_buf_simple *buf)
{
    struct bt_mesh_chat_cli *chat = model->rt->user_data;
    uint64_t sync_time;

    sync_time = net_buf_simple_pull_le64(buf);

	chat->sync_time = sync_time;
    
	init_counter(chat);
	

    return 0;
}

static void init_counter(struct bt_mesh_chat_cli *chat) {
	chat->counter_dev = DEVICE_DT_GET(TIMER);
    if (!device_is_ready(chat->counter_dev)) {
        LOG_ERR("Counter device not ready");
        return;
    }

	int err = counter_start(chat->counter_dev);
    if (err != 0) {
        LOG_ERR("Failed to start counter: %d", err);
        return;
    }
}

/* .. include_startingpoint_chat_cli_rst_2 */
const struct bt_mesh_model_op _bt_mesh_chat_cli_op[] = {
	{
        BT_MESH_CHAT_CLI_OP_SCAN_INFO,
        BT_MESH_LEN_MIN(BT_MESH_CHAT_CLI_MSG_LEN_SCAN_INFO),
        handle_scan_info
    },
	{
		BT_MESH_CHAT_CLI_OP_MESSAGE,
		BT_MESH_LEN_MIN(BT_MESH_CHAT_CLI_MSG_MINLEN_MESSAGE),
		handle_message
	},
	{
		BT_MESH_CHAT_CLI_OP_TIME_SYNC,
		BT_MESH_LEN_EXACT(BT_MESH_CHAT_CLI_MSG_LEN_TIME_SYNC),
		handle_time_sync
	},
	{
		BT_MESH_CHAT_CLI_OP_HEARTBEAT,
		BT_MESH_LEN_EXACT(BT_MESH_CHAT_CLI_MSG_LEN_HEART_BEAT),
		handle_heartbeat
	},

	BT_MESH_MODEL_OP_END,
};
/* .. include_endpoint_chat_cli_rst_2 */

static int bt_mesh_chat_cli_update_handler(const struct bt_mesh_model *model)
{
	struct bt_mesh_chat_cli *chat = model->rt->user_data;

	/* Continue publishing current presence. */
	encode_presence(model->pub->msg, chat->presence);

	return 0;
}

/* .. include_startingpoint_chat_cli_rst_3 */
#ifdef CONFIG_BT_SETTINGS
static int bt_mesh_chat_cli_settings_set(const struct bt_mesh_model *model,
					 const char *name,
					 size_t len_rd,
					 settings_read_cb read_cb,
					 void *cb_arg)
{
	struct bt_mesh_chat_cli *chat = model->rt->user_data;

	if (name) {
		return -ENOENT;
	}

	ssize_t bytes = read_cb(cb_arg, &chat->presence,
				sizeof(chat->presence));
	if (bytes < 0) {
		return bytes;
	}

	if (bytes != 0 && bytes != sizeof(chat->presence)) {
		return -EINVAL;
	}

	return 0;
}
#endif
/* .. include_endpoint_chat_cli_rst_3 */

/* .. include_startingpoint_chat_cli_rst_4 */
static int bt_mesh_chat_cli_init(const struct bt_mesh_model *model)
{
	struct bt_mesh_chat_cli *chat = model->rt->user_data;

	chat->model = model;

	net_buf_simple_init_with_data(&chat->pub_msg, chat->buf,
				      sizeof(chat->buf));
	chat->pub.msg = &chat->pub_msg;
	chat->pub.update = bt_mesh_chat_cli_update_handler;

	return 0;
}
/* .. include_endpoint_chat_cli_rst_4 */

/* .. include_startingpoint_chat_cli_rst_5 */
static int bt_mesh_chat_cli_start(const struct bt_mesh_model *model)
{
	struct bt_mesh_chat_cli *chat = model->rt->user_data;

	if (chat->handlers->start) {
		chat->handlers->start(chat);
	}

	return 0;
}
/* .. include_endpoint_chat_cli_rst_5 */

/* .. include_startingpoint_chat_cli_rst_6 */
static void bt_mesh_chat_cli_reset(const struct bt_mesh_model *model)
{
	struct bt_mesh_chat_cli *chat = model->rt->user_data;

	chat->presence = BT_MESH_CHAT_CLI_PRESENCE_AVAILABLE;

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		(void) bt_mesh_model_data_store(model, true, NULL, NULL, 0);
	}
}
/* .. include_endpoint_chat_cli_rst_6 */

/* .. include_startingpoint_chat_cli_rst_7 */
const struct bt_mesh_model_cb _bt_mesh_chat_cli_cb = {
	.init = bt_mesh_chat_cli_init,
	.start = bt_mesh_chat_cli_start,
#ifdef CONFIG_BT_SETTINGS
	.settings_set = bt_mesh_chat_cli_settings_set,
#endif
	.reset = bt_mesh_chat_cli_reset,
};
/* .. include_endpoint_chat_cli_rst_7 */

/* .. include_startingpoint_chat_cli_rst_8 */
int bt_mesh_chat_cli_presence_set(struct bt_mesh_chat_cli *chat,
			 enum bt_mesh_chat_cli_presence presence)
{
	if (presence != chat->presence) {
		chat->presence = presence;

		if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
			(void) bt_mesh_model_data_store(chat->model, true,
						NULL, &presence,
						sizeof(chat->presence));
		}
	}

	encode_presence(chat->model->pub->msg, chat->presence);

	return bt_mesh_model_publish(chat->model);
}
/* .. include_endpoint_chat_cli_rst_8 */

int bt_mesh_chat_cli_presence_get(struct bt_mesh_chat_cli *chat,
				  uint16_t addr)
{
	struct bt_mesh_msg_ctx ctx = {
		.addr = addr,
		.app_idx = chat->model->keys[0],
		.send_ttl = BT_MESH_TTL_DEFAULT,
		.send_rel = true,
	};

	BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_CHAT_CLI_OP_PRESENCE_GET,
				 BT_MESH_CHAT_CLI_MSG_LEN_PRESENCE_GET);
	bt_mesh_model_msg_init(&buf, BT_MESH_CHAT_CLI_OP_PRESENCE_GET);

	return bt_mesh_model_send(chat->model, &ctx, &buf, NULL, NULL);
}



int bt_mesh_chat_cli_message_send(struct bt_mesh_chat_cli *chat,
				  const uint8_t *msg)
{
	struct net_buf_simple *buf = chat->model->pub->msg;

	bt_mesh_model_msg_init(buf, BT_MESH_CHAT_CLI_OP_MESSAGE);

	net_buf_simple_add_mem(buf, msg,
			       strnlen(msg,
				       CONFIG_BT_MESH_CHAT_CLI_MESSAGE_LENGTH));
	net_buf_simple_add_u8(buf, '\0');

	return bt_mesh_model_publish(chat->model);
}

int bt_mesh_chat_cli_send_scan_info(struct bt_mesh_chat_cli *chat,
                                    const struct bt_le_scan_recv_info *info)
{
    struct net_buf_simple *buf = chat->model->pub->msg;
    bt_mesh_model_msg_init(buf, BT_MESH_CHAT_CLI_OP_SCAN_INFO);

    uint64_t now_ms = get_current_time(chat->counter_dev);

    // Add address
    net_buf_simple_add_mem(buf, &info->addr->a, BT_ADDR_SIZE);

    // Add RSSI
    net_buf_simple_add_u8(buf, info->rssi);

    // Add time sent
    net_buf_simple_add_le32(buf, now_ms);

    return bt_mesh_model_publish(chat->model);
}

int bt_mesh_chat_cli_time_sync_send(struct bt_mesh_chat_cli *chat, uint64_t *time)
{
    struct net_buf_simple *msg = chat->model->pub->msg;

    bt_mesh_model_msg_init(msg, BT_MESH_CHAT_CLI_OP_TIME_SYNC);
    net_buf_simple_add_le64(msg, *time);

    return bt_mesh_model_publish(chat->model);
}

int bt_mesh_chat_cli_send_heartbeat(struct bt_mesh_chat_cli *chat)
{
	struct bt_mesh_heartbeat_msg heartbeat = {
	.device_id = chat -> model->id,
	.time_sent = get_current_time((struct device *)chat->counter_dev)
	};

    struct net_buf_simple *buf = chat->model->pub->msg;
    bt_mesh_model_msg_init(buf, BT_MESH_CHAT_CLI_OP_HEARTBEAT);
    
    // Add time sent
    net_buf_simple_add_le32(buf, heartbeat.time_sent);


    // Add device id
    net_buf_simple_add_le16(buf, heartbeat.device_id);
	
    return bt_mesh_model_publish(chat->model);
}


/* .. include_startingpoint_chat_cli_rst_9 */
int bt_mesh_chat_cli_private_message_send(struct bt_mesh_chat_cli *chat,
					  uint16_t addr,
					  const uint8_t *msg)
{
	struct bt_mesh_msg_ctx ctx = {
		.addr = addr,
		.app_idx = chat->model->keys[0],
		.send_ttl = BT_MESH_TTL_DEFAULT,
		.send_rel = true,
	};

	BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_CHAT_CLI_OP_PRIVATE_MESSAGE,
				 BT_MESH_CHAT_CLI_MSG_MAXLEN_MESSAGE);
	bt_mesh_model_msg_init(&buf, BT_MESH_CHAT_CLI_OP_PRIVATE_MESSAGE);

	net_buf_simple_add_mem(&buf, msg,
			       strnlen(msg,
				       CONFIG_BT_MESH_CHAT_CLI_MESSAGE_LENGTH));
	net_buf_simple_add_u8(&buf, '\0');

	return bt_mesh_model_send(chat->model, &ctx, &buf, NULL, NULL);
}
/* .. include_endpoint_chat_cli_rst_9 */
// Update the get_current_time function
uint64_t get_current_time(struct device *counter_dev)  // Changed return type to uint64_t
{
    int err;
    uint32_t now_ticks;
    uint64_t now_us;
    uint64_t now_ms;  

    err = counter_get_value(counter_dev, &now_ticks);
    if (err) {
        LOG_ERR("Failed to read counter value (err %d)", err);
        return 0;
    }

    now_us = counter_ticks_to_us(counter_dev, now_ticks);
    now_ms = now_us / 1000;

    return now_ms;
}