#include "recorder.hpp"
#include <portaudio.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>

const int FRAME_DURATION_MS = 20;
const float VAD_START_THRESHOLD = 500.0f;      // Threshold for voice activity to start recording
const float VAD_STOP_THRESHOLD = 300.0f;       // Threshold for voice activity to stop recording
const int MAX_SILENCE_FRAMES_BEFORE_STOP = 50; // Number of consecutive frames below stop threshold to end recording (1 second for 20ms frames)

MicrophoneRecorder::MicrophoneRecorder(int sampleRate, int channels)
    : sampleRate(sampleRate), channels(channels), initialized(false)
{
  if (Pa_Initialize() == paNoError)
  {
    initialized = true;
  }
}

MicrophoneRecorder::~MicrophoneRecorder()
{
  if (initialized)
  {
    Pa_Terminate();
  }
}

bool MicrophoneRecorder::isInitialized() const
{
  return initialized;
}

float MicrophoneRecorder::computeRMS(const int16_t *data, size_t numSamples)
{
  float sum = 0.0f;
  for (size_t i = 0; i < numSamples; ++i)
  {
    sum += data[i] * data[i];
  }
  return std::sqrt(sum / numSamples);
}

bool MicrophoneRecorder::recordWithVAD(const std::string &outputFile)
{
  if (!initialized)
  {
    std::cerr << "Error: PortAudio not initialized." << std::endl;
    return false;
  }

  const int frameSize = sampleRate * FRAME_DURATION_MS / 1000;
  std::vector<int16_t> frameBuffer(frameSize);
  std::vector<int16_t> speechBuffer;

  PaStream *stream;
  PaError err = Pa_OpenDefaultStream(&stream, channels, 0, paInt16, sampleRate, frameSize, nullptr, nullptr);
  if (err != paNoError)
  {
    std::cerr << "Error opening audio stream: " << Pa_GetErrorText(err) << std::endl;
    return false;
  }

  err = Pa_StartStream(stream);
  if (err != paNoError)
  {
    std::cerr << "Error starting audio stream: " << Pa_GetErrorText(err) << std::endl;
    Pa_CloseStream(stream);
    return false;
  }

  std::cout << "[VAD] Listening for voice..." << std::endl;
  bool recording = false;
  int silenceCount = 0;

  while (true)
  {
    err = Pa_ReadStream(stream, frameBuffer.data(), frameSize);
    if (err != paNoError)
    {
      std::cerr << "Error reading from stream: " << Pa_GetErrorText(err) << std::endl;
      break;
    }

    float energy = computeRMS(frameBuffer.data(), frameSize);

    if (!recording && energy > VAD_START_THRESHOLD)
    {
      std::cout << "[VAD] Voice detected. Recording..." << std::endl;
      recording = true;
      silenceCount = 0; // Reset silence counter when voice starts
    }

    if (recording)
    {
      speechBuffer.insert(speechBuffer.end(), frameBuffer.begin(), frameBuffer.end());

      if (energy < VAD_STOP_THRESHOLD)
      {
        silenceCount++;
        if (silenceCount > MAX_SILENCE_FRAMES_BEFORE_STOP)
        {
          std::cout << "[VAD] Sustained silence detected. Stopping recording." << std::endl;
          break;
        }
      }
      else
      {
        silenceCount = 0; // Reset silence timer on voice detected again
      }
    }
  }

  Pa_StopStream(stream);
  Pa_CloseStream(stream);

  if (!speechBuffer.empty())
  {
    writeWavFile(outputFile, speechBuffer);
    std::cout << "[Recorder] Audio saved to " << outputFile << std::endl;
    return true;
  }

  std::cout << "[Recorder] No speech detected during recording session." << std::endl;
  return false;
}

void MicrophoneRecorder::writeWavFile(const std::string &filename, const std::vector<int16_t> &data)
{
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open())
  {
    std::cerr << "Error: Could not open WAV file for writing: " << filename << std::endl;
    return;
  }

  // WAV header construction
  int32_t byteRate = sampleRate * channels * sizeof(int16_t);
  int32_t dataSize = data.size() * sizeof(int16_t);
  int32_t chunkSize = 36 + dataSize; // RIFF + WAVE + fmt + data + subchunk sizes

  file.write("RIFF", 4);
  file.write(reinterpret_cast<const char *>(&chunkSize), 4);
  file.write("WAVEfmt ", 8);

  int32_t subchunk1Size = 16;
  int16_t audioFormat = 1; // PCM
  int16_t bitsPerSample = 16;
  int16_t blockAlign = channels * sizeof(int16_t);

  file.write(reinterpret_cast<const char *>(&subchunk1Size), 4);
  file.write(reinterpret_cast<const char *>(&audioFormat), 2);
  file.write(reinterpret_cast<const char *>(&channels), 2);
  file.write(reinterpret_cast<const char *>(&sampleRate), 4);
  file.write(reinterpret_cast<const char *>(&byteRate), 4);
  file.write(reinterpret_cast<const char *>(&blockAlign), 2);
  file.write(reinterpret_cast<const char *>(&bitsPerSample), 2);

  file.write("data", 4);
  file.write(reinterpret_cast<const char *>(&dataSize), 4);
  file.write(reinterpret_cast<const char *>(data.data()), dataSize);

  file.close();
}