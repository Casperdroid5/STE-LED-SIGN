#include <FastLED.h>

// Pin Definitions
#define LED_STRIP1_PIN    5     // Data pin for first LED strip (45 LEDs)
#define LED_STRIP2_PIN    13    // Data pin for second LED strip (52 WS2812B LEDs)
#define BUTTON_PIN        18    // Pin for button input
#define BUTTON_LED_PIN    4     // Pin for button's built-in LED

// LED Configuration
#define NUM_LEDS1         45    // Total number of LEDs in first strip
#define LEDS_PER_CHIP     3     // Number of LEDs per addressable chip (for strip 1)
#define NUM_CHIPS         (NUM_LEDS1 / LEDS_PER_CHIP)
#define NUM_LEDS2         52    // Total number of LEDs in second strip (WS2812B)

// Button debounce configuration
#define DEBOUNCE_DELAY    50    // Debounce time in milliseconds

// Define the arrays of LEDs for both strips
CRGB leds1[NUM_LEDS1];
CRGB leds2[NUM_LEDS2];

// Button state variables
int buttonState = HIGH;             // Current button state (HIGH = not pressed with pull-up)
int lastButtonState = HIGH;         // Previous button state
unsigned long lastDebounceTime = 0; // Last time the button state changed

// Mode control
int currentMode = 0;
const int NUM_MODES = 5;  // Total number of available modes

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  Serial.println("ESP32 Dual LED Strip Controller with Button Mode Selection");
  
  // Set pin modes
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Button with internal pull-up resistor
  pinMode(BUTTON_LED_PIN, OUTPUT);    // Button LED as output
  digitalWrite(BUTTON_LED_PIN, LOW);  // Initially off
  
  // Initialize FastLED for both strips
  FastLED.addLeds<WS2812B, LED_STRIP1_PIN, GRB>(leds1, NUM_LEDS1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, LED_STRIP2_PIN, GRB>(leds2, NUM_LEDS2).setCorrection(TypicalLEDStrip);
  
  // Set global brightness
  FastLED.setBrightness(64);  // 0-255
  
  // Initially turn all LEDs off
  FastLED.clear();
  FastLED.show();
}

void loop() {
  // Check if the button is pressed (with debounce)
  checkButton();
  
  // Run the current display mode
  runCurrentMode();
}

void checkButton() {
  // Read the button state
  int reading = digitalRead(BUTTON_PIN);
  
  // If the button state changed, reset the debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  // If the reading has been stable for the debounce period
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    // If the button state has changed
    if (reading != buttonState) {
      buttonState = reading;
      
      // If the button is pressed (LOW with pull-up resistor)
      if (buttonState == LOW) {
        // Change to the next mode
        currentMode = (currentMode + 1) % NUM_MODES;
        Serial.print("Mode changed to: ");
        Serial.println(currentMode);
        
        // Flash the button LED to indicate the new mode
        indicateModeWithButtonLED();
      }
    }
  }
  
  // Save the current reading as the last button state
  lastButtonState = reading;
}

void indicateModeWithButtonLED() {
  // Flash the button LED based on the current mode (1-5 flashes)
  for (int i = 0; i <= currentMode; i++) {
    // Turn LED on
    digitalWrite(BUTTON_LED_PIN, HIGH);
    delay(200);
    
    // Turn LED off
    digitalWrite(BUTTON_LED_PIN, LOW);
    delay(200);
  }
}

void runCurrentMode() {
  switch (currentMode) {
    case 0:
      // Rainbow - inverse patterns on each strip
      rainbowCycle(20);
      break;
    case 1:
      // Complementary colors on each strip
      dualColorMode(50);
      break;
    case 2:
      // Chase patterns in opposite directions
      dualChaseMode(50);
      break;
    case 3:
      // Sparkle effect on both strips
      dualSparkleMode(100, 20);
      break;
    case 4:
      // Alternating patterns
      alternatingPulseMode(20);
      break;
  }
}

// Rainbow effect with inverse patterns
void rainbowCycle(int speedDelay) {
  static uint8_t hue1 = 0;
  static uint8_t hue2 = 128; // Offset for second strip
  
  // First strip - normal rainbow
  fill_rainbow(leds1, NUM_LEDS1, hue1, 255 / NUM_LEDS1);
  
  // Second strip - reversed rainbow
  fill_rainbow(leds2, NUM_LEDS2, hue2, -255 / NUM_LEDS2);
  
  FastLED.show();
  
  // Increment hues in opposite directions
  hue1++;
  hue2--;
  
  delay(speedDelay);
}

