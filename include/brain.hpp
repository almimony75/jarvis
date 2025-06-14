#pragma once
#include <string>
#include <vector> // For storing chat history

struct ChatMessage
{
  std::string role;
  std::string content;
};

class LlamaClient
{
public:
  LlamaClient(const std::string &serverUrl, const std::string &modelName);
  ~LlamaClient();

  // Sends the user message to the LLM, managing conversation history internally.
  std::string chat(const std::string &userMessage);

  // Clears the conversation history
  void resetHistory();

private:
  std::string serverUrl;
  std::string modelName;
  std::vector<ChatMessage> conversationHistory;

  std::string sendHttpRequest(const std::string &jsonPayload);

  // Escapes strings for JSON payload.
  std::string escapeJsonString(const std::string &s);
};