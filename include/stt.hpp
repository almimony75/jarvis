#pragma once
#include <string>

class SpeechToText
{
public:
  SpeechToText(const std::string &whisperPath, const std::string &modelPath);

  // Transcribe a given WAV file and return the result as a string
  std::string transcribe(const std::string &audioFilePath);

private:
  std::string whisperPath; // Path to the Whisper binary
  std::string modelPath;   // Path to the .bin model (e.g., ggml-base.en.bin)
};
