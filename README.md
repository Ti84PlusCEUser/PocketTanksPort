# PocketTanksPort

PocketTanksPort is an ECE319K embedded-systems game for the TI MSPM0G3507 LaunchPad. It uses the LaunchPad controls, an ST7735 display, and the onboard/audio DAC support to run the game directly on the microcontroller.

## Requirements

- Code Composer Studio with TI Arm Clang support
- TI MSPM0 SDK 2.03.00.07 (or a compatible installed SDK)
- MSPM0G3507 LaunchPad with its XDS110 debug connection

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
