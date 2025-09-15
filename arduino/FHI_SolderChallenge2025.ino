/// @file    Blink.ino
/// @brief   Blink the first LED of an LED strip
/// @example Blink.ino

// install the library to use FastLED.h
// https://docs.arduino.cc/libraries/fastled/
#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 25

#define LETTER_SPACING 0

#define NUM_PINS 28
// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI
#define DATA_PIN 29
#define CLOCK_PIN 13

// Define the array of leds
CRGB leds[NUM_LEDS];

// Checkmark pattern
int checkmark[] = { 3, 9, 13, 17, 21 };
int checkmarkSize = sizeof(checkmark) / sizeof(checkmark[0]);

// Cross pattern
int cross[] = { 0, 6, 12, 18, 23, 20, 16, 8, 4 };
int crossSize = sizeof(cross) / sizeof(cross[0]);

// Pointer to faulty led
int pointer[] = { 2, 7, 12, 17, 11, 13 };
int pointerSize = sizeof(pointer) / sizeof(pointer[0]);

// Snake sequence
int snakeOrder[] = { 0, 5, 10, 15, 20, 21, 16, 11, 6, 1, 2, 7, 12, 17, 24, 22, 18, 13, 8, 3, 4, 9, 14, 19, 23 };
int snakeSize = sizeof(snakeOrder) / sizeof(snakeOrder[0]);

#define NUM_ROWS 5
#define NUM_COLS 5


// Array for each symbol  
const byte FONT_5x5[][NUM_ROWS] = {

  /* 'P' */ {
    0b11100,
    0b10010,
    0b11100,
    0b10000,
    0b10000
    },

  /* 'A' */ {
    0b01000,
    0b10100,
    0b11100,
    0b10100,
    0b10100
    },

  /* 'S' */ {
    0b01100,
    0b10000,
    0b01000,
    0b00100,
    0b11000},

  /* ! */ {
    0b10000,
    0b10000,
    0b10000,
    0b00000,
    0b10000
    }, 

  /* 'F' */ {
    0b11100,
    0b10000,
    0b11000,
    0b10000,
    0b10000
    },

  /* I */ {
    0b11100,
    0b01000,
    0b01000,
    0b01000,
    0b11100
    }, 

  /* L */ {
    0b10000,
    0b10000,
    0b10000,
    0b10000,
    0b11110
    }, 

  /* ' ' */ {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000
    }, // space
};

 
// LED map for letter scrolling
const uint8_t LED_MAP[NUM_ROWS][NUM_COLS] = {
  { 0,  5, 10, 15, 20 },
  { 1,  6, 11, 16, 21 },
  { 2,  7, 12, 17, 24 },
  { 3,  8, 13, 18, 22 },
  { 4,  9, 14, 19, 23 }
};

// store patterns
int fontIndexFromChar(char c) {
  switch (c) {
    case 'P': return 0;
    case 'A': return 1;
    case 'S': return 2;
    case '!': return 3;
    case 'F': return 4;
    case 'I': return 5;
    case 'L': return 6;
    case ' ': return 7;
    default:  return 7; // unknown -> space
  }
}

// matrix coordinates
inline int indexFromRowCol(int row, int col) {
  return LED_MAP[row][col];
}


//GPIO pins
int gpioPins[29] = {
  0, 1, 2, 3, 4, 5, 6, 7,
  8, 9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23,
  24, 25, 26, 27, 28
};



// draw a single character column-by-column onto the 5x5 at a given global column
void drawTextColumnAt(int visCol, int globalCol, const char* text, CRGB color) {
  if (visCol < 0 || visCol >= NUM_COLS) return;

  // each character is NUM_COLS wide + 1 blank column spacing
  const int charWidth = NUM_COLS;
  const int spacing   = LETTER_SPACING;
  const int stride    = charWidth + spacing;

  if (globalCol < 0) return;

  // which character & which column inside that character?
  int charIdx     = globalCol / stride;
  int colInChar   = globalCol % stride;

  // outside the input string?
  int textLen = strlen(text);
  if (charIdx >= textLen) return;

  // spacing column?
  if (colInChar >= charWidth) return;

  // fetch the character's 5 row bitmap
  char c = toupper(text[charIdx]);
  int fi = fontIndexFromChar(c);

  // Copy pixels for this visible column
  for (int row = 0; row < NUM_ROWS; row++) {
    bool on = ((FONT_5x5[fi][row] >> (NUM_COLS - 1 - colInChar)) & 0x01);
     int idx = indexFromRowCol(row, visCol);
    if (idx >= 0 && idx < NUM_LEDS) {
      leds[idx] = on ? color : CRGB::Black;
    }
  }
}

