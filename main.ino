/**
 * @file      program1.ino
 * @author    K.T.Kraft@protonmail.com
 * @license   Gnu Affero Licence
 * @note      Arduino esp version 2.0.9 Setting , not support esp 3.x
 *            Tools ->
 *                  Board:"ESP32S3 Dev Module"
 *                  USB CDC On Boot:"Enable"
 *                  CPU Frequency: "240MHz (WiFi)"
 *                  Core Debug Level: "Verbose"
 *                  USB DFU On Boot: "Disabled"
 *                  Erase All Flash Before Sketch Upload: "Disabled"
 *                  Events Run On: "Core 1"
 *                  Flash Mode: "QI0 80MHz"
 *                  Flash Size: "16MB (128Mb)"
 *                  JTAG Adapter: "Disabled"
 *                  Arduino Runs On: "Core 1"
 *                  USB Firmware MSC On Boot: "Disabled"
 *                  Partition Scheme: "16M Flash (3MB APP/9.9MB FATFS)"
 *                  PSRAM: "OPI PSRAM"
 *                  Upload Mode: "UART0/Hardware CDC"
 *                  Upload Speed: "921600"
 *                  USB Mode: "Hardware CDC and JTAG"
 *                  Programmer: "Esptool"
 */

#define ENABLE_IR_SENDER

#include <LilyGoLib.h>

#include <Blackout.hpp>

#include <wifiSniff.hpp>

#include <Shell.hpp>

void setup()
{
    
    //Set up the display
    watch.begin();

}

void loop(){

    switch (launcher()){
        case 0:
            watch.shutdown();
            break;
        case 1:
            blackout();
            break;
        case 2:
            //wifiApp(); break; //Incomplete Removed Temporarily
            appLaunchError();
            break;
        case 3:
            emuShell(); //Incomplete Removed Temporarily
            break;
        default: 
            appLaunchError();
            break;
    }


}

u8_t launcher() {

    int16_t x, y;

    const char apps[20][20] = {"ShutDown", "Nighty", "WifiApp", "EmuShell"}; 

    u8_t appPage = 0;

    watch.setRotation(1);
    watch.fillScreen(TFT_BLACK);
    watch.setCursor(0, 0);

    watch.fillScreen(TFT_BLACK);
    watch.setTextSize(1);
    watch.setTextColor(TFT_PURPLE);

    watch.setRotation(2);

    watch.drawRect(10, 10, 150, 110,TFT_PURPLE);
    watch.drawRect(10, 120, 150, 110,TFT_PURPLE);

    watch.drawRect(160, 10, 70, 110,TFT_PURPLE);
    watch.drawRect(160, 120, 70, 110,TFT_PURPLE);

    watch.setCursor(40, 100);
    watch.println(F(""));

    appDisplay(apps[appPage], apps[appPage + 1 % 20]);

    u8_t move = 0;
    u8_t follow = 210;

    while (true) {
        move = (move + 1) % 220;
        follow = (follow + 1) % 230;

        for (u8_t i = 1; i < 5; i++ ){
            watch.setTextColor(TFT_PURPLE);
            watch.setRotation(i);
            watch.setCursor(move, 1);
            watch.println(F("="));
            watch.setTextColor(TFT_BLACK);
            watch.setCursor(follow, 1);
            watch.println(F("="));
        }

        watch.setRotation(2);

        if (watch.getPoint(&x, &y)) {
            watch.invertDisplay(0);
            watch.invertDisplay(1);

            move = 1;
            follow = 0;


            watch.setTextColor(TFT_PURPLE);
            watch.setCursor(40, 100);
            if (x > 160) {
                if( y > 120){
                    appPage = (appPage + 2) % 20;
                }else{
                    appPage = (appPage + 20 - 2) % 20;
                }
                appDisplay(apps[appPage], apps[appPage + 1 % 20]);
                while (watch.getTouched()){
                    watch.invertDisplay(0);
                    watch.invertDisplay(1);
                    delay(100);
                }
            }else{
                if(y > 120){
                    return appPage + 1 % 20;
                }else{
                    return appPage;
                }
            }

        }

        delay(10);
    }
}

void appDisplay(const char appName1 [20],const char appName2 [20]){
    watch.fillRect(12, 12, 146, 106, TFT_BLACK);
    watch.fillRect(12, 122, 146, 106, TFT_BLACK);
    watch.setCursor(20, 20);
    watch.println(F(appName1));
    watch.setCursor(20, 140);
    watch.println(F(appName2));
}

void appLaunchError(){
    while (watch.getTouched()){
        watch.fillScreen(TFT_BLACK);
        watch.setRotation(2);
        watch.setTextSize(2);
        watch.setTextColor(TFT_WHITE);
        watch.setCursor(10, 40);
        watch.println(F("it seems like this app doesn't exist yet"));
    }
}


