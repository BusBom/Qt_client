
# 🚌 <span style="font-size: 24px;">Busbom - Smart Bus Queue Guide System (Qt Client)</span>

**Busbom** is a smart bus stop system based on network cameras and STM32 LED displays. <br />
This Qt-based client provides a real-time dashboard for bus approach detection, passenger guidance, and remote configuration.

<br />

---

## Features

<p align="center">
  <img src="https://github.com/user-attachments/assets/a0240b9e-c41c-44dc-99aa-14c655807bed" width="48%" alt="Login Page" />
  <img src="https://github.com/user-attachments/assets/62f33f5d-5dd4-4cb5-9a47-23e6cd47abff" width="48%" alt="Main Dashboard" />
</p>


- **Login & Sign Up UI**
  - User authentication via email and password
  - Password confirmation and duplicate email validation
  - Warning dialogs for login failures and invalid inputs
 
<br />
<br />
<br />
<br />


<img width="800" height="400" alt="image" src="https://github.com/user-attachments/assets/582fc79f-f4b4-45fb-8908-80618f22d8bd" />

    

- **Live Dashboard**
  - Platform selection and real-time status indicators (Server, Camera, Display)
  - RTSP live video stream and recorded video playback
  - Bus number and platform assignment display

<br />
<br />
<br />
<br />



<p align="center">
  <img src="https://github.com/user-attachments/assets/bd3f1c30-53e0-45c6-baa6-54833c5e34ab" width="48%" alt="Image 1" />
  <img src="https://github.com/user-attachments/assets/3662ff95-c98b-4e80-9acd-e3de13731b2e" width="48%" alt="Image 2" />
</p>

<p align="center">
  <img src="https://github.com/user-attachments/assets/92c0a574-0e38-4064-94d6-32af4f36089a" width="48%" alt="Image 3" />
  <img src="https://github.com/user-attachments/assets/32c274fd-defc-4f4f-b03f-4ce8dedee29e" width="48%" alt="Image 4" />
</p>


- **Settings**
  - Network: Set API URL, port, auto-connect
  - Camera: Adjust brightness, contrast, exposure, saturation
  - ROI: Designate platform areas directly on the captured image
  - Sleep Mode: Schedule stream off/on time (e.g., 01:00 to 05:00)

<br />
<br />
<br />
<br />

---

## Architecture

```plaintext
[LoginPage] ⇄ [SignUpPage] ⇨ [MainWindow]
                                 └── [SettingsDialog]
                                        ├── Network Settings
                                        ├── Camera Settings (with preview)
                                        ├── ROI Settings (draw directly on canvas)
                                        └── Sleep Mode Configuration
```
<br />
<br />


---
##  Build Instructions

### 1. Dependencies

Make sure the following packages are installed:

- Qt 6.x (`Widgets`, `Network`, `Multimedia`, `Sql`)
- OpenCV
- CMake ≥ 3.19

### 2. Build Steps

```bash
git clone https://github.com/yourusername/BusbomQt.git
cd BusbomQt
mkdir build && cd build
cmake ..
cmake --build .
```

> ※ Make sure to configure your `OpenCV_DIR` correctly in `CMakeLists.txt`.
<br />
<br />

---

## Required Files

Place the following files in the **same directory as the compiled executable** (usually the `build/` folder):

### `config.json`

This file contains server and video stream information.  
Here’s an example format:

```json
{
  "api_base_url": "https://192.168.0.xx",
  "camera_url": "https://192.168.0.xx",
  "video_url": "http://192.168.0.xx",
  "rtsp_url": "rtsp://192.168.0.xx/profile2/media.smp"
}
```
<br />

### SSL Certificates (for HTTPS)

If your server requires HTTPS, also place the following:

- `client.cert.pem`
- `client.key.pem`
- `ca.cert.pem`
<br />
<br />

---

## Database

- Uses **SQLite** with a local file `user.db`
- Automatically created on first launch
- Table structure:
  ```sql
  CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    email TEXT UNIQUE,
    password TEXT
  );
  ```
<br />
<br />

---
