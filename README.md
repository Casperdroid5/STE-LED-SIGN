(Scroll down for English)

# STE LED-bord - Gebruikershandleiding

## Overzicht

Het STE LED-bord is een aanpasbaar LED-display bestaande uit de letters "STE" en een omringende cirkel van LED's. Deze handleiding biedt instructies voor het bedienen van het bord, het wijzigen van effecten en het aanpassen van kleuren en animaties.

## Hardware Componenten

- **Letters**: 50 individueel adresseerbare WS2812B LED's die de letters "STE" vormen
  - S: 21 LED's
  - T: 11 LED's
  - E: 18 LED's
- **Cirkel**: 48 individueel adresseerbare WS2811 LED's die een cirkel rond de letters vormen
- **Bedieningsknop**: Enkele knop voor het wijzigen van modi en het activeren van Bluetooth-configuratie
- **Knop LED**: Indicatielampje op de knop
- **ESP32 Controller**: Microcontroller die de LED-animaties en Bluetooth-connectiviteit bestuurt

## Basis Bediening

### Aan/Uit Schakelen

Sluit de 12V-voedingskabel aan om het bord in te schakelen. Koppel los om uit te schakelen.
Zorg ervoor dat de barrel-connector geaard is aan de buitenzijde en positief aan de binnenzijde.

### Knopbediening

Het bord heeft een enkele knop die de werking regelt:

- **Korte druk**: Wisselt tussen de vier standaard animatiemodi voor de letters
- **Lange druk** (3 seconden ingedrukt houden): Schakelt Bluetooth-modus in/uit, aangegeven door een korte flits van de knop
- **Zeer lange druk** (10 seconden ingedrukt houden): Fabrieksinstellingen herstellen, aangegeven door alle LED's die rood knipperen, gevolgd door fabrieksinstellingen getoond door LED's

### Animatiemodi

Het bord heeft vier ingebouwde animatiemodi:

1. **Effen kleurmodus**: Zowel letters als cirkel tonen effen kleuren
2. **Letter Chase-modus**: Een "slang"-effect loopt door de letters terwijl de cirkel zijn effect behoudt
3. **Sequentiële lettermodus**: Letters lichten achtereenvolgens op (S, dan T, dan E) terwijl de cirkel zijn effect behoudt
4. **Letter Pulse-modus**: Letters vervagen in en uit terwijl de cirkel zijn effect behoudt

## Bluetooth-bediening

Voor geavanceerde aanpassingen kunt u het bord via Bluetooth bedienen vanaf uw smartphone of computer.

APPS om Bluetooth-commando's te verzenden:
Voor Android in Google Play: Serial Bluetooth Terminal (getest, werkt)
Voor iOS in Appstore: Bluetooth Terminal (niet getest)

### Bluetooth-modus activeren

1. Houd de knop 3 seconden ingedrukt tot de knop-LED oplicht
2. De cirkel knippert blauw tijdens het wachten op een verbinding
3. Maak verbinding met "STE_LED_Control" vanaf uw Bluetooth-apparaat
4. De cirkel blijft continu blauw zodra verbinding is gemaakt (en het eerste commando bijvoorbeeld "?", is verstuurd)

### Bluetooth-commando's

Stuur de volgende tekstopdrachten via Bluetooth om uw bord aan te passen:

