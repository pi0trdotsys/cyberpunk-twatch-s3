#include <LilyGoLib.h>
#include <LV_Helper.h>

static lv_obj_t *lbl_time;
static lv_obj_t *lbl_date;
static uint32_t  lastMillis   = 0;
static uint32_t  lastActivity = 0;

#define BRIGHT        DEVICE_MAX_BRIGHTNESS_LEVEL
#define DIM           30
#define TIMEOUT_DIM   2000
#define TIMEOUT_OFF   10000
#define TIMEOUT_SLEEP 15000

typedef enum { STATE_BRIGHT, STATE_DIM, STATE_OFF } DispState;
static DispState dispState = STATE_BRIGHT;

void setupRTC() {
    const char *months[] = {"Jan","Feb","Mar","Apr","May","Jun",
                             "Jul","Aug","Sep","Oct","Nov","Dec"};

    // Parsuj czas kompilacji po stałych pozycjach
    int c_day  = ((__DATE__[4] == ' ') ? 0 : (__DATE__[4] - '0')) * 10
                 + (__DATE__[5] - '0');
    int c_year = (__DATE__[7]-'0')*1000 + (__DATE__[8]-'0')*100
                 + (__DATE__[9]-'0')*10  + (__DATE__[10]-'0');
    int c_month = 1;
    for (int i = 0; i < 12; i++) {
        if (strncmp(__DATE__, months[i], 3) == 0) { c_month = i + 1; break; }
    }
    int c_hour = (__TIME__[0]-'0')*10 + (__TIME__[1]-'0');
    int c_min  = (__TIME__[3]-'0')*10 + (__TIME__[4]-'0');
    int c_sec  = (__TIME__[6]-'0')*10 + (__TIME__[7]-'0');

    // Czas kompilacji jako sekundy (przybliżone)
    long c_total = c_hour * 3600L + c_min * 60 + c_sec;

    // Odczyt RTC
    struct tm t;
    instance.rtc.getDateTime(&t);
    long r_total = t.tm_hour * 3600L + t.tm_min * 60 + t.tm_sec;

    Serial.printf("RTC:      %d-%02d-%02d %02d:%02d:%02d\n",
                  1900 + t.tm_year, t.tm_mon + 1, t.tm_mday,
                  t.tm_hour, t.tm_min, t.tm_sec);
    Serial.printf("Compiled: %d-%02d-%02d %02d:%02d:%02d\n",
                  c_year, c_month, c_day, c_hour, c_min, c_sec);

    // Ustaw jeśli rok < 2024 lub różnica czasu > 5 minut
    long diff = abs(r_total - c_total);
    bool wrongYear = (1900 + t.tm_year < 2024);
    bool bigDiff   = (diff > 300 && diff < 86100); // >5min, <23h55min (wyklucza północ)

    if (wrongYear || bigDiff) {
        Serial.println("Setting RTC from compile time");
        instance.rtc.setDateTime(c_year, c_month, c_day, c_hour, c_min, c_sec);
    } else {
        Serial.println("RTC OK, skipping");
    }
}

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
    setupRTC();

    beginLvglHelper(instance);
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(scr, lv_color_hex(0x00FF41), LV_PART_MAIN);
    lbl_time = lv_label_create(scr);
    lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_label_set_text(lbl_time, "00:00");
    lv_obj_align(lbl_time, LV_ALIGN_TOP_MID, 0, 10);
    lbl_date = lv_label_create(scr);
    lv_label_set_text(lbl_date, "---");
    lv_obj_align(lbl_date, LV_ALIGN_TOP_MID, 0, 70);

    instance.setBrightness(BRIGHT);
    lastActivity = millis();
    updateTime();
}

void loop() {
    instance.loop();

    if (instance.getTouched()) {
        resetActivity();
    }

    uint32_t idle = millis() - lastActivity;

    if (dispState == STATE_BRIGHT && idle >= TIMEOUT_DIM) {
        instance.setBrightness(DIM);
        dispState = STATE_DIM;
    } else if (dispState == STATE_DIM && idle >= TIMEOUT_OFF) {
        instance.setBrightness(0);
        dispState = STATE_OFF;
    } else if (dispState == STATE_OFF && idle >= TIMEOUT_SLEEP) {
        instance.lightSleep(WAKEUP_SRC_TOUCH_PANEL);
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
