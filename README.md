# Hermes: Joystick Radio Controller

**Hermes** is a wireless joystick controller system built on **FreeRTOS** and **CMSIS**, designed for:

- Real-time joystick input capture
- nRF24L01 wireless radio transmission
- Visual feedback via OLED display
- Remote control applications
  
Built for the **STM32F103C8T6 (Blue Pill)**, Hermes provides reliable wireless joystick control with integrated feedback systems and modular communication architecture.

---

## Project Highlights

- **Joystick Input**: Analog X/Y axes (-1000 to +1000 range) with button support
- **nRF24L01 Radio**: 2.4GHz wireless transmission with automatic acknowledgment
- **Real-time Processing**: FreeRTOS task scheduling with rate limiting (2Hz transmission)
- **Visual Feedback**: OLED display showing joystick status and connection state
- **LED Indicators**: Debug LEDs for transmission status and hardware testing
- **Modular Architecture**: Hub-based design with sensor, controller, and actuator layers
- **CMSIS Bare-Metal**: No HAL dependencies, direct hardware control
- **Communication Protocol**: Packet-based with checksums and error detection
- **ORION Submodule Integration**: Extended functionality and modularity support
---


## Software Design Overview

1. Task Architecture

This table outlines the tasks in the system, their responsibilities, priorities, and communication mechanisms.
**This section needs to be updated!**

| Task Name         | Responsibility                               | Priority | Communication                    |
|------------------|----------------------------------------------|----------|----------------------------------|
| **SensorTask**    | Polls ultrasonic, IMU, thermal sensors       | Med      | Sends readings via **queues**    |
| **MotorTask**     | Drives motors using PID control              | High     | Gets target direction from queue |
| **PathFinding**   | Receives sensor input, computes direction    | Med      | Queue input/output               |
| **SafetyTask**    | Monitors danger conditions (thermal, power)  | High     | Uses **event group + notify**    |
| **FailsafeTask**  | Triggers alarm, stops motors in emergency    | High     | Reacts to event bits             |
| **CommTask**      | Handles telemetry and manual command input   | Low      | **Queue + semaphore** for UART   |
| **ManualControl** | Overrides auto pathing based on command mode | Med      | Waits on a **binary semaphore**  |
| **LED/DebugTask** | Visual debug with LED pattern/status blink   | Low      | Receives status messages         |


2. Inter-Task Communication
**This section needs to be updated!**
- SensorTask → PathFinding: via Queue<SensorPacket>
- PathFinding → MotorTask: via Queue<DriveCommand>
- Safety events (thermal, power): via EventGroup or TaskNotify
- Manual control trigger: via BinarySemaphore or mode bit flag
- CommTask → ManualControl: via Queue<Command> + mode flag
- Telemetry logging: via MessageBuffer or streaming queue



3. Conceptual Communication Flow
```
[ Ultrasonic (polling) ] → [ Sensor Hub (task) ] → 
                            [ Controller (task) ] → 
                            [ Actuator Hub (task) ] → [ LEDs, LCD, Motors ]

[ BigSound (interrupt) ] → [ ISR Handler ] → 
                            [ Controller (task) ] → 
                            [ Actuator Hub (task) ] → [ LEDs, LCD, Motors ]
```
- Sensors: Collect raw data from environment (IMU, Ultrasonic, etc.)
- Sensor Hub: Polls sensors, aggregates and timestamps data
- Controller: Performs logic decisions (PID, pathfinding, safety checks)
- Actuator Hub: Receives commands and routes them to the appropriate actuator
- Actuators: Motor, LED, Alarm, Display, etc.

4. Data Flow and Task Design (FreeRTOS)
   1. Tasks
	  - SensorTask: Reads raw sensor data, sends it to SensorDataQueue.
	  - ControllerTask: Receives from SensorDataQueue, processes data, sends output to ControlCommandQueue.
	  - ActuatorTask: Reads from ControlCommandQueue, executes actuator commands.
	2. Queues
	  - SensorDataQueue: Transmits hrms_sensor_data_t (struct with all sensor data).
	  - ControlCommandQueue: Transmits hrms_actuator_command_t (per-actuator commands).

