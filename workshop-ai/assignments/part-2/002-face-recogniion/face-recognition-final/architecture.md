# Face Recognition LED — Architecture

Target board: **ESP32-S3-EYE**  
Main file: **`main/app_main.cpp`**

---

## High-level pipeline

```mermaid
flowchart LR
    Cam[Camera] --> FC[Frame capture pipeline]
    FC --> Detect[Detect task<br/>every frame]
    Detect --> DetCB[detect_result_cb]
    DetCB -->|face found| RecEvt[RECOGNIZE event<br/>throttled 500 ms]
    DetCB -->|always| LCD1[LCD bounding boxes]
    RecEvt --> RecTask[Recognition task<br/>one-shot]
    RecTask --> DB[(face.db)]
    RecTask --> RecCB[recognition_result_cb]
    RecCB -->|sim: match| LED[Green LED ON]
    RecCB -->|who? unknown| LED2[Green LED OFF]
    RecCB --> LCD2[LCD name / similarity label]
    DetCB -->|no face| LED2
```

---

## Workshop step progression

```mermaid
flowchart TD
    S0[Step 0: Clean S3 baseline] --> S1[Step 1: LED on any detection]
    S1 --> S2[Step 2: LED only on recognition]
    S2 --> S3[Step 3: Auto-trigger RECOGNIZE]
    S3 --> S4[Step 4: Re-register callbacks]
    S4 --> S5[Step 5: Throttle 500 ms]
    S5 --> S6[Step 6: Add comments]
    S6 --> Done[Final solution]
```

---

## Callback flow

```mermaid
sequenceDiagram
    participant Cam as Camera
    participant Det as Detect task
    participant App as WhoRecognitionAppLCDWithCallback
    participant Rec as Recognition task
    participant DB as face.db
    participant LCD as LCD
    participant LED as Green LED

    loop Every frame
        Cam->>Det: New frame
        Det->>App: detect_result_cb(result)
        alt No face
            App->>LED: OFF
        else Face detected
            App->>Rec: RECOGNIZE event (max every 500 ms)
        end
        App->>LCD: Parent detect_result_cb → boxes
    end

    Rec->>DB: Compare face embedding
    Rec->>App: recognition_result_cb(result)
    alt Known face (contains sim:)
        App->>LED: ON
    else Unknown face (who?)
        App->>LED: OFF
    end
    App->>LCD: Parent recognition_result_cb → label
```

---

## LED decision logic

```mermaid
flowchart TD
    Start([Frame processed]) --> HasFace{Face detected?<br/>det_res not empty}
    HasFace -->|No| LedOff1[LED OFF]
    HasFace -->|Yes| Throttle{500 ms elapsed?}
    Throttle -->|No| KeepDetect[Keep detecting]
    Throttle -->|Yes| Trigger[Send RECOGNIZE event]
    Trigger --> RecResult{Recognition result}
    RecResult -->|contains sim:| LedOn[LED ON]
    RecResult -->|who?| LedOff2[LED OFF]
    RecResult -->|enroll/delete msg| NoChange[LED unchanged]
```

---

## Class structure

```mermaid
classDiagram
    class WhoRecognitionAppBase {
        #m_recognition
        #m_frame_cap
    }
    class WhoRecognitionAppLCD {
        #recognition_result_cb()
        #detect_result_cb()
        +run()
    }
    class WhoRecognitionAppLCDWithCallback {
        -m_last_recognition_trigger
        -trigger_recognition()
        #recognition_result_cb() LED on/off
        #detect_result_cb() throttle + trigger
    }

    WhoRecognitionAppBase <|-- WhoRecognitionAppLCD
    WhoRecognitionAppLCD <|-- WhoRecognitionAppLCDWithCallback
```

---

## Key concepts

| Term | Meaning |
|------|---------|
| **Detection** | A face is visible in the frame (`result.det_res` not empty) |
| **Recognition** | The face matches someone enrolled in `face.db` (result contains `sim:`) |
| **Unknown face** | Face detected but not in database (result is `"who?"`) |
| **RECOGNIZE event** | FreeRTOS event bit that tells the recognition task to run one identity check |

## Hardware buttons (ESP32-S3-EYE)

| Button | Action |
|--------|--------|
| UP | Enroll current face into `face.db` |
| PLAY | Manual recognize (optional after auto-trigger is added) |
| DOWN | Delete last enrolled face |
