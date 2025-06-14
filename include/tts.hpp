#ifndef TTS_HPP
#define TTS_HPP

#include <string>

class TTS
{
public:
  // Constructor accepts paths to the Piper executable and model, plus an optional speaker id.
  TTS(const std::string &piperPath, const std::string &modelPath, int speaker_id = 0);

  // Synthesizes speech from the provided text, streams it in real-time, and returns true on success.
  bool speak(const std::string &text);

private:
  std::string piperPath; // Path to the Piper binary.
  std::string modelPath; // Path to the downloaded voice model (ONNX file).
  int speaker_id;        // For multi-speaker models; default is speaker 0.

  // Optionally, you can store the sample rate for streaming; typically 22050 Hz per Piper docs.
  int sampleRate = 22050;
};

#endif // TTS_HPP
