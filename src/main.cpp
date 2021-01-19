#include <M5Stack.h>

#include "MHZ19.h"

#include "sensor_beacon.h"

#define RX_PIN 16     // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 17     // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600 // Device to MH-Z19 Serial baudrate (should not be changed)

MHZ19 myMHZ19; // Constructor for library
HardwareSerial mySerial(1);
SensorBeacon SBeacon((uint8_t)SensorBeacon::Sensor::Co2, 0, 0x01);

static int32_t s_mode = 0;
static uint32_t s_last = 0;
static uint32_t s_calib_time = 0;
static bool s_B_long_press = false;
static int32_t s_co2_ave = 0;
static int32_t s_co2_ave_cnt = 0;

void setup()
{
    // initialize the M5Stack object
    M5.begin();
    M5.Speaker.begin();
    M5.Speaker.mute();
    /*
    Power chip connected to gpio21, gpio22, I2C device
    Set battery charging voltage and current
    If used battery, please call this function in your project
  */
    M5.Power.begin();

    // initialize the MHZ19
    mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
    myMHZ19.begin(mySerial);
    myMHZ19.autoCalibration(false); // Turn auto calibration ON (OFF autoCalibration(false))

    // Lcd Setting
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(WHITE, BLACK);

    // initialize the Sensor beacon
    SBeacon.setup(500,500);
    SBeacon.start();
}

void loop()
{

    /* note: getCO2() default is command "CO2 Unlimited". This returns the correct CO2 reading even
    if below background CO2 levels or above range (useful to validate sensor). You can use the
    usual documented command with getCO2(false) */
    if (millis() - s_last > 2000)
    {
        s_last = millis();

        uint16_t CO2 = myMHZ19.getCO2(); // Request CO2 (as ppm)
        if ((CO2 < 10000) && (CO2 > 0))
        {
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.printf("CO2 (ppm): %5d\n", CO2);

            SBeacon.setCo2(CO2);
            SBeacon.stop();
            SBeacon.setAdv();
            SBeacon.start();

            // Auto calib (Interval average)
            s_co2_ave += CO2;
            if (s_co2_ave_cnt >= 30)
            {
                if ((s_co2_ave / 30) <= 380)
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
                }
                s_co2_ave = 0;
                s_co2_ave_cnt = 0;
            }
            else
            {
                s_co2_ave_cnt++;
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
