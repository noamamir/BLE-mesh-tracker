/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file
 * @brief Model handler
 */

#ifndef MODEL_HANDLER_H__
#define MODEL_HANDLER_H__

#include <zephyr/bluetooth/mesh.h>

#ifdef __cplusplus
extern "C" {
#endif

// Structure to hold advertising device info
struct adv_device_info {
    uint8_t addr[6];
    int8_t rssi;
};


const struct bt_mesh_comp *model_handler_init(void);
void send_adv_message(const struct bt_le_scan_recv_info *device);
void initiate_time_sync(uint64_t *time);

#ifdef __cplusplus
}
#endif

#endif /* MODEL_HANDLER_H__ */
