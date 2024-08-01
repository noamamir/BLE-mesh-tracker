/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/mesh.h>
#include "chat_cli.h"
#include "mesh/net.h"
#include <string.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(chat);

#define UART_DEVICE_NODE DT_NODELABEL(uart0)

static const struct device *uart_dev;


static void pass_to_computer_service(const char *uuid ,const bt_addr_t *addr, int8_t rssi);
bool is_master_device(void);
static void uart_send_string(const char *str);
static int handle_time_sync(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                            struct net_buf_simple *buf);
#define UART_DEVICE_NODE DT_NODELABEL(uart0)


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


int handle_scan_info(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                     struct net_buf_simple *buf)
{
    bt_addr_t addr;
    int8_t rssi;
 	char uuid_str[UUID_STR_LEN];

    // Extract address from the message
    memcpy(&addr, net_buf_simple_pull_mem(buf, BT_ADDR_SIZE), BT_ADDR_SIZE);

    // Extract RSSI from the message
    rssi = net_buf_simple_pull_u8(buf);

    // Convert address to string for logging
    char addr_str[BT_ADDR_STR_LEN];
    bt_addr_to_str(&addr, addr_str, sizeof(addr_str));

 	// Convert the source address (UUID) to string
    snprintf(uuid_str, sizeof(uuid_str), "%04x", ctx->addr);

    // Check if this device is the master device
    if (is_master_device()) {
        pass_to_computer_service(uuid_str, &addr, rssi);
    }

    return 0;
}


bool is_master_device(void)
{
#if defined(CONFIG_DEVICE_IS_MASTER)
    return true;
#else
    return false;
#endif
}




// Function to pass data to a computer service
static void pass_to_computer_service(const char *uuid ,const bt_addr_t *addr, int8_t rssi)
{
    // Implement the logic to pass the data to your computer service
    // This could involve sending data over UART, USB, or another interface


	char json_buffer[100];
    snprintf(json_buffer, sizeof(json_buffer), 
             "{\"uuid\":\"%s\",\"addr\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"rssi\":%d}\n",
             uuid,
             addr->val[5], addr->val[4], addr->val[3],
             addr->val[2], addr->val[1], addr->val[0],
             rssi);
    
	uart_send_string(json_buffer);

    // uart_tx(uart_dev, json_buffer, strlen(json_buffer), 3 *);

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
	// TODO: printk("handled message %s \n", buf);
	if (chat->handlers->message) {
		chat->handlers->message(chat, ctx, msg);
	}

	return 0;
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
    // Add address
    net_buf_simple_add_mem(buf, &info->addr->a, BT_ADDR_SIZE);

    // Add RSSI
    net_buf_simple_add_u8(buf, info->rssi);
	
    return bt_mesh_model_publish(chat->model);
}

static int handle_time_sync(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                            struct net_buf_simple *buf)
{
    struct bt_mesh_chat_cli *chat = model->rt->user_data;
    uint64_t sync_time;

    sync_time = net_buf_simple_pull_le64(buf);

	chat->sync_time = sync_time;
    
    if (chat->handlers->time_sync) {
        chat->handlers->time_sync(chat, ctx, &sync_time);
    }

    return 0;
}

int bt_mesh_chat_cli_time_sync_send(struct bt_mesh_chat_cli *chat, uint64_t time)
{
    struct net_buf_simple *msg = chat->model->pub->msg;

    bt_mesh_model_msg_init(msg, BT_MESH_CHAT_CLI_OP_TIME_SYNC);
    net_buf_simple_add_le64(msg, time);

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
