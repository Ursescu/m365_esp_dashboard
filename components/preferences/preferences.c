#include "preferences.h"

#include <string.h>

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"

#define TAG "preferences"

nvs_handle_t handle;
Preferences dashboard_preferences;

/*
 *  Default config preferences. Update it when preferences 
 *  structure is changed.
 */ 
static const Preferences default_preferences = (Preferences){
    .size = 10,
    .max_speed = 20,
    .min_speed = 15,
    .name = "Default",
};

bool preferences_init(void) {
    esp_err_t err;

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "NVS partition was truncated and needs to be erased");
        return false;
    }

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle");
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);

    if (err != ESP_OK) {
        ESP_LOGI(TAG, "nvs_open failed: %s", esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(TAG, "Done");

    /*
     * Read the saved preferences from NVS into the extern dashboard_preferences.
     */
    ESP_LOGI(TAG, "Reading saved preferences NVS");

    // Read the size of memory space required for preferinces
    size_t saved_size = 0;  // value will default to 0, if not set yet in NVS
    err = nvs_get_blob(handle, "preferences", NULL, &saved_size);

    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Failed to get preferences size: %s", esp_err_to_name(err));
        return false;
    }

    // Read previously saved preferences if available
    if (saved_size == sizeof(Preferences)) {
        // Size of preferences matching .

        err = nvs_get_blob(handle, "preferences", &dashboard_preferences, &saved_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read the preferences: %s", esp_err_to_name(err));
            return false;
        }
    } else {
        // Drop saved preferences, missmatch or not saved.
        ESP_LOGI(TAG, "Drop saved preferences, missmatch of preference structure size. Set the default preferences");
        memcpy(&dashboard_preferences, &default_preferences, sizeof(Preferences));
    }

    nvs_close(handle);

    return true;
}

bool preferences_save(void) {
    esp_err_t err;

    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle...");
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(TAG, "Done");

    err = nvs_set_blob(handle, "preferences", &dashboard_preferences, sizeof(Preferences));

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save preferences failed: %s", esp_err_to_name(err));
        nvs_close(handle);
        return false;
    }

    // Commit
    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit preferences: %s", esp_err_to_name(err));
        nvs_close(handle);
        return false;
    }

    // Close
    nvs_close(handle);
    return true;
}

void preferences_print(void) {
    ESP_LOGI(TAG, "size: %hhu\n", dashboard_preferences.size);
    ESP_LOGI(TAG, "max speed: %hhu\n", dashboard_preferences.max_speed);
    ESP_LOGI(TAG, "min speed: %hhu\n", dashboard_preferences.min_speed);
    ESP_LOGI(TAG, "name: %s\n", dashboard_preferences.name);
}
