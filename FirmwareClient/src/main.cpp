#include "esp_https_ota.h"
#define CONFIG_FIRMWARE_UPGRADE_URL ""
#define SERVER_CERT_PEM_START ""

esp_err_t do_firmware_upgrade()
{
    esp_http_client_config_t config = {
        .url = CONFIG_FIRMWARE_UPGRADE_URL,
        .cert_pem = (char *)SERVER_CERT_PEM_START,
    };

    esp_err_t ret = esp_https_ota(&config);
    if (ret == ESP_OK) {
        esp_restart();
    } else {
        return ESP_FAIL;
    }
    return ESP_OK;
}


void app_main(){
    while(1){
        do_firmware_upgrade();
    }
}