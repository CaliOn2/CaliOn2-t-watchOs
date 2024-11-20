// CREDIT TO https://github.com/agrimpelhuber/esp8266-tvbgone
#include <LilyGoLib.h>
#include <overlay.hpp>
#include <IRsend.h>
#include <IRremoteESP8266.h>
#include <WORLD_IR_CODES.h>

IRsend irsend(2);

void xmitCodeElement(uint16_t ontime, uint16_t offtime, uint8_t PWM_code );
void delay_ten_us(uint16_t us);
uint8_t read_bits(uint8_t count);
uint16_t rawData[300];

#define MAX_WAIT_TIME 65535

extern const IrCode* const NApowerCodes[];
extern const IrCode* const EUpowerCodes[];
extern uint8_t num_NAcodes, num_EUcodes;

uint8_t bitsleft_r = 0;
uint8_t bits_r=0;
uint8_t code_ptr;
volatile const IrCode * powerCode;

uint8_t read_bits(uint8_t count)
{
  uint8_t i;
  uint8_t tmp=0;

  // we need to read back count bytes
  for (i=0; i<count; i++) {
    // check if the 8-bit buffer we have has run out
    if (bitsleft_r == 0) {
      // in which case we read a new byte in
      bits_r = powerCode->codes[code_ptr++];
      //DEBUGP(putstring("\n\rGet byte: ");
      //putnum_uh(bits_r);
      //);
      // and reset the buffer size (8 bites in a byte)
      bitsleft_r = 8;
    }
    // remove one bit
    bitsleft_r--;
    // and shift it off of the end of 'bits_r'
    tmp |= (((bits_r >> (bitsleft_r)) & 1) << (count-1-i));
  }
  // return the selected bits in the LSB part of tmp
  return tmp;
}

uint16_t ontime, offtime;
uint8_t i,num_codes;
uint8_t region;


void drawPentagram(int8_t x, int8_t y, uint32_t color) {
    watch.drawCircle(120 + x, 120 + y, 100, color);
    watch.drawLine(120 + x, 220 + y, 179 + x , 39 + y, color);
    watch.drawLine(120 + x, 220 + y, 61 + x, 39 + y, color);
    watch.drawLine(179 + x, 39 + y, 25 + x, 151 + y, color);
    watch.drawLine(61 + x, 39 + y, 215 + x, 151 + y, color);
    watch.drawFastHLine(25 + x, 151 + y, 190, color);
}

void delay_ten_us(uint16_t us) {
  uint8_t timer;
  while (us != 0) {
    // for 8MHz we want to delay 80 cycles per 10 microseconds
    // this code is tweaked to give about that amount.
    for (timer=0; timer <= DELAY_CNT; timer++) {
      NOP;
      NOP;
    }
    NOP;
    us--;
  }
}

void sendAllCodes() 
{
  bool endingEarly = false; //will be set to true if the user presses the button during code-sending 
      
  // determine region from REGIONSWITCH: 1 = NA, 0 = EU (defined in main.h)

    region = EU;
    num_codes = num_EUcodes;

  // for every POWER code in our collection
  for (i=0 ; i<num_codes; i++) 
  {


    // point to next POWER code, from the right database
    
      powerCode = EUpowerCodes[i];
    
    // Read the carrier frequency from the first byte of code structure
    const uint8_t freq = powerCode->timer_val;
    // set OCR for Timer1 to output this POWER code's carrier frequency



    // Get the number of pairs, the second byte from the code struct
    const uint8_t numpairs = powerCode->numpairs;

    // Get the number of bits we use to index into the timer table
    // This is the third byte of the structure
    const uint8_t bitcompression = powerCode->bitcompression;

    // For EACH pair in this code....
    code_ptr = 0;
    for (uint8_t k=0; k<numpairs; k++) {
      uint16_t ti;

      // Read the next 'n' bits as indicated by the compression variable
      // The multiply by 4 because there are 2 timing numbers per pair
      // and each timing number is one word long, so 4 bytes total!
      ti = (read_bits(bitcompression)) * 2;

      // read the onTime and offTime from the program memory
      ontime = powerCode->times[ti];  // read word 1 - ontime
      offtime = powerCode->times[ti+1];  // read word 2 - offtime

      rawData[k*2] = ontime * 10;
      rawData[(k*2)+1] = offtime * 10;
      yield();
    }

    // Send Code with library
    irsend.sendRaw(rawData, (numpairs*2) , freq);
    yield();
    //Flush remaining bits, so that next code starts
    //with a fresh set of 8 bits.
    bitsleft_r=0;

    // visible indication that a code has been output.
    drawPentagram(0, 0, TFT_MAGENTA);
    
    // delay 205 milliseconds before transmitting next POWER code
    delay_ten_us(20500);

    // if user is pushing (holding down) TRIGGER button, stop transmission early 
    if (watch.getTouched()) 
    {
      while (watch.getTouched()){
        yield();
      }
      endingEarly = true;
      delay_ten_us(50000); //500ms delay 
      drawPentagram(0, 0, TFT_RED);
      drawPentagram(0, 0, TFT_ORANGE);
      drawPentagram(0, 0, TFT_YELLOW);
      drawPentagram(0, 0, TFT_GREEN);
      //pause for ~1.3 sec to give the user time to release the button so that the code sequence won't immediately start again.
      delay_ten_us(MAX_WAIT_TIME); // wait 655.350ms
      delay_ten_us(MAX_WAIT_TIME); // wait 655.350ms
      break; //exit the POWER code "for" loop
    }
    
  } //end of POWER code for loop

  if (endingEarly==false)
  {
    //pause for ~1.3 sec, then flash the visible LED 8 times to indicate that we're done
    delay_ten_us(MAX_WAIT_TIME); // wait 655.350ms
    delay_ten_us(MAX_WAIT_TIME); // wait 655.350ms
    //quickflashLEDx(8);
  }

} //end of sendAllCodes

int blackout(){
    int16_t x, y;
    int8_t offsetX, offsetY;
    watch.fillScreen(TFT_BLACK);
    watch.setRotation(2);
    watch.setTextSize(2);
    watch.setTextColor(TFT_PURPLE);
    drawPentagram(0, 0, TFT_DARKGREY);
    overlay_display();

    irsend.begin();

    irsend.calibrate();
    
    while (true){
        if (watch.getPoint(&x, &y)){
            if (overlay_detect(x, y)){
                return 0;
            }
            while (watch.getTouched()){
                watch.fillScreen(TFT_BLACK);
                offsetX = random(-10, 10);
                offsetY = random(-10, 10);
                watch.fillScreen(TFT_BLACK);
                drawPentagram(offsetX, offsetY, TFT_DARKGREY);
            }

            watch.fillScreen(TFT_BLACK);

            sendAllCodes();

            drawPentagram(0, 0, TFT_DARKGREY);
            overlay_display();
        }
        delay(100);
    }
    return 0;
}
