# RFID Scan & Trans - Architecture Overview

## Application Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    RFID Scan & Trans Application                 │
│                         (rfid_scan_trans.c)                      │
└────────────────────────────┬────────────────────────────────────┘
                             │
                ┌────────────┴────────────┐
                │                         │
        ┌───────▼────────┐       ┌───────▼────────┐
        │  UI Framework  │       │  Core Logic    │
        │  (Flipper SDK) │       │  & Storage     │
        └────────┬────────┘       └────────┬───────┘
                 │                         │
    ┌────────────┼────────────┐           │
    │            │            │           │
┌───▼───┐   ┌───▼───┐   ┌───▼───┐   ┌───▼────────┐
│Submenu│   │Widget │   │Popup  │   │VariableItem│
└───────┘   └───────┘   └───────┘   └────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                        Scene Manager                             │
│                      (9 Interactive Scenes)                      │
└────────────────────────────┬────────────────────────────────────┘
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
    ┌───▼────┐         ┌────▼─────┐        ┌────▼─────┐
    │ Start  │         │ Capture  │        │ Library  │
    │ Scene  │         │  Scene   │        │  Scene   │
    └────────┘         └──────────┘        └──────────┘
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
    ┌───▼─────────┐    ┌────▼────────┐    ┌─────▼──────┐
    │ Signal Info │    │   Export    │    │ Transmit   │
    │    Scene    │    │   Scene     │    │   Scene    │
    └─────────────┘    └─────────────┘    └────────────┘
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
    ┌───▼────────┐     ┌────▼────────┐    ┌─────▼────────┐
    │ Settings   │     │  Edit Name  │    │  Edit Notes  │
    │   Scene    │     │    Scene    │    │    Scene     │
    └────────────┘     └─────────────┘    └──────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                        Library Modules                           │
└────────────────────────────┬────────────────────────────────────┘
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
    ┌───▼────────────┐  ┌───▼────────────┐  ┌───▼────────────┐
    │ Signal Storage │  │ Signal Export  │  │Signal Transmit │
    │   (storage.c)  │  │  (export.c)    │  │  (transmit.c)  │
    │                │  │                │  │                │
    │ • Save         │  │ • Flipper      │  │ • Clone        │
    │ • Load         │  │ • CSV          │  │ • Emulate      │
    │ • Delete       │  │ • Wiegand      │  │ • Replay       │
    │ • Backup       │  │ • Proxmark3    │  │ • Generate     │
    │ • Load All     │  │ • JSON         │  │                │
    └────────────────┘  └────────────────┘  └────────────────┘
```

## Data Flow

```
┌─────────────┐
│   User      │
│  Actions    │
└──────┬──────┘
       │
       ▼
┌─────────────────────────────────────────────┐
│           Scene Manager                     │
│  (Routes user input to active scene)        │
└──────────────────┬──────────────────────────┘
                   │
    ┌──────────────┼──────────────┐
    │              │              │
    ▼              ▼              ▼
┌────────┐   ┌──────────┐   ┌─────────┐
│Capture │   │ Library  │   │Settings │
│ Scene  │   │  Scene   │   │ Scene   │
└───┬────┘   └────┬─────┘   └────┬────┘
    │             │              │
    ▼             ▼              ▼
┌─────────────────────────────────────────────┐
│           Application State                 │
│  • signals[100]                             │
│  • signal_count                             │
│  • power_level, repeat_count, delay_ms      │
│  • auto_save_enabled, continuous_mode       │
└──────────────────┬──────────────────────────┘
                   │
    ┌──────────────┼──────────────┐
    │              │              │
    ▼              ▼              ▼
┌─────────┐   ┌─────────┐   ┌──────────┐
│Storage  │   │ Export  │   │Transmit  │
│Library  │   │Library  │   │ Library  │
└────┬────┘   └────┬────┘   └────┬─────┘
     │             │              │
     ▼             ▼              ▼
