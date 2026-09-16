/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 * SPDX-License-Identifier: LicenseRef-Espressif-Modified-MIT
 *
 * Auto-generated device configuration file
 * DO NOT MODIFY THIS FILE MANUALLY
 *
 * See LICENSE file for details.
 */

#include <stdlib.h>
#include "esp_board_device.h"
#include "dev_audio_codec.h"
#include "dev_camera.h"
#include "dev_display_lcd.h"
#include "dev_fs_fat.h"
#include "dev_gpio_expander.h"
#include "dev_lcd_touch.h"
#include "dev_ledc_ctrl.h"
#include "dev_power_ctrl.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"

// Device configuration structures
const static dev_audio_codec_config_t esp_bmgr_audio_dac_cfg = {
    .name = "audio_dac",
    .chip = "es8388",
    .type = "audio_codec",
    .data_if_type = 0,
    .adc_enabled = false,
    .adc_max_channel = 0,
    .adc_channel_mask = 0x3,
    .adc_channel_labels = "",
    .adc_init_gain = 0,
    .dac_enabled = true,
    .dac_max_channel = 0,
    .dac_channel_mask = 0x0,
    .dac_init_gain = 0,
    .pa_cfg = {
        .name = NULL,
        .port = -1,
        .active_level = 0,
        .gain = 0.0,
    },
    .i2c_cfg = {
        .name = "i2c_master",
        .port = 0,
        .address = 32,
        .frequency = 400000,
    },
    .i2s_cfg = {
        .name = "i2s_audio_out",
        .port = 0,
        .clk_src = 0,
        .tx_aux_out_io = -1,
        .tx_aux_out_line = 0,
        .tx_aux_out_invert = false,
    },
    .adc_cfg = {
        .periph_name = NULL,
        .sample_rate_hz = 0,
        .max_store_buf_size = 0,
        .conv_frame_size = 0,
        .conv_mode = 0,
        .format = 0,
        .pattern_num = 0,
        .cfg_mode = 0,
        .cfg = {
            .single_unit = {
                .unit_id = 0,
                .atten = 0,
                .bit_width = 0,
                .channel_id = {},
            },
        },
    },
    .metadata = NULL,
    .metadata_size = 0,
    .mclk_enabled = false,
    .aec_enabled = false,
    .eq_enabled = false,
    .alc_enabled = false,
};

const static dev_audio_codec_config_t esp_bmgr_audio_adc_cfg = {
    .name = "audio_adc",
    .chip = "es7210",
    .type = "audio_codec",
    .data_if_type = 0,
    .adc_enabled = true,
    .adc_max_channel = 0,
    .adc_channel_mask = 0x3,
    .adc_channel_labels = "",
    .adc_init_gain = 0,
    .dac_enabled = false,
    .dac_max_channel = 0,
    .dac_channel_mask = 0x0,
    .dac_init_gain = 0,
    .pa_cfg = {
        .name = NULL,
        .port = -1,
        .active_level = 0,
        .gain = 0.0,
    },
    .i2c_cfg = {
        .name = "i2c_master",
        .port = 0,
        .address = 128,
        .frequency = 400000,
    },
    .i2s_cfg = {
        .name = "i2s_audio_in",
        .port = 0,
        .clk_src = 0,
        .tx_aux_out_io = -1,
        .tx_aux_out_line = 0,
        .tx_aux_out_invert = false,
    },
    .adc_cfg = {
        .periph_name = NULL,
        .sample_rate_hz = 0,
        .max_store_buf_size = 0,
        .conv_frame_size = 0,
        .conv_mode = 0,
        .format = 0,
        .pattern_num = 0,
        .cfg_mode = 0,
        .cfg = {
            .single_unit = {
                .unit_id = 0,
                .atten = 0,
                .bit_width = 0,
                .channel_id = {},
            },
        },
    },
    .metadata = NULL,
    .metadata_size = 0,
    .mclk_enabled = false,
    .aec_enabled = false,
    .eq_enabled = false,
    .alc_enabled = false,
};

const static dev_fs_fat_config_t esp_bmgr_fs_sdcard_cfg = {
    .name = "fs_sdcard",
    .mount_point = "/sdcard",
    .frequency = SDMMC_FREQ_HIGHSPEED,
    .vfs_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16384,
    },
    .sub_type = "sdmmc",
    .sub_cfg = {
        .sdmmc = {
            .slot = SDMMC_HOST_SLOT_0,
            .bus_width = 4,
            .slot_flags = SDMMC_SLOT_FLAG_INTERNAL_PULLUP,
            .pins = {
                .clk = 0,
                .cmd = 0,
                .d0 = 0,
                .d1 = 0,
                .d2 = 0,
                .d3 = 0,
                .d4 = 0,
                .d5 = 0,
                .d6 = 0,
                .d7 = 0,
                .cd = -1,
                .wp = -1,
            },
            .ldo_chan_id = 4,
        },
    },
};

