#include <LilyGoLib.h>

bool overlay_display(){
    watch.fillTriangle(200, 0, 239, 40, 239, 0, TFT_BLACK);
    watch.drawTriangle(200, 0, 239, 40, 239, 0, TFT_PURPLE);
    return 0;
}

bool overlay_detect(int16_t x, int16_t y){
    if (x - 190 > y){
        while(watch.getTouched()){
            delay(100);
        }
        return 1;
    }
    return 0;
}