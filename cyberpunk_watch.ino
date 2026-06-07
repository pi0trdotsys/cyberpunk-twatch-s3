#include <LilyGoLib.h>
#include <LV_Helper.h>

static lv_obj_t *lbl_time;
static lv_obj_t *lbl_date;
static uint32_t  lastMillis   = 0;
static uint32_t  lastActivity = 0;
static bool      sleeping     = false;

#define BRIGHT        DEVICE_MAX_BRIGHTNESS_LEVEL
#define DIM           30
#define TIMEOUT_DIM   5000
#define TIMEOUT_OFF   15000
#define TIMEOUT_SLEEP 30000

typedef enum { STATE_BRIGHT, STATE_DIM, STATE_OFF } DispState;
static DispState dispState = STATE_BRIGHT;

void updateTime() {
    struct tm t;
    instance.rtc.getDateTime(&t);
    if (t.tm_year < 100 || t.tm_year > 200) return;
    char buf[32];
    const char *dni[]  = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
    const char *mies[] = {"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
    int wday = (t.tm_wday >= 0 && t.tm_wday <= 6) ? t.tm_wday : 0;
    int mon  = (t.tm_mon  >= 0 && t.tm_mon  <= 11) ? t.tm_mon  : 0;
    snprintf(buf, sizeof(buf), "%02d:%02d", t.tm_hour, t.tm_min);
    lv_label_set_text(lbl_time, buf);
    snprintf(buf, sizeof(buf), "%s %02d %s %04d",
             dni[wday], t.tm_mday, mies[mon], 1900 + t.tm_year);
    lv_label_set_text(lbl_date, buf);
}

void resetActivity() {
    lastActivity = millis();
    dispState = STATE_BRIGHT;
    instance.setBrightness(BRIGHT);
}

void setup() {
    Serial.begin(115200);
    instance.begin(NO_HW_GPS);
    instance.rtc.setDateTime(2026, 6, 6, 12, 2, 0);

    beginLvglHelper(instance);
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(scr, lv_color_hex(0x00FF41), LV_PART_MAIN);
    lbl_time = lv_label_create(scr);
    lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_label_set_text(lbl_time, "00:00");
    lv_obj_align(lbl_time, LV_ALIGN_TOP_MID, 0, 10);
    lbl_date = lv_label_create(scr);
    lv_label_set_text(lbl_date, "SAT 06 JUN 2026");
    lv_obj_align(lbl_date, LV_ALIGN_TOP_MID, 0, 70);

    instance.setBrightness(BRIGHT);
    lastActivity = millis();
}

void loop() {
    instance.loop();

    // Dotknięcie ekranu — resetuj aktywność
    if (instance.getTouched()) {
        resetActivity();
    }

    uint32_t idle = millis() - lastActivity;

    // Stopniowe usypianie
    if (dispState == STATE_BRIGHT && idle >= TIMEOUT_DIM) {
        instance.setBrightness(DIM);
        dispState = STATE_DIM;
    } else if (dispState == STATE_DIM && idle >= TIMEOUT_OFF) {
        instance.setBrightness(0);
        dispState = STATE_OFF;
    } else if (dispState == STATE_OFF && idle >= TIMEOUT_SLEEP) {
        // Light sleep — po wybudzeniu kontynuuje od następnej linii
        instance.lightSleep(WAKEUP_SRC_TOUCH_PANEL);
        // Wybudzenie — przywróć jasność i czas
        resetActivity();
        updateTime();
    }

    if (millis() - lastMillis >= 1000) {
        lastMillis = millis();
        if (dispState != STATE_OFF) {
            updateTime();
        }
    }

    lv_task_handler();
    delay(5);
}