┌──────────────────────────────────────────────┐
│         Hardware / Filesystem                │
│  • SD Card (/ext/rfid_signals/)             │
│  • RFID Hardware (furi_hal_rfid_*)          │
│  • Notification LEDs                         │
└──────────────────────────────────────────────┘
```

## Signal Lifecycle

```
┌─────────────┐
│   Capture   │  User holds RFID card near device
│   Signal    │
└──────┬──────┘
       │
       ▼
┌─────────────────────────────────────────────┐
│  Auto-detect Protocol                       │
│  • Analyze UID length                       │
│  • Determine protocol type                  │
│  • Set protocol name                        │
└──────┬──────────────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────────────┐
│  Create Signal Object                       │
│  • Generate name (Card_timestamp)           │
│  • Store UID bytes                          │
│  • Record metadata (time, strength)         │
│  • Initialize flags                         │
└──────┬──────────────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────────────┐
│  Auto-save (if enabled)                     │
│  • Write to SD: /ext/rfid_signals/*.rfid    │
│  • Binary format for efficiency             │
└──────┬──────────────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────────────┐
│  Store in Memory                            │
│  • Add to signals[] array                   │
│  • Increment signal_count                   │
│  • Set as selected signal                   │
└──────┬──────────────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────────────┐
│  Display Signal Info                        │
│  • Show all metadata                        │
│  • Provide action menu                      │
└──────┬──────────────────────────────────────┘
       │
   ┌───┴────┬──────────┬──────────┐
   │        │          │          │
   ▼        ▼          ▼          ▼
┌──────┐ ┌────────┐ ┌───────┐ ┌───────┐
│Export│ │Transmit│ │ Edit  │ │Delete │
└──────┘ └────────┘ └───────┘ └───────┘
```

## Export Flow

```
┌─────────────────────────────────────────────┐
│  User Selects Export Format                 │
└──────┬──────────────────────────────────────┘
       │
   ┌───┴──────┬────────┬────────┬────────┐
   │          │        │        │        │
   ▼          ▼        ▼        ▼        ▼
┌──────┐  ┌──────┐ ┌────────┐ ┌─────┐ ┌──────┐
│.sub  │  │.csv  │ │.wiegand│ │.pm3 │ │.json │
│      │  │      │ │        │ │     │ │      │
│Format│  │Format│ │ Format │ │Fmt  │ │Format│
└──┬───┘  └──┬───┘ └───┬────┘ └──┬──┘ └──┬───┘
   │         │         │         │       │
   └─────────┴─────────┴─────────┴───────┘
                      │
                      ▼
         ┌────────────────────────┐
         │   Generate Content     │
         │  • Format signal data  │
         │  • Add headers/metadata│
         │  • Format specific info│
         └────────┬───────────────┘
                  │
                  ▼
         ┌────────────────────────┐
         │   Write to SD Card     │
         │  /ext/rfid_signals/    │
         │  [name].[extension]    │
         └────────┬───────────────┘
                  │
                  ▼
         ┌────────────────────────┐
         │   Success Notification │
         │   • LED feedback       │
         │   • Return to menu     │
         └────────────────────────┘
```

## Transmission Flow

```
┌─────────────────────────────────────────────┐
│  User Selects Transmission Mode             │
└──────┬──────────────────────────────────────┘
       │
   ┌───┴────┬──────────┬──────────┬──────────┐
   │        │          │          │          │
   ▼        ▼          ▼          ▼          │
┌──────┐ ┌────────┐ ┌───────┐ ┌──────────┐  │
│Clone │ │Emulate │ │Replay │ │ Generate │  │
└──┬───┘ └───┬────┘ └───┬───┘ └────┬─────┘  │
   │         │          │          │         │
   └─────────┴──────────┴──────────┘         │
                  │                           │
                  ▼                           │
         ┌────────────────────────┐          │
         │  Validate Signal       │          │
         │  • Check UID length    │          │
         │  • Check protocol      │          │
         └────────┬───────────────┘          │
                  │                           │
                  ▼                           │
         ┌────────────────────────┐          │
         │  Initialize Hardware   │          │
         │  • Reset RFID pins     │          │
         │  • Set power level     │          │
         │  • Set frequency       │          │
         └────────┬───────────────┘          │
                  │                           │
                  ▼                           │
         ┌────────────────────────┐          │
         │  Transmit Signal       │◄─────────┘
         │  • Apply repeat count  │   (Loop for
         │  • Apply delay         │    Generate)
         │  • LED feedback        │
         └────────┬───────────────┘
                  │
                  ▼
         ┌────────────────────────┐
         │  Cleanup & Stop        │
         │  • Stop emulation      │
         │  • Reset pins          │
         │  • Success notification│
         └────────────────────────┘
```

## Settings Flow

```
┌─────────────────────────────────────────────┐
│           Settings Scene                     │
└──────┬──────────────────────────────────────┘
       │
   ┌───┴────┬────────────┬────────────┬────────┐
   │        │            │            │        │
   ▼        ▼            ▼            ▼        │
┌──────┐ ┌──────┐ ┌────────────┐ ┌─────────┐ │
│Power │ │Repeat│ │   Delay    │ │Auto-Save│ │
│Level │ │Count │ │            │ │         │ │
│      │ │      │ │            │ │         │ │
│ 0-7  │ │ 1-50 │ │  0-2000ms  │ │ ON/OFF  │ │
└──┬───┘ └──┬───┘ └─────┬──────┘ └────┬────┘ │
   │        │           │             │       │
   └────────┴───────────┴─────────────┘       │
                  │                            │
                  ▼                            │
         ┌────────────────────────┐           │
         │  Update App State      │           │
         │  • Store in memory     │◄──────────┘
         │  • Apply immediately   │
         └────────┬───────────────┘
                  │
                  ▼
         ┌────────────────────────┐
         │  Used in Transmission  │
         │  • Power affects range │
         │  • Repeat count        │
         │  • Delay timing        │
         └────────────────────────┘
```

## Key Design Principles

1. **Modularity**: Separate concerns into lib/ and scenes/
2. **Scene-based UI**: Each interaction is a scene with enter/event/exit
3. **State Management**: Central RfidAppState structure
4. **Hardware Abstraction**: Use Flipper HAL for hardware access
5. **Error Handling**: Return values checked, user notifications
6. **Data Persistence**: Auto-save and backup prevent data loss
7. **User Experience**: Visual feedback, organized menus, favorites
8. **Extensibility**: Easy to add new protocols, export formats, modes

## Memory Layout

```
Stack (4KB)
├── Application Context (RfidApp)
│   ├── GUI Components (pointers)
│   ├── Scene Manager
│   └── App State (RfidAppState)
│       ├── signals[100] (~50KB)
│       ├── Settings
│       └── UI Buffers
└── Scene-specific data

Heap
├── Allocated strings (FuriString)
├── File handles
└── Temporary buffers
```

## Performance Characteristics

- **Startup**: Fast (<1s) - loads signals from SD
- **Signal Capture**: Immediate (~100ms detection)
- **Library Navigation**: Instant (<50ms per scene)
- **Export**: Fast (~100-500ms depending on format)
- **Transmission**: Variable (depends on mode and settings)
- **Memory Usage**: ~52KB for signal storage + SDK overhead
- **Storage**: ~400 bytes per signal on SD card

## Security Considerations

1. **Data Protection**: Binary format on SD card
2. **Encryption Flag**: Ready for future encryption implementation
3. **Access Control**: File permissions via Flipper filesystem
4. **Validation**: Signal integrity checks before transmission
5. **Legal Compliance**: Warnings in README and UI

---

**Architecture Version**: 1.0
**Last Updated**: 2026-01-07
**Author**: RFID Scan & Trans Development Team