const static dev_io_expander_config_t esp_bmgr_gpio_expander_cfg = {
    .name = "gpio_expander",
    .type = "gpio_expander",
    .chip = "pi3ioe",
    .i2c_name = "i2c_master",
    .i2c_addr_count = 1,
    .i2c_addr = {134},
    .max_pins = 8,
    .output_io_mask = 127,
    .output_io_level_mask = 118,
    .enable_mode_set = true,
    .output_io_mode_mask = 0,
    .io_pullup_mask = 0,
    .io_pulldown_mask = 0,
    .input_io_mask = 128,
};

const static dev_io_expander_config_t esp_bmgr_gpio_expander_2_cfg = {
    .name = "gpio_expander_2",
    .type = "gpio_expander",
    .chip = "pi3ioe",
    .i2c_name = "i2c_master",
    .i2c_addr_count = 1,
    .i2c_addr = {136},
    .max_pins = 8,
    .output_io_mask = 249,
    .output_io_level_mask = 9,
    .enable_mode_set = true,
    .output_io_mode_mask = 0,
    .io_pullup_mask = 0,
    .io_pulldown_mask = 0,
    .input_io_mask = 0,
};

const static dev_ledc_ctrl_config_t esp_bmgr_lcd_brightness_cfg = {
    .name = "lcd_brightness",
    .type = "ledc_ctrl",
    .ledc_name = "ledc_backlight",
    .default_percent = 100,
};

const static dev_display_lcd_config_t esp_bmgr_display_lcd_cfg = {
    .name = "display_lcd",
    .chip = "tab5_lcd",
    .sub_type = "dsi",
    .lcd_width = 720,
    .lcd_height = 1280,
    .swap_xy = false,
    .mirror_x = false,
    .mirror_y = false,
    .need_reset = true,
    .invert_color = false,
    .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
    .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
    .bits_per_pixel = 16,
    .sub_cfg = {
        .dsi = {
            .dsi_name = "dsi_display",
            .ldo_name = "ldo_mipi",
            .reset_gpio_num = -1,
            .reset_active_high = 0,
            .dbi_config = {
                .virtual_channel = 0,
                .lcd_cmd_bits = 8,
                .lcd_param_bits = 8,
            },
            .dpi_config = {
                .virtual_channel = 0,
                .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
                .dpi_clock_freq_mhz = 80,
                .in_color_format = LCD_COLOR_FMT_RGB565,
                .out_color_format = LCD_COLOR_FMT_RGB565,
                .num_fbs = 1,
                .flags = {
                    .disable_lp = false,
                },
                .video_timing = {
                    .h_size = 720,
                    .v_size = 1280,
                    .hsync_back_porch = 140,
                    .hsync_pulse_width = 40,
                    .hsync_front_porch = 40,
                    .vsync_back_porch = 20,
                    .vsync_pulse_width = 4,
                    .vsync_front_porch = 20,
                },
            },
            .use_dma2d = true,
        },
    },
};

const static dev_power_ctrl_config_t esp_bmgr_touch_power_ctrl_cfg = {
    .name = "touch_power_ctrl",
    .sub_type = "gpio",
    .sub_cfg = {
        .gpio = {
            .gpio_name = "gpio_power_touch",
            .active_level = 0,
        },
    },
};

const static dev_lcd_touch_config_t esp_bmgr_lcd_touch_cfg = {
    .name = "lcd_touch",
    .chip = "tab5_touch",
    .type = "lcd_touch",
    .sub_type = "i2c",
    .touch_config = {
        .x_max = 720,
        .y_max = 1280,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .process_coordinates = NULL,
        .interrupt_callback = NULL,
        .user_data = NULL,
        .driver_data = NULL,
    },
    .sub_cfg = {
        .i2c = {
            .i2c_name = "i2c_master",
            .i2c_addr_count = 2,
            .i2c_addr = {0x28, 0xaa, 0x00, 0x00},
            .io_i2c_config = {
                .dev_addr = 0,
                .control_phase_bytes = 1,
                .dc_bit_offset = 0,
                .lcd_cmd_bits = 16,
                .lcd_param_bits = 0,
                .scl_speed_hz = 100000,
                .flags = {
                    .dc_low_on_data = false,
                    .disable_control_phase = true,
                },
                .transaction_timeout_ms = 0,
            },
        },
    },
};

const static dev_camera_config_t esp_bmgr_camera_cfg = {
    .name = "camera",
    .type = "camera",
    .sub_type = "csi",
    .sub_cfg = {
        .csi = {
            .i2c_name = "i2c_master",
            .i2c_freq = 100000,
            .ldo_name = "ldo_mipi",
            .reset_io = -1,
            .pwdn_io = -1,
            .dont_init_ldo = true,
            .xclk_config = {
            },
        },
    },
};

