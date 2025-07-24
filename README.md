# Smart Embedded HTTP Server in C

A lightweight, multithreaded HTTP server written in pure C simulating embedded hardware interactions. It demonstrates mastery over:

* TCP/IP socket programming
* Embedded device simulation (sensor + actuator)
* Token bucket rate limiting
* Thread-safe server concurrency
* Low-level memory handling and request parsing

---

## 🔧 Features

* 🧠 Handles multiple clients using `pthreads`
* 🌐 Simulated routes for:

  * `/sensor/temp` → returns mock temperature sensor data
  * `/led?state=on|off` → controls simulated LED (POST only)
  * `/device/status` → full device status in JSON
* 📊 IP-based token bucket rate limiter (5 req burst @ 1 rps)
* ⚙️ Embedded-style device simulation

---

## 🧪 API Endpoints

### `GET /sensor/temp`

Returns a fake temperature value:

```json
{"temperature": 27.45}
```

### `POST /led?state=on` or `state=off`

Controls a simulated LED:

```json
{"led": "on"}
```

### `GET /device/status`

Returns full status of all embedded components:

```json
{"temperature": 28.52, "led": "off"}
```

---

## 🚀 Build & Run

```bash
cd smart_Server
make
./smart_server
```

Then open your browser or use curl:

```bash
curl http://127.0.0.1:8282/sensor/temp
curl -X POST http://127.0.0.1:8282/led?state=on
```

---

## ⚡ Project Goals

This project was designed to:

* Mimic real-world embedded + networked device patterns
* Use only low-level C + POSIX without frameworks
* Demonstrate understanding of:

  * TCP/IP + sockets
  * Multithreaded client handling
  * Embedded system interfaces (sensors, actuators)
  * Secure rate-limiting against abuse

---

## 📎 Future Extensions

* Add `/config` route to simulate EEPROM-style settings
* Add persistent sensor log buffer (ring buffer)
* Serve `dashboard.html` to visualize data live
* Implement WebSocket server for real-time updates

---

MIT License
Copyright (c) [2025] [Arjun Rao] 
