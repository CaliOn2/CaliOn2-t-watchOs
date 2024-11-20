#include <LilyGoLib.h>
#include <overlay.hpp>


// nand or xor and load store plus minus times (shift left) (shift right) jump compare load program 15 0000 0 0 000 000 

#include <overlay.hpp>
#include <LilyGoLib.h>

char hexToHexChar(int16_t hexNum){
    const int conversionArray[16] = {
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'A', 'B',
        'C', 'D', 'E', 'F'
    };
    return conversionArray[hexNum];
}

char binToBinChar (int16_t binNum) {
    const int conversionArray[2] = {
        '0', '1'
    };
    return conversionArray[binNum];
}

void textDisplayNewLine(char (*textBufP[24])[24]){
    uint8_t lineHeight;
    watch.fillRect(10, 0, 190, 200, TFT_BLACK);
    for (int p = 0; p < 24; p++) {
        lineHeight = 184 - p * 8;
        watch.setCursor(10, lineHeight);
        for (int i = 0; i < 24; i++){
            watch.print((*textBufP[p])[i]);
        }
    }
}

void textDisplay(char (*textBufP[24])[24]){
    const uint8_t lineHeight = 184;
    watch.setCursor(10, lineHeight);
    for (int i = 0; i < 24; i++){
        watch.print((*textBufP[0])[i]);
    }
}

void textNewLine(char (*textBufP[24])[24]) {
    char (*placeholdP)[24];

    for (int p = 0; p < 24; p++) {
        placeholdP = textBufP[0];
        textBufP[0] = textBufP[p];
        textBufP[p] = placeholdP;
    }

    for (int p = 0; p < 24; p++) {
        (*textBufP[0])[p] = ' ';
    }

    placeholdP = nullptr;
}

void lineLetters() {
    uint8_t lineHeight;
    for (int p = 0; p < 24; p++) {
        lineHeight = 184 - p * 8;
        watch.setCursor(0, lineHeight);
        watch.printf("%c:", 65 + p);
    }
    return;
}

void orI(uint16_t cache[16], uint16_t instruction) {
    cache[(instruction >> 4) & 0b1111] = cache[(instruction >> 12) & 0b1111] | cache[(instruction >> 8) & 0b1111];
}

void xorI(uint16_t cache[16], uint16_t instruction) {
    cache[(instruction >> 4) & 0b1111] = cache[(instruction >> 12) & 0b1111] ^ cache[(instruction >> 8) & 0b1111];
}

void andI(uint16_t cache[16], uint16_t instruction) {
    cache[(instruction >> 4) & 0b1111] = cache[(instruction >> 12) & 0b1111] & cache[(instruction >> 8) & 0b1111];
}

void nandI(uint16_t cache[16], uint16_t instruction) {
    cache[(instruction >> 4) & 0b1111] = ~(cache[(instruction >> 12) & 0b1111] & cache[(instruction >> 8) & 0b1111]);
}

void plusI(uint16_t cache[16], uint16_t instruction) {
    cache[(instruction >> 4) & 0b1111] = cache[(instruction >> 12) & 0b1111] + cache[(instruction >> 8) & 0b1111];
}

void minusI(uint16_t cache[16], uint16_t instruction) {
    cache[(instruction >> 4) & 0b1111] = cache[(instruction >> 12) & 0b1111] - cache[(instruction >> 8) & 0b1111];
}

void timesI(uint16_t cache[16], uint16_t instruction) {
    cache[(instruction >> 4) & 0b1111] = cache[(instruction >> 12) & 0b1111] * cache[(instruction >> 8) & 0b1111]; 
}

void shftLI(uint16_t cache[16], uint16_t instruction) {
    cache[(instruction >> 4) & 0b1111] = cache[(instruction >> 12) & 0b1111] << cache[(instruction >> 8) & 0b1111];
}