| Commando | Beschrijving | Voorbeeld |
|---------|-------------|---------|
| `L,R,G,B` | Stel letterkleur in (0-255 voor elk) | `L,255,0,0` voor rode letters |
| `C,R,G,B` | Stel cirkelkleur in (0-255 voor elk) | `C,0,255,0` voor groene cirkel |
| `B,VAL` | Stel helderheid in (0-125) | `B,100` voor 80% helderheid |
| `LE,1/0` | Letters inschakelen/uitschakelen | `LE,0` om letters uit te schakelen |
| `CE,1/0` | Cirkel inschakelen/uitschakelen | `CE,1` om cirkel in te schakelen |
| `CF,0/1/2` | Stel cirkeleffect in (0=effen, 1=regenboog, 2=kleurverlopende slang) | `CF,2` voor kleurverlopende slang effect |
| `LS,VAL` | Stel snelheid letteranimatie in (10-500ms) | `LS,20` voor snellere letteranimaties |
| `CS,VAL` | Stel snelheid cirkelanimatie in (10-500ms) | `CS,200` voor langzamere cirkelanimaties |
| `S` | Huidige instellingen opslaan in geheugen | `S` om alle instellingen op te slaan |
| `X` | Bluetooth-modus afsluiten | `X` om terug te keren naar normale werking |
| `?` | Toon hulp/commandolijst | `?` om alle beschikbare commando's te zien |

### Cirkel-effecten

- **Effen kleur** (CF,0): Cirkel toont een effen kleur (ingesteld met C,R,G,B)
- **Regenboog** (CF,1): Cirkel toont een regenboogpatroon
- **Kleurverlopende slang** (CF,2): Cirkel toont een slangachtig bewegend patroon met verschillende kleuren

### Animatiesnelheid

- **Letteranimatiesnelheid** (LS,VAL): Regelt de snelheid van letteranimaties (chase, sequentieel, pulse)
  - Lagere waarden = snellere animatie (bereik 10-500ms)
  - Standaard is 50ms
  
- **Cirkelanimatiesnelheid** (CS,VAL): Regelt de snelheid van cirkelanimaties (vooral voor regenboog en kleurverlopende slang)
  - Lagere waarden = snellere animatie (bereik 10-500ms)
  - Standaard is 50ms

### Instellingen opslaan

Wijzigingen via Bluetooth worden automatisch opgeslagen na 60 seconden, of u kunt het commando `S` sturen om direct op te slaan. Instellingen blijven bewaard wanneer het bord wordt uitgeschakeld.

## Fabrieksinstellingen herstellen

Als u de standaardinstellingen moet herstellen:
1. Houd de knop 10 seconden ingedrukt
2. Het bord knippert drie keer rood
3. Het bord zal opnieuw opstarten met standaardinstellingen:
   - Modus: Effen kleur
   - Letterkleur: Geel
   - Cirkelkleur: Geel
   - Helderheid: Maximaal (125)
   - Letters: Ingeschakeld
   - Cirkel: Ingeschakeld
   - Cirkeleffect: Effen
   - Animatiesnelheden: 50ms

## Probleemoplossing

- **Bord reageert niet**: Controleer stroomaansluiting en probeer het apparaat uit en weer in te schakelen
- **Kan geen verbinding maken via Bluetooth**: Zorg ervoor dat Bluetooth is ingeschakeld op uw apparaat, probeer zowel het bord als uw Bluetooth-apparaat opnieuw op te starten
- **Bord bevriest**: Gebruik de resetknop (zie fabrieksinstellingen herstellen) of koppel de stroom los en sluit deze opnieuw aan
- **Instellingen worden niet opgeslagen**: Probeer het `S`-commando expliciet te versturen om opslaan te forceren

## Technische specificaties

- **Ingangsspanning**: 12V DC via voedingsadapter met 5,2mm barrel jack
- **Stroomverbruik**: Tot 12W bij maximale helderheid (begrensd op 125) met alle LED's aan
- **Bluetooth**: Bluetooth Classic
- **Microcontroller**: ESP32-WROOM-32
- **LED-type**: WS2812B (letters) en WS2811 (buitenste cirkel)
- **Geheugen**: Instellingen opgeslagen in EEPROM



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

APPS to send Bluetooth commands:
For Android in Google Play: Serial Bluetooth Terminal (tested, works)
For IOS in Appstore: Bluetooth Terminal (not tested)

### Entering Bluetooth Mode

1. Hold the button for 3 seconds until the button LED lights up
2. The circle will blink blue while waiting for a connection
3. Connect to "STE_LED_Control" from your Bluetooth device
4. The circle will stay solid blue once connected (and after the first command for example: "?" has been send.

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
