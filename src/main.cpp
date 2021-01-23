#include <M5Stack.h>
#include <Wire.h>
#include <CircularBuffer.h>
#include "QuickStats.h"
#include <Adafruit_BME280.h>
#include "MHZ19.h"

#include "sensor_beacon.h"

#define RX_PIN 16     // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 17     // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600 // Device to MH-Z19 Serial baudrate (should not be changed)

#define SEALEVELPRESSURE_HPA (1013.25)

MHZ19 myMHZ19; // Constructor for library
HardwareSerial mySerial(1);
SensorBeacon SBeacon((uint8_t)SensorBeacon::Sensor::Co2 |
                         (uint8_t)SensorBeacon::Sensor::Temp |
                         (uint8_t)SensorBeacon::Sensor::Hum |
                         (uint8_t)SensorBeacon::Sensor::Press,
                     0, 0x01);
QuickStats stats;    //initialize an instance of this class
Adafruit_BME280 bme; // I2C

static int32_t s_mode = 0;
static uint32_t s_last = 0;
static uint32_t s_calib_time = 0;
static bool s_B_long_press = false;
static float s_temp[16] = {0};
static int8_t s_temp_pos = 0;
static CircularBuffer<uint16_t, 100> s_buffer;

void setup()
{
    Serial.begin(115200);

    // initialize the M5Stack object
    M5.begin();
    M5.Speaker.begin();
    M5.Speaker.mute();
    /* Power chip connected to gpio21, gpio22, I2C device
    Set battery charging voltage and current
    If used battery, please call this function in your project */
    M5.Power.begin();

    // initialize the MHZ19
    mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
    myMHZ19.begin(mySerial);
    myMHZ19.autoCalibration(false); // Turn auto calibration ON (OFF autoCalibration(false))

    // initialize the BME280
    bme.begin(0x76, &Wire);

    // Lcd Setting
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(WHITE, BLACK);

    // initialize the Sensor beacon
    SBeacon.setup(1000, 1000);
    SBeacon.start();
}

void loop()
{
    /* note: getCO2() default is command "CO2 Unlimited". This returns the correct CO2 reading even
    if below background CO2 levels or above range (useful to validate sensor). You can use the
    usual documented command with getCO2(false) */
    if (millis() - s_last > 5000)
    {
        //printValues();
        s_last = millis();

        uint16_t CO2 = myMHZ19.getCO2(); // Request CO2 (as ppm)
        float temp = bme.readTemperature();
        float press = bme.readPressure() / 100.0F;
        float hum = bme.readHumidity();
        if (CO2 < 10000)
        {
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.printf("CO2 (ppm): %5d\n", CO2);

            SBeacon.setPress((uint16_t)press);
            SBeacon.setTemp((int16_t)(temp * 100));
            SBeacon.setHum((uint8_t)hum);
            SBeacon.setCo2(CO2);
            SBeacon.stop();
            SBeacon.setAdv();
            SBeacon.start();

            s_temp[s_temp_pos] = myMHZ19.getTemperature(true, false);
            M5.Lcd.printf("Temp(co2): %.2f\n", s_temp[s_temp_pos]);

            float tmp_sd = stats.stdev(s_temp, 16);
            M5.Lcd.printf("Temp(sd): %.4f\n", tmp_sd);
            s_temp_pos++;
            s_temp_pos &= 0xf;

            M5.Lcd.printf("Temp : %.3f\n", temp);
            M5.Lcd.printf("Hum  : %.3f\n", hum);
            M5.Lcd.printf("Press: %.3f\n", press);

            if (tmp_sd >= 0.07f)
            {
                s_buffer.clear();
            }

            if (CO2 != 0)
            {
                s_buffer.push(CO2);
            }

            if (s_buffer.isFull())
            {
                int32_t size = s_buffer.size();
                uint32_t ave = 0;
                for (int i = 0; i < size; i++)
                {
                    ave += s_buffer[i];
                }
                ave /= size;
                M5.Lcd.printf("Ave(co2): %4d\n", ave);

                if (ave <= 380)
                {
                    // 12H cycle
                    if ((s_calib_time == 0) || ((millis() - s_calib_time) > (60 * 60 * 12)))
                    {
                        M5.Lcd.printf("\nCalibrating..");
                        myMHZ19.calibrate(); // Take a reading which be used as the zero point for 400 ppm
                        delay(3000);
                        M5.Lcd.clear(BLACK);
                        s_calib_time = millis();
                    }
                    s_buffer.clear();
                }
            }
            else
            {
                M5.Lcd.printf("Ave(co2): *%3d\n",s_buffer.size());
            }
        }
    }

    M5.update();
    if (M5.BtnA.wasPressed())
    {
        switch (s_mode)
        {
        case 0:
            M5.Lcd.sleep();
            M5.Lcd.setBrightness(0);
            s_mode++;
            break;
        case 1:
            M5.Lcd.wakeup();
            M5.Lcd.setBrightness(1);
            s_mode++;
            break;
        default:
            M5.Lcd.wakeup();
            M5.Lcd.setBrightness(200);
            s_mode = 0;
            break;
        }
    }

    if (M5.BtnB.pressedFor(3000))
    {
        s_B_long_press = true;
        M5.Lcd.wakeup();
        M5.Lcd.setBrightness(200);
        s_mode = 0;
        M5.Lcd.printf("\nCalibrating..");
        myMHZ19.calibrate(); // Take a reading which be used as the zero point for 400 ppm
        delay(3000);
        M5.Lcd.clear(BLACK);
    }
    if (M5.BtnB.wasReleased())
    {
        if (s_B_long_press)
        {
            s_B_long_press = false;
        }
    }
}
