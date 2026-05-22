# ESP32 Embedded Tetris System

A custom embedded gaming system recreating Tetris using an ESP32-DevKitC-32E microcontroller, 320×480 SPI TFT LCD display, and physical hardware controls. Designed to explore embedded systems development, firmware architecture, graphics optimization, and real-time hardware integration.

---

## Demo

Project Demonstration

[![Watch Demo](https://img.youtube.com/vi/FNaCIDfjUdA/maxresdefault.jpg)](https://youtube.com/shorts/FNaCIDfjUdA)

Embedded gameplay demonstration showing the ESP32 Tetris system running directly on physical hardware.

---

## Features

- Fully playable embedded Tetris implementation
- Classic Tetris gameplay systems
  - Hold piece functionality
  - Soft drop mechanics
  - Piece rotation system
- Dynamic level progression
- Optimized graphics rendering to reduce display flickering
- Physical push button controls
- SPI display communication

---

## Hardware

| Component | Purpose |
|------------|----------|
| ESP32-DevKitC-32E | Main microcontroller |
| 320×480 SPI TFT LCD | Graphics display |
| Tactile Push Buttons | User input |
| Breadboard | Rapid prototyping |
| Jumper Wires | Hardware integration |

---

## Controls

| Input | Action |
|--------|---------|
| LEFT | Move Piece Left |
| RIGHT | Move Piece Right |
| ROTATE | Rotate Piece |
| DROP | Soft Drop |
| HOLD | Hold / Swap Piece |
| START | Start / Return Menu |

---

## Software Stack

- C++
- ESP32
- Arduino IDE
- SPI Communication
- LovyanGFX
- Embedded Systems Development

---

### Rendering Systems

- Incremental redraw optimization
- Reduced display refresh flickering
- Hardware button polling
- SPI graphics rendering pipeline
- Optimized embedded display communication

---

## Engineering Challenges

### Graphics Performance

Initial implementations continuously refreshed the display, causing visible screen tearing and poor gameplay responsiveness.

Optimized rendering techniques were implemented to update only modified screen regions, significantly improving visual smoothness.

### Embedded Hardware Integration

Integrated SPI display communication, GPIO button inputs, persistent storage systems, and embedded graphics rendering within ESP32 hardware constraints.

### Real-Time Game Logic

Implemented game-state management systems including collision detection, piece movement, hold mechanics, scoring, level progression, and multiple gameplay modes.

---

## Wiring

### TFT Display

| TFT Display Pin | ESP32 GPIO |
|-----------------|------------|
| CLK             | GPIO18     |
| SDA             | GPIO23     |
| CS              | GPIO5      |
| DC              | GPIO2      |
| RST             | GPIO4      |
| VCC             | 3V3        |
| GND             | GND        |
| BL              | 3V3        |

### Buttons

| Control | ESP32 GPIO |
|----------|------------|
| LEFT     | GPIO32     |
| RIGHT    | GPIO33     |
| ROTATE   | GPIO27     |
| DROP     | GPIO26     |
| HOLD     | GPIO19     |
| START    | GPIO14     |

### Button Circuit Layout

| Wiring Type | Connection |
|--------------|------------|
| Button Input | GPIO → Button |
| Ground Side | Button → GND Rail |
| Configuration | INPUT_PULLUP |

Each push button is configured using ESP32 internal pull-up resistors:

```
GPIO → Button → GND
```

---

## Future Improvements

- Custom PCB design
- Battery-powered handheld system
- 3D printed enclosure
- Sound effects
- Ghost piece implementation
- Advanced rotation kick system
- Additional statistics tracking
- Hardware power optimization

---

Built as a personal embedded systems project exploring firmware development, hardware-software integration, graphics optimization, and embedded game architecture.
