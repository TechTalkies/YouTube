#include "bsp_board.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "driver/i2s_pdm.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"

#define TAG "xiao-esp32s3-sense"
#define SPI_DMA_CHAN   SPI_DMA_CH_AUTO

sdmmc_host_t host = SDSPI_HOST_DEFAULT();
sdmmc_card_t *card;
i2s_chan_handle_t rx_handle = NULL;

// Deinit SD card
esp_err_t bsp_sdcard_deinit(char *mount_point)
{
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    spi_bus_free(host.slot);
    return ESP_OK;
}

// Init SD card
esp_err_t bsp_sdcard_init(char *mount_point, size_t max_files)
{
    esp_err_t ret;
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 8 * 1024
    };
    ESP_LOGI(TAG, "Initializing SD card");

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = CONFIG_EXAMPLE_SPI_MOSI_GPIO,
        .miso_io_num = CONFIG_EXAMPLE_SPI_MISO_GPIO,
        .sclk_io_num = CONFIG_EXAMPLE_SPI_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ret;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = CONFIG_EXAMPLE_SPI_CS_GPIO;
    slot_config.host_id = host.slot;

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ret;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}

// Special config for dev board
esp_err_t bsp_board_init(uint32_t sample_rate, int channel_format, int bits_per_chan)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_handle));

    esp_err_t ret_val = ESP_OK;
    i2s_pdm_rx_config_t pdm_rx_cfg = {
        .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(sample_rate),
        /* The default mono slot is the left slot (whose 'select pin' of the PDM microphone is pulled down) */
        .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG(bits_per_chan, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .clk = CONFIG_EXAMPLE_I2S_CLK_GPIO,
            .din = CONFIG_EXAMPLE_I2S_DATA_GPIO,
            .invert_flags = {
                .clk_inv = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_pdm_rx_mode(rx_handle, &pdm_rx_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));

    return ESP_OK;
}

// Play audio
esp_err_t bsp_audio_play(const int16_t* data, int length, TickType_t ticks_to_wait)
{
    // TODO: Implement function
    return ESP_FAIL;
}

// Get the record pcm data
esp_err_t bsp_get_feed_data(bool is_get_raw_channel, int16_t *buffer, int buffer_len)
{
    size_t bytes_read;
    esp_err_t ret = i2s_channel_read(rx_handle, buffer, buffer_len, &bytes_read, 1000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read data from I2S");
        return ret;
    }
    return ESP_OK;
}

// Get the record channel number
int bsp_get_feed_channel(void)
{
    return 1;
}

// Get the input format of the board
char* bsp_get_input_format(void)
{
    return "M";
}

// Set play volume
esp_err_t bsp_audio_set_play_vol(int volume)
{
    // TODO: Implement function
    return ESP_FAIL;
}

// Get play volume
esp_err_t bsp_audio_get_play_vol(int *volume)
{
    // TODO: Implement function
    return ESP_FAIL;
}