void shftRI(uint16_t cache[16], uint16_t instruction) {
    cache[(instruction >> 4) & 0b1111] = cache[(instruction >> 12) & 0b1111] >> cache[(instruction >> 8) & 0b1111];
}

void setI(uint16_t cache[16], uint16_t instruction) { 
    cache[(instruction >> 4) & 0b1111] = instruction >> 8;
}

void loadI(uint16_t cache[16], uint16_t instruction, uint8_t storage[64000]) {
    cache[(instruction >> 4) & 0b1111] = storage[cache[(instruction >> 12) & 0b1111]];
}

void storeI(uint16_t cache[16], uint16_t instruction, uint8_t storage[64000]) {
    storage[cache[(instruction >> 4) & 0b1111]] = cache[(instruction >> 12) & 0b1111]; 
}

void getProcCountI(uint16_t cache[16], uint16_t instruction, uint16_t *procCount) {
    cache[(instruction >> 4) & 0b1111] = *procCount;
}

void compI(uint16_t cache[16], uint16_t instruction, uint16_t *procCount) {
    if (cache[(instruction >> 12) & 0b1111] == cache[(instruction >> 8) & 0b1111]){
        *procCount = cache[(instruction >> 4) & 0b1111];
    }
}

void displayTextI(uint16_t cache[16], uint16_t instruction, uint8_t storage[64000], char (*textBufP[24])[24]) {
    const uint16_t adress = cache[(instruction >> 12) & 0b1111];
    const uint8_t outputType = (instruction >> 4) & 0b1111;

    textNewLine(textBufP);
    textDisplayNewLine(textBufP);

    switch (outputType) {
        case 0b1111:
            (*textBufP[0])[0] = '0';
            (*textBufP[0])[1] = 'x';
            for (int i = 0; i < 4; i++) {
                (*textBufP[0])[2 + i] = hexToHexChar((adress >> (12 - i * 4)) & 0b1111);
            }
            textDisplay(textBufP);
            break;
        case 0b1110:
            (*textBufP[0])[0] = '0';
            (*textBufP[0])[1] = 'b';
            for (int i = 0; i < 16; i++) {
                (*textBufP[0])[2 + i] = binToBinChar((adress >> (15 - i)) & 0b1);
            }
            textDisplay(textBufP);
            break;
        default:
            const uint16_t duration = cache[(instruction) >> 8 & 0b1111];
            uint16_t line = 0;
            for (int i = 0; i < duration; i++) {
                if (line == 24) {
                    line = 0;
                    textNewLine(textBufP);
                    textDisplayNewLine(textBufP);
                } else {
                    line++;
                }
                (*textBufP[0])[line] = storage[adress + duration];
                textDisplay(textBufP);
            }
            break;
    }
}

void loadExternalI() {
    
}

bool instructionInterpreter(uint8_t storage[64000], uint16_t *procCount, uint16_t cache[16], char (*textBufP[24])[24]) {
    const uint16_t instruction = (storage[*procCount] << 8) + storage[*procCount + 1];
    switch (instruction & 0b1111) {
        case 0b0000:
            orI(cache, instruction);
            if (instruction == 0 ) {
                return false;
            }
            break;
        case 0b0001:
            xorI(cache, instruction);
            break;
        case 0b0010:
            andI(cache, instruction);
            break;
        case 0b0011:
            nandI(cache, instruction);
            break;
        case 0b0100:
            plusI(cache, instruction);
            break;
        case 0b0101:
            minusI(cache, instruction);
            break;
        case 0b0110:
            timesI(cache, instruction);
            break;
        case 0b0111:
            shftLI(cache, instruction);
            break;
        case 0b1000:
            shftRI(cache, instruction);
            break;
        case 0b1001:
            setI(cache, instruction);
            break;
        case 0b1010:
            loadI(cache, instruction, storage);
            break;
        case 0b1011:
            storeI(cache, instruction, storage);
            break;
        case 0b1100:
            getProcCountI(cache, instruction, procCount);
            break;
        case 0b1101:
            compI(cache, instruction, procCount);
            break;
        case 0b1110:
            displayTextI(cache, instruction, storage, textBufP);
            break;
        case 0b1111:
            loadExternalI();
            break;
    }
    printf("%X\n", instruction);
    *procCount += 2;
    return true;
}

