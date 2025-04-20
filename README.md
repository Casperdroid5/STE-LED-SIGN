# STE LED Sign - User Manual

## Overview

The STE LED Sign is a customizable LED display consisting of the "STE" letters and a surrounding circle of LEDs. This manual provides instructions for operating the sign, changing effects, and customizing colors and animations.

## Hardware Components

- **Letters**: 50 individually addressable WS2812B LEDs forming the "STE" letters
  - S: 21 LEDs
  - T: 11 LEDs
  - E: 18 LEDs
- **Circle**: 48 individually addressable WS2811 LEDs forming a circle around the letters
- **Control Button**: Single button for changing modes and entering Bluetooth configuration
- **Button LED**: Indicator light on the button
- **ESP32 Controller**: Microcontroller that runs the LED animations and Bluetooth connectivity

## Basic Operation

### Power On/Off

Connect the 12V power cable to turn on the sign. Disconnect to turn off.
Make sure the barrel connector is gnd on the outer barrel and positive on the inner.

### Button Controls

The sign has a single button that controls the operation:

- **Short Press**: Changes between the four standard animation modes for the letters
- **Long Press** (hold for 3 seconds): Enters/exits Bluetooth mode, indicated by a short flash of the button
- **Very Long Press** (hold for 10 seconds): Factory reset (restores default settings), indicated by all leds flashing red, followed by factory settings shown by leds

### Animation Modes

The sign has four built-in animation modes:

1. **Solid Color Mode**: Both letters and circle display solid colors
2. **Letter Chase Mode**: A "snake" effect runs through the letters while the circle maintains its effect
3. **Sequential Letters Mode**: Letters light up in sequence (S, then T, then E) while the circle maintains its effect
4. **Letter Pulse Mode**: Letters fade in and out while the circle maintains its effect

## Bluetooth Control

For advanced customization, you can control the sign via Bluetooth from your smartphone or computer.

### Entering Bluetooth Mode

1. Hold the button for 3 seconds until the button LED lights up
2. The circle will blink blue while waiting for a connection
3. Connect to "STE_LED_Control" from your Bluetooth device
4. The circle will stay solid blue once connected

### Bluetooth Commands

Send the following text commands via Bluetooth to customize your sign:

| Command | Description | Example |
|---------|-------------|---------|
| `L,R,G,B` | Set letter color (0-255 for each) | `L,255,0,0` for red letters |
| `C,R,G,B` | Set circle color (0-255 for each) | `C,0,255,0` for green circle |
| `B,VAL` | Set brightness (0-125) | `B,100` for 80% brightness |
| `LE,1/0` | Enable/disable letters | `LE,0` to turn off letters |
| `CE,1/0` | Enable/disable circle | `CE,1` to turn on circle |
| `CF,0/1/2` | Set circle effect (0=solid, 1=rainbow, 2=hue snake) | `CF,2` for hue snake effect |
| `LS,VAL` | Set letter animation speed (10-500ms) | `LS,20` for faster letter animations |
| `CS,VAL` | Set circle animation speed (10-500ms) | `CS,200` for slower circle animations |
| `S` | Save current settings to memory | `S` to save all settings |
| `X` | Exit Bluetooth mode | `X` to return to normal operation |
| `?` | Show help/command list | `?` to see all available commands |

### Circle Effects

- **Solid Color** (CF,0): Circle displays a solid color (set with C,R,G,B)
- **Rainbow** (CF,1): Circle displays a rainbow pattern
- **Hue Snake** (CF,2): Circle displays a snake-like moving pattern with different colors

### Animation Speed

- **Letter Animation Speed** (LS,VAL): Controls the speed of letter animations (chase, sequential, pulse)
  - Lower values = faster animation (range 10-500ms)
  - Default is 50ms
  
- **Circle Animation Speed** (CS,VAL): Controls the speed of circle animations (especially for rainbow and hue snake)
  - Lower values = faster animation (range 10-500ms)
  - Default is 50ms

### Saving Settings

Any changes made via Bluetooth will be automatically saved after 60 seconds, or you can send the `S` command to save immediately. Settings are preserved when the sign is powered off.

## Factory Reset

If you need to restore the default settings:
1. Hold the button for 10 seconds
2. The sign will flash red three times
3. The sign will restart with default settings:
   - Mode: Solid Color
   - Letter Color: Yellow
   - Circle Color: Yellow
   - Brightness: Maximum (125)
   - Letters: Enabled
   - Circle: Enabled
   - Circle Effect: Solid
   - Animation Speeds: 50ms

## Troubleshooting

- **Sign not responding**: Check power connection and try power cycling the device
- **Cannot connect via Bluetooth**: Ensure Bluetooth is enabled on your device, try restarting both the sign and your Bluetooth device
- **Sign freezes**: Use the reset button (see factory reset) or disconnect and reconnect power
- **Settings not saving**: Try sending the `S` command explicitly to force save

## Technical Specifications

- **Input Voltage**: 12V DC via power adapter with 5.2mm barrel jack 
- **Current Draw**: Up to 12W at maximum brightness (capped to 125) with all LEDs on
- **Bluetooth**: Bluetooth Classic
- **Microcontroller**: ESP32-WROOM-32
- **LED Type**: WS2812B (letters) and WS2811 (outer circle)
- **Memory**: Settings stored in EEPROM