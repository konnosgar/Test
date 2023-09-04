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
    I80Initialise(I80TransferDoneCallback, &BlitBufferPointer);
    RendBufferPointer = &VideoBuffer1;
    BlitBufferPointer = RendBufferPointer;
    // BlitBufferPointer = &VideoBuffer1;

    int LoopCounter = 0;

    int LineYPosition = 0;
    int Direction = 1;

    TColor24 LineColor = { .Color = {255, 255, 255} };

    uint8_t Red = 5, Green = -1, Blue = 1;
    uint8_t RedDir = 1, GreenDir = 254, BlueDir = 1;

    TaskHandle_t BlitTaskHandle = NULL;
    xTaskCreatePinnedToCore(I80TransferTask, "Blit", 4096, (void*)&BlitBufferPointer, configMAX_PRIORITIES - 1, &BlitTaskHandle, CORE_ID_PRO);
    configASSERT( BlitTaskHandle );
    ESP_LOGI(TAG, "Blit Task Created!");

    esp_task_wdt_deinit();

    while(1)
    {
        // ESP_LOGI(TAG, "[REND] Taking@%p", RendBufferPointer->Buffer);
        if(xSemaphoreTake(RendBufferPointer->RenderSemaphore, portMAX_DELAY))
        {
            memset(RendBufferPointer->Buffer, 0x00, I80_LCD_H_RES * I80_LCD_V_RES * 3);

            // memset(buf1 + (I80_LCD_H_RES * LineYPosition * 3), 0x00, I80_LCD_H_RES * 3);
            for(int Lines = 0; Lines < LineYPosition; Lines++)
            {
                for(int i = 0; i < 128; i++) 
                {
                    SetPixel24(i, Lines, LineColor, RendBufferPointer->Buffer);
                }
            }
            // ESP_LOGI(TAG, "Rendered %d", LineYPosition);

            // BlitBufferPointer = RendBufferPointer;

            // ESP_LOGI(TAG, "[REND] Giving@%p", RendBufferPointer->Buffer);
            xSemaphoreGive(BlitSemaphore);

            RendBufferPointer = (TVideoBuffer*)RendBufferPointer->NextBufferPointer;
            // ESP_LOGI(TAG, "Switched buffers %p", RendBufferPointer->Buffer);

            // BlitBufferPointer = (TVideoBuffer*)BlitBufferPointer->NextBufferPointer;

            

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

        // vTaskDelay(1);
        // ESP_LOGI(TAG, "LOOP %d", LineYPosition);
    }
}
