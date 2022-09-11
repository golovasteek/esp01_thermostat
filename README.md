# Smart Thermostat based on ESP-01 board

![ESP-01 Relay module](/docs/ESP-01_relay.jpg)

The software to run on the thermostat to control temperature in the
room with a floor heating.

The target board is ESP-01, but it should be possible to run it on
any ESP8266 board.

The project uses DS18b20 as temperature sensors

GPIO 2 is used to connect with senosor
GPIO 0 is used to control relay, relay is supposed to be active low.
