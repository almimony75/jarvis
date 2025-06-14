#pragma once
#include <string>
#include <vector>
#include <cstdint>

class MicrophoneRecorder
{
public:
  MicrophoneRecorder(int sampleRate = 16000, int channels = 1);
  ~MicrophoneRecorder();

  bool isInitialized() const;
  bool recordWithVAD(const std::string &outputFile);

private:
  bool initialized;
  int sampleRate;
  int channels;

  float computeRMS(const int16_t *data, size_t numSamples);
  void writeWavFile(const std::string &filename, const std::vector<int16_t> &data);
};
