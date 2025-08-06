
# 🚌 <span style="font-size: 24px;">Busbom - Smart Bus Queue Guide System (Qt Client)</span>

**Busbom** is a smart bus stop system based on network cameras and STM32 LED displays. <br />
This Qt-based client provides a real-time dashboard for bus approach detection, passenger guidance, and remote configuration.

<br />

---

## ✨ Features
<p align="center">
  <img width="488" height="381" alt="image" src="https://github.com/user-attachments/assets/a0240b9e-c41c-44dc-99aa-14c655807bed" />
  <img width="488" height="381" alt="image" src="https://github.com/user-attachments/assets/62f33f5d-5dd4-4cb5-9a47-23e6cd47abff" />
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
  <img width="500" height="400" alt="image" src="https://github.com/user-attachments/assets/bd3f1c30-53e0-45c6-baa6-54833c5e34ab" />

  <img width="500" height="400" alt="image" src="https://github.com/user-attachments/assets/3662ff95-c98b-4e80-9acd-e3de13731b2e" />

  <img width="500" height="400" alt="image" src="https://github.com/user-attachments/assets/92c0a574-0e38-4064-94d6-32af4f36089a" />

  <img width="500" height="400" alt="image" src="https://github.com/user-attachments/assets/32c274fd-defc-4f4f-b03f-4ce8dedee29e" />

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

## 🧩Architecture

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

## 🛠 Configuration

Before running the application, make sure to place the `config.json` file in your **build directory**.

<details>
<summary>Example <code>config.json</code></summary>

```json
{
  "api_base_url": "https://192.168.0.xx",
  "camera_url": "https://192.168.0.xx",
  "video_url": "http://192.168.0.xx",
  "rtsp_url": "rtsp://192.168.0.xx/profile2/media.smp"
}