bool emuShell() {
    watch.fillScreen(TFT_BLACK);
    watch.setRotation(2);
    watch.setTextColor(TFT_BLACK);
    watch.fillRect(200, 0, 40, 240, TFT_DARKCYAN);
    watch.fillRect(122, 200, 118, 40,TFT_PURPLE);
    watch.fillRect(0, 200, 118, 40,TFT_PURPLE);
    watch.setTextSize(3);
    watch.setCursor(50, 195);
    watch.printf("0");
    watch.setCursor(170, 195);
    watch.printf("1");
    watch.setTextSize(1);
    watch.setCursor(0, 10);
    watch.setTextColor(TFT_PURPLE);

    lineLetters();

    int16_t x,y;
    uint16_t cache[16] = {
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        0, 0, 0, 0,
        0, 0, 0, 0
    };
    static uint8_t storage[64000];
    for (uint16_t i = 0; i < 64000; i++) {
        storage[i] = 0;
    }
    char (*textBufP[24])[24];
    char textBuf[24][24];
    for (uint16_t i = 0; i < 24; i++) {
        for (uint16_t p = 0; p < 24; p++) {
            textBuf[i][p] = 32;
        }
        textBufP[i] = &textBuf[i];
    }
    uint16_t storagePointer = 0;
    uint8_t byteBit = 0;
    uint8_t line = 0;
    bool running = false;
    uint8_t runCount = 0;


    overlay_display();
    while (true) {
        if (watch.getPoint(&x, &y)){
            if (overlay_detect(x, y)){
                return 0;
            }
            if(y > 200){
                if (x > 120) {
                    //1
                    storage[storagePointer] = storage[storagePointer] | (0b10000000 >> byteBit);
                    (*textBufP[0])[line] = '1';
                } else {
                    //0
                    storage[storagePointer] = storage[storagePointer] & ~(0b10000000 >> byteBit);
                    (*textBufP[0])[line] = '0';
                }
                if (byteBit == 7) {
                    byteBit = 0;
                    storagePointer++;                        
                } else {
                    byteBit++;
                }
                printf("Input Received\n");
                printf("this is what its 0 looks like: %02x\n", ~(0x01 << byteBit));
                printf("byte: %02x\n", storage[storagePointer]);
                line++;
                if (line == 4) {
                    line++;
                    (*textBufP[0])[line] = ' ';
                } else if (line == 9) {
                    line++;
                    (*textBufP[0])[line] = ' ';
                } else if (line == 14) {
                    line++;
                    (*textBufP[0])[line] = ' ';
                } else if (line == 19) {
                    line = 0;
                    textNewLine(textBufP);
                    textDisplayNewLine(textBufP);
                }
                textDisplay(textBufP);
            } else if (x > 200) {
                if (running == true) {
                    watch.fillTriangle(200, 130, 200, 150, 239, 140, TFT_GREEN);
                    running = false;
                } else {
                    watch.fillTriangle(200, 130, 200, 150, 239, 140, TFT_RED);
                    storagePointer = 0;
                    running = true;
                }
            }
            while (watch.getTouched()) {
                    delay(10);
            }
        }

        while (running == true && runCount < 200) {
            if (!instructionInterpreter(storage, &storagePointer, cache, textBufP)) {
                watch.fillTriangle(200, 130, 200, 150, 239, 140, TFT_GREEN);
                running = false;
                textNewLine(textBufP);
                textDisplayNewLine(textBufP);
                line = 0;
            }
            printf("procCount: %02x\n", storagePointer);
        }
        runCount = 0;
    }
}