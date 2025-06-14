#include "recorder.hpp"
#include "stt.hpp"
#include "tts.hpp"
#include "brain.hpp"
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <thread> // For std::this_thread::sleep_for
#include <chrono> // For delays
#include <cstdio> // For std::remove


int main()
{
  CURLcode curl_init_res = curl_global_init(CURL_GLOBAL_ALL);
  if (curl_init_res != CURLE_OK) {
      std::cerr << "ERROR: Failed to initialize libcurl: " << curl_easy_strerror(curl_init_res) << std::endl;
      return 1;
  }
  // --- Configuration Paths ---
  const std::string WHISPER_CLI_PATH = "../bin/whisper-cli";
  const std::string WHISPER_MODEL_PATH = "../models/ggml-base.en.bin";
  const std::string PIPER_CLI_PATH = "../bin/piper";
  const std::string PIPER_MODEL_PATH = "../models/en_US-lessac-high.onnx";
  const int PIPER_SPEAKER_ID = 0;

  const std::string LLAMA_SERVER_URL = "http://127.0.0.1:8080/v1/chat/completions";
  const std::string LLAMA_MODEL_NAME = "Meta Llama 3.1 8B Instruct"; 

  // --- Initialize Components ---
  MicrophoneRecorder recorder;
  if (!recorder.isInitialized())
  {
    std::cerr << "ERROR: Failed to initialize MicrophoneRecorder (PortAudio)." << std::endl;
    return 1;
  }

  SpeechToText stt(WHISPER_CLI_PATH, WHISPER_MODEL_PATH);
  TTS tts(PIPER_CLI_PATH, PIPER_MODEL_PATH, PIPER_SPEAKER_ID);
  LlamaClient llamaClient(LLAMA_SERVER_URL, LLAMA_MODEL_NAME);

  
  std::cout << "Voice Assistant Initialized. Say 'Hello Assistant' or your chosen wake word." << std::endl;
  std::cout << "Press Ctrl+C to exit." << std::endl;

  while (true)
  {
    const std::string TEMP_AUDIO_FILE = "recorded_input.wav";

    // 1. Listen for user input
    std::cout << "\n[Assistant]: Listening for speech..." << std::endl;
    if (!recorder.recordWithVAD(TEMP_AUDIO_FILE))
    {
      std::cerr << "[Assistant]: No speech detected or recording failed. Trying again." << std::endl;
      // Add a small delay to prevent busy-waiting if VAD always fails
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      continue;
    }

    // 2. Perform Speech-to-Text (STT)
    std::cout << "[Assistant]: Transcribing..." << std::endl;
    std::string user_text = stt.transcribe(TEMP_AUDIO_FILE);

    // Clean up the temporary audio file
    std::remove(TEMP_AUDIO_FILE.c_str());

    if (user_text.empty())
    {
      std::cerr << "[Assistant]: Transcription failed or empty. Trying again." << std::endl;
      continue;
    }
    std::cout << "[You]: " << user_text << std::endl;

    // Optional: Implement a wake word detection here if desired
    // For simplicity, the assistant responds to all speech inputs for now.
    // if (user_text.find("Assistant") == std::string::npos && user_text.find("assistant") == std::string::npos) {
    //     std::cout << "[Assistant]: Wake word not detected. Listening again." << std::endl;
    //     continue;
    // }

    // 3. Query Llama.cpp LLM
    std::cout << "[Assistant]: Thinking (querying LLM)..." << std::endl;
    std::string llm_response = llamaClient.chat(user_text);

    if (llm_response.empty())
    {
      std::cerr << "[Assistant]: LLM did not provide a response or an error occurred. Trying again." << std::endl;
      tts.speak("I'm sorry, I couldn't generate a response for that.");
      continue;
    }

    // 4. Perform Text-to-Speech (TTS) and play
    std::cout << "[Assistant]: Speaking response..." << std::endl;
    if (!tts.speak(llm_response))
    {
      std::cerr << "[Assistant]: Failed to synthesize or play speech." << std::endl;
    }
  }

  // Optional: Call global curl cleanup here if uncommenting above
  curl_global_cleanup();

  return 0;
}