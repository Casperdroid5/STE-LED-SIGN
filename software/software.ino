#include <FastLED.h>
#include <EEPROM.h>
#include <BluetoothSerial.h>

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` in ESP-IDF to enable it
#endif

// LED strip definitions
#define LED_PIN_LETTERS 13
#define LED_PIN_CIRCLE 5
#define BUTTON_PIN 18
#define BUTTON_LED_PIN 4

// LED counts
#define NUM_LEDS_S 21
#define NUM_LEDS_T 11
#define NUM_LEDS_E 18
#define NUM_LEDS_LETTERS (NUM_LEDS_S + NUM_LEDS_T + NUM_LEDS_E)
#define NUM_LEDS_CIRCLE 48

// LED arrays
CRGB lettersLeds[NUM_LEDS_LETTERS];
CRGB circleLeds[NUM_LEDS_CIRCLE];

// Letter segments for easier access
#define S_START 0
#define S_END (NUM_LEDS_S - 1)
#define T_START NUM_LEDS_S
#define T_END (NUM_LEDS_S + NUM_LEDS_T - 1)
#define E_START (NUM_LEDS_S + NUM_LEDS_T)
#define E_END (NUM_LEDS_LETTERS - 1)

// Animation speed control (lower = faster)
uint16_t letterAnimSpeed = 50;    // Default letter animation speed (milliseconds)
uint16_t circleAnimSpeed = 50;    // Default circle animation speed (milliseconds)

// Separate update timers for letters and circle
unsigned long lastLetterUpdateTime = 0;
unsigned long lastCircleUpdateTime = 0;

// Variables for snake-like hue shift
uint8_t circlePosition = 0;       // Position for snake-like effect
uint8_t circleHueOffset = 30;     // Hue difference between adjacent LEDs

// Button handling
int buttonState = HIGH;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long buttonPressStartTime = 0;
boolean buttonLongPressed = false;
boolean buttonVeryLongPressed = false;

// Mode control
#define NUM_MODES 4  // Regular modes (not including Bluetooth)
int currentMode = 0;
boolean bluetoothMode = false;

// Animation timing
unsigned long lastUpdateTime = 0;
unsigned long updateInterval = 50;  // 50ms for smooth animations

// EEPROM configuration
#define EEPROM_SIZE 32
#define MODE_ADDRESS 0
#define LETTER_COLOR_R_ADDRESS 1
#define LETTER_COLOR_G_ADDRESS 2
#define LETTER_COLOR_B_ADDRESS 3
#define CIRCLE_COLOR_R_ADDRESS 4
#define CIRCLE_COLOR_G_ADDRESS 5
#define CIRCLE_COLOR_B_ADDRESS 6
#define BRIGHTNESS_ADDRESS 7
#define LETTER_ENABLED_ADDRESS 8
#define CIRCLE_ENABLED_ADDRESS 9
#define CIRCLE_EFFECT_ADDRESS 10
#define LETTER_SPEED_ADDRESS 11
#define CIRCLE_SPEED_ADDRESS 12

unsigned long lastEepromWrite = 0;
#define WRITE_INTERVAL 60000  // Save every 1 minute after change (changed from 5s to 60s)
bool eepromNeedsSaving = false;

// LED color and state
CRGB letterColor = CRGB::Yellow;  // Default letter color
CRGB circleColor = CRGB::Yellow;  // Default circle color
uint8_t brightness = 125;         // Default brightness (max 125 as requested)
bool lettersEnabled = true;       // Letters enabled by default
bool circleEnabled = true;        // Circle enabled by default
uint8_t circleEffect = 0;         // 0=solid, 1=rainbow, 2=hue shift

// Animation variables
uint8_t hue = 0;
uint8_t chasePosition = 0;

// Sequential letter variables
int sequenceStep = 0;
unsigned long lastSequenceChange = 0;
unsigned long sequenceInterval = 800;

// Button press durations
const unsigned long BT_LONG_PRESS_DURATION = 3000;      // 3 seconds for BT mode
const unsigned long RESET_LONG_PRESS_DURATION = 10000;  // 10 seconds for reset

// Bluetooth
BluetoothSerial SerialBT;
unsigned long bluetoothStartTime = 0;
const unsigned long BLUETOOTH_TIMEOUT = 120000;  // 2 minutes timeout if no connection
bool bluetoothConnected = false;
unsigned long lastBluetoothActivity = 0;
char btBuffer[64];
int btBufferIndex = 0;

// Forward declarations for all functions
void clearAllLeds();
void loadSettingsFromEEPROM();
void saveSettingsToEEPROM();
void solidColorMode();
void letterChaseMode();
void sequentialLettersMode();
void letterPulseMode();
void handleBluetoothMode();
void processBluetooth();
void processBtCommand(char* command);
void enterBluetoothMode();
void exitBluetoothMode();
void resetDevice();
void checkButton();
void blinkButtonLED();

void setup() {
  Serial.begin(115200);

  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Init ledstrips
  FastLED.addLeds<WS2812B, LED_PIN_LETTERS, GRB>(lettersLeds, NUM_LEDS_LETTERS);
  FastLED.addLeds<WS2811, LED_PIN_CIRCLE, BRG>(circleLeds, NUM_LEDS_CIRCLE);

  // Initialize the button pin as input with pullup
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize button LED pin as output
  pinMode(BUTTON_LED_PIN, OUTPUT);
  digitalWrite(BUTTON_LED_PIN, LOW);

  // Load saved settings from EEPROM
  loadSettingsFromEEPROM();

  // Set initial brightness
  FastLED.setBrightness(brightness);

  // Start with all LEDs off
  clearAllLeds();
  FastLED.show();

  // Initialize Bluetooth (but don't start it yet)
  SerialBT.begin("STE_LED_Control");
  SerialBT.end();  // Turn off BT by default

  Serial.println("Enhanced LED controller initialized");
}

void loadSettingsFromEEPROM() {
  // Load mode
  currentMode = EEPROM.read(MODE_ADDRESS);
  if (currentMode >= NUM_MODES) {
    currentMode = 0;  // Default to mode 0 if stored value is invalid
  }

  // Load colors
  letterColor.r = EEPROM.read(LETTER_COLOR_R_ADDRESS);
  letterColor.g = EEPROM.read(LETTER_COLOR_G_ADDRESS);
  letterColor.b = EEPROM.read(LETTER_COLOR_B_ADDRESS);

  circleColor.r = EEPROM.read(CIRCLE_COLOR_R_ADDRESS);
  circleColor.g = EEPROM.read(CIRCLE_COLOR_G_ADDRESS);
  circleColor.b = EEPROM.read(CIRCLE_COLOR_B_ADDRESS);

  // If colors are all zeros (likely first boot), set defaults
  if (letterColor.r == 0 && letterColor.g == 0 && letterColor.b == 0) {
    letterColor = CRGB::Yellow;
  }
  if (circleColor.r == 0 && circleColor.g == 0 && circleColor.b == 0) {
    circleColor = CRGB::Yellow;
  }

  // Load brightness
  brightness = EEPROM.read(BRIGHTNESS_ADDRESS);
  if (brightness == 0 || brightness > 125) {
    brightness = 125;  // Default/max brightness
  }

  // Load enabled states
  lettersEnabled = EEPROM.read(LETTER_ENABLED_ADDRESS) > 0;
  circleEnabled = EEPROM.read(CIRCLE_ENABLED_ADDRESS) > 0;

  // Load circle effect
  circleEffect = EEPROM.read(CIRCLE_EFFECT_ADDRESS);
  if (circleEffect > 2) {  // 0=solid, 1=rainbow, 2=hue shift
    circleEffect = 0;
  }

  // Load animation speeds
  letterAnimSpeed = EEPROM.read(LETTER_SPEED_ADDRESS);
  circleAnimSpeed = EEPROM.read(CIRCLE_SPEED_ADDRESS);
  
  // Apply defaults if values are invalid
  if (letterAnimSpeed < 10 || letterAnimSpeed > 500) {
    letterAnimSpeed = 50;
  }
  
  if (circleAnimSpeed < 10 || circleAnimSpeed > 500) {
    circleAnimSpeed = 50;
  }

  Serial.println("Settings loaded from EEPROM");
}

void loop() {
  // Check for button press
  checkButton();
  
  // Update animations based on mode
  if (!bluetoothMode) {
    unsigned long currentTime = millis();
    
    // Run the appropriate animation based on current mode
    switch (currentMode) {
      case 0:
        // Mode 0: All yellow (or custom colors if set)
        solidColorMode();
        break;
      case 1:
        // Mode 1: Snake/chase effect
        letterChaseMode();
        break;
      case 2:
        // Mode 2: Sequential letters
        sequentialLettersMode();
        break;
      case 3:
        // Mode 3: Pulsing letters
        letterPulseMode();
        break;
    }
    
    // Update LEDs
    FastLED.show();
    
    // Check if we need to save settings to EEPROM
    if (eepromNeedsSaving && (millis() - lastEepromWrite > WRITE_INTERVAL)) {
      saveSettingsToEEPROM();
      eepromNeedsSaving = false;
      lastEepromWrite = millis();
    }
  } else {
    // Handle Bluetooth mode
    handleBluetoothMode();
  }
}

void saveSettingsToEEPROM() {
  // Save mode
  EEPROM.write(MODE_ADDRESS, currentMode);

  // Save colors
  EEPROM.write(LETTER_COLOR_R_ADDRESS, letterColor.r);
  EEPROM.write(LETTER_COLOR_G_ADDRESS, letterColor.g);
  EEPROM.write(LETTER_COLOR_B_ADDRESS, letterColor.b);

  EEPROM.write(CIRCLE_COLOR_R_ADDRESS, circleColor.r);
  EEPROM.write(CIRCLE_COLOR_G_ADDRESS, circleColor.g);
  EEPROM.write(CIRCLE_COLOR_B_ADDRESS, circleColor.b);

  // Save brightness
  EEPROM.write(BRIGHTNESS_ADDRESS, brightness);

  // Save enabled states
  EEPROM.write(LETTER_ENABLED_ADDRESS, lettersEnabled ? 1 : 0);
  EEPROM.write(CIRCLE_ENABLED_ADDRESS, circleEnabled ? 1 : 0);

  // Save circle effect
  EEPROM.write(CIRCLE_EFFECT_ADDRESS, circleEffect);

  // Save animation speeds
  EEPROM.write(LETTER_SPEED_ADDRESS, letterAnimSpeed);
  EEPROM.write(CIRCLE_SPEED_ADDRESS, circleAnimSpeed);
  
  // Commit the writes
  EEPROM.commit();

  Serial.println("Settings saved to EEPROM");
}

// Mode 0: Solid color
void solidColorMode() {
  if (lettersEnabled) {
    fill_solid(lettersLeds, NUM_LEDS_LETTERS, letterColor);
  } else {
    fill_solid(lettersLeds, NUM_LEDS_LETTERS, CRGB::Black);
  }
  
  if (circleEnabled) {
    // Apply the circle effect
    switch (circleEffect) {
      case 0: // Solid color
        fill_solid(circleLeds, NUM_LEDS_CIRCLE, circleColor);
        break;
      case 1: // Rainbow effect
        fill_rainbow(circleLeds, NUM_LEDS_CIRCLE, hue, 255 / NUM_LEDS_CIRCLE);
        break;
      case 2: // Hue snake effect
        for (int i = 0; i < NUM_LEDS_CIRCLE; i++) {
          // Calculate offset from the current position
          int pos = (circlePosition + i) % NUM_LEDS_CIRCLE;
          circleLeds[pos] = CHSV(hue + (i * circleHueOffset), 255, 255);
        }
        break;
    }
  } else {
    fill_solid(circleLeds, NUM_LEDS_CIRCLE, CRGB::Black);
  }
}

// Mode 1: Letter chase (snake effect)
void letterChaseMode() {
  unsigned long currentTime = millis();
  
  // Update letter animations based on letter animation speed
  if (currentTime - lastLetterUpdateTime >= letterAnimSpeed) {
    lastLetterUpdateTime = currentTime;
    
    if (lettersEnabled) {
      // Fade all LEDs slightly
      fadeToBlackBy(lettersLeds, NUM_LEDS_LETTERS, 40);
      
      // Chase through letter LEDs
      int letterPos = chasePosition % NUM_LEDS_LETTERS;
      lettersLeds[letterPos] = letterColor;
      
      // Update position for next frame
      chasePosition = (chasePosition + 1) % NUM_LEDS_LETTERS;
    } else {
      fill_solid(lettersLeds, NUM_LEDS_LETTERS, CRGB::Black);
    }
  }
  
  // Update circle animations based on circle animation speed
  if (currentTime - lastCircleUpdateTime >= circleAnimSpeed) {
    lastCircleUpdateTime = currentTime;
    
    // Update hue and position for animations
    hue++;
    circlePosition = (circlePosition + 1) % NUM_LEDS_CIRCLE;
    
    if (circleEnabled) {
      // Apply the circle effect
      switch (circleEffect) {
        case 0: // Solid color
          fill_solid(circleLeds, NUM_LEDS_CIRCLE, circleColor);
          break;
        case 1: // Rainbow effect
          fill_rainbow(circleLeds, NUM_LEDS_CIRCLE, hue, 255 / NUM_LEDS_CIRCLE);
          break;
        case 2: // Hue snake effect
          for (int i = 0; i < NUM_LEDS_CIRCLE; i++) {
            // Calculate offset from the current position
            int pos = (circlePosition + i) % NUM_LEDS_CIRCLE;
            circleLeds[pos] = CHSV(hue + (i * circleHueOffset), 255, 255);
          }
          break;
      }
    } else {
      fill_solid(circleLeds, NUM_LEDS_CIRCLE, CRGB::Black);
    }
  }
}

// Mode 2: Sequential letters with adjustable speed
void sequentialLettersMode() {
  unsigned long currentTime = millis();
  
  // Update letter animations based on letter animation speed
  if (currentTime - lastLetterUpdateTime >= letterAnimSpeed * 10) { // Slower for sequential letters
    lastLetterUpdateTime = currentTime;
    
    // Move to next step in sequence
    sequenceStep = (sequenceStep + 1) % 6;
    
    if (lettersEnabled) {
      // Clear all letters first
      fill_solid(lettersLeds, NUM_LEDS_LETTERS, CRGB::Black);
      
      // Set appropriate letter based on sequence step
      switch (sequenceStep) {
        case 0: // S on
          fill_solid(&lettersLeds[S_START], NUM_LEDS_S, letterColor);
          break;
        case 1: // All off
          // Already cleared above
          break;
        case 2: // T on
          fill_solid(&lettersLeds[T_START], NUM_LEDS_T, letterColor);
          break;
        case 3: // All off
          // Already cleared above
          break;
        case 4: // E on
          fill_solid(&lettersLeds[E_START], NUM_LEDS_E, letterColor);
          break;
        case 5: // All off
          // Already cleared above
          break;
      }
    } else {
      fill_solid(lettersLeds, NUM_LEDS_LETTERS, CRGB::Black);
    }
  }
  
  // Update circle animations based on circle animation speed
  if (currentTime - lastCircleUpdateTime >= circleAnimSpeed) {
    lastCircleUpdateTime = currentTime;
    
    // Update hue and position for animations
    hue++;
    circlePosition = (circlePosition + 1) % NUM_LEDS_CIRCLE;
    
    if (circleEnabled) {
      // Apply the circle effect
      switch (circleEffect) {
        case 0: // Solid color
          fill_solid(circleLeds, NUM_LEDS_CIRCLE, circleColor);
          break;
        case 1: // Rainbow effect
          fill_rainbow(circleLeds, NUM_LEDS_CIRCLE, hue, 255 / NUM_LEDS_CIRCLE);
          break;
        case 2: // Hue snake effect
          for (int i = 0; i < NUM_LEDS_CIRCLE; i++) {
            // Calculate offset from the current position
            int pos = (circlePosition + i) % NUM_LEDS_CIRCLE;
            circleLeds[pos] = CHSV(hue + (i * circleHueOffset), 255, 255);
          }
          break;
      }
    } else {
      fill_solid(circleLeds, NUM_LEDS_CIRCLE, CRGB::Black);
    }
  }
}

// Mode 3: Letter pulse effect with adjustable speed
void letterPulseMode() {
  unsigned long currentTime = millis();
  
  // Update letter animations based on letter animation speed
  if (currentTime - lastLetterUpdateTime >= letterAnimSpeed) {
    lastLetterUpdateTime = currentTime;
    
    if (lettersEnabled) {
      // Calculate sine wave for brightness
      uint8_t pulseBrightness = beatsin8(60000 / letterAnimSpeed, 20, 255);
      
      // Apply brightness to letters
      uint8_t r = scale8(letterColor.r, pulseBrightness);
      uint8_t g = scale8(letterColor.g, pulseBrightness);
      uint8_t b = scale8(letterColor.b, pulseBrightness);
      
      fill_solid(lettersLeds, NUM_LEDS_LETTERS, CRGB(r, g, b));
    } else {
      fill_solid(lettersLeds, NUM_LEDS_LETTERS, CRGB::Black);
    }
  }
  
  // Update circle animations based on circle animation speed
  if (currentTime - lastCircleUpdateTime >= circleAnimSpeed) {
    lastCircleUpdateTime = currentTime;
    
    // Update hue and position for animations
    hue++;
    circlePosition = (circlePosition + 1) % NUM_LEDS_CIRCLE;
    
    if (circleEnabled) {
      // Apply the circle effect
      switch (circleEffect) {
        case 0: // Solid color
          fill_solid(circleLeds, NUM_LEDS_CIRCLE, circleColor);
          break;
        case 1: // Rainbow effect
          fill_rainbow(circleLeds, NUM_LEDS_CIRCLE, hue, 255 / NUM_LEDS_CIRCLE);
          break;
        case 2: // Hue snake effect
          for (int i = 0; i < NUM_LEDS_CIRCLE; i++) {
            // Calculate offset from the current position
            int pos = (circlePosition + i) % NUM_LEDS_CIRCLE;
            circleLeds[pos] = CHSV(hue + (i * circleHueOffset), 255, 255);
          }
          break;
      }
    } else {
      fill_solid(circleLeds, NUM_LEDS_CIRCLE, CRGB::Black);
    }
  }
}

void handleBluetoothMode() {
  // Process any Bluetooth commands first
  processBluetooth();
  
  // Check for Bluetooth timeout if not connected
  if (!bluetoothConnected) {
    if (millis() - bluetoothStartTime > BLUETOOTH_TIMEOUT) {
      Serial.println("Bluetooth timeout - no connection");
      exitBluetoothMode();
      return;
    }
    
    // Blink blue on the circle while waiting for connection
    static unsigned long lastBlink = 0;
    static bool blinkState = false;
    
    if (millis() - lastBlink > 500) {
      lastBlink = millis();
      blinkState = !blinkState;
      
      if (blinkState) {
        fill_solid(circleLeds, NUM_LEDS_CIRCLE, CRGB::Blue);
      } else {
        fill_solid(circleLeds, NUM_LEDS_CIRCLE, CRGB::Black);
      }
      
      // Force an immediate update of the LEDs
      FastLED.show();
    }
  } else {
    // When connected but not processing a command, maintain the correct display
    
    // Keep letters with current color
    if (lettersEnabled) {
      fill_solid(lettersLeds, NUM_LEDS_LETTERS, letterColor);
    } else {
      fill_solid(lettersLeds, NUM_LEDS_LETTERS, CRGB::Black);
    }
    
    // IMPORTANT: Maintain circle LEDs according to current settings
    if (circleEnabled) {
      // Update animations for effects that change over time
      unsigned long currentTime = millis();
      
      // Update circle animations based on circle animation speed
      if (currentTime - lastCircleUpdateTime >= circleAnimSpeed) {
        lastCircleUpdateTime = currentTime;
        
        // Update position for snake effect
        circlePosition = (circlePosition + 1) % NUM_LEDS_CIRCLE;
        
        // Update hue for rainbow and hue shift effects
        hue++;
        
        switch (circleEffect) {
          case 0: // Solid color
            fill_solid(circleLeds, NUM_LEDS_CIRCLE, circleColor);
            break;
          case 1: // Rainbow effect
            fill_rainbow(circleLeds, NUM_LEDS_CIRCLE, hue, 255 / NUM_LEDS_CIRCLE);
            break;
          case 2: // Hue snake effect
            for (int i = 0; i < NUM_LEDS_CIRCLE; i++) {
              // Calculate offset from the current position
              int pos = (circlePosition + i) % NUM_LEDS_CIRCLE;
              circleLeds[pos] = CHSV(hue + (i * circleHueOffset), 255, 255);
            }
            break;
        }
        
        // Update LEDs
        FastLED.show();
      }
    } else {
      fill_solid(circleLeds, NUM_LEDS_CIRCLE, CRGB::Black);
    }
  }
  
  // Keep button LED on in Bluetooth mode
  digitalWrite(BUTTON_LED_PIN, HIGH);
  
  // Check if we need to save settings to EEPROM
  if (eepromNeedsSaving && (millis() - lastEepromWrite > WRITE_INTERVAL)) {
    saveSettingsToEEPROM();
    eepromNeedsSaving = false;
    lastEepromWrite = millis();
  }
}

void processBluetooth() {
  // Check if data is available
  while (SerialBT.available()) {
    char c = SerialBT.read();

    // Mark as connected and reset activity timer
    bluetoothConnected = true;
    lastBluetoothActivity = millis();

    // Process incoming character
    if (c == '\n' || c == '\r') {
      if (btBufferIndex > 0) {
        btBuffer[btBufferIndex] = '\0';  // Null-terminate
        processBtCommand(btBuffer);
        btBufferIndex = 0;  // Reset buffer
      }
    } else if (btBufferIndex < sizeof(btBuffer) - 1) {
      btBuffer[btBufferIndex++] = c;
    }
  }

  // Check for inactivity timeout (2 minutes)
  if (bluetoothConnected && (millis() - lastBluetoothActivity > BLUETOOTH_TIMEOUT)) {
    Serial.println("Bluetooth timeout - inactivity");
    bluetoothConnected = false;
    // Don't exit BT mode, just mark as disconnected
  }
}

void processBtCommand(char* command) {
  Serial.print("BT Command: ");
  Serial.println(command);
  
  bool updateDisplay = false;  // Flag to indicate if we need to refresh the display
  
  // Command format:
  // L,R,G,B    - Set letter color (0-255 for each)
  // C,R,G,B    - Set circle color (0-255 for each)
  // B,VAL      - Set brightness (0-125)
  // LE,1/0     - Enable/disable letters
  // CE,1/0     - Enable/disable circle
  // CF,0/1/2   - Set circle effect (0=solid, 1=rainbow, 2=hue snake)
  // LS,VAL     - Set letter animation speed (10-500ms)
  // CS,VAL     - Set circle animation speed (10-500ms)
  // S          - Save settings
  // X          - Exit BT mode
  // ?          - Help
  
  // Process command based on first character
  if (command[0] == 'L' && command[1] == ',') {
    // Letter color command
    int r = 0, g = 0, b = 0;
    sscanf(command + 2, "%d,%d,%d", &r, &g, &b);
    r = constrain(r, 0, 255);
    g = constrain(g, 0, 255);
    b = constrain(b, 0, 255);
    
    letterColor = CRGB(r, g, b);
    eepromNeedsSaving = true;
    
    // Apply to letters immediately if enabled
    if (lettersEnabled) {
      fill_solid(lettersLeds, NUM_LEDS_LETTERS, letterColor);
      updateDisplay = true;
    }
    
    SerialBT.printf("Letter color set to R:%d G:%d B:%d\n", r, g, b);
  }
  else if (command[0] == 'C' && command[1] == ',') {
    // Circle color command
    int r = 0, g = 0, b = 0;
    sscanf(command + 2, "%d,%d,%d", &r, &g, &b);
    r = constrain(r, 0, 255);
    g = constrain(g, 0, 255);
    b = constrain(b, 0, 255);
    
    // Store the circle color
    circleColor = CRGB(r, g, b);
    eepromNeedsSaving = true;
    
    // Apply to circle immediately if enabled and in solid color mode
    if (circleEnabled && circleEffect == 0) {
      fill_solid(circleLeds, NUM_LEDS_CIRCLE, circleColor);
      updateDisplay = true;
    }
    
    // Debug output with actual values
    Serial.printf("Debug - Circle color set to R:%d G:%d B:%d\n", circleColor.r, circleColor.g, circleColor.b);
    SerialBT.printf("Circle color set to R:%d G:%d B:%d\n", r, g, b);
  }
  else if (command[0] == 'B' && command[1] == ',') {
    // Brightness command
    int b = atoi(command + 2);
    b = constrain(b, 0, 125);  // Max 125 as requested
    
    brightness = b;
    FastLED.setBrightness(brightness);
    eepromNeedsSaving = true;
    updateDisplay = true;
    
    SerialBT.printf("Brightness set to %d\n", b);
  }
  else if (command[0] == 'L' && command[1] == 'E' && command[2] == ',') {
    // Letter enable/disable
    int enable = atoi(command + 3);
    lettersEnabled = (enable != 0);
    eepromNeedsSaving = true;
    
    // Update letters immediately
    if (lettersEnabled) {
      fill_solid(lettersLeds, NUM_LEDS_LETTERS, letterColor);
    } else {
      fill_solid(lettersLeds, NUM_LEDS_LETTERS, CRGB::Black);
    }
    updateDisplay = true;
    
    SerialBT.printf("Letters %s\n", lettersEnabled ? "enabled" : "disabled");
  }
  else if (command[0] == 'C' && command[1] == 'E' && command[2] == ',') {
    // Circle enable/disable
    int enable = atoi(command + 3);
    circleEnabled = (enable != 0);
    eepromNeedsSaving = true;
    
    // Update circle immediately
    if (circleEnabled) {
      switch (circleEffect) {
        case 0: // Solid color
          fill_solid(circleLeds, NUM_LEDS_CIRCLE, circleColor);
          break;
        case 1: // Rainbow effect
          fill_rainbow(circleLeds, NUM_LEDS_CIRCLE, hue, 255 / NUM_LEDS_CIRCLE);
          break;
        case 2: // Hue snake effect
          for (int i = 0; i < NUM_LEDS_CIRCLE; i++) {
            // Calculate offset from the current position
            int pos = (circlePosition + i) % NUM_LEDS_CIRCLE;
            circleLeds[pos] = CHSV(hue + (i * circleHueOffset), 255, 255);
          }
          break;
      }
    } else {
      fill_solid(circleLeds, NUM_LEDS_CIRCLE, CRGB::Black);
    }
    updateDisplay = true;
    
    SerialBT.printf("Circle %s\n", circleEnabled ? "enabled" : "disabled");
  }
  else if (command[0] == 'C' && command[1] == 'F' && command[2] == ',') {
    // Circle effect
    int effect = atoi(command + 3);
    effect = constrain(effect, 0, 2);
    circleEffect = effect;
    eepromNeedsSaving = true;
    
    // Update circle effect immediately
    if (circleEnabled) {
      switch (circleEffect) {
        case 0: // Solid color
          fill_solid(circleLeds, NUM_LEDS_CIRCLE, circleColor);
          break;
        case 1: // Rainbow effect
          fill_rainbow(circleLeds, NUM_LEDS_CIRCLE, hue, 255 / NUM_LEDS_CIRCLE);
          break;
        case 2: // Hue snake effect
          for (int i = 0; i < NUM_LEDS_CIRCLE; i++) {
            // Calculate offset from the current position
            int pos = (circlePosition + i) % NUM_LEDS_CIRCLE;
            circleLeds[pos] = CHSV(hue + (i * circleHueOffset), 255, 255);
          }
          break;
      }
      updateDisplay = true;
    }
    
    const char* effectNames[] = {"Solid", "Rainbow", "Hue Snake"};
    SerialBT.printf("Circle effect set to: %s\n", effectNames[effect]);
  }
  else if (command[0] == 'L' && command[1] == 'S' && command[2] == ',') {
    // Letter animation speed
    int speed = atoi(command + 3);
    speed = constrain(speed, 10, 500);  // Limit range
    
    letterAnimSpeed = speed;
    eepromNeedsSaving = true;
    
    SerialBT.printf("Letter animation speed set to %d ms\n", speed);
  }
  else if (command[0] == 'C' && command[1] == 'S' && command[2] == ',') {
    // Circle animation speed
    int speed = atoi(command + 3);
    speed = constrain(speed, 10, 500);  // Limit range
    
    circleAnimSpeed = speed;
    eepromNeedsSaving = true;
    
    SerialBT.printf("Circle animation speed set to %d ms\n", speed);
  }
  else if (command[0] == 'S') {
    // Save settings
    saveSettingsToEEPROM();
    eepromNeedsSaving = false;
    
    SerialBT.println("Settings saved to EEPROM");
  }
  else if (command[0] == 'X') {
    // Exit BT mode
    SerialBT.println("Exiting Bluetooth mode");
    exitBluetoothMode();
  }
  else if (command[0] == '?') {
    // Help
    SerialBT.println("Commands:");
    SerialBT.println("L,R,G,B    - Set letter color (0-255 for each)");
    SerialBT.println("C,R,G,B    - Set circle color (0-255 for each)");
    SerialBT.println("B,VAL      - Set brightness (0-125)");
    SerialBT.println("LE,1/0     - Enable/disable letters");
    SerialBT.println("CE,1/0     - Enable/disable circle");
    SerialBT.println("CF,0/1/2   - Circle effect (0=solid, 1=rainbow, 2=hue snake)");
    SerialBT.println("LS,VAL     - Letter animation speed (10-500ms)");
    SerialBT.println("CS,VAL     - Circle animation speed (10-500ms)");
    SerialBT.println("S          - Save settings");
    SerialBT.println("X          - Exit BT mode");
  }
  else {
    SerialBT.println("Unknown command. Send ? for help.");
  }
  
  // If a display update is needed, show immediately
  if (updateDisplay) {
    FastLED.show();
  }
}

void enterBluetoothMode() {
  if (!bluetoothMode) {
    bluetoothMode = true;

    // Start Bluetooth
    SerialBT.begin("STE_LED_Control");

    // Set start time for timeout
    bluetoothStartTime = millis();
    lastBluetoothActivity = millis();

    // Initially not connected
    bluetoothConnected = false;

    // Visual indication
    digitalWrite(BUTTON_LED_PIN, HIGH);

    Serial.println("Entering Bluetooth mode");
  }
}

void exitBluetoothMode() {
  if (bluetoothMode) {
    bluetoothMode = false;
    
    // End Bluetooth
    SerialBT.end();
    
    // Turn off button LED
    digitalWrite(BUTTON_LED_PIN, LOW);
    
    // Save any pending settings
    if (eepromNeedsSaving) {
      saveSettingsToEEPROM();
      eepromNeedsSaving = false;
    }
    
    // Most important: apply current settings to LEDs before returning to normal mode
    if (lettersEnabled) {
      fill_solid(lettersLeds, NUM_LEDS_LETTERS, letterColor);
    } else {
      fill_solid(lettersLeds, NUM_LEDS_LETTERS, CRGB::Black);
    }
    
    if (circleEnabled) {
      switch (circleEffect) {
        case 0: // Solid color
          fill_solid(circleLeds, NUM_LEDS_CIRCLE, circleColor);
          break;
        case 1: // Rainbow effect
          fill_rainbow(circleLeds, NUM_LEDS_CIRCLE, hue, 255 / NUM_LEDS_CIRCLE);
          break;
        case 2: // Hue snake effect
          for (int i = 0; i < NUM_LEDS_CIRCLE; i++) {
            // Calculate offset from the current position
            int pos = (circlePosition + i) % NUM_LEDS_CIRCLE;
            circleLeds[pos] = CHSV(hue + (i * circleHueOffset), 255, 255);
          }
          break;
      }
    } else {
      fill_solid(circleLeds, NUM_LEDS_CIRCLE, CRGB::Black);
    }
    
    // Force update before exiting
    FastLED.show();
    
    Serial.println("Exited Bluetooth mode");
  }
}

void resetDevice() {
  Serial.println("RESETTING DEVICE");

  // Visual indication - flash all LEDs red 3 times
  for (int i = 0; i < 3; i++) {
    // All red
    fill_solid(lettersLeds, NUM_LEDS_LETTERS, CRGB::Red);
    fill_solid(circleLeds, NUM_LEDS_CIRCLE, CRGB::Red); 
    FastLED.show();
    delay(200);

    // All off
    clearAllLeds();
    FastLED.show();
    delay(200);
  }

  // Reset to default values
  currentMode = 0;
  bluetoothMode = false;
  letterColor = CRGB::Yellow;
  circleColor = CRGB::Yellow;
  brightness = 125;
  lettersEnabled = true;
  circleEnabled = true;
  circleEffect = 0;
  letterAnimSpeed = 50;
  circleAnimSpeed = 50;

  // Save defaults to EEPROM
  saveSettingsToEEPROM();

  // Restart the device (ESP32 specific)
  ESP.restart();
}

void checkButton() {
  // Read the state of the button
  int reading = digitalRead(BUTTON_PIN);

  // Check if input has changed
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // If the input is stable
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state has changed
    if (reading != buttonState) {
      buttonState = reading;

      // Button has been pressed
      if (buttonState == LOW) {
        buttonPressStartTime = millis();
        buttonLongPressed = false;
        buttonVeryLongPressed = false;
      }
      // Button has been released
      else if (buttonState == HIGH) {
        unsigned long pressDuration = millis() - buttonPressStartTime;

        // Check for very long press (reset)
        if (pressDuration >= RESET_LONG_PRESS_DURATION) {
          // Reset already handled during press
        }
        // Check for long press (Bluetooth toggle)
        else if (pressDuration >= BT_LONG_PRESS_DURATION) {
          if (!bluetoothMode) {
            enterBluetoothMode();
          } else {
            exitBluetoothMode();
          }
        }
        // Short press (mode change) - only if not in Bluetooth mode
        else if (!bluetoothMode) {
          // Change to next mode
          currentMode = (currentMode + 1) % NUM_MODES;
          Serial.print("Changed to mode: ");
          Serial.println(currentMode);

          // Mark for saving to EEPROM
          eepromNeedsSaving = true;

          // Blink button LED to acknowledge
          blinkButtonLED();
        }
      }
    }
  }

  // If button is currently pressed, check for long press thresholds
  if (buttonState == LOW) {
    unsigned long pressDuration = millis() - buttonPressStartTime;

    // Check for very long press (reset)
    if (pressDuration >= RESET_LONG_PRESS_DURATION && !buttonVeryLongPressed) {
      buttonVeryLongPressed = true;
      resetDevice();
    }
    // Check for long press (Bluetooth mode)
    else if (pressDuration >= BT_LONG_PRESS_DURATION && !buttonLongPressed) {
      buttonLongPressed = true;
      // Visual feedback that long press was detected
      digitalWrite(BUTTON_LED_PIN, HIGH);
      delay(100);
      digitalWrite(BUTTON_LED_PIN, LOW);
    }
  }

  // Save the reading for the next loop
  lastButtonState = reading;
}

void blinkButtonLED() {
  digitalWrite(BUTTON_LED_PIN, HIGH);
  delay(100);
  digitalWrite(BUTTON_LED_PIN, LOW);
}

void clearAllLeds() {
  fill_solid(lettersLeds, NUM_LEDS_LETTERS, CRGB::Black);
  fill_solid(circleLeds, NUM_LEDS_CIRCLE, CRGB::Black);
}