// scroll the text 
void scrollText(const char* text, uint16_t stepDelayMs, CRGB color) {
  // total columns in the virtual text
  const int charWidth = NUM_COLS;
  const int spacing   = LETTER_SPACING;
  const int stride    = charWidth + spacing;
  int textCols = strlen(text) * stride;

  // sweep from off-screen right to off-screen left
  for (int offset = NUM_COLS; offset >= -textCols; offset--) {
    // Clear frame
    for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;

    // render visible window columns
    for (int visCol = 0; visCol < NUM_COLS; visCol++) {
      int globalCol = visCol - offset; // which text column appears in this window column?
      drawTextColumnAt(visCol, globalCol, text, color);
    }

    FastLED.show();
    delay(stepDelayMs);
  }
}


void setup() {
  Serial.begin(115200);

  // Initialize pins
  for (int i = 0; i < 28; i++) {
    pinMode(gpioPins[i], OUTPUT);
  }

  // Uncomment/edit one of the following lines for your leds arrangement.
  // ## Clockless types ##
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

  

  // FastLED.addLeds<SM16703, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<TM1829, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<TM1812, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<TM1809, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<TM1804, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS1903, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS1903B, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS1904, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS2903, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
  // FastLED.addLeds<WS2852, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
  // FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
  // FastLED.addLeds<GS1903, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<SK6812, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
  // FastLED.addLeds<SK6822, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<APA106, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<PL9823, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<SK6822, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2813, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<APA104, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2811_400, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<GE8822, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<GW6205, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<GW6205_400, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<LPD1886, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<LPD1886_8BIT, DATA_PIN, RGB>(leds, NUM_LEDS);
  // ## Clocked (SPI) types ##
  // FastLED.addLeds<LPD6803, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
  // FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
  // FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2803, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<SM16716, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<P9813, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);  // BGR ordering is typical
  // FastLED.addLeds<DOTSTAR, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);  // BGR ordering is typical
  // FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);  // BGR ordering is typical
  // FastLED.addLeds<SK9822, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);  // BGR ordering is typical
  FastLED.setBrightness(3);

  // for (int dot = 0; dot < NUM_LEDS; dot++) {

  //   for (int Hue = 0; Hue < 255; dot++) {
  //     Hue = Hue + 10;
  //     if (Hue == 0) {
  //       Hue = 10;
  //     }
  //     leds[dot] = CHSV(Hue, 255, 255);
  //     FastLED.show();
  //   }
  // }

  Serial.println("Ready: Type commands (p1, p2, ok, cross)");
}




void loop() {

  // leds[25] = leds[0];

  // for(int i = 0; i < 25 ; i++){
  //   leds[i] = leds[i+1];
  //   delay(3);
  // }
  //   FastLED.show();

  // Blink the last LED (faulty one)
  // leds[24] = CRGB::Red;
  // FastLED.show();
  // delay(500);
  // leds[24] = CRGB::Black;
  // FastLED.show();
  // delay(500);


  // // Parse all LEDs
  // for (int i=0; i<25; i++){
  // // Turn the LED on, then pause
  // leds[i] = CRGB::Red;
  // FastLED.show();
  // delay(100);
  // // Now turn the LED off, then pause
  // leds[i] = CRGB::Black;
  // FastLED.show();
  // delay(100);
  // }

  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');  // Read command from PuTTY
    command.trim();                                 // Remove spaces/newlines
    // type the commands to activate functions
    if (command == "p1") {
      Serial.println("pattern 1 selected");
      pattern1();
    } else if (command == "p2") {
      Serial.println("pattern 2 selected");
      pattern2();
    } else if (command == "p3") {
      Serial.println("show the faulty LED");
      ledParse();
    } else if (command == "s") {
      Serial.println("show snake animation");
      snakePattern();
    } else if (command == "ok") {
      Serial.println("show checkmark");
      showCheckmark();
    } else if (command == "cross") {
      Serial.println("show cross");
      showCross();
    }else if (command == "pass") {
      Serial.println("scroll P");
      scrollText("PASS!",210, CRGB::Green); 
    }
    else if (command == "fail") {
    Serial.println("scroll FAIL");
    scrollText("FAIL!", 210, CRGB::Red);
    } else {
      Serial.println("Unknown command");
    }
  }

  // ledParse();
  // delay(2000);

  // showPointer();
  // delay(1000);


  // showCheckmark();
  // delay(1000);

  // showCross();
  // delay(1000);

}



