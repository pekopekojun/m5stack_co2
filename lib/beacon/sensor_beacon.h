#ifndef SENSOR_BEACON_H
#define SENSOR_BEACON_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <stdint.h>

class SensorBeacon
{
private:
    BLEAdvertising *pAdvertising;
    char adv_packet[15];

public:
    enum class Sensor
    {
        Temp = 1,
        Hum = 2,
        Press = 4,
        Lum = 8,
        Co2 = 16,
    };
    
    SensorBeacon(uint8_t stype);
    void setup(void);
    void set_battery_type(uint8_t type);
    void set_remaining_battery(uint16_t mv);
    void set_temp(int16_t v);
    void set_hum(int8_t v);
    void set_press(uint16_t v);
    void set_lum(uint16_t v);
    void set_co2(uint16_t v);
    void set_adv(void);
    void start(void);
    void stop(void);
};

#endif // SENSOR_BEACON_H