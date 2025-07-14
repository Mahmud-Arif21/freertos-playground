# Lesson 7: Interrupt Handling

## 📚 Prerequisites

To follow this lesson, you should have:

- Basic knowledge of Arduino programming (e.g., `Serial.begin`, `pinMode`).
- Familiarity with FreeRTOS tasks and semaphores (covered in previous lessons).
- Understanding of software timers in FreeRTOS (covered in Lesson 6).
- An ESP32 development board.
- Arduino IDE with the ESP32 board package installed.

For setup instructions, visit: ESP32 Arduino Setup Guide.

## ⚡ Introduction to Interrupt Handling in FreeRTOS

Interrupts are the heartbeat of real-time embedded systems, letting microcontrollers react instantly to events like button presses or sensor spikes. In FreeRTOS, smart interrupt handling keeps your system responsive without chaos. This lesson dives into **task notifications**, **semaphores**, **critical sections**, and the pitfalls of **direct ISR processing**, all while pushing **deferred processing** to keep ISRs lightning-fast.

### 📘 Definitions

#### ⚡ Interrupts in FreeRTOS

- **Instant Action**: Hardware events that pause tasks to run an ISR.
- **Keep It Quick**: ISRs should be short, avoiding delays or unsafe calls.
- **Delegate Work**: Use notifications or semaphores to pass jobs to tasks.
- **Safe Tools**: Use ISR-friendly APIs like `xTaskNotifyFromISR()`.

#### 🔔 Task Notifications

- **Fast Ping**: A direct, lightweight signal from ISR to task.
- **No Waiting**: ISR exits fast, leaving tasks to handle the rest.
- **Perfect For**: Simple alerts, like a new order shouted out.

#### 🔒 Semaphores

- **Sync Master**: Coordinates between ISRs and tasks with a signal.
- **Binary Mode**: Flags an event—like a VIP reservation being claimed.
- **Perfect For**: Managing shared resources or orderly task triggers.

#### 🛡️ Critical Sections

- **Atomic Zone**: Temporarily disables interrupts to protect shared data.
- **Race Buster**: Prevents corruption when ISRs and tasks access the same variables.
- **Lightning Fast**: Minimal overhead for simple shared resource protection.
- **Perfect For**: Quick variable updates that must be atomic.

#### 🚨 Direct ISR Processing (Not Recommended)

- **All-In ISR**: Handles everything inside the interrupt.
- **Danger Zone**: Blocks other interrupts and tasks.
- **Last Resort**: Only for tiny, critical actions.

---

## ❓ When to Use Each?

| Feature | ⚡ Task Notifications | 🔒 Semaphores | 🛡️ Critical Sections | 🚨 Direct ISR Processing |
| --- | --- | --- | --- | --- |
| **Execution** | ISR notifies task | ISR releases semaphore | Protects shared data | All logic in ISR |
| **Blocking** | Non-blocking (ISR) | Non-blocking (ISR) | Minimal blocking | Potentially blocking |
| **Complexity** | Simple, lightweight | Moderate | Simple, atomic | Simple but risky |
| **Latency** | Low (fast ISR exit) | Low (fast ISR exit) | Ultra-low | High (long ISR) |
| **Use Case** | Event signaling | Synchronization | Shared variable access | Quick, minimal logic |
| **Resource Usage** | Minimal | Moderate | Minimal | Minimal but disruptive |

---

## 🍳 Restaurant Chaos Analogy

Picture a **wild restaurant kitchen**—chefs juggling flaming pans, waiters dodging spills, and orders flying in like a storm. Here's how interrupt handling plays out:

### ⚡ Task Notifications — Waiter's Urgent Yell

- **Scene**: A waiter bursts in, yelling, "Table 5 needs steak, NOW!" (ISR notifies task).
- **Chef's Move**: The chef grunts "Got it!" (minimal ISR code) while flipping a burger.
- **Next Step**: When the burger's done, the chef tackles the steak (task processes notification).

**Vibe**:

- Lightning-fast shout.
- Chef stays cool, handles it later.
- Perfect for quick, no-fuss updates.

### 🔒 Semaphores — VIP Reservation Board

- **Scene**: A waiter scribbles a VIP order on a glowing neon board—one slot only (ISR releases semaphore).
- **Chef's Move**: The head chef, eyeing the board, snags the VIP slot when free (task takes semaphore) and whips up a masterpiece.
- **Twist**: Only one chef can claim it at a time—others wait, until the board's clear (semaphore ensures order).

**Vibe**:

- Flashy, exclusive signal.
- Chefs sync up, no stepping on toes.
- Ideal for VIP-level coordination.

### 🛡️ Critical Sections —  Ice Dispenser Button

- **Scene**: Ice machine with 30-second refill cycle, bartender starts filling just as kitchen needs ice urgently
- **Problem**: Kitchen staff interrupts cycle by pressing button repeatedly → machine stops mid-cycle, incomplete fill, next user gets no ice
- **Solution**: Button becomes unresponsive during active cycle, status light shows "DISPENSING" then "READY"
- **Result**: Each user gets their full ice portion without interruption, no wasted ice or incomplete fills

**Vibe**:

- One-at-a-time register access.
- No scrambled money chaos.
- Perfect for quick, atomic transactions.

### 🚨 Direct ISR Processing — Waiter Turns Chef (Chaos Ensues)

