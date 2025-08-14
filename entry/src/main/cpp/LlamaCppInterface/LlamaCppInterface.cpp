#include "LlamaCppInterface.h"
#include "llama.h"
#include <iostream>
#include <sstream>
#include <algorithm>

LlamaCppInterface::LlamaCppInterface() 
    : model_(nullptr), context_(nullptr), modelLoaded_(false) {
    // Initialize llama.cpp backend
    llama_backend_init();
}

LlamaCppInterface::~LlamaCppInterface() {
    unloadModel();
    llama_backend_free();
}

bool LlamaCppInterface::loadModel(const std::string& modelPath, int contextSize, int threads) {
    if (modelLoaded_) {
        unloadModel();
    }

    // Set up model parameters
    llama_model_params model_params = llama_model_default_params();
    
    // Load the model
    model_ = llama_load_model_from_file(modelPath.c_str(), model_params);
    if (!model_) {
        setError("Failed to load model from: " + modelPath);
        return false;
    }

    // Set up context parameters
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = contextSize;
    ctx_params.n_threads = threads;
    ctx_params.n_threads_batch = threads;

    // Create context
    context_ = llama_new_context_with_model(model_, ctx_params);
    if (!context_) {
        setError("Failed to create context");
        llama_free_model(model_);
        model_ = nullptr;
        return false;
    }

    modelLoaded_ = true;
    chatHistory_.clear();
    lastError_.clear();
    return true;
}

void LlamaCppInterface::unloadModel() {
    if (context_) {
        llama_free(context_);
        context_ = nullptr;
    }
    if (model_) {
        llama_free_model(model_);
        model_ = nullptr;
    }
    modelLoaded_ = false;
    chatHistory_.clear();
}

bool LlamaCppInterface::isModelLoaded() const {
    return modelLoaded_;
}

std::string LlamaCppInterface::generateText(const std::string& prompt, int maxTokens, float temperature, float topP) {
    if (!modelLoaded_) {
        setError("Model not loaded");
        return "";
    }

    // Tokenize the prompt
    std::vector<int> tokens = tokenize(prompt);
    if (tokens.empty()) {
        setError("Failed to tokenize prompt");
        return "";
    }

    // Set up sampling parameters
    llama_sampling_params sparams = {};
    sparams.top_p = topP;
    sparams.temp = temperature;
    sparams.n_max = maxTokens;

    llama_sampling_context* ctx_sampling = llama_sampling_init(sparams);
    if (!ctx_sampling) {
        setError("Failed to initialize sampling context");
        return "";
    }

    // Generate text
    std::vector<int> output_tokens;
    
    // Process the prompt tokens
    if (llama_decode(context_, llama_batch_get_one(tokens.data(), tokens.size(), 0, 0))) {
        setError("Failed to process prompt tokens");
        llama_sampling_free(ctx_sampling);
        return "";
    }

    // Generate tokens
    for (int i = 0; i < maxTokens; ++i) {
        llama_token new_token_id = llama_sampling_sample(ctx_sampling, context_, NULL);
        
        if (llama_token_is_eog(model_, new_token_id)) {
            break;
        }

        output_tokens.push_back(new_token_id);

        // Prepare the next token for decoding
        if (llama_decode(context_, llama_batch_get_one(&new_token_id, 1, tokens.size() + i, 0))) {
            setError("Failed to decode token");
            break;
        }
    }

    llama_sampling_free(ctx_sampling);
    
    return detokenize(output_tokens);
}

std::string LlamaCppInterface::chatCompletion(const std::string& userInput, const std::string& systemPrompt) {
    if (!modelLoaded_) {
        setError("Model not loaded");
        return "";
    }

    // Build chat prompt
    std::ostringstream prompt;
    
    if (!systemPrompt.empty()) {
        prompt << "System: " << systemPrompt << "\n\n";
    }
    
    // Add chat history
    for (const auto& entry : chatHistory_) {
        prompt << entry << "\n";
    }
    
    prompt << "User: " << userInput << "\nAssistant: ";
    
    // Generate response
    std::string response = generateText(prompt.str(), 150, 0.8f, 0.95f);
    
    if (!response.empty()) {
        // Add to chat history
        chatHistory_.push_back("User: " + userInput);
        chatHistory_.push_back("Assistant: " + response);
        
        // Keep history manageable (last 10 exchanges)
        if (chatHistory_.size() > 20) {
            chatHistory_.erase(chatHistory_.begin(), chatHistory_.begin() + 2);
        }
    }
    
    return response;
}

void LlamaCppInterface::clearChatHistory() {
    chatHistory_.clear();
}

std::string LlamaCppInterface::getModelInfo() const {
    if (!modelLoaded_) {
        return "No model loaded";
    }
    
    std::ostringstream info;
    info << "Model loaded: " << (model_ ? "Yes" : "No") << "\n";
    info << "Context size: " << llama_n_ctx(context_) << "\n";
    info << "Vocabulary size: " << llama_n_vocab(model_) << "\n";
    
    return info.str();
}

std::string LlamaCppInterface::getLastError() const {
    return lastError_;
}

void LlamaCppInterface::setError(const std::string& error) {
    lastError_ = error;
    std::cerr << "LlamaCpp Error: " << error << std::endl;
}

std::vector<int> LlamaCppInterface::tokenize(const std::string& text) const {
    if (!model_) {
        return {};
    }
    
    int n_tokens = text.length() + 100;  // Estimate tokens needed
    std::vector<llama_token> tokens(n_tokens);
    
    int actual_tokens = llama_tokenize(model_, text.c_str(), text.length(), 
                                       tokens.data(), n_tokens, true, false);
    
    if (actual_tokens < 0) {
        return {};
    }
    
    tokens.resize(actual_tokens);
    std::vector<int> result;
    result.reserve(actual_tokens);
    
    for (const auto& token : tokens) {
        result.push_back(static_cast<int>(token));
    }
    
    return result;
}

std::string LlamaCppInterface::detokenize(const std::vector<int>& tokens) const {
    if (!model_ || tokens.empty()) {
        return "";
    }
    
    std::string result;
    
    for (int token : tokens) {
        const char* token_str = llama_token_to_piece(context_, static_cast<llama_token>(token), false);
        if (token_str) {
            result += token_str;
        }
    }
    
    return result;
}