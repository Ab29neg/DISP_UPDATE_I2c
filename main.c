#include <Wire.h>
#include <SPI.h>
#include <PCA9557.h>
#include <lvgl.h>
#include "ui.h"
#include "gfx_conf.h"
#include "speed.h"
#include "soc.h"
#include "range.h"
#include "brake.h"
#include "charging.h"

static lv_disp_draw_buf_t draw_buf;
static lv_color_t disp_draw_buf1[screenWidth * screenHeight / 10];
static lv_color_t disp_draw_buf2[screenWidth * screenHeight / 10];
static lv_disp_drv_t disp_drv;
//PCA9557 Out;

int nano_addr = 0x08;  // Fixed Nano I2C address

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    tft.pushImageDMA(area->x1, area->y1, w, h, (lgfx::rgb565_t*)&color_p->full);
    lv_disp_flush_ready(disp);
}
 
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    uint16_t touchX, touchY;
    bool touched = tft.getTouch(&touchX, &touchY);
    if (!touched) {
        data->state = LV_INDEV_STATE_REL;
    } else {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

int read_speed_from_nano(uint8_t addr) {
    Wire1.beginTransmission(addr);
    // Wire1.write(0x01);  // Or some register/command your Nano expects
    // if (Wire1.endTransmission(false) != 0) {
    //     Serial.println("Nano I2C not responding");
    //     return -1;
    // }

    if (Wire1.requestFrom(addr, (uint8_t)1) == 1) {
        int speed = Wire1.read();
        return speed;
    }

    return -1;
}


void setup() {
    Serial.begin(115200);
    Serial.println("LVGL Dashboard Demo");

    tft.begin();

  //  Wire.begin(19, 20, 400000);  // SDA = 19, SCL = 20
    delay(100);

     //Out.reset();
    // Out.setMode(IO_OUTPUT);
    // Out.setState(IO0, IO_LOW);
    // Out.setState(IO1, IO_LOW);
    // delay(20);
    // Out.setState(IO0, IO_HIGH);
    // delay(100);
    // Out.setMode(IO1, IO_INPUT);

   
    tft.fillScreen(TFT_BLACK);

    // LVGL initialization
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf1, disp_draw_buf2, screenWidth * screenHeight / 10);
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.full_refresh = 1;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    ui_init();
    Serial.println("Setup Done");
}

void loop() {
    lv_timer_handler();
    delay(100);

    int speed = read_speed_from_nano(nano_addr);
    if (speed >= 0) {
        update_speed(speed);
            Serial.print("Recieved speed: ");
        Serial.println(speed);
    } else {
        Serial.println("Speed not updated");
    }
}
