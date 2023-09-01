#include "uControllerI80.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"

#include "lvgl.h"
#define LVGL_TICK_PERIOD_MS    1

// static const char *TAG = "main";

void example_lvgl_demo_ui(lv_disp_t *disp);

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}
static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}
static void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

typedef struct {
    uint8_t R, G, B;
} Color24;

typedef struct {
    union {
        struct {
            uint8_t R, G, B;
        } Colors;
        uint8_t Color[3];
    };
} Color18;

static inline void SetPixel18(const int aX, const int aY, const Color18 aColor, Color18* aBuffer)
{
    aBuffer += (aY << 7) + aX;
    *aBuffer = aColor;
}

static inline void SetPixel(const int aX, const int aY, const Color24 aColor, uint8_t* aBuffer)
{
    uint32_t Color = aColor.R | (aColor.G << 8) | (aColor.B << 16);
    aBuffer += aX * 3;
    aBuffer += 8 << aY;
    aBuffer[0] = aColor.R;
    aBuffer[1] = aColor.G;
    aBuffer[2] = aColor.B;
    // *aBuffer = ( (uint32_t)*aBuffer & (Color << (8 * aY)) );
}

void app_main(void)
{
    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
    static lv_disp_drv_t disp_drv;      // contains callback functions

    // I80Initialise(example_notify_lvgl_flush_ready, &disp_drv);
    I80Initialise(I80TransferDoneCallback, NULL);

    ESP_LOGI(TAG, "Initialize LVGL library");
    // lv_init();

    // Allocate draw buffers for LGVL
    uint8_t *buf1 = NULL;
    Color18 *buf2 = NULL;

#if CONFIG_EXAMPLE_LCD_I80_COLOR_IN_PSRAM
    buf1 = (uint8_t*)heap_caps_aligned_alloc(I80_PSRAM_DATA_ALIGNMENT, I80_LCD_H_RES * I80_LCD_V_RES * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    buf1 = (uint8_t*)heap_caps_malloc(I80_LCD_H_RES * I80_LCD_V_RES * 3 /*sizeof(lv_color_t)*/, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
#endif
    assert(buf1);
#if CONFIG_EXAMPLE_LCD_I80_COLOR_IN_PSRAM
    buf2 = (Color18*)heap_caps_aligned_alloc(I80_PSRAM_DATA_ALIGNMENT, I80_LCD_H_RES * I80_LCD_V_RES * sizeof(Color18), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    buf2 = (Color18*)heap_caps_malloc(I80_LCD_H_RES * I80_LCD_V_RES * sizeof(Color18), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
#endif
    assert(buf2);
    ESP_LOGI(TAG, "buf1@%p, buf2@%p", buf1, buf2);

    memset(buf1, 0x00, I80_LCD_H_RES * I80_LCD_V_RES * 3);
    memset(buf2, 0x00, I80_LCD_H_RES * I80_LCD_V_RES * sizeof(Color18));

    int LoopCounter = 0;

    int LineYPosition = 0;
    int Direction = 1;

    Color24 LineColor = { 255, 0, 0 };
    Color18 LineColor18 = { .Color = {255, 0, 0} };

    uint8_t Red = 5, Green = 1, Blue = 1;
    uint8_t RedDir = 1, GreenDir = 1, BlueDir = 1;

    while(1)
    {
        while(I80TransferDone == 0) { ; }
        memset(buf2, 0xFF, I80_LCD_H_RES * I80_LCD_V_RES * 3);
        // memset(buf1 + (I80_LCD_H_RES * LineYPosition * 3), 0x00, I80_LCD_H_RES * 3);
        for(int i = 0; i < 128; i++) 
        {
            SetPixel18(i, LineYPosition, LineColor18, buf2);
        }

        Red += RedDir;
        if(Red == 4 || Red == 252) RedDir = -RedDir;

        LineColor18.Colors.R = Red;
        LineColor18.Colors.G = Green;
        LineColor18.Colors.B = Blue;
        
        I80TransferFullSynced(buf2);

        LineYPosition += Direction;
        if(LineYPosition == 160) Direction = -1;
        else if(LineYPosition == 0) Direction = 1;

        vTaskDelay(1);
        // ESP_LOGI(TAG, "LOOP %d", LineYPosition);
    }

    // initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, I80_LCD_H_RES * I80_LCD_V_RES);

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = I80_LCD_H_RES;
    disp_drv.ver_res = I80_LCD_V_RES;
    disp_drv.flush_cb = example_lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = PanelHandle;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

    ESP_LOGI(TAG, "Display LVGL animation");
    example_lvgl_demo_ui(disp);

    while (1) {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }
}
