#include "nina_client.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "nina_client";

// Helper to perform GET request and parse JSON
static cJSON *http_get_json(const char *url) {
    esp_http_client_config_t config = {
        .url = url,
        .timeout_ms = 5000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) return NULL;

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return NULL;
    }

    int content_length = esp_http_client_fetch_headers(client);
    if (content_length < 0) {
        ESP_LOGE(TAG, "HTTP client fetch headers failed");
        esp_http_client_cleanup(client);
        return NULL;
    }

    // Read data
    char *buffer = malloc(content_length + 1);
    if (!buffer) {
        ESP_LOGE(TAG, "Failed to allocate buffer for HTTP response");
        esp_http_client_cleanup(client);
        return NULL;
    }

    int total_read_len = 0, read_len;
    while (total_read_len < content_length) {
        read_len = esp_http_client_read(client, buffer + total_read_len, content_length - total_read_len);
        if (read_len <= 0) {
            break;
        }
        total_read_len += read_len;
    }
    buffer[total_read_len] = '\0';

    esp_http_client_cleanup(client);

    cJSON *json = cJSON_Parse(buffer);
    free(buffer);
    return json;
}

static void find_running_exposure(cJSON *item, NinaData *data) {
    if (!item) return;

    cJSON *status = cJSON_GetObjectItem(item, "Status");
    if (status && status->valuestring && strcmp(status->valuestring, "RUNNING") == 0) {
        // Check if this is an exposure item
        cJSON *exposure_count = cJSON_GetObjectItem(item, "ExposureCount");
        cJSON *iterations = cJSON_GetObjectItem(item, "Iterations");
        
        if (exposure_count && iterations) {
            data->exposure_count = exposure_count->valueint;
            data->exposure_iterations = iterations->valueint;
            
            cJSON *name = cJSON_GetObjectItem(item, "Name");
            if (name && name->valuestring) strncpy(data->sequence_item_name, name->valuestring, sizeof(data->sequence_item_name) - 1);
            
            cJSON *type = cJSON_GetObjectItem(item, "Type");
            if (type && type->valuestring) strncpy(data->exposure_type, type->valuestring, sizeof(data->exposure_type) - 1);

            cJSON *exp_time = cJSON_GetObjectItem(item, "ExposureTime");
            if (exp_time) data->exposure_time = exp_time->valuedouble;

            // Calculate progress for the *batch*
            if (data->exposure_iterations > 0) {
                data->exposure_progress = (float)data->exposure_count / (float)data->exposure_iterations;
            }
            return; // Found it
        }
    }
    
    // Recurse into Items
    cJSON *sub_items = cJSON_GetObjectItem(item, "Items");
    if (sub_items && cJSON_IsArray(sub_items)) {
        cJSON *child = NULL;
        cJSON_ArrayForEach(child, sub_items) {
            find_running_exposure(child, data);
            if (data->exposure_iterations > 0) return; // Found inside
        }
    }
}

void nina_client_get_data(const char *base_url, NinaData *data) {
    char url[256];
    cJSON *json, *response, *item;

    data->connected = false; 
    data->exposure_progress = 0.0f;
    data->exposure_count = 0;
    data->exposure_iterations = 0;
    data->exposure_time = 0.0;
    strcpy(data->sequence_item_name, "");
    strcpy(data->exposure_type, "");

    // 1. Get Camera Info
    snprintf(url, sizeof(url), "%sequipment/camera/info", base_url);
    // ESP_LOGI(TAG, "Requesting URL: %s", url);
    json = http_get_json(url);
    if (json) {
        data->connected = true; 
        response = cJSON_GetObjectItem(json, "Response");
        if (response) {
            item = cJSON_GetObjectItem(response, "CameraState");
            if (item && item->valuestring) {
                strncpy(data->camera_status, item->valuestring, sizeof(data->camera_status) - 1);
            } else {
                strcpy(data->camera_status, "Unknown");
            }
        }
        cJSON_Delete(json);
    } else {
        strcpy(data->camera_status, "Comm Error");
    }

    // 2. Get Sequence Info
    snprintf(url, sizeof(url), "%ssequence/json", base_url);
    json = http_get_json(url);
    if (json) {
        data->connected = true;
        response = cJSON_GetObjectItem(json, "Response");
        if (response && cJSON_IsArray(response)) {
             cJSON *root_item = NULL;
             cJSON_ArrayForEach(root_item, response) {
                 find_running_exposure(root_item, data);
                 if (data->exposure_iterations > 0) break;
             }
        }
        cJSON_Delete(json);
    }

    // 2. Get Mount Info
    snprintf(url, sizeof(url), "%sequipment/mount/info", base_url);
    json = http_get_json(url);
    if (json) {
        data->connected = true;
        response = cJSON_GetObjectItem(json, "Response");
        if (response) {
            // Mount Status
            cJSON *slewing = cJSON_GetObjectItem(response, "Slewing");
            cJSON *tracking = cJSON_GetObjectItem(response, "TrackingEnabled");
            cJSON *parked = cJSON_GetObjectItem(response, "AtPark");
            
            if (slewing && cJSON_IsTrue(slewing)) strcpy(data->mount_status, "Slewing");
            else if (parked && cJSON_IsTrue(parked)) strcpy(data->mount_status, "Parked");
            else if (tracking && cJSON_IsTrue(tracking)) strcpy(data->mount_status, "Tracking");
            else strcpy(data->mount_status, "Stopped");

            // Coordinates
            cJSON *coords = cJSON_GetObjectItem(response, "Coordinates");
            if (coords) {
                item = cJSON_GetObjectItem(coords, "RA");
                if (item) data->ra = item->valuedouble;
                item = cJSON_GetObjectItem(coords, "Dec");
                if (item) data->dec = item->valuedouble;
            }
        }
        cJSON_Delete(json);
    }

    // 3. Get Guider Info
    snprintf(url, sizeof(url), "%sequipment/guider/info", base_url);
    json = http_get_json(url);
    if (json) {
        response = cJSON_GetObjectItem(json, "Response");
        if (response) {
            cJSON *rms = cJSON_GetObjectItem(response, "RMSError");
            if (rms) {
                cJSON *total = cJSON_GetObjectItem(rms, "Total");
                if (total) {
                    item = cJSON_GetObjectItem(total, "Arcseconds"); // Or "Pixel"
                    // The schema said Arcseconds: integer, but let's check double too
                    if (item) data->guider_error_rms = (float)item->valuedouble;
                }
            }
        }
        cJSON_Delete(json);
    }

    // Target Name - Placeholder as it's not easily available in basic info
    strcpy(data->target_name, "Unknown Target"); 
}
