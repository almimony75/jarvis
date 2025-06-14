#include "brain.hpp"
#include <iostream>
#include <sstream>   // For building JSON strings
#include <algorithm> // For std::replace

#include <curl/curl.h>

// Callback function to capture response data from libcurl
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

LlamaClient::LlamaClient(const std::string &serverUrl, const std::string &modelName)
    : serverUrl(serverUrl), modelName(modelName)
{
  // Initialize conversation history with a system message
  conversationHistory.push_back({"system", "You are a helpful assistant."});
}

LlamaClient::~LlamaClient()
{
  // libcurl global init/cleanup should ideally be done once in main.cpp
}

void LlamaClient::resetHistory()
{
  conversationHistory.clear();
  conversationHistory.push_back({"system", "You are a helpful assistant."}); // Re-add system message
}

std::string LlamaClient::chat(const std::string &userMessage)
{
  // Add user message to history
  conversationHistory.push_back({"user", userMessage});

  // Build the JSON payload from conversation history
  std::stringstream json_payload_ss;
  json_payload_ss << R"({)";
  json_payload_ss << R"("model": ")" << escapeJsonString(modelName) << R"(",)";
  json_payload_ss << R"("messages": [)";

  for (size_t i = 0; i < conversationHistory.size(); ++i)
  {
    json_payload_ss << R"({"role": ")" << escapeJsonString(conversationHistory[i].role)
                    << R"(", "content": ")" << escapeJsonString(conversationHistory[i].content) << R"("})";
    if (i < conversationHistory.size() - 1)
    {
      json_payload_ss << ",";
    }
  }
  json_payload_ss << R"(],)";
  json_payload_ss << R"("max_tokens": 200)"; // Adjust as needed
  json_payload_ss << R"(})";

  std::string jsonPayload = json_payload_ss.str();

  // Send the HTTP request using libcurl
  std::string response = sendHttpRequest(jsonPayload);

  // Parse the response to extract the LLM's message
  std::string llmContent = "";
  size_t contentPos = response.find(R"("content":")");
  if (contentPos != std::string::npos)
  {
    contentPos += std::string(R"("content":")").length();
    size_t endContentPos = response.find(R"("})", contentPos);
    if (endContentPos != std::string::npos)
    {
      llmContent = response.substr(contentPos, endContentPos - contentPos);
      // Basic unescaping of JSON special characters (a JSON library is recommended for robust parsing)
      std::replace(llmContent.begin(), llmContent.end(), '\\', ' ');
    }
  }

  if (!llmContent.empty())
  {
    conversationHistory.push_back({"assistant", llmContent}); // Add LLM response to history
  }

  return llmContent;
}

std::string LlamaClient::sendHttpRequest(const std::string &jsonPayload)
{
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  curl = curl_easy_init();
  if (curl)
  {
    curl_easy_setopt(curl, CURLOPT_URL, serverUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    long http_code = 0; // To store the HTTP response code

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
      readBuffer = ""; // Clear buffer on error
    }
    else
    {
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
  }
  return readBuffer;
}

std::string LlamaClient::escapeJsonString(const std::string &s)
{
  std::string escaped_s;
  escaped_s.reserve(s.length() * 2); // Pre-allocate memory to avoid reallocations

  for (char c : s)
  {
    switch (c)
    {
    case '"':
      escaped_s += "\\\"";
      break;
    case '\\':
      escaped_s += "\\\\";
      break;
    case '\b': // Backspace
      escaped_s += "\\b";
      break;
    case '\f': // Form feed
      escaped_s += "\\f";
      break;
    case '\n': // Line feed (Newline)
      escaped_s += "\\n";
      break;
    case '\r': // Carriage return
      escaped_s += "\\r";
      break;
    case '\t': // Horizontal tab
      escaped_s += "\\t";
      break;
    default:
      // Escape other control characters (0x00 to 0x1F) as \u00XX
      if (static_cast<unsigned char>(c) < 0x20)
      {
        char hex[5];
        sprintf(hex, "\\u%04x", static_cast<unsigned char>(c));
        escaped_s += hex;
      }
      else
      {
        escaped_s += c;
      }
      break;
    }
  }
  return escaped_s;
}