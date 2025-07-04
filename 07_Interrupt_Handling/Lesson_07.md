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

Interrupts are the heartbeat of real-time embedded systems, letting microcontrollers react instantly to events like button presses or sensor spikes. In FreeRTOS, smart interrupt handling keeps your system responsive without chaos. This lesson dives into **task notifications**, **semaphores**, and the pitfalls of **direct ISR processing**, all while pushing **deferred processing** to keep ISRs lightning-fast.

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

#### 🚨 Direct ISR Processing (Not Recommended)

- **All-In ISR**: Handles everything inside the interrupt.
- **Danger Zone**: Blocks other interrupts and tasks.
- **Last Resort**: Only for tiny, critical actions.

---

## ❓ When to Use Each?

| Feature | ⚡ Task Notifications | 🔒 Semaphores | 🚨 Direct ISR Processing |
| --- | --- | --- | --- |
| **Execution** | ISR notifies task | ISR releases semaphore | All logic in ISR |
| **Blocking** | Non-blocking (ISR) | Non-blocking (ISR) | Potentially blocking |
| **Complexity** | Simple, lightweight | Moderate | Simple but risky |
| **Latency** | Low (fast ISR exit) | Low (fast ISR exit) | High (long ISR) |
| **Use Case** | Event signaling | Synchronization | Quick, minimal logic |
| **Resource Usage** | Minimal | Moderate | Minimal but disruptive |

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

### 🚨 Direct ISR Processing

- **Tiny Tweaks**: Flip a pin super quick.
- **Test Runs**: Hacky interrupt checks.

---

## Learning Objectives

By the end, you'll:

1. Learn interrupts and deferred processing in FreeRTOS.
2. Get a better look at task notifications and semaphores.
3. Hook up an ISR to the ESP32 touch sensor.
4. Pick the perfect interrupt trick for your project.

---

## 1. Basic Example: Serial Monitor

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

---

## 2. Practical Example: Multiple Touch Sensors with LED Patterns

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

---

## 3. Best Practices and Guidelines

- **Task Notifications**: Fast pings for simple stuff.
- **Semaphores**: Sync up shared chaos.
- **Direct ISR**: Don't—unless it's tiny.
- **ISR Rules**: Short, ISR-safe, yield if needed.
- **Trouble Spots**: Check task creation, debounce inputs.

---

## Key Points Summary

- **Task Notifications** ⚡: Quick shouts, low overhead.
- **Semaphores** 🔒: Neon-board order, sync magic.
- **Direct ISR** 🚨: Waiter-cooks-badly vibes—skip it.
- **Performance**: Notifications > Semaphores > Direct ISR.
- **Hardware**: GPIO 2 (LED), GPIO 4/0/2 (T0/T1/T2), tasks on Core 0.

**Note**: Interactive visuals exist but aren't here.
