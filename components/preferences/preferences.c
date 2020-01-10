#include "nvs.h"
#include "preferences.h"
#include "utils.h"

const char* nvs_errors[] = {"OTHER", "NOT_INITIALIZED", "NOT_FOUND", "TYPE_MISMATCH", "READ_ONLY", "NOT_ENOUGH_SPACE", "INVALID_NAME", "INVALID_HANDLE", "REMOVE_FAILED", "KEY_TOO_LONG", "PAGE_FULL", "INVALID_STATE", "INVALID_LENGHT"};
#define nvs_error(e) (((e) > ESP_ERR_NVS_BASE) ? nvs_errors[(e) & ~(ESP_ERR_NVS_BASE)] : nvs_errors[0])

nvs_handle_t handle;

Preferences storedPreferences;
Preferences currentPreferences;

bool preferences_init() {
    esp_err_t err = nvs_open("M365", NVS_READWRITE, &handle);

    if (err) {
        ESP_LOGE(TAG, "nvs_open failed: %s", nvs_error(err));
        return false;
    }
    return true;
}
