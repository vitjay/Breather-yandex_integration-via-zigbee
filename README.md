# ESP32-C6 Breather Zigbee (Yandex Smart Home)

Скетч `breather_yandex_zigbee.ino` реализует Zigbee End Device для бризера с:

- управление: `вкл/выкл` и `скорость потока`;
- телеметрия: `температура`, `влажность`, `ресурс фильтра`.

## Что требуется

- Плата **ESP32-C6** с поддержкой Zigbee в Arduino Core (ветка `esp32` от Espressif).
- Zigbee-координатор, который умеет интеграцию в Яндекс Дом.

## Как использовать

1. Откройте скетч в Arduino IDE.
2. Выберите плату ESP32-C6.
3. При необходимости поменяйте пины:
   - `PIN_FAN_PWM`
   - `PIN_FAN_ENABLE`
   - `PIN_TEMP_HUM_SENSOR`
4. Замените функции-заглушки на реальные датчики:
   - `readTemperatureC()`
   - `readHumidityPercent()`
   - `readFilterResourcePercent()`
5. Загрузите скетч в устройство.
6. Переведите Zigbee-координатор в режим сопряжения.


## Что такое `Zigbee.h`

`Zigbee.h` — это **не файл из этого репозитория**. Он входит в Arduino Core от Espressif для ESP32 (пакет плат `esp32`) с поддержкой Zigbee для ESP32-C6/H2.

После установки/обновления пакета плат ESP32 в Arduino IDE заголовок подтягивается автоматически из установленного core, поэтому подключается как системная библиотека:

```cpp
#include <Zigbee.h>
```

Если IDE пишет `Zigbee.h: No such file or directory`, обычно это означает, что:

- установлен старый `esp32` core без Zigbee API;
- выбрана не Zigbee-совместимая плата/чип;
- пакет плат установлен некорректно и его нужно переустановить.


## Откуда берутся `ZigbeeFan`, `ZigbeeTempSensor` и другие команды

Строка:

```cpp
ZigbeeFan zbFan(ZB_ENDPOINT_FAN);
```

использует **класс `ZigbeeFan` из той же библиотеки `Zigbee.h`** (Arduino Core Espressif). Это готовые C++-обёртки Zigbee-кластеров (endpoints/devices), которые предоставляет core, например:

- `ZigbeeFan` — управление вентилятором (on/off, speed);
- `ZigbeeTempSensor` — датчик температуры;
- `ZigbeeHumiditySensor` — датчик влажности;
- `ZigbeeAnalog` — универсальный аналоговый sensor/value endpoint.

То есть эти «команды» не объявлены в вашем скетче — они приходят из установленного пакета плат ESP32.

Типичные пути, где можно увидеть исходники после установки `esp32` core:

- Linux: `~/.arduino15/packages/esp32/hardware/esp32/<version>/libraries/Zigbee/src/`
- Windows: `%LOCALAPPDATA%\Arduino15\packages\esp32\hardware\esp32\<version>\libraries\Zigbee\src\`
- macOS: `~/Library/Arduino15/packages/esp32/hardware/esp32/<version>/libraries/Zigbee/src/`

## Маппинг в Яндекс

Обычно через Zigbee-шлюз будут видны:

- fan/switch (on/off);
- fan speed (0..100%);
- temperature sensor;
- humidity sensor;
- analog sensor (ресурс фильтра в %).

Если шлюз не публикует `analog` как отдельный параметр, используйте маппинг на стороне шлюза (например, через внешний конвертер Zigbee2MQTT / шаблон в интеграции).
