#include "uControllerI80.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "string.h"

// static const char *TAG = "main";

typedef struct {
    union {
        struct {
            uint8_t R, G, B;
        } Colors;
        uint8_t Color[3];
    };
} Color24;

static inline void SetPixel24(const int aX, const int aY, const Color24 aColor, Color24* aBuffer)
{
    aBuffer = aBuffer + (aY << 7) + aX;
    *aBuffer = aColor;
}

void app_main(void)
{
    // I80Initialise(example_notify_lvgl_flush_ready, &disp_drv);
    I80Initialise(I80TransferDoneCallback, NULL);

    // Allocate draw buffers for LGVL
    uint8_t *buf1 = NULL;
    Color24 *buf2 = NULL;

#if CONFIG_EXAMPLE_LCD_I80_COLOR_IN_PSRAM
    buf1 = (uint8_t*)heap_caps_aligned_alloc(I80_PSRAM_DATA_ALIGNMENT, I80_LCD_H_RES * I80_LCD_V_RES * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    buf1 = (uint8_t*)heap_caps_malloc(I80_LCD_H_RES * I80_LCD_V_RES * 3 /*sizeof(lv_color_t)*/, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
#endif
    assert(buf1);
#if CONFIG_EXAMPLE_LCD_I80_COLOR_IN_PSRAM
    buf2 = (Color24*)heap_caps_aligned_alloc(I80_PSRAM_DATA_ALIGNMENT, I80_LCD_H_RES * I80_LCD_V_RES * sizeof(Color24), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    buf2 = (Color24*)heap_caps_malloc(I80_LCD_H_RES * I80_LCD_V_RES * sizeof(Color24), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
#endif
    assert(buf2);
    ESP_LOGI(TAG, "buf1@%p, buf2@%p", buf1, buf2);

    memset(buf1, 0x00, I80_LCD_H_RES * I80_LCD_V_RES * 3);
    memset(buf2, 0x00, I80_LCD_H_RES * I80_LCD_V_RES * sizeof(Color24));

    int LoopCounter = 0;

    int LineYPosition = 0;
    int Direction = 1;

    Color24 LineColor = { .Color = {255, 0, 0} };

    uint8_t Red = 5, Green = -1, Blue = 1;
    uint8_t RedDir = 1, GreenDir = 254, BlueDir = 1;

    while(1)
    {
        while(I80TransferDone == 0) { ; }
        memset(buf2, 0xFF, I80_LCD_H_RES * I80_LCD_V_RES * 3);
        // memset(buf1 + (I80_LCD_H_RES * LineYPosition * 3), 0x00, I80_LCD_H_RES * 3);
        for(int i = 0; i < 128; i++) 
        {
            SetPixel24(i, LineYPosition, LineColor, buf2);
        }

        if(Red == 0 || Red == 255) 
        {
            if(Green == 0 || Green == 255)
            {
                if(Blue == 0 || Blue == 255)
                {
                    RedDir = -RedDir;
                    GreenDir = -GreenDir;
                    BlueDir = -BlueDir;

                    Red += RedDir;
                    Green += GreenDir;
                    Blue += BlueDir;
                } else Blue += BlueDir;
            } else Green += GreenDir;
        } else Red += RedDir;

        LineColor.Colors.R = Red;
        LineColor.Colors.G = Green;
        LineColor.Colors.B = Blue;
        
        I80TransferFullSynced(buf2);

        LineYPosition += Direction;
        if(LineYPosition == 160) Direction = -1;
        else if(LineYPosition == 0) Direction = 1;

        vTaskDelay(1);
        // ESP_LOGI(TAG, "LOOP %d", LineYPosition);
    }
}
