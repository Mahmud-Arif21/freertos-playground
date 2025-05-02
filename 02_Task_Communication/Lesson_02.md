# Lesson02: Task Communication

## Description

This lesson includes two examples showcasing FreeRTOS task communication and control on an ESP32:

1. **Incremental Blink**: Demonstrates a sender/receiver model where integer values are passed via a queue to blink an LED multiple times.
2. **Touch Blink**: Shows how a touch sensor task can toggle a blinking task, illustrating task suspension and resumption.

---

## Example A: Incremental Blink

### Code Overview

- **`LED_PIN`**: Defines the onboard LED pin.
- **`numberQueue`**: Queue for passing integer counts.
- **`senderTask`**: Sends values 1–5 cyclically every 3 s.
- **`receiverTask`**: Receives values and blinks the LED accordingly.
- **`setup()`**: Initializes Serial, configures LED, creates queue and tasks on cores 0 & 1.
- **`loop()`**: Empty (scheduler-driven).

### Key Snippets

#### 1. Pin & Queue Declaration

```cpp
#define LED_PIN 2
QueueHandle_t numberQueue;  // Holds up to 5 ints
```

> ### Note: A queue acts like a shared mailbox where tasks can safely send and receive data.
> - **Only one task can access the queue at a time**
> - **Data is retrieved in the same order it was sent**
> - **Tasks can wait for data / space or they can fail immediately**

#### 2. Task Creation

```cpp
numberQueue = xQueueCreate(5, sizeof(int));
xTaskCreatePinnedToCore(senderTask, "Sender Task", 2048, NULL, 1, &senderTaskHandle, 0);
xTaskCreatePinnedToCore(receiverTask, "Receiver Task", 2048, NULL, 1, &receiverTaskHandle, 1);
```

*Creates a queue and two tasks pinned to separate cores.*

#### 3. senderTask

```cpp
void senderTask(void* p) {
  int c = 1;
  for (;;) {
    xQueueSend(numberQueue, &c, pdMS_TO_TICKS(1000));
    c = (c % 5) + 1;
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}
```

*Cycles values 1–5, sending them every 3 s.*

#### 4. receiverTask

```cpp
void receiverTask(void* p) {
  int v;
  for (;;) {
    if (xQueueReceive(numberQueue, &v, pdMS_TO_TICKS(5000))) {
      for (int i = 0; i < v; i++) {
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(200));
        digitalWrite(LED_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(200));
      }
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}
```

*Receives count ************`v`************ and blinks LED ************`v`************ times.*

---

## Example B: Touch Blink

### Code Overview

- **`LED_PIN`** & **`TOUCH_PIN`**: LED and touch sensor definitions.
- **`blinkTask`**: Toggles LED state every 500 ms; runs on Core 0.
- **`touchTask`**: Monitors capacitive touch on GPIO4 (T0); toggles `blinkTask` suspension/resumption on touch.
- **`setup()`**: Starts Serial, creates both tasks with appropriate priorities & cores.
- **`loop()`**: Deletes itself (scheduler-driven).

### Key Snippets

#### 1. Pin Definitions

```cpp
#define LED_PIN 2
#define TOUCH_PIN T0  // GPIO4
```

#### 2. blinkTask

```cpp
void blinkTask(void* p) {
  pinMode(LED_PIN, OUTPUT);
  bool state = false;
  for (;;) {
    state = !state;
    digitalWrite(LED_PIN, state);
    Serial.printf("LED %s [Core %d]\n", state?"ON":"OFF", xPortGetCoreID());
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
```

*Continually toggles LED, printing its state.*

#### 3. touchTask

```cpp
void touchTask(void* p) {
  const int threshold = 20;
  bool last = false;
  for (;;) {
    int val = touchRead(TOUCH_PIN);
    bool curr = (val < threshold);
    if (curr && !last) {
      if (eTaskGetState(blinkTaskHandle) == eSuspended)
        vTaskResume(blinkTaskHandle);
      else
        vTaskSuspend(blinkTaskHandle);
      Serial.printf("Touch %d => %s [Core %d]\n", val,
        eTaskGetState(blinkTaskHandle)==eSuspended?"PAUSED":"ACTIVE",
        xPortGetCoreID());
    }
    last = curr;
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
```

*Detects touch edges, toggles blinkTask, and logs activity.*

---

## How to Run

### Incremental Blink

1. Open `Incremental_Blink.ino`, select ESP32 board & port; upload.
2. Monitor Serial at 115200 baud.

### Touch Blink

1. Open `Touch_Blink.ino`, ensure touch wiring on GPIO4.
2. Select board & port; upload.
3. Serial Monitor at 115200 baud.
4. Touch D4 pin to toggle blinking.

---

> **Full code and inline comments are available in each ************************`.ino`************************ file for deeper insights.**

