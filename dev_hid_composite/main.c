#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "bsp/board_api.h"
#include "tusb.h"
#include "hardware/gpio.h"
#include "usb_descriptors.h"

// Pin definitions
#define PIN_BTN_UP      10
#define PIN_BTN_DOWN    11
#define PIN_BTN_LEFT    12
#define PIN_BTN_RIGHT   13
#define PIN_BTN_MODE    14
#define PIN_LED_MODE    15

enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
bool circle_mode = false;
uint32_t last_toggle_time = 0;

void led_blinking_task(void);
void hid_task(void);

int main(void) {
  board_init();
  tud_init(BOARD_TUD_RHPORT);

  // Button setup
  gpio_init(PIN_BTN_UP);     gpio_set_dir(PIN_BTN_UP, GPIO_IN);     gpio_pull_up(PIN_BTN_UP);
  gpio_init(PIN_BTN_DOWN);   gpio_set_dir(PIN_BTN_DOWN, GPIO_IN);   gpio_pull_up(PIN_BTN_DOWN);
  gpio_init(PIN_BTN_LEFT);   gpio_set_dir(PIN_BTN_LEFT, GPIO_IN);   gpio_pull_up(PIN_BTN_LEFT);
  gpio_init(PIN_BTN_RIGHT);  gpio_set_dir(PIN_BTN_RIGHT, GPIO_IN);  gpio_pull_up(PIN_BTN_RIGHT);
  gpio_init(PIN_BTN_MODE);   gpio_set_dir(PIN_BTN_MODE, GPIO_IN);   gpio_pull_up(PIN_BTN_MODE);

  // LED setup
  gpio_init(PIN_LED_MODE);   gpio_set_dir(PIN_LED_MODE, GPIO_OUT);
  gpio_put(PIN_LED_MODE, 0); // off initially

  if (board_init_after_tusb) board_init_after_tusb();

  while (1) {
    tud_task(); // TinyUSB device task
    led_blinking_task();
    hid_task();
  }
}


// USB events
void tud_mount_cb(void)    { blink_interval_ms = BLINK_MOUNTED; }
void tud_umount_cb(void)  { blink_interval_ms = BLINK_NOT_MOUNTED; }
void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}
void tud_resume_cb(void) {
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

// HID reporting
void hid_task(void) {
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;
  uint32_t now = board_millis();
  if (now - start_ms < interval_ms) return;
  start_ms += interval_ms;

  // Toggle mode button (debounced)
  static bool last_mode_btn = true;
  bool mode_btn = gpio_get(PIN_BTN_MODE);
  if (!mode_btn && last_mode_btn && (now - last_toggle_time > 300)) {
    circle_mode = !circle_mode;
    gpio_put(PIN_LED_MODE, circle_mode);
    last_toggle_time = now;
  }
  last_mode_btn = mode_btn;

  int8_t dx = 0, dy = 0;

  if (!circle_mode) {
    // Directional button logic with acceleration
    bool btns[4] = {
      !gpio_get(PIN_BTN_UP),
      !gpio_get(PIN_BTN_DOWN),
      !gpio_get(PIN_BTN_LEFT),
      !gpio_get(PIN_BTN_RIGHT)
    };

    static uint32_t press_time[4] = {0, 0, 0, 0};
    for (int i = 0; i < 4; i++) {
      if (btns[i]) {
        if (press_time[i] == 0) press_time[i] = now;
      } else {
        press_time[i] = 0;
      }
    }

    int speeds[4] = {0, 0, 0, 0};
    for (int i = 0; i < 4; i++) {
      uint32_t t = (press_time[i] == 0) ? 0 : (now - press_time[i]);
      if (t > 1000) speeds[i] = 5;
      else if (t > 500) speeds[i] = 3;
      else if (t > 100) speeds[i] = 1;
    }

    dy = speeds[0] * -1 + speeds[1]; // Up is negative Y
    dx = speeds[3] - speeds[2];      // Right - Left

  } else {
    // Circle mode
    static float angle = 0.0f;
    angle += 0.1f;
    if (angle > 6.28318f) angle -= 6.28318f;
    dx = (int8_t)(3.0f * cosf(angle));
    dy = (int8_t)(3.0f * sinf(angle));
  }

  tud_hid_mouse_report(REPORT_ID_MOUSE, 0, dx, dy, 0, 0);
}

// Blinking status LED (on-board)
void led_blinking_task(void) {
  static uint32_t start_ms = 0;
  static bool led_state = false;
  if (!blink_interval_ms) return;

  if (board_millis() - start_ms < blink_interval_ms) return;
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = !led_state;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) bufsize;
// No output report handling required for this project
}

// Invoked when received GET_REPORT control request.
// Must return the report content (or 0 if not supported).
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;
// Not used in this project
return 0;
}
