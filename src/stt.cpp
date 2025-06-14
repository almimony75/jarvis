#include "stt.hpp"
#include <cstdlib>  // For std::system
#include <sstream>  // For std::ostringstream
#include <iterator> // For std::istreambuf_iterator
#include <fstream>  // For std::ifstream
#include <iostream> // For std::cerr

SpeechToText::SpeechToText(const std::string &whisperPath, const std::string &modelPath)
    : whisperPath(whisperPath), modelPath(modelPath) {}

std::string SpeechToText::transcribe(const std::string &audioFilePath)
{
  // Construct the command to run whisper-cli
  std::ostringstream command;
  command << whisperPath
          << " -m " << modelPath
          << " -f " << audioFilePath
          << " --language en"
          << " --output-txt"
          << " --output-file temp_transcript"; // Whisper writes the full transcript to this file

  // Execute the whisper-cli command
  int result = std::system(command.str().c_str());
  if (result != 0)
  {
    std::cerr << "[Whisper] Transcription failed with code: " << result << std::endl;
    return "";
  }

  // Read the transcribed text from the temporary output file
  std::ifstream transcriptFile("temp_transcript.txt");
  if (!transcriptFile.is_open())
  {
    std::cerr << "[Whisper] Failed to open temporary transcript file: temp_transcript.txt" << std::endl;
    return "";
  }
  std::string transcript((std::istreambuf_iterator<char>(transcriptFile)),
                         std::istreambuf_iterator<char>());
  transcriptFile.close(); // Close the file stream

  // Optional: Delete the temporary file if you don't need it for debugging
  // std::remove("temp_transcript.txt");

  return transcript;
}