// Device descriptor array
static const char* esp_bmgr_audio_dac_deps[] = {
    "gpio_expander",
};
static const char* esp_bmgr_display_lcd_deps[] = {
    "gpio_expander",
    "lcd_touch",
};
static const char* esp_bmgr_lcd_touch_deps[] = {
    "gpio_expander",
};
static const char* esp_bmgr_camera_deps[] = {
    "gpio_expander",
};

const esp_board_device_desc_t g_esp_board_devices[] = {
    {
        .next = &g_esp_board_devices[1],
        .name = "audio_dac",
        .chip = "es8388",
        .type = "audio_codec",
        .sub_type = NULL,
        .cfg = &esp_bmgr_audio_dac_cfg,
        .cfg_size = sizeof(esp_bmgr_audio_dac_cfg),
        .init_skip = false,
        .depends_on = esp_bmgr_audio_dac_deps,
        .depends_on_num = 1,
    },
    {
        .next = &g_esp_board_devices[2],
        .name = "audio_adc",
        .chip = "es7210",
        .type = "audio_codec",
        .sub_type = NULL,
        .cfg = &esp_bmgr_audio_adc_cfg,
        .cfg_size = sizeof(esp_bmgr_audio_adc_cfg),
        .init_skip = false,
        .depends_on = NULL,
        .depends_on_num = 0,
    },
    {
        .next = &g_esp_board_devices[3],
        .name = "fs_sdcard",
        .chip = NULL,
        .type = "fs_fat",
        .sub_type = "sdmmc",
        .cfg = &esp_bmgr_fs_sdcard_cfg,
        .cfg_size = sizeof(esp_bmgr_fs_sdcard_cfg),
        .init_skip = false,
        .depends_on = NULL,
        .depends_on_num = 0,
    },
    {
        .next = &g_esp_board_devices[4],
        .name = "gpio_expander",
        .chip = "pi3ioe",
        .type = "gpio_expander",
        .sub_type = NULL,
        .cfg = &esp_bmgr_gpio_expander_cfg,
        .cfg_size = sizeof(esp_bmgr_gpio_expander_cfg),
        .init_skip = false,
        .depends_on = NULL,
        .depends_on_num = 0,
    },
    {
        .next = &g_esp_board_devices[5],
        .name = "gpio_expander_2",
        .chip = "pi3ioe",
        .type = "gpio_expander",
        .sub_type = NULL,
        .cfg = &esp_bmgr_gpio_expander_2_cfg,
        .cfg_size = sizeof(esp_bmgr_gpio_expander_2_cfg),
        .init_skip = false,
        .depends_on = NULL,
        .depends_on_num = 0,
    },
    {
        .next = &g_esp_board_devices[6],
        .name = "lcd_brightness",
        .chip = NULL,
        .type = "ledc_ctrl",
        .sub_type = NULL,
        .cfg = &esp_bmgr_lcd_brightness_cfg,
        .cfg_size = sizeof(esp_bmgr_lcd_brightness_cfg),
        .init_skip = false,
        .depends_on = NULL,
        .depends_on_num = 0,
    },
    {
        .next = &g_esp_board_devices[7],
        .name = "display_lcd",
        .chip = "tab5_lcd",
        .type = "display_lcd",
        .sub_type = "dsi",
        .cfg = &esp_bmgr_display_lcd_cfg,
        .cfg_size = sizeof(esp_bmgr_display_lcd_cfg),
        .init_skip = false,
        .depends_on = esp_bmgr_display_lcd_deps,
        .depends_on_num = 2,
    },
    {
        .next = &g_esp_board_devices[8],
        .name = "touch_power_ctrl",
        .chip = NULL,
        .type = "power_ctrl",
        .sub_type = "gpio",
        .cfg = &esp_bmgr_touch_power_ctrl_cfg,
        .cfg_size = sizeof(esp_bmgr_touch_power_ctrl_cfg),
        .init_skip = false,
        .depends_on = NULL,
        .depends_on_num = 0,
    },
    {
        .next = &g_esp_board_devices[9],
        .name = "lcd_touch",
        .chip = "tab5_touch",
        .type = "lcd_touch",
        .sub_type = "i2c",
        .cfg = &esp_bmgr_lcd_touch_cfg,
        .cfg_size = sizeof(esp_bmgr_lcd_touch_cfg),
        .init_skip = false,
        .power_ctrl_device = "touch_power_ctrl",
        .depends_on = esp_bmgr_lcd_touch_deps,
        .depends_on_num = 1,
    },
    {
        .next = NULL,
        .name = "camera",
        .chip = NULL,
        .type = "camera",
        .sub_type = "csi",
        .cfg = &esp_bmgr_camera_cfg,
        .cfg_size = sizeof(esp_bmgr_camera_cfg),
        .init_skip = false,
        .depends_on = esp_bmgr_camera_deps,
        .depends_on_num = 1,
    },
};
