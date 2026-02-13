#pragma once

#include "lvgl.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Data structure holding status for one NINA instance
typedef struct {
    bool connected;
    char target_name[64];
    char camera_status[32]; // e.g., "Exposing", "Idle"
    float exposure_progress; // 0.0 to 1.0
    
    // New Sequence Info
    char sequence_item_name[64];
    int exposure_count;
    int exposure_iterations;
    double exposure_time;
    char exposure_type[16];

    double ra;
    double dec;
    float guider_error_rms;
    int focuser_position;
    char mount_status[32]; // e.g., "Tracking", "Slewing"
} NinaData;

// Initialize the dashboard UI
void nina_dashboard_init(void);

// Update the data for a specific instance (0 or 1)
void nina_dashboard_update(int instance_index, const NinaData *data);

// Cycle the view (Summary -> Inst 1 -> Inst 2 -> Summary)
void nina_dashboard_cycle_view(void);

#ifdef __cplusplus
}
#endif
