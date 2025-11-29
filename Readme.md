Here is a **short, clean, simple README.md** that contains **ONLY** what you asked for:

✅ Local development setup
✅ Dependencies to install
✅ Commands to build & run

Perfect for developers.

---

# 📘 **README.md (Minimal – Local Development Only)**

# Secure Chat – Local Development Setup

This project contains:

* **Python FastAPI backend**
* **C++ client (libsodium + CPR + JSON)**

Below are the exact steps to set up and run locally.

---

# ⚙️ 1. Install Dependencies

## 🔵 A. Python (Server)

Install:

```bash
pip install fastapi uvicorn[standard] passlib[bcrypt]
```

---

## 🔵 B. MSYS2 (Windows Build Environment for C++ Client)

Download MSYS2:
[https://www.msys2.org](https://www.msys2.org)

Open **MSYS2 MinGW64** terminal and run:

```bash
pacman -Syu
```

Restart terminal, then install:

```bash
pacman -S \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-make \
    mingw-w64-x86_64-libsodium \
    mingw-w64-x86_64-cpr \
    mingw-w64-x86_64-nlohmann-json
```

---

# 🟧 2. Run Server (Python)

Run in **CMD / PowerShell / Anaconda** (NOT MSYS2):

```bash
cd server
python server.py
```

Server will start at:

```
http://127.0.0.1:8000
```

---

# 🟦 3. Build Client (C++)

Open **MSYS2 MinGW64** terminal:

```bash
cd /d/secure_chat/client
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

This creates:

```
chat_client.exe
```

---

# 🎮 4. Usage

Run all commands in **MSYS2 MinGW64**:

### Register Users

```bash
./chat_client.exe register Alice 123
./chat_client.exe register Bob 123
```

### Send Message (Alice → Bob)

```bash
./chat_client.exe send Alice 123 Bob "Hello Bob!"
```

### Listen for Messages (Bob)

Open another MSYS2 window:

```bash
cd /d/secure_chat/client/build
./chat_client.exe listen Bob 123
```

---

# ✔ Done

This is the complete setup needed for **local development**.

