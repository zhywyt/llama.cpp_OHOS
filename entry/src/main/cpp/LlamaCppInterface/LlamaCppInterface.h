#ifndef LLAMA_CPP_INTERFACE_H
#define LLAMA_CPP_INTERFACE_H

#include <string>
#include <vector>
#include <memory>

struct llama_model;
struct llama_context;

class LlamaCppInterface {
public:
    LlamaCppInterface();
    ~LlamaCppInterface();
    
    // Model management
    bool loadModel(const std::string& modelPath, int contextSize = 2048, int threads = 4);
    void unloadModel();
    bool isModelLoaded() const;
    
    // Text generation
    std::string generateText(const std::string& prompt, int maxTokens = 100, float temperature = 0.8f, float topP = 0.95f);
    
    // Chat functionality
    std::string chatCompletion(const std::string& userInput, const std::string& systemPrompt = "");
    void clearChatHistory();
    
    // Status and info
    std::string getModelInfo() const;
    std::string getLastError() const;

private:
    struct llama_model* model_;
    struct llama_context* context_;
    std::vector<std::string> chatHistory_;
    std::string lastError_;
    bool modelLoaded_;
    
    void setError(const std::string& error);
    std::vector<int> tokenize(const std::string& text) const;
    std::string detokenize(const std::vector<int>& tokens) const;
};

#endif // LLAMA_CPP_INTERFACE_H