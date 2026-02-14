#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char wifi_ssid[32];
    char wifi_pass[64];
    char api_url_1[128];
    char api_url_2[128];
    char ntp_server[64];
    char filter_colors[512];  // JSON string: {"L":"#60a5fa","R":"#ef4444","G":"#10b981","B":"#3b82f6","Ha":"#f43f5e","Sii":"#a855f7","Oiii":"#06b6d4"}
} app_config_t;

void app_config_init(void);
app_config_t *app_config_get(void);
void app_config_save(const app_config_t *config);
void app_config_factory_reset(void);
uint32_t app_config_get_filter_color(const char *filter_name);

#ifdef __cplusplus
}
#endif
