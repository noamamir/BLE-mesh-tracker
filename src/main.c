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

LOG_MODULE_REGISTER(chat, CONFIG_LOG_DEFAULT_LEVEL);
static void init_scanner(void);
static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
                    struct net_buf_simple *buf);
static void scan_cb_new(const struct bt_le_scan_recv_info *info,
		     struct net_buf_simple *buf);
static void scan_recv_cb(const struct bt_le_scan_recv_info *info, struct net_buf_simple *buf);

static void bt_ready(int err)
{
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");
     // Add a small delay before starting the scanner

    // Initialize scanner after mesh is initialized
  

    dk_leds_init();
    dk_buttons_init(NULL);
 
    err = bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init());
    if (err) {
        printk("Initializing mesh failed (err %d)\n", err);
        return;
    }

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }

    /* This will be a no-op if settings_load() loaded provisioning info */
    bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

    printk("Mesh settings initialized\n");

    static struct bt_le_scan_cb scan_callbacks = {
        .recv = scan_recv_cb,
        // You can also set .timeout if you want to handle scan timeouts
    };

    bt_le_scan_cb_register(&scan_callbacks);
    // k_sleep(K_SECONDS(4));
    // init_scanner();

   
}

int main(void)
{
	int err;

	printk("Initializing...\n");

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
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

    printk("Device found: %s (RSSI %d)\n", addr, info->rssi);

    // send_adv_message(&info);

    // Add your custom logic here to handle the scanned device
}

// Callback for handling BLE scan results
static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
                    struct net_buf_simple *buf)
{
    struct adv_device_info device;
    memcpy(device.addr, addr->a.val, 6);
    device.rssi = rssi;

	char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	printk("Device found: %s (RSSI %d)\n", addr_str, rssi);

	// printk("ADDR=%s RSSI=%d ADV=%d %s\n", addr, device_info->recv_info->rssi, device_info->recv_info->adv_props, str);
	send_adv_message(&device);
    // Publish device info to the mesh network
    // bt_mesh_model_publish(model, OP_ADV_DEVICE_REPORT, &device, sizeof(device), NULL);

}

static void init_scanner(void)
{
    struct bt_le_scan_param scan_param = {
        .type = BT_LE_SCAN_TYPE_PASSIVE,
        .options = BT_LE_SCAN_OPT_NONE,
        .interval = BT_GAP_SCAN_SLOW_INTERVAL_2,
        .window = BT_GAP_SCAN_SLOW_WINDOW_2,
    };
    int err;

    printk("Starting scanner...\n");

    // Check if scanning is already active
    if (bt_le_scan_stop() == -EALREADY) {
        printk("Scanning was not active\n");
    }

    if (!bt_is_ready()) {
        printk("Bluetooth stack not ready\n");
        return;
    }

    // Try to start scanning with the specified parameters
    err = bt_le_scan_start(&scan_param, scan_cb);
    if (err) {
        printk("Scanning failed to start (err %d)\n", err);
        
        // If the parameters are invalid, try with default parameters
        if (err == -EINVAL) {
            printk("Trying with default scan parameters...\n");
            err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, scan_cb);
            if (err) {
                printk("Default scanning failed to start (err %d)\n", err);
            } else {
                printk("Default scanning started successfully\n");
            }
        }
    } else {
        printk("Scanning started successfully\n");
    }
}

