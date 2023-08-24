
// TODO: Add config option in menuconfig
#if CONFIG_EXAMPLE_LCD_I80_COLOR_IN_PSRAM
// PCLK frequency can't go too high as the limitation of PSRAM bandwidth
#define I80_LCD_PIXEL_CLOCK_HZ     (2 * 1000 * 1000)
#else
#define I80_LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)
#endif // CONFIG_EXAMPLE_LCD_I80_COLOR_IN_PSRAM

/* Display Specific */
// TODO: Move to Display Specific configuration file
// The pixel number in horizontal and vertical
#define I80_LCD_H_RES              128
#define I80_LCD_V_RES              160

// Bit number used to represent command and parameter
#define I80_LCD_CMD_BITS           8
#define I80_LCD_PARAM_BITS         8

#if CONFIG_EXAMPLE_LCD_I80_CONTROLLER_ILI9341
#endif

/* PSRAM */ 
// Supported alignment: 16, 32, 64. A higher alignment can enable higher burst transfer size, 
// thus a higher i80 bus throughput.
#define I80_PSRAM_DATA_ALIGNMENT   64

/* I80 Bus */
// TODO: Add config option in menuconfig
#define I80_LCD_BUS_WIDTH   8