```
+---------------------+
|    Main Entry       |   (hermes.c)
+----------+----------+
           |
           v
+---------------------+
|     System Init     |  (board init, clocks, RTOS)
+----------+----------+
           |
           v
+=====================+
||   Task Manager    ||  ← starts all system tasks
+=====================+
           |
           v
+--------------------------------------+
|         System Services Layer        |
+-------------------+------------------+
| Sensor Hub        | Actuator Hub     |
| (all sensors)     | (all motors, LED)|
+-------------------+------------------+
| Communication     | Logging          |
| (Manual + Remote) | (Telemetry/Debug)|
+-------------------+------------------+
| Safety Monitor    | Power Manager    |
+-------------------+------------------+

Tasks in Services Layer exchange data via FreeRTOS queues/events/mutexes
           |
           v
+---------------------+
| Application Logic   | ← Pathfinding, PID, AI, etc.
+---------------------+

```

## WARNING
FreeRTOS/portable/GCC/ARM_CM3/port.c
```code 
const portISR_t * const pxVectorTable = portSCB_VTOR_REG;
(void)pxVectorTable;  // Suppress unused variable warning added by me.
```

## Hardware Requirements

| Component             | Description                                              |
| --------------------- | -------------------------------------------------------- |
| **MCU**               | STM32F103C8T6 (Blue Pill) – Cortex-M3, 72 MHz, 20 KB RAM |
| **Motors**            | 6 DC or geared motors with PWM drivers (e.g., L298N)     |
| **Motor Driver**      | 2x L298N or equivalent                                   |
| **Ultrasonic Sensor** | HC-SR04 or similar                                       |
| **IMU Sensor**        | MPU6050 (via I2C)                                        |
| **Thermal Sensor**    | MLX90614 or analog sensor                                |
| **Power Monitor**     | Voltage divider or INA219                                |
| **Alarm**             | Buzzer (digital control)                                 |
| **Radio Module**      | NRF24L01, LoRa, or UART-based module                     |
| **LEDs**              | Debug/status LEDs                                        |
| **Button**            | For mode toggle / emergency stop                         |
| **Power Source**      | Li-ion battery pack with regulation                      |

---

## Project Structure

```
hermes/
├── src/                    # Hermes source code
│   ├── actuators/         # LED, OLED, alarm drivers
│   ├── communications/    # nRF24L01 radio modules
│   ├── controls/          # Joystick processing and radio transmission  
│   ├── sensors/           # Joystick, temperature, potentiometer
│   └── system/            # FreeRTOS task management
├── include/               # Header files
│   └── ORIONConfig.h     # ORION submodule configuration
├── ORION/                 # Git submodule (external project)
├── FreeRTOS/             # FreeRTOS kernel
├── CMSIS/                # ARM CMSIS drivers
├── SUBMODULES.md         # Submodule management guide
└── Makefile              # Build system with ORION integration
```

---

## Getting Started

### Prerequisites toolchain for Development Environment (Fedora Linux)

```bash
sudo dnf install arm-none-eabi-gcc-cs arm-none-eabi-newlib
arm-none-eabi-gcc --version
```

### 1. Clone the Repository

```bash
# Clone with submodules
git clone --recursive https://github.com/msoodb/hermes.git
cd hermes

# OR clone then initialize submodules
git clone https://github.com/msoodb/hermes.git
cd hermes
git submodule init
git submodule update
```

### 2. Build and Flash

```bash
make                # Compile the project (includes ORION if available)
make flash          # Flash via ST-Link  
make help           # Show detailed build options and ORION info
```

### 3. ORION Submodule Integration

Hermes includes ORION as a git submodule for extended functionality:

- **Automatic Integration**: ORION sources are automatically compiled when available in `ORION/src/`
- **Configuration**: Use `include/ORIONConfig.h` for Hermes-specific ORION settings  
- **Current Status**: ORION submodule is present but contains no source files yet
- **Updates**: Only update ORION from within its own directory (`cd ORION/`), not from Hermes

```bash
# Check ORION status
git submodule status

# Update ORION to latest (when available)
git submodule update --remote ORION
```

See `SUBMODULES.md` for detailed submodule management instructions.

### License
This project is licensed under the GNU General Public License v3. See the LICENSE file for details.

---
