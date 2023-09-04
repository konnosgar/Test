#ifndef _uControllerI80_
#define _uControllerI80_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
// #include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

#include "Configuration.h"
#include "PinNumbers.h"
#include "DisplayInit.h"

static const char *TAG = "uControllerI80";

static esp_lcd_i80_bus_handle_t I80BusHandle = NULL;
static esp_lcd_panel_io_handle_t PanelIOHandle = NULL;
static esp_lcd_panel_handle_t PanelHandle = NULL;

bool I80TransferDone = 1;

static DRAM_ATTR SemaphoreHandle_t BlitSemaphore = NULL;
static DRAM_ATTR SemaphoreHandle_t RenderSemaphore = NULL;


static IRAM_ATTR bool I80TransferDoneCallback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    portBASE_TYPE HigherPriorityTaskWoken = pdTRUE;

    xSemaphoreGiveFromISR(RenderSemaphore, &HigherPriorityTaskWoken);

    return HigherPriorityTaskWoken == pdTRUE; // Whether a high priority task is woken up by this function

    // I80TransferDone = 1;
    // return true;
}

void I80InitialiseBus()
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

void I80InitialisePanelIO(esp_lcd_panel_io_color_trans_done_cb_t aColorTransferDoneCallback, void* aCallbackParameter)
{
    ESP_LOGI(TAG, "Initialize I80 panel io");

    // esp_lcd_panel_io_handle_t io_handle = NULL;

    esp_lcd_panel_io_i80_config_t IOConfig = {
        .cs_gpio_num = I80_PIN_NUM_CS,
        .pclk_hz = I80_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .on_color_trans_done = aColorTransferDoneCallback, // example_notify_lvgl_flush_ready,
        .user_ctx = aCallbackParameter, //&disp_drv,
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

    // TODO: Add TE  Pin initialisation
    gpio_config_t TEPinConfig = {
        .mode = GPIO_MODE_DEF_INPUT,
        .pin_bit_mask = 1ULL << I80_PIN_NUM_TE
    };
    ESP_ERROR_CHECK(gpio_config(&TEPinConfig));

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(I80BusHandle, &IOConfig, &PanelIOHandle));
}

void I80InitialisePanelDevice()
{
    ESP_LOGI(TAG, "Initialize I80 panel handle");

    // esp_lcd_panel_handle_t panel_handle = NULL;

    esp_lcd_panel_dev_config_t PanelDeviceConfig = {
        .reset_gpio_num = I80_PIN_NUM_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 18, //16,
        .flags = {
            .reset_active_high = 0 // RESET is active LOW
        },
        .vendor_config = NULL
    };

    // TODO: Make lcd new panel init device agnostic or parameterised
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(PanelIOHandle, &PanelDeviceConfig, &PanelHandle));
}

void SendTScreenCommandArray(const TScreenCommand *aCmdArr)
{
	// Send inititialisation commands to the TFT
	int index = 0;
	while (aCmdArr[index].DataBytes != TFT_INIT_CMD_ARRAY_END)
	{
		printf("[ INIT ] Screen Init Command, %02x \n", aCmdArr[index].Cmd);

        esp_lcd_panel_io_tx_param(PanelIOHandle, aCmdArr[index].Cmd, aCmdArr[index].Data, aCmdArr[index].DataBytes & TFT_INIT_CMD_DATABYTES_MASK);

		if (aCmdArr[index].DataBytes & TFT_INIT_DELAY)
			vTaskDelay(pdMS_TO_TICKS(20)); //vTaskDelay(portTICK_PERIOD_MS);
		index++;
	}
}

void I80InitialisePanelSettings()
{
    esp_lcd_panel_reset(PanelHandle);
    esp_lcd_panel_init(PanelHandle);

    SendTScreenCommandArray(ILI9163_InitCommandArray);
}

void I80Initialise(esp_lcd_panel_io_color_trans_done_cb_t aColorTransferDoneCallback, void* aCallbackParameter)
{
    BlitSemaphore   = xSemaphoreCreateBinary();
    RenderSemaphore = xSemaphoreCreateBinary();

    I80InitialiseBus();

    I80InitialisePanelIO(aColorTransferDoneCallback, aCallbackParameter);

    I80InitialisePanelDevice();

    I80InitialisePanelSettings();
}

static inline void I80TransferFull(const void *aBuffer)
{
    // TODO: Add Tearing Pin control

    // esp_lcd_panel_draw_bitmap(PanelHandle, 0, 0, I80_LCD_H_RES, I80_LCD_V_RES, aBuffer);
    esp_lcd_panel_io_tx_color(PanelIOHandle, ILI9163_RAMWR, aBuffer, I80_LCD_H_RES * I80_LCD_V_RES * 3);

    // panel_st7789_draw_bitmap(...);
    // panel_io_i80_tx_color(...);
}

static inline void I80TransferFullSynced(const void *aBuffer)
{
    while(gpio_get_level(I80_PIN_NUM_TE) == 0) { ; }
    esp_lcd_panel_io_tx_color(PanelIOHandle, ILI9163_RAMWR, aBuffer, I80_LCD_H_RES * I80_LCD_V_RES * 3);
    I80TransferDone = 0;
}

TaskFunction_t I80TransferTask(const void *aBuffer)
{
    ESP_LOGI(TAG, "Entered Blit Task");
    while(1)
    {
        if(xSemaphoreTake(BlitSemaphore, portMAX_DELAY))
        {
            // ESP_LOGI(TAG, "Blit");
            I80TransferFullSynced(aBuffer);
        }
    }
}

static inline void I80TransferPartial(const int aXStart, const int aYStart, const int aXEnd, const int aYEnd, const void *aBuffer)
{
    esp_lcd_panel_draw_bitmap(PanelHandle, aXStart, aYStart, aXEnd, aYEnd, aBuffer);
}

#endif