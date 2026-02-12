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

## Маппинг в Яндекс

Обычно через Zigbee-шлюз будут видны:

- fan/switch (on/off);
- fan speed (0..100%);
- temperature sensor;
- humidity sensor;
- analog sensor (ресурс фильтра в %).

Если шлюз не публикует `analog` как отдельный параметр, используйте маппинг на стороне шлюза (например, через внешний конвертер Zigbee2MQTT / шаблон в интеграции).
