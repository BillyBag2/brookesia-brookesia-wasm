#ifndef LV_CONF_H
#define LV_CONF_H

/* Minimal LVGL configuration for the first Brookesia browser build. */
#define LV_COLOR_DEPTH 16
#define LV_MEM_SIZE (8U * 1024U * 1024U)
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_USE_OS LV_OS_NONE

/* Decode in-memory JPEG resources used by Brookesia applications. */
#define LV_USE_TJPGD 1
#define LV_USE_FS_MEMFS 1
#define LV_FS_MEMFS_LETTER 'M'

#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1

#endif
