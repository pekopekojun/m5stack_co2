

#include "sensor_beacon.h"

SensorBeacon::SensorBeacon(uint8_t stype, uint8_t btype, uint8_t id)
{
    const char adv_packet_internal[27 - 3]{
        /*BLE 4.1 */
        0xFF, 0xFF, /*00-01 Company ID : NO Company */
        0x01,       /*02    Version: 0x01:18-24  reserved  */
        id,         /*03    Beacon ID */
        btype,      /*04    Battery type: 0x00:wired supply
                                          0x01:Li-ion ptrn1
                                          0x02:Li-ion ptrn2 */
        stype,      /*05    Enable Sensor: bit0:Temp
                                            bit1:Hum
                                            bit2:Press
                                            bit3:Lum
                                            bit4:CO2*/
        0x00,       /*06    Error Code (reserved)*/
        0x00, 0x00, /*07-08 remaining battery
                                        Battery type: 0x00 then always 0x0000
                                                      0x01 then [mV]
                                                      0x02 then [%]*/

        0x00, 0x00, /*09-10 Temp  Value: signed short [0.01 degree] */
        0x00,       /*11    Hum   Value: unsigned char [%] */
        0x00, 0x00, /*12-13 Press Value: unsigned short [hPa]*/
        0x00, 0x00, /*14-15 Lum   Value: unsigned short [lux] */
        0x00, 0x00, /*16-17 CO2  Value: Unsigned short [ppm] */
        0x00, 0x00, /*18-24 (reserved)*/
        0x00, 0x00,
        0x00, 0x00};

    memcpy(adv_packet, adv_packet_internal, sizeof(adv_packet_internal));
}

void SensorBeacon::setup(uint32_t advint_min_ms, uint32_t advint_max_ms)
{
    uint16_t min = (advint_min_ms * 1000) / 625;
    uint16_t max = (advint_max_ms * 1000) / 625;
    pAdvertising = BLEDevice::getAdvertising();

    BLEDevice::init("");
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->setMinInterval(min);
    pAdvertising->setMaxInterval(max);

    setAdv();
}

void SensorBeacon::setErrorCode(uint8_t v)
{
    adv_packet[6] = v;
}

void SensorBeacon::setBatteryInfo(uint16_t v)
{
    adv_packet[7] = v & 0xff;
    adv_packet[8] = (v >> 8) & 0xff;
}

void SensorBeacon::setTemp(int16_t v)
{
    adv_packet[9] = v & 0xff;
    adv_packet[10] = (v >> 8) & 0xff;
}

void SensorBeacon::setHum(uint8_t v)
{
    adv_packet[11] = v;
}

void SensorBeacon::setPress(uint16_t v)
{
    adv_packet[12] = v & 0xff;
    adv_packet[13] = (v >> 8) & 0xff;
}

void SensorBeacon::setLum(uint16_t v)
{
    adv_packet[14] = v & 0xff;
    adv_packet[15] = (v >> 8) & 0xff;
}

void SensorBeacon::setCo2(uint16_t v)
{
    adv_packet[16] = v & 0xff;
    adv_packet[17] = (v >> 8) & 0xff;
}

void SensorBeacon::start(void)
{
    pAdvertising->start();
}

void SensorBeacon::stop(void)
{
    pAdvertising->stop();
}

void SensorBeacon::setAdv(void)
{
    BLEAdvertisementData adv_data = BLEAdvertisementData();

    adv_data.setName("sbn"); /* 3byte only */
    adv_data.setManufacturerData(std::string(adv_packet, sizeof(adv_packet)));
    pAdvertising->setAdvertisementData(adv_data);
}
