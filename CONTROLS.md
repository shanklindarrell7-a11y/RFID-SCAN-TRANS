# 🎛️ RFID-SCAN-TRANS Controls

This document describes the control scheme for the RFID-SCAN-TRANS Flipper Zero application. The application supports multiple modes of operation, each with its own set of controls.

## Overview

The RFID-SCAN-TRANS application has five main operating modes:
1. **Main Menu** - Central hub for navigation
2. **Library Mode** - Browse and select saved RFID signals
3. **Transmit Mode** - Configure and transmit RFID signals
4. **Settings Mode** - Configure application settings
5. **Duplication Mode** - Duplicate RFID signals

---

## Main Menu

The main menu is the central hub of the application where you can start signal capture or navigate to other modes.

| Button | Action |
|--------|--------|
| **OK** | Start/stop signal capture |
| **RIGHT** | Enter Library mode |
| **LEFT** | Enter Settings mode |
| **UP** | Enter Transmit mode |
| **DOWN** | Enter Duplication mode |
| **BACK (Long Press)** | Exit application |

### Usage Notes:
- Press and hold **BACK** to exit the application completely
- Short press **BACK** in Main Menu does not exit the app
- Signal capture runs in the background while in other modes (if started)

---

## Library Mode

Library mode allows you to browse, navigate, and select from saved RFID signals.

| Button | Action |
|--------|--------|
| **UP** | Navigate to previous signal |
| **DOWN** | Navigate to next signal |
| **LEFT** | Jump backward 10 signals |
| **RIGHT** | Jump forward 10 signals |
| **OK** | Select the current signal |
| **BACK** | Return to Main Menu |

### Usage Notes:
- Signal list is sorted alphabetically by name
- If there are fewer than 10 signals, **LEFT/RIGHT** jump to the beginning/end
- Selected signal can be used for transmission or duplication
- **BACK** returns to Main Menu without selecting a signal

---

## Transmit Mode

Transmit mode allows you to configure transmission parameters and send RFID signals.

| Button | Action |
|--------|--------|
| **UP** | Increase power level |
| **DOWN** | Decrease power level |
| **LEFT** | Decrease repeat count |
| **RIGHT** | Increase repeat count |
| **OK** | Start/stop transmission |
| **BACK** | Cancel and return to Main Menu |

### Configuration Options:

#### Power Level
- Range: 1-10 (1 = lowest, 10 = highest)
- Default: 5
- Affects transmission range and power consumption

#### Repeat Count
- Range: 1-100 transmissions
- Default: 1
- Number of times the signal will be repeated

### Usage Notes:
- Configure power level and repeat count before starting transmission
- Press **OK** to begin transmission
- Press **OK** again to stop transmission early
- **BACK** cancels transmission and returns to Main Menu
- Transmission status is displayed on screen

---

## Settings Mode

Settings mode allows you to configure application preferences and parameters.

| Button | Action |
|--------|--------|
| **UP** | Navigate to previous setting |
| **DOWN** | Navigate to next setting |
| **LEFT** | Decrease setting value / Navigate options left |
| **RIGHT** | Increase setting value / Navigate options right |
| **OK** | Save current setting / Execute action |
| **BACK** | Save all changes and return to Main Menu |

### Available Settings:

1. **Capture Frequency**
   - Range: 125 kHz, 134 kHz (LF RFID frequencies)
   - Default: 125 kHz

2. **Auto-Save Captures**
   - Options: Enabled / Disabled
   - Default: Enabled
   - Automatically saves captured signals

3. **Signal Naming**
   - Options: Auto / Manual / Timestamp
   - Default: Auto
   - How captured signals are named

4. **Storage Location**
   - Options: Internal / SD Card
   - Default: SD Card
   - Where signals are stored

5. **Clear Library**
   - Action: Removes all saved signals
   - Requires confirmation with **OK**

### Usage Notes:
- Changes are saved when navigating between settings
- **BACK** saves all changes and returns to Main Menu
- Some settings may require app restart to take effect
- **Clear Library** requires double confirmation for safety

---

## Duplication Mode

Duplication mode allows you to copy RFID signals and modify them.

| Button | Action |
|--------|--------|
| **UP** | Navigate to previous signal in source list |
| **DOWN** | Navigate to next signal in source list |
| **LEFT** | Navigate to previous modification option |
| **RIGHT** | Navigate to next modification option |
| **OK** | Confirm duplication with current modifications |
| **BACK** | Cancel and return to Main Menu |

### Modification Options:

1. **Exact Copy**
   - Creates an exact duplicate of the selected signal

2. **Increment ID**
   - Creates a copy with ID incremented by specified value
   - Useful for duplicating sequential access cards

3. **Custom ID**
   - Allows manual entry of a new ID for the duplicated signal

### Usage Notes:
- Select a source signal using **UP/DOWN**
- Choose modification type with **LEFT/RIGHT**
- Press **OK** to create the duplicate
- **BACK** cancels duplication and returns to Main Menu
- Duplicated signals are saved to the library automatically

---

## Global Shortcuts

These shortcuts work across all modes:

| Button Combination | Action |
|-------------------|--------|
| **BACK (Long Press)** | Exit to Main Menu (from any mode) or Exit app (from Main Menu) |

---

## Tips & Best Practices

1. **Battery Conservation**
   - Lower power levels in Transmit mode conserve battery
   - Exit the app when not in use with **BACK (Long Press)**

2. **Signal Management**
   - Use descriptive names for signals in Settings
   - Regularly organize your library
   - Back up important signals to SD card

3. **Safe Operation**
   - Start with lower power levels when testing
   - Be aware of local regulations regarding RFID transmission
   - Always have permission before transmitting/duplicating RFID signals

4. **Navigation Efficiency**
   - Use **LEFT/RIGHT** in Library mode for quick navigation
   - Familiarize yourself with the control layout for faster operation

---

## Troubleshooting

**Issue**: Buttons not responding
- **Solution**: Ensure app is in the correct mode (check screen display)

**Issue**: Cannot select signal in Library mode
- **Solution**: Ensure library is not empty; capture or load signals first

**Issue**: Transmission not working
- **Solution**: Check power level settings and ensure signal is selected

**Issue**: Settings not saving
- **Solution**: Always exit Settings mode with **BACK** to ensure changes are saved

---

## Version History

- **v1.0** - Initial control scheme documentation

---

For more information about the RFID-SCAN-TRANS application, see the main [README.md](README.md).
