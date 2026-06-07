# Cyberpunk Watch Face — LilyGo T-Watch S3

Cyberpunkowy watch face dla LilyGo T-Watch S3 (Radio-SX1262) z zielonym tekstem na czarnym tle.

## Wygląd

Zielony terminal (#00FF41) na czarnym tle, font Montserrat 48px.
Godzina w dużym formacie, data poniżej.

## Hardware

- **Board:** LilyGo T-Watch S3, wariant Radio-SX1262
- **MCU:** ESP32-S3
- **Display:** 240x240 px

## Biblioteki

- [LilyGoLib](https://github.com/Xinyuan-LilyGO/LilyGoLib)
- [SensorLib](https://github.com/lewisxhe/SensorsLib) v0.3.3
- [RadioLib](https://github.com/jgromes/RadioLib)
- LVGL v9 (dolaczony do LilyGoLib)

## Ustawienia Arduino IDE

- Board: LilyGo T-Watch S3
- Board Revision: Radio-SX1262
- Partition: 16M Flash (3M APP/9.9MB FATFS)
- USB CDC On Boot: Enabled
- CPU Frequency: 240MHz WiFi

## Funkcje

- Wyswietlanie godziny i daty w czasie rzeczywistym (RTC PCF8563)
- Auto-dimming: pelna jasnosc -> 5s -> przyciemnienie (DIM 30) -> 15s -> ekran off -> 30s -> light sleep
- Wybudzenie dotykiem ekranu

## Upload

Wymaga recznego trybu download:
1. Przytrzymaj BOOT
2. Nacisnij crown (reset)
3. Pusc BOOT podczas Connecting... w esptool

## Poprawki bibliotek

- instance.begin(NO_HW_GPS) - wymagane, wariant Radio-SX1262 nie ma GPS
- Bufory LVGL zmniejszone do lv_buffer_size / 2 w LV_Helper_v9.cpp (linie 202, 204)
- strftime() zastapiony przez snprintf z tablicami nazw (crash na niezainicjalizowanym RTC)
