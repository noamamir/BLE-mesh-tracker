/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Nordic Mesh light sample
 */
#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <bluetooth/mesh/dk_prov.h>
#include <dk_buttons_and_leds.h>
#include "model_handler.h"
#include "chat_cli.h"
#include <zephyr/logging/log.h>
#include <bluetooth/scan.h>
#include <mesh/mesh.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <string.h>
#include <stdlib.h>

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_console)
#define UART_BUF_SIZE 256

static const struct device *uart_dev;
static uint8_t uart_rx_buf[UART_BUF_SIZE];
static uint8_t uart_tx_buf[UART_BUF_SIZE];

LOG_MODULE_REGISTER(chat, CONFIG_LOG_DEFAULT_LEVEL);
// static void init_scanner(void);
static void scan_recv_cb(const struct bt_le_scan_recv_info *info, struct net_buf_simple *buf);
static void uart_cb(const struct device *dev, void *user_data);

static void uart_cb(const struct device *dev, void *user_data)
{
    uint8_t c;

    if (!uart_irq_update(dev)) {
        return;
    }

    static uint8_t rx_buf[UART_BUF_SIZE];
    static int rx_buf_pos;

    while (uart_irq_rx_ready(dev)) {
        uart_fifo_read(dev, &c, 1);

        if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
            // Null terminate the string
            rx_buf[rx_buf_pos] = '\0';
            
            // Process the command
            if (strncmp((char *)rx_buf, "SYNC_TIME:", 10) == 0) {
                uint64_t time = strtoull((char *)rx_buf + 10, NULL, 10);
                initiate_time_sync(time);
                
                // Send acknowledgement
                char ack[] = "Time sync received\n";
                uart_fifo_fill(dev, ack, strlen(ack));
            }
            
            // Reset buffer
            rx_buf_pos = 0;
        } else if (rx_buf_pos < sizeof(rx_buf) - 1) {
            rx_buf[rx_buf_pos++] = c;
        }
    }
}

static void bt_ready(int err)
{
    if (err) {
        LOG_INF("Bluetooth init failed (err %d)\n", err);
        return;
    }

    LOG_INF("Bluetooth initialized\n");
     // Add a small delay before starting the scanner

    // Initialize scanner after mesh is initialized
  

    dk_leds_init();
    dk_buttons_init(NULL);
 
    err = bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init());
    if (err) {
        LOG_WRN("Initializing mesh failed (err %d)\n", err);
        return;
    }

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }

    /* This will be a no-op if settings_load() loaded provisioning info */
    bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

    LOG_INF("Mesh settings initialized\n");

    static struct bt_le_scan_cb scan_callbacks = {
        .recv = scan_recv_cb,
        // You can also set .timeout if you want to handle scan timeouts
    };

    bt_le_scan_cb_register(&scan_callbacks);
}

static int uart_init(void)
{
    uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);
    if (!device_is_ready(uart_dev)) {
        printk("UART device not found!\n");
        return -ENODEV;
    }

    uart_irq_callback_user_data_set(uart_dev, uart_cb, NULL);
    uart_irq_rx_enable(uart_dev);

    return 0;
}

int main(void)
{
	int err;
	LOG_INF("Initializing...\n");
    
    err = uart_init();
    if (err) {
        LOG_WRN("UART init failed (err %d)\n", err);
        return 0;
    }


	err = bt_enable(bt_ready);
	if (err) {
		LOG_WRN("Bluetooth init failed (err %d)\n", err);
	}

    if (is_master_device()) {
        LOG_INF("Device is master device \n");
    } else {
        LOG_INF("Device is slave device \n");
    }

	 // Keep the main thread running
    while (1) {
        k_sleep(K_SECONDS(1));
    }

	return 0;
}

static void scan_recv_cb(const struct bt_le_scan_recv_info *info, struct net_buf_simple *buf)
{
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(info->addr, addr, sizeof(addr));


    if (addr[0] == 'F' && addr[1] == 'F' && addr[3] == 'F' && addr[4] == 'F') {
        if (!is_master_device()) {
            LOG_INF("Device found: %s (RSSI %d)\n", addr, info->rssi);
        }
        send_adv_message(info);
    }
}



// static void init_scanner(void)
// {
//     struct bt_le_scan_param scan_param = {
//         .type = BT_LE_SCAN_TYPE_PASSIVE,
//         .options = BT_LE_SCAN_OPT_NONE,
//         .interval = BT_GAP_SCAN_SLOW_INTERVAL_2,
//         .window = BT_GAP_SCAN_SLOW_WINDOW_2,
//     };
//     int err;

//     printk("Starting scanner...\n");

//     // Check if scanning is already active
//     if (bt_le_scan_stop() == -EALREADY) {
//         printk("Scanning was not active\n");
//     }

//     if (!bt_is_ready()) {
//         printk("Bluetooth stack not ready\n");
//         return;
//     }

//     // Try to start scanning with the specified parameters
//     // err = bt_le_scan_start(&scan_param, scan_cb);
//     // if (err) {
//     //     printk("Scanning failed to start (err %d)\n", err);
        
//     //     // If the parameters are invalid, try with default parameters
//     //     if (err == -EINVAL) {
//     //         printk("Trying with default scan parameters...\n");
//     //         err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, scan_cb);
//     //         if (err) {
//     //             printk("Default scanning failed to start (err %d)\n", err);
//     //         } else {
//     //             printk("Default scanning started successfully\n");
//     //         }
//     //     }
//     // } else {
//     //     printk("Scanning started successfully\n");
//     // }
// }