// Complementary colors on each strip
void dualColorMode(int speedDelay) {
  static int currentLed1 = 0;
  static int currentLed2 = 0;
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate > speedDelay) {
    // Strip 1 - Red to Green to Blue cycle
    leds1[currentLed1] = CHSV(map(currentLed1, 0, NUM_LEDS1, 0, 255), 255, 255);
    
    // Strip 2 - Complementary colors (offset by 128 in hue)
    leds2[currentLed2] = CHSV(map(currentLed2, 0, NUM_LEDS2, 128, 128+255), 255, 255);
    
    FastLED.show();
    
    // Update LED positions
    currentLed1 = (currentLed1 + 1) % NUM_LEDS1;
    currentLed2 = (currentLed2 + 1) % NUM_LEDS2;
    
    // Clear LEDs when starting over
    if (currentLed1 == 0) fill_solid(leds1, NUM_LEDS1, CRGB::Black);
    if (currentLed2 == 0) fill_solid(leds2, NUM_LEDS2, CRGB::Black);
    
    lastUpdate = millis();
  }
}

// Chase patterns in opposite directions
void dualChaseMode(int speedDelay) {
  static int phase1 = 0;
  static int phase2 = 0;
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate > speedDelay) {
    // Clear all LEDs
    fill_solid(leds1, NUM_LEDS1, CRGB::Black);
    fill_solid(leds2, NUM_LEDS2, CRGB::Black);
    
    // Strip 1 - Blue chase, normal direction
    for (int i = 0; i < NUM_LEDS1; i += 3) {
      leds1[i + phase1] = CRGB::Blue;
    }
    
    // Strip 2 - Red chase, reverse direction
    for (int i = 0; i < NUM_LEDS2; i += 3) {
      leds2[i + phase2] = CRGB::Red;
    }
    
    FastLED.show();
    
    // Update phases in opposite directions
    phase1 = (phase1 + 1) % 3;
    phase2 = (phase2 - 1 + 3) % 3; // Ensure it stays positive
    
    lastUpdate = millis();
  }
}

// Sparkle effect on both strips with different colors
void dualSparkleMode(int density, int speedDelay) {
  // Fade existing LEDs
  fadeToBlackBy(leds1, NUM_LEDS1, 10);
  fadeToBlackBy(leds2, NUM_LEDS2, 10);
  
  // Add random sparkles with different colors
  for (int i = 0; i < density; i++) {
    // First strip - white sparkles
    int pos1 = random16(NUM_LEDS1);
    leds1[pos1] += CRGB::White;
    
    // Second strip - cyan sparkles
    int pos2 = random16(NUM_LEDS2);
    leds2[pos2] += CRGB::Cyan;
  }
  
  FastLED.show();
  delay(speedDelay);
}

// Alternating pulse patterns
void alternatingPulseMode(int speedDelay) {
  static uint8_t hue = 0;
  static bool pulseFirst = true;
  static int brightness = 0;
  static int fadeAmount = 5;
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate > speedDelay) {
    // Set colors based on current hue
    CRGB color1 = CHSV(hue, 255, 255);
    CRGB color2 = CHSV(hue + 128, 255, 255); // Complementary color
    
    if (pulseFirst) {
      // Pulse first strip, dim second strip
      fill_solid(leds1, NUM_LEDS1, color1);
      fill_solid(leds2, NUM_LEDS2, color2.fadeToBlackBy(200));
    } else {
      // Pulse second strip, dim first strip
      fill_solid(leds1, NUM_LEDS1, color1.fadeToBlackBy(200));
      fill_solid(leds2, NUM_LEDS2, color2);
    }
    
    // Adjust brightness for pulsing effect
    brightness += fadeAmount;
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount;
      
      // When we hit minimum brightness, toggle which strip pulses
      // and update the hue
      if (brightness <= 0) {
        pulseFirst = !pulseFirst;
        hue += 32; // Change color
      }
    }
    
    // Apply the brightness to both strips
    for (int i = 0; i < NUM_LEDS1; i++) {
      leds1[i].nscale8(pulseFirst ? brightness : 255 - brightness);
    }
    for (int i = 0; i < NUM_LEDS2; i++) {
      leds2[i].nscale8(pulseFirst ? 255 - brightness : brightness);
    }
    
    FastLED.show();
    lastUpdate = millis();
  }
}

// Function to handle triads of LEDs for strip 1 (if needed)
void setChipColor(int chipIndex, CRGB color) {
  // Calculate the starting index of the first LED in this chip
  int startIndex = chipIndex * LEDS_PER_CHIP;
  
  // Set all LEDs in this chip to the same color
  for (int i = 0; i < LEDS_PER_CHIP; i++) {
    leds1[startIndex + i] = color;
  }
}