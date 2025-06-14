# 🧠 Jarvis

> "A powerful, conversational AI assistant inspired by Jarvis—brought to life by sleepless nights and caffeine."

Welcome to **Jarvis**: your locally-running, privacy-respecting voice assistant. Built in pure C++ and powered by the open-source legends of `llama.cpp`, `whisper.cpp`, and `piper-tts`, this project aims to put a miniature Iron Man-style AI on your Linux box.

---

## 🤖 Features

- 🗣️ Full voice assistant powered by local LLMs
    
- 🧏 Offline speech recognition using `whisper.cpp`
    
- 🗯️ Text-to-speech via `piper-tts`
    
- 🔌 100% local – no internet, no cloud, no snooping
    
- ⚙️ Modular: STT, TTS, brain – all replaceable or extendable
    
- 💡 Simple setup with scripts to automate everything
    
- 🚀 Runs fine on Arch (btw)
    

---

## 🏗️ Project Structure

text

`jarvis/ ├── src/         # C++ source files ├── include/     # C++ header files ├── setup.sh     # Installs everything ├── buildMe.sh   # Compiles Jarvis ├── testMe.sh    # Runs a quick health check └── bin/         # Compiled Jarvis lives here`

---

## 📦 Under the Hood

|Component|Tech Used|
|---|---|
|💬 LLM|`llama.cpp`|
|🧏 STT|`whisper.cpp`|
|🔊 TTS|`piper-tts`|
|🧠 Lang|C++ (because why not suffer beautifully?)|
|🧪 Tests|`testMe.sh` script|

Everything runs locally. No GPU? No problem. No cloud? Even better.

---

## 🔮 Roadmap

- Streaming responses
    
- Multithreaded architecture
    
- JSON-based config (change personality without recompiling!)
    
- Tool calling support (maybe your Jarvis will book your therapy)
    
- Self-destruct command (don’t worry, it only wipes the repo... probably)
    

---

## 🧪 How to Summon Jarvis

bash

`sudo ./setup.sh      # Installs all dependencies and fetches models sudo ./testMe.sh     # Runs checks to make sure everything is installed sudo ./buildMe.sh    # Compiles the beast`

🟢 If everything worked, you'll find the executable in `./bin/jarvis`.

Then:

bash

`cd bin ./llama-server -m ../models/Meta-Llama-3.1-8B-Instruct.Q4_0.gguf  # to run the server`

And in another terminal:

bash

`./jarvis`

---

## 🧙 Summoning Ritual Complete

Thanks for checking out Jarvis. Contributions, feedback, and evil plans welcome.

> “Sometimes you just need a voice in your head. Preferably one that compiles.”

---

## ⚠️ Known Limitations

- Only tested on Arch Linux (btw)
    
- Doesn’t actually make coffee or destroy the world... yet
    

---
## 🧩 External Libraries

Jarvis relies on several amazing open-source projects:

- [`llama.cpp`](https://github.com/ggerganov/llama.cpp) — Local LLM inference engine
    
- [`whisper.cpp`](https://github.com/ggerganov/whisper.cpp) — Offline speech recognition
    
- [`piper`](https://github.com/rhasspy/piper) — Text-to-speech synthesis

---

> **Disclaimer:** Not the real Iron Man’s Jarvis—mine consumes more memory, and is 100% open-source. No billionaire genius required!

---