- **Scene**: A waiter drops their tray, grabs a spatula, and starts cooking the order themselves (ISR handles all logic).
- **Mess**: The waiter's stuck at the stove, other tables starve, and the kitchen's a circus.
- **Fix**: "Just take the order, dude—let the chefs cook!" (Quick ISR, defer to task).

**Vibe**:

- Waiter's out of their depth.
- Clogs the whole operation.
- Avoid unless it's a one-second fix.

---

## 🌌 Real-World Applications

### ⚡ Task Notifications

- **Button Press**: Flip an LED on a tap.
- **Sensor Alert**: Ping a task to log data.

### 🔒 Semaphores

- **Shared Gear**: Lock a motor for one task at a time.
- **Event Queue**: Handle interrupts in sequence.

### 🛡️ Critical Sections

- **Counter Updates**: Safely increment shared counters from ISRs.
- **Status Flags**: Atomic updates to system state variables.
- **Configuration Changes**: Protect system settings during updates.

### 🚨 Direct ISR Processing

- **Tiny Tweaks**: Flip a pin super quick.
- **Test Runs**: Hacky interrupt checks.

---

## Learning Objectives

By the end, you'll:

1. Learn interrupts and deferred processing in FreeRTOS.
2. Get a better look at task notifications, semaphores, and critical sections.
3. Hook up an ISR to the ESP32 touch sensor.
4. Understand when to use critical sections for shared data protection.
5. Pick the perfect interrupt trick for your project.

---

## Code Examples

### 1. Basic Example: Serial Monitor

`Interrupt_Basic.ino` fakes an interrupt with serial input. Type 'p' to notify a task, which spits out a message.

**How It Works**:

- 'p' acts like an interrupt.
- Task yells back via Serial.

```cpp
void simulateInterrupt() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'p') {
      xTaskNotify(printTaskHandle, 0, eNoAction);
    }
  }
}

void printTask(void* p) {
  while (1) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    Serial.println("Interrupt event occurred!");
  }
}
```

### 2. Practical Example: Multiple Touch Sensors with LED Patterns

`Interrupt_Touch_LED.ino` uses three ESP32 touch sensors to trigger different LED patterns, showcasing the power of interrupt-driven pattern selection.

**How It Works**:

- Touch T0 (GPIO 4) → Rapid blink pattern
- Touch T1 (GPIO 0) → Slow pulse pattern  
- Touch T2 (GPIO 2) → SOS pattern

```cpp
void IRAM_ATTR touch1ISR() {
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  xTaskNotifyFromISR(ledTaskHandle, PATTERN_RAPID, eSetValueWithOverwrite, &higherPriorityTaskWoken);
  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void ledTask(void* p) {
  while (1) {
    uint32_t pattern = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    switch(pattern) {
      case PATTERN_RAPID: rapidBlink(); break;
      case PATTERN_PULSE: slowPulse(); break;
      case PATTERN_SOS: sosPattern(); break;
    }
  }
}
```

### 3. Advanced Example: Critical Section Protection

`Interrupt_CriticalSection.ino` demonstrates safe interrupt handling using critical sections to protect shared variables between ISRs and tasks.

**How It Works**:

- Interactive commands: 'i' (interrupt), 'r' (read), 's' (continuous mode)
- Critical sections protect shared counter from race conditions
- Shows proper ISR-safe and task-safe critical section usage

```cpp
void IRAM_ATTR simulatedISR() {
  // ISR-safe critical section
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  portENTER_CRITICAL_ISR(&mux);
  interruptCounter++;  // Atomic increment
  portEXIT_CRITICAL_ISR(&mux);
  
  // Notify task
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  xTaskNotifyFromISR(counterTaskHandle, 0, eNoAction, &higherPriorityTaskWoken);
  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

// Task-safe critical section
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
portENTER_CRITICAL(&mux);
uint32_t safeRead = interruptCounter;
portEXIT_CRITICAL(&mux);
```

---

## 3. Best Practices and Guidelines

### ISR Guidelines
- **Keep ISRs Short**: Minimize time spent in interrupt handlers
- **Use ISR-Safe APIs**: Only call FreeRTOS functions ending in `FromISR`
- **Defer Processing**: Use notifications/semaphores to hand off work to tasks
- **Yield When Needed**: Call `portYIELD_FROM_ISR()` if higher priority task woken

### Critical Section Rules
- **Minimize Duration**: Keep critical sections as short as possible
- **Use Correct API**: `portENTER_CRITICAL_ISR()` in ISRs, `portENTER_CRITICAL()` in tasks
- **Avoid Blocking**: Never call blocking functions inside critical sections
- **Nested Handling**: FreeRTOS handles nested critical sections automatically

### General Best Practices
- **Task Notifications**: Fast pings for simple stuff
- **Semaphores**: Sync up shared chaos
- **Critical Sections**: Atomic protection for shared data
- **Direct ISR**: Don't—unless it's tiny
- **Trouble Spots**: Check task creation, debounce inputs

---

## Key Points Summary

- **Task Notifications** ⚡: Quick shouts, low overhead
- **Semaphores** 🔒: Neon-board order, sync magic
- **Critical Sections** 🛡️: Atomic vault protection for shared data
- **Direct ISR** 🚨: Waiter-cooks-badly vibes—skip it
- **Performance**: Notifications > Critical Sections > Semaphores > Direct ISR
- **Hardware**: GPIO 2 (LED), GPIO 4/0/2 (T0/T1/T2), tasks on Core 0

**Note**: Interactive visuals exist but aren't here.
