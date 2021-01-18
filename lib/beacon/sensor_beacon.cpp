

#include "sensor_beacon.h"

SensorBeacon::SensorBeacon(uint8_t stype)
{
    const char adv_packet_internal[15]{
        0xe0, 0x00, /*00-01 Company ID : Google */
        0x01,       /*02    Beacon Number */
        0x00,       /*03    Error Code (reserved)*/
        0x00,       /*04    Battery type: 0x00:wired supply
                                      0x01:Li-ion ptrn1
                                      0x02:Li-ion ptrn2 */
        0x00, 0x00, /*05-06 remaining battery
                                        Battery type: 0x00 then always 0x0000
                                                      0x01 then [mV]
                                                      0x02 then [%]*/
        stype,      /*07    Enable Sensor: bit0:Temp
                                            bit1:Hum
                                            bit2:Press
                                            bit3:Lum
                                            bit4:CO2*/
        0x00, 0x00, /*08-09 Temp  Value: signed short [0.01 degree]*/
        0x00,       /*10    Hum   Value: unsigned char [%]*/
        0x00, 0x00, /*11-12 Press Value: unsigned short [hPa]*/
        0x00, 0x00, /*13-14 Lum   Value: unsigned short [lux] */
                    /*0x00, 0x00, / 15-16 CO2  Value: Unsigned short [ppm]*/
    };
    memcpy(adv_packet, adv_packet_internal, 15);
}

void SensorBeacon::setup(void)
{
    pAdvertising = BLEDevice::getAdvertising();

    BLEDevice::init("");
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->setMinInterval(320);
    pAdvertising->setMaxInterval(320);

    set_adv();
}

void SensorBeacon::set_battery_type(uint8_t type)
{
    adv_packet[4] = type;
}

void SensorBeacon::set_remaining_battery(uint16_t mv)
{
    adv_packet[5] = mv & 0xff;
    adv_packet[6] = (mv >> 8) & 0xff;
}

void SensorBeacon::set_temp(int16_t v)
{
    adv_packet[8] = v & 0xff;
    adv_packet[9] = (v >> 8) & 0xff;
}

void SensorBeacon::set_hum(int8_t v)
{
    adv_packet[10] = v;
}

void SensorBeacon::set_press(uint16_t v)
{
    adv_packet[11] = v & 0xff;
    adv_packet[12] = (v >> 8) & 0xff;
}

void SensorBeacon::set_lum(uint16_t v)
{
    adv_packet[13] = v & 0xff;
    adv_packet[14] = (v >> 8) & 0xff;
}

void SensorBeacon::set_co2(uint16_t v)
{
    //adv_packet[15] = v & 0xff;
    //adv_packet[16] = (v >> 8) & 0xff;
}

void SensorBeacon::start(void)
{
    pAdvertising->start();
}

void SensorBeacon::stop(void)
{
    pAdvertising->stop();
}

void SensorBeacon::set_adv(void)
{
    BLEAdvertisementData adv_data = BLEAdvertisementData();

    adv_data.setName("SensorBeacon");
    adv_data.setManufacturerData(std::string(adv_packet, sizeof(adv_packet)));
    pAdvertising->setAdvertisementData(adv_data);
}
