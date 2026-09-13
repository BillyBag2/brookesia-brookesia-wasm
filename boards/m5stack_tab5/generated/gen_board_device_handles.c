/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 * SPDX-License-Identifier: LicenseRef-Espressif-Modified-MIT
 *
 * Auto-generated device handle definition file
 * DO NOT MODIFY THIS FILE MANUALLY
 *
 * See LICENSE file for details.
 */

#include <stddef.h>
#include "esp_board_device.h"
#include "dev_audio_codec.h"
#include "dev_camera.h"
#include "dev_display_lcd.h"
#include "dev_fs_fat.h"
#include "dev_gpio_expander.h"
#include "dev_lcd_touch.h"
#include "dev_ledc_ctrl.h"
#include "dev_power_ctrl.h"

// Device handle array
esp_board_device_handle_t g_esp_board_device_handles[] = {
    {
        .next = &g_esp_board_device_handles[1],
        .name = "audio_dac",
        .chip = "es8388",
        .type = "audio_codec",
        .device_handle = NULL,
        .init = dev_audio_codec_init,
        .deinit = dev_audio_codec_deinit
    },
    {
        .next = &g_esp_board_device_handles[2],
        .name = "audio_adc",
        .chip = "es7210",
        .type = "audio_codec",
        .device_handle = NULL,
        .init = dev_audio_codec_init,
        .deinit = dev_audio_codec_deinit
    },
    {
        .next = &g_esp_board_device_handles[3],
        .name = "fs_sdcard",
        .chip = NULL,
        .type = "fs_fat",
        .device_handle = NULL,
        .init = dev_fs_fat_init,
        .deinit = dev_fs_fat_deinit
    },
    {
        .next = &g_esp_board_device_handles[4],
        .name = "gpio_expander",
        .chip = "pi3ioe",
        .type = "gpio_expander",
        .device_handle = NULL,
        .init = dev_gpio_expander_init,
        .deinit = dev_gpio_expander_deinit
    },
    {
        .next = &g_esp_board_device_handles[5],
        .name = "gpio_expander_2",
        .chip = "pi3ioe",
        .type = "gpio_expander",
        .device_handle = NULL,
        .init = dev_gpio_expander_init,
        .deinit = dev_gpio_expander_deinit
    },
    {
        .next = &g_esp_board_device_handles[6],
        .name = "lcd_brightness",
        .chip = NULL,
        .type = "ledc_ctrl",
        .device_handle = NULL,
        .init = dev_ledc_ctrl_init,
        .deinit = dev_ledc_ctrl_deinit
    },
    {
        .next = &g_esp_board_device_handles[7],
        .name = "display_lcd",
        .chip = "tab5_lcd",
        .type = "display_lcd",
        .device_handle = NULL,
        .init = dev_display_lcd_init,
        .deinit = dev_display_lcd_deinit
    },
    {
        .next = &g_esp_board_device_handles[8],
        .name = "touch_power_ctrl",
        .chip = NULL,
        .type = "power_ctrl",
        .device_handle = NULL,
        .init = dev_power_ctrl_init,
        .deinit = dev_power_ctrl_deinit
    },
    {
        .next = &g_esp_board_device_handles[9],
        .name = "lcd_touch",
        .chip = "tab5_touch",
        .type = "lcd_touch",
        .device_handle = NULL,
        .init = dev_lcd_touch_init,
        .deinit = dev_lcd_touch_deinit
    },
    {
        .next = NULL,
        .name = "camera",
        .chip = NULL,
        .type = "camera",
        .device_handle = NULL,
        .init = dev_camera_init,
        .deinit = dev_camera_deinit
    },
};
