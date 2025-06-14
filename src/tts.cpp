#include "tts.hpp"
#include <cstdlib> // For system()
#include <sstream>
#include <iostream>

TTS::TTS(const std::string &piperPath, const std::string &modelPath, int speaker_id)
    : piperPath(piperPath), modelPath(modelPath), speaker_id(speaker_id)
{
  // Constructor initializes paths and speaker ID.
  // Additional checks (e.g., if piperPath or modelPath are valid) can be added here.
}

bool TTS::speak(const std::string &text)
{
  // Construct the shell command to pipe text to Piper and stream raw audio to aplay.
  // Example command: echo "Hello world" | ./piper --model /path/to/model.onnx --output-raw --speaker 0 | aplay -r 22050 -f S16_LE -t raw -
  std::ostringstream command;
  command << "echo \"" << text << "\" | " << piperPath
          << " --model " << modelPath
          << " --output-raw"
          << " --speaker " << speaker_id
          << " | aplay -r " << sampleRate << " -f S16_LE -t raw -";

  std::string cmd = command.str();

  // Execute the command. This call will block until Piper synthesizes and aplay finishes playing.
  int ret = system(cmd.c_str());
  if (ret != 0)
  {
    std::cerr << "Error: TTS command failed with code " << ret << std::endl;
    return false;
  }
  return true;
}