void pattern1() {

  for (int i = 0; i < 22; i++) {  // Stop before 22 to handle 22,23,24 separately
    if (i % 2 == 0) {
      leds[i] = CRGB::Green;  // Even index ON
    } else {
      leds[i] = CRGB::Black;  // Odd index OFF
    }
  }

  // Handle these seperately
  leds[22] = CRGB::Black;
  leds[23] = CRGB::Green;
  leds[24] = CRGB::Green;

  FastLED.show();

  // GPIO pattern, enable only odd numbers
  for (int i = 0; i <= NUM_PINS; i++) {
    if (i % 2 == 0) {
      digitalWrite(gpioPins[i], LOW);
      Serial.print("GPIO");
      Serial.print(gpioPins[i]);
      Serial.println(" = LOW");
    } else {
      digitalWrite(gpioPins[i], HIGH);
      Serial.print("GPIO");
      Serial.print(gpioPins[i]);
      Serial.println(" = HIGH");
    }
  }

  



  delay(3000);

  // Turn everything OFF
  for (int i = 0; i < 25; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();

  // Set all pins LOW
  for (int i = 0; i <= NUM_PINS; i++) {
    digitalWrite(gpioPins[i], LOW);
  }
  Serial.println("All pins LOW now");

}

void pattern2() {

  for (int i = 0; i < 22; i++) {  // Stop before 22 to handle 22,23,24 separately
    if (i % 2 == 0) {
      leds[i] = CRGB::Black;  // Even index OFF
    } else {
      leds[i] = CRGB::Green;  // Odd index ON
    }
  }

  leds[22] = CRGB::Green;
  leds[23] = CRGB::Black;
  leds[24] = CRGB::Black;

  FastLED.show();
  
  // Inverted GPIO pattern, enable only even numbers
  for (int i = 0; i <= NUM_PINS; i++) {
    if (i % 2 == 0) {
      digitalWrite(gpioPins[i], HIGH);
      Serial.print("GPIO");
      Serial.print(gpioPins[i]);
      Serial.println(" = HIGH");
    } else {
      digitalWrite(gpioPins[i], LOW);
      Serial.print("GPIO");
      Serial.print(gpioPins[i]);
      Serial.println(" = LOW");
    }
  }

  delay(3000);

  // Turn everything OFF
  for (int i = 0; i < 25; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();

   // Set all pins LOW
  for (int i = 0; i <= NUM_PINS; i++) {
    digitalWrite(gpioPins[i], LOW);
  }
  Serial.println("All pins LOW now");
}


void showPointer() {
  for (int i = 0; i < pointerSize; i++) {
    leds[pointer[i]] = CRGB::Green;
  }
  //Blink the last LED (faulty one)
  leds[24] = CRGB::Red;

  FastLED.show();
  delay(2000);

  // Turn them off
  for (int i = 0; i < pointerSize; i++) {
    leds[pointer[i]] = CRGB::Black;
  }
  leds[24] = CRGB::Black;

  FastLED.show();
}

void ledParse() {
  //Parse all LEDs
  for (int i = 0; i < 24; i++) {
    // Turn the LED on, then pause
    leds[i] = CRGB::Green;
    FastLED.show();
    delay(100);
    // Now turn the LED off, then pause
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(100);

    if (i == 21) {
      leds[24] = CRGB::Green;
      FastLED.show();
      delay(100);
      // Now turn the LED off, then pause
      leds[24] = CRGB::Black;
      FastLED.show();
      delay(100);
    }
  }

  // Turn on all LEDs except D24
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i == 24) continue;
    leds[i] = CRGB::Green;
  }
  leds[24] = CRGB::Red;
  FastLED.show();
  delay(2000);

  // Turn all LEDs off
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  delay(1000);
}

void showCheckmark() {
  //Turn on LEDs for the checkmark one by one
  for (int i = 0; i < checkmarkSize; i++) {
    leds[checkmark[i]] = CRGB::Green;
    FastLED.show();
    delay(100);
  }

  FastLED.show();
  delay(2000);

  // Turn them off
  for (int i = 0; i < checkmarkSize; i++) {
    leds[checkmark[i]] = CRGB::Black;
  }
  FastLED.show();
}

void showCross() {
  // Turn on LEDs for the cross in one by one
  for (int i = 0; i < crossSize; i++) {
    leds[cross[i]] = CRGB::Red;
    FastLED.show();
    delay(100);
  }
  FastLED.show();
  delay(2000);

  // Turn them off
  for (int i = 0; i < crossSize; i++) {
    leds[cross[i]] = CRGB::Black;
  }
  FastLED.show();
}

void snakePattern() {
  for (int i = 0; i < snakeSize; i++) {
    leds[snakeOrder[i]] = CRGB::Orange;  // Turn ON current LED
    FastLED.show();
    delay(150);

    // Optional: turn OFF previous LED for moving dot effect
    if (i > 0) {
      leds[snakeOrder[i - 1]] = CRGB::Black;
    }
  }

  // Turn off last LED
  leds[snakeOrder[snakeSize - 1]] = CRGB::Black;
  FastLED.show();
  delay(500);
}

/*
  // Turn the LED on, then pause
  leds[4] = CRGB::Red;
  FastLED.show();
  delay(500);
  // Now turn the LED off, then pause
  leds[4] = CRGB::Black;
  FastLED.show();
  delay(500);

*/
