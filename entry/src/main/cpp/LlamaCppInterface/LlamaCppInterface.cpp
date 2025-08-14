#include "LlamaCppInterface.h"
#include "llama.h"
#include <iostream>
#include <sstream>
#include <algorithm>

LlamaCppInterface::LlamaCppInterface() 
    : model_(nullptr), context_(nullptr), modelLoaded_(false) {
    // Initialize llama.cpp backend
    llama_backend_init();
    ggml_backend_load_all();
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
    model_ = llama_model_load_from_file(modelPath.c_str(), model_params);
    if (!model_) {
        setError("Failed to load model from: " + modelPath);
        return false;
    }

    // Set up context parameters
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = contextSize;
    ctx_params.n_threads = threads;
    ctx_params.n_threads_batch = threads;
    ctx_params.n_batch = contextSize;

    // Create context
    context_ = llama_init_from_model(model_, ctx_params);
    if (!context_) {
        setError("Failed to create context");
        llama_model_free(model_);
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
        llama_model_free(model_);
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

    const llama_vocab* vocab = llama_model_get_vocab(model_);
    
    // Initialize sampler
    llama_sampler* sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(sampler, llama_sampler_init_top_p(topP, 1));
    llama_sampler_chain_add(sampler, llama_sampler_init_temp(temperature));
    llama_sampler_chain_add(sampler, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

    // Tokenize the prompt
    std::vector<int> tokens = tokenize(prompt);
    if (tokens.empty()) {
        setError("Failed to tokenize prompt");
        llama_sampler_free(sampler);
        return "";
    }

    // Convert to llama_token
    std::vector<llama_token> llama_tokens;
    for (int token : tokens) {
        llama_tokens.push_back(static_cast<llama_token>(token));
    }

    // Process the prompt tokens
    llama_batch batch = llama_batch_get_one(llama_tokens.data(), llama_tokens.size());
    if (llama_decode(context_, batch) != 0) {
        setError("Failed to process prompt tokens");
        llama_sampler_free(sampler);
        return "";
    }

    // Generate tokens
    std::string result;
    for (int i = 0; i < maxTokens; ++i) {
        llama_token new_token_id = llama_sampler_sample(sampler, context_, -1);
        
        if (llama_vocab_is_eog(vocab, new_token_id)) {
            break;
        }

        // Convert token to text
        char buf[256];
        int n = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
        if (n > 0) {
            result += std::string(buf, n);
        }

        // Prepare the next token for decoding
        llama_batch next_batch = llama_batch_get_one(&new_token_id, 1);
        if (llama_decode(context_, next_batch) != 0) {
            setError("Failed to decode token");
            break;
        }
    }

    llama_sampler_free(sampler);
    return result;
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
    
    const llama_vocab* vocab = llama_model_get_vocab(model_);
    
    std::ostringstream info;
    info << "Model loaded: " << (model_ ? "Yes" : "No") << "\n";
    info << "Context size: " << llama_n_ctx(context_) << "\n";
    info << "Vocabulary size: " << llama_vocab_n_tokens(vocab) << "\n";
    
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
    
    const llama_vocab* vocab = llama_model_get_vocab(model_);
    const bool is_first = true; // Assume this is first tokenization for simplicity
    
    int n_tokens = -llama_tokenize(vocab, text.c_str(), text.length(), NULL, 0, is_first, true);
    if (n_tokens <= 0) {
        return {};
    }
    
    std::vector<llama_token> tokens(n_tokens);
    int actual_tokens = llama_tokenize(vocab, text.c_str(), text.length(), 
                                       tokens.data(), n_tokens, is_first, true);
    
    if (actual_tokens < 0) {
        return {};
    }
    
    std::vector<int> result;
    result.reserve(actual_tokens);
    
    for (int i = 0; i < actual_tokens; ++i) {
        result.push_back(static_cast<int>(tokens[i]));
    }
    
    return result;
}

std::string LlamaCppInterface::detokenize(const std::vector<int>& tokens) const {
    if (!model_ || tokens.empty()) {
        return "";
    }
    
    const llama_vocab* vocab = llama_model_get_vocab(model_);
    std::string result;
    
    for (int token : tokens) {
        char buf[256];
        int n = llama_token_to_piece(vocab, static_cast<llama_token>(token), buf, sizeof(buf), 0, true);
        if (n > 0) {
            result += std::string(buf, n);
        }
    }
    
    return result;
}