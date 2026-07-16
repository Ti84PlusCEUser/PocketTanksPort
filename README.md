# PocketTanksPort

PocketTanksPort is an ECE319K embedded-systems game for the TI MSPM0G3507 LaunchPad. It uses the LaunchPad controls, an ST7735 display, and the onboard/audio DAC support to run the game directly on the microcontroller.

## Requirements

- Code Composer Studio with TI Arm Clang support
- TI MSPM0 SDK 2.03.00.07 (or a compatible installed SDK)
- MSPM0G3507 LaunchPad with its XDS110 debug connection

## Hardware parts

- TI MSPM0G3507 LaunchPad
- 128x160 ST7735R SPI TFT display (3.3 V logic)
- Linear slide potentiometer for shot power
- Two momentary pushbuttons for the game controls
- Optional additional pushbuttons on PA24 and PA27 (the firmware scans these inputs, but the game does not currently use them)
- Five-resistor binary-weighted DAC/audio circuit and a small amplified speaker or audio input
- Breadboard, jumper wires, and a USB cable for the LaunchPad

Use the resistor-ladder and audio-output circuit from the ECE319K DAC lab. The resistor values are not defined in this software repository; do not connect a speaker directly to a GPIO pin.

## Wiring and pin mapping

All external inputs must share LaunchPad ground. `Switch_Init()` configures PA24-PA27 as GPIO inputs with no internal pull resistors, so wire each button so that released = logic 0 and pressed = logic 1 (for example, an external pulldown resistor with the button connected to 3.3 V).

### ST7735R display

| Display signal | LaunchPad signal | BoosterPack header reference |
| --- | --- | --- |
| VCC | 3.3 V |  |
| GND | GND |  |
| SCK / SCL | PB9, SPI1 SCLK | J1.7 |
| MOSI / SDA | PB8, SPI1 PICO | J2.15 |
| CS | PB6, SPI1 CS0 | J2.13 |
| DC / RS / A0 | PA13, GPIO | J4.31 |
| RESET | PB15, GPIO | J2.17 |

### Game controls

The firmware reads the four inputs as a nibble from PA24 (bit 0) through PA27 (bit 3):

| Input | Mask | In-game function |
| --- | ---: | --- |
| PA25 | `0x02` | Select Spanish at startup; confirm/fire during both players' turns |
| PA26 | `0x04` | Select English at startup; advance the firing angle by 5 degrees |
| PA24 | `0x01` | Scanned but currently unused |
| PA27 | `0x08` | Scanned but currently unused |

The startup screen's wording is currently "Down for SPANISH" and "Left for ENGLISH"; the table above reflects the actual GPIO masks used by the firmware. The angle cycles from 0 through 180 degrees. The potentiometer controls shot power, and the firmware samples it on PB18 (ADC1 channel 5) at the game timer rate. Its conversion is inverted, so the physical direction that produces more power depends on how the potentiometer is wired.

### Audio DAC

The 5-bit DAC outputs the least-significant bit on PB0 and the most-significant bit on PB4:

| DAC bit | GPIO |
| ---: | --- |
| 0 (LSB) | PB0 |
| 1 | PB1 |
| 2 | PB2 |
| 3 | PB3 |
| 4 (MSB) | PB4 |

Connect those five signals to the resistor-ladder/audio circuit, with a common ground and suitable amplification or audio input.

## Playing the game

After choosing a language, the right and left tanks take alternating turns. During a turn, move the potentiometer to set **Power**, press the angle button (PA26) once for each 5-degree increase, and press the fire button (PA25) to launch. Each tank starts with three hit points; the game runs for up to five rounds and displays the winner and scores at the end.

## Build and run in Code Composer Studio

1. Clone or download this repository.
2. In Code Composer Studio, choose **File -> Import Project(s)**.
3. Select the repository folder and import the `PocketTanksPort` project.
4. Select the `Debug` build configuration and choose **Project -> Build Project** (or press `Ctrl+B`).
5. Connect the MSPM0G3507 LaunchPad.
6. Choose **Run -> Debug Project -> PocketTanksPort**.
7. Press **Resume** or `F5` to run the program on the board.

The first build creates the generated `Debug/` directory locally. It is intentionally ignored by Git; CCS will regenerate it on each machine.

## Attribution and licensing

The project includes helper code and the applicable ValvanoWare license in `ValvanoWare-license.txt`. Please retain that license and the source-file attributions when redistributing the project.
