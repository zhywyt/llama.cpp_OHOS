#include "LlamaCppNapi.h"
#include "LlamaCppInterface.h"
#include <memory>
#include <string>

// Global instance of LlamaCpp interface
static std::unique_ptr<LlamaCppInterface> g_llamaCpp = nullptr;

namespace LlamaCppNapi {

    static LlamaCppInterface* getInstance() {
        if (!g_llamaCpp) {
            g_llamaCpp = std::make_unique<LlamaCppInterface>();
        }
        return g_llamaCpp.get();
    }

    napi_value LoadModel(napi_env env, napi_callback_info info) {
        size_t argc = 4;
        napi_value args[4] = {nullptr};
        
        napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        
        if (argc < 1) {
            napi_throw_error(env, nullptr, "Missing model path parameter");
            return nullptr;
        }
        
        // Get model path
        size_t pathLen = 0;
        napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLen);
        std::string modelPath(pathLen, '\0');
        napi_get_value_string_utf8(env, args[0], &modelPath[0], pathLen + 1, &pathLen);
        
        // Get optional parameters
        int contextSize = 2048;
        int threads = 4;
        
        if (argc >= 2) {
            napi_get_value_int32(env, args[1], &contextSize);
        }
        if (argc >= 3) {
            napi_get_value_int32(env, args[2], &threads);
        }
        
        bool success = getInstance()->loadModel(modelPath, contextSize, threads);
        
        napi_value result;
        napi_get_boolean(env, success, &result);
        return result;
    }

    napi_value UnloadModel(napi_env env, napi_callback_info info) {
        getInstance()->unloadModel();
        return nullptr;
    }

    napi_value IsModelLoaded(napi_env env, napi_callback_info info) {
        bool loaded = getInstance()->isModelLoaded();
        napi_value result;
        napi_get_boolean(env, loaded, &result);
        return result;
    }

    napi_value GenerateText(napi_env env, napi_callback_info info) {
        size_t argc = 4;
        napi_value args[4] = {nullptr};
        
        napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        
        if (argc < 1) {
            napi_throw_error(env, nullptr, "Missing prompt parameter");
            return nullptr;
        }
        
        // Get prompt
        size_t promptLen = 0;
        napi_get_value_string_utf8(env, args[0], nullptr, 0, &promptLen);
        std::string prompt(promptLen, '\0');
        napi_get_value_string_utf8(env, args[0], &prompt[0], promptLen + 1, &promptLen);
        
        // Get optional parameters
        int maxTokens = 100;
        float temperature = 0.8f;
        float topP = 0.95f;
        
        if (argc >= 2) {
            napi_get_value_int32(env, args[1], &maxTokens);
        }
        if (argc >= 3) {
            double temp;
            napi_get_value_double(env, args[2], &temp);
            temperature = static_cast<float>(temp);
        }
        if (argc >= 4) {
            double top_p;
            napi_get_value_double(env, args[3], &top_p);
            topP = static_cast<float>(top_p);
        }
        
        std::string response = getInstance()->generateText(prompt, maxTokens, temperature, topP);
        
        napi_value result;
        napi_create_string_utf8(env, response.c_str(), response.length(), &result);
        return result;
    }

    napi_value ChatCompletion(napi_env env, napi_callback_info info) {
        size_t argc = 2;
        napi_value args[2] = {nullptr};
        
        napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        
        if (argc < 1) {
            napi_throw_error(env, nullptr, "Missing user input parameter");
            return nullptr;
        }
        
        // Get user input
        size_t inputLen = 0;
        napi_get_value_string_utf8(env, args[0], nullptr, 0, &inputLen);
        std::string userInput(inputLen, '\0');
        napi_get_value_string_utf8(env, args[0], &userInput[0], inputLen + 1, &inputLen);
        
        // Get optional system prompt
        std::string systemPrompt = "";
        if (argc >= 2) {
            size_t systemLen = 0;
            napi_get_value_string_utf8(env, args[1], nullptr, 0, &systemLen);
            systemPrompt.resize(systemLen);
            napi_get_value_string_utf8(env, args[1], &systemPrompt[0], systemLen + 1, &systemLen);
        }
        
        std::string response = getInstance()->chatCompletion(userInput, systemPrompt);
        
        napi_value result;
        napi_create_string_utf8(env, response.c_str(), response.length(), &result);
        return result;
    }

    napi_value ClearChatHistory(napi_env env, napi_callback_info info) {
        getInstance()->clearChatHistory();
        return nullptr;
    }

    napi_value GetModelInfo(napi_env env, napi_callback_info info) {
        std::string info = getInstance()->getModelInfo();
        napi_value result;
        napi_create_string_utf8(env, info.c_str(), info.length(), &result);
        return result;
    }

    napi_value GetLastError(napi_env env, napi_callback_info info) {
        std::string error = getInstance()->getLastError();
        napi_value result;
        napi_create_string_utf8(env, error.c_str(), error.length(), &result);
        return result;
    }

}