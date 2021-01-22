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
    char adv_packet[27-3];

public:
    enum class Sensor
    {
        Temp = 0x01,
        Hum = 0x02,
        Press = 0x04,
        Lum = 0x08,
        Co2 = 0x10
    };

    SensorBeacon(uint8_t stype, uint8_t btype, uint8_t id);
    void setup(uint32_t advint_min_ms,  uint32_t advint_max_ms);
    void start(void);
    void stop(void);
    void setErrorCode(uint8_t v);
    void setBatteryInfo(uint16_t v);
    void setTemp(int16_t v);
    void setHum(uint8_t v);
    void setPress(uint16_t v);
    void setLum(uint16_t v);
    void setCo2(uint16_t v);
    void setAdv(void);
};

#endif // SENSOR_BEACON_H