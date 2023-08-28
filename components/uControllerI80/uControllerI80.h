#ifndef _uControllerI80_
#define _uControllerI80_

#include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

#include "Configuration.h"
#include "PinNumbers.h"

static const char *TAG = "uControllerI80";

esp_lcd_i80_bus_handle_t I80BusHandle = NULL;
esp_lcd_panel_io_handle_t PanelIOHandle = NULL;
esp_lcd_panel_handle_t PanelHandle = NULL;

void InitialiseBus()
{
    ESP_LOGI(TAG, "Initialise I80 bus");

    // esp_lcd_i80_bus_handle_t i80_bus = NULL;

    esp_lcd_i80_bus_config_t BusConfig = 
    {
        .dc_gpio_num = I80_PIN_NUM_DC,
        .wr_gpio_num = I80_PIN_NUM_PCLK,
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .data_gpio_nums = {
            I80_PIN_NUM_DATA0,
            I80_PIN_NUM_DATA1,
            I80_PIN_NUM_DATA2,
            I80_PIN_NUM_DATA3,
            I80_PIN_NUM_DATA4,
            I80_PIN_NUM_DATA5,
            I80_PIN_NUM_DATA6,
            I80_PIN_NUM_DATA7
        },
        .bus_width = I80_LCD_BUS_WIDTH,
        .max_transfer_bytes = I80_LCD_H_RES * I80_LCD_V_RES * 3, //sizeof(uint16_t) + (4 * 5120),
        .psram_trans_align = I80_PSRAM_DATA_ALIGNMENT,
        .sram_trans_align = 4,
    };

    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&BusConfig, &I80BusHandle));
}

void InitialisePanelIO()
{
    ESP_LOGI(TAG, "Initialize I80 panel io");

    // esp_lcd_panel_io_handle_t io_handle = NULL;

    esp_lcd_panel_io_i80_config_t IOConfig = {
        .cs_gpio_num = I80_PIN_NUM_CS,
        .pclk_hz = I80_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL, // example_notify_lvgl_flush_ready,
        .user_ctx = NULL, //&disp_drv,
        .lcd_cmd_bits = I80_LCD_CMD_BITS,
        .lcd_param_bits = I80_LCD_PARAM_BITS,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .flags = {
            .cs_active_high = 0,        // OK /CS is active LOW
            .reverse_color_bits = 0,
            .swap_color_bytes = 0,      // Swap can be done in DMA
            .pclk_active_neg = 0,       // OK Rising Edge trigger
            .pclk_idle_low = 0          // OK PCLK is idle HIGH. H-L-H sequences
        },
    };

    // TODO: Add RDX Pin initialisation

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(I80BusHandle, &IOConfig, &PanelIOHandle));
}

void InitialisePanelDevice()
{
    ESP_LOGI(TAG, "Initialize I80 panel handle");

    // esp_lcd_panel_handle_t panel_handle = NULL;

    esp_lcd_panel_dev_config_t PanelDeviceConfig = {
        .reset_gpio_num = I80_PIN_NUM_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16,
        .flags = {
            .reset_active_high = 0 // RESET is active LOW
        },
        .vendor_config = NULL
    };

    // TODO: Make lcd new panel init device agnostic or parameterised
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(PanelIOHandle, &PanelDeviceConfig, &PanelHandle));
}

void InitialisePanelSettings()
{
    // TODO: Add Screen command array transmission here.

    // LCD Init Commands
    esp_lcd_panel_reset(PanelHandle);
    esp_lcd_panel_init(PanelHandle);

    esp_lcd_panel_swap_xy(PanelHandle, true);
    esp_lcd_panel_invert_color(PanelHandle, false);

    esp_lcd_panel_io_tx_param(PanelIOHandle, 0x3A, (uint8_t[]) { 0xE5 }, 1);    // Interface Pixel Format
    esp_lcd_panel_io_tx_param(PanelIOHandle, 0xF2, (uint8_t[]) { 0 }, 1); // 3Gamma function disable
    esp_lcd_panel_io_tx_param(PanelIOHandle, 0x26, (uint8_t[]) { 1 }, 1); // Gamma curve 1 selected
    esp_lcd_panel_io_tx_param(PanelIOHandle, 0xE0, (uint8_t[]) {          // Set positive gamma
        0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 }, 15);
    esp_lcd_panel_io_tx_param(PanelIOHandle, 0xE1, (uint8_t[]) {          // Set negative gamma
        0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F }, 15);


    // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(PanelHandle, true));
}

void Initialise()
{
    InitialiseBus();

    InitialisePanelIO();

    InitialisePanelDevice();

    InitialisePanelSettings();
}

static inline void I80TransferFull(const void *aBuffer)
{
    esp_lcd_panel_draw_bitmap(PanelHandle, 0, 0, I80_LCD_H_RES, I80_LCD_V_RES, aBuffer);

    // panel_st7789_draw_bitmap(...);
    // panel_io_i80_tx_color(...);
}

static inline void I80TransferPartial(const int aXStart, const int aYStart, const int aXEnd, const int aYEnd, const void *aBuffer)
{
    esp_lcd_panel_draw_bitmap(PanelHandle, aXStart, aYStart, aXEnd, aYEnd, aBuffer);
}

#endif