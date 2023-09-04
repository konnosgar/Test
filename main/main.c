#include "uControllerI80.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "string.h"
#include "esp_task_wdt.h"

#define CORE_ID_PRO (0)
#define CORE_ID_APP (1)

// static const char *TAG = "main";

static inline void SetPixel24(const int aX, const int aY, const TColor24 aColor, TColor24* aBuffer)
{
    aBuffer = aBuffer + (aY << 7) + aX;
    *aBuffer = aColor;
}

void app_main(void)
{
    // Initialise I80Controller Bus, IO and Panel Handle
    I80Initialise(I80TransferDoneCallback, NULL);

//     // Allocate Draw Buffers
//     uint8_t *buf1 = NULL;
//     TColor24 *buf2 = NULL;

// #if CONFIG_EXAMPLE_LCD_I80_COLOR_IN_PSRAM
//     buf1 = (uint8_t*)heap_caps_aligned_alloc(I80_PSRAM_DATA_ALIGNMENT, I80_LCD_H_RES * I80_LCD_V_RES * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
// #else
//     buf1 = (uint8_t*)heap_caps_malloc(I80_LCD_H_RES * I80_LCD_V_RES * 3 /*sizeof(lv_color_t)*/, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
// #endif
//     assert(buf1);
// #if CONFIG_EXAMPLE_LCD_I80_COLOR_IN_PSRAM
//     buf2 = (TColor24*)heap_caps_aligned_alloc(I80_PSRAM_DATA_ALIGNMENT, I80_LCD_H_RES * I80_LCD_V_RES * sizeof(TColor24), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
// #else
//     buf2 = (TColor24*)heap_caps_malloc(I80_LCD_H_RES * I80_LCD_V_RES * sizeof(TColor24), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
// #endif
//  assert(buf2);
//  ESP_LOGI(TAG, "buf1@%p, buf2@%p", buf1, buf2);

    // // Initialise Draw Buffers, Fill White
    // memset(buf1, 0x00, I80_LCD_H_RES * I80_LCD_V_RES * 3);
    // memset(buf2, 0x00, I80_LCD_H_RES * I80_LCD_V_RES * sizeof(TColor24));

    int LoopCounter = 0;

    int LineYPosition = 0;
    int Direction = 1;

    TColor24 LineColor = { .Color = {255, 255, 255} };

    uint8_t Red = 5, Green = -1, Blue = 1;
    uint8_t RedDir = 1, GreenDir = 254, BlueDir = 1;

    TaskHandle_t BlitTaskHandle = NULL;
    xTaskCreatePinnedToCore(I80TransferTask, "Blit", 4096, (void*)&VideoBuffer1, configMAX_PRIORITIES - 1, &BlitTaskHandle, CORE_ID_PRO);
    configASSERT( BlitTaskHandle );
    ESP_LOGI(TAG, "Blit Task Created!");

    while(1)
    {
        if(xSemaphoreTake(RenderSemaphore, 10))
        {
            memset(VideoBuffer1.Buffer, 0x00, I80_LCD_H_RES * I80_LCD_V_RES * 3);

            // memset(buf1 + (I80_LCD_H_RES * LineYPosition * 3), 0x00, I80_LCD_H_RES * 3);
            for(int i = 0; i < 128; i++) 
            {
                SetPixel24(i, LineYPosition, LineColor, VideoBuffer1.Buffer);
            }

            // if(Red == 0 || Red == 255) 
            // {
            //     if(Green == 0 || Green == 255)
            //     {
            //         if(Blue == 0 || Blue == 255)
            //         {
            //             RedDir = -RedDir;
            //             GreenDir = -GreenDir;
            //             BlueDir = -BlueDir;

            //             Red += RedDir;
            //             Green += GreenDir;
            //             Blue += BlueDir;
            //         } else Blue += BlueDir;
            //     } else Green += GreenDir;
            // } else Red += RedDir;

            // LineColor.Colors.R = Red;
            // LineColor.Colors.G = Green;
            // LineColor.Colors.B = Blue;
            
            LineYPosition += Direction;
            if(LineYPosition == 159) Direction = -1;
            else if(LineYPosition == 0) Direction = 1;
        }

        xSemaphoreGive(BlitSemaphore);

        // vTaskDelay(1);
        // ESP_LOGI(TAG, "LOOP %d", LineYPosition);
    }
}
