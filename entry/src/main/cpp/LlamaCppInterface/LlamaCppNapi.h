#ifndef LLAMA_CPP_NAPI_H
#define LLAMA_CPP_NAPI_H

#include "napi/native_api.h"

namespace LlamaCppNapi {
    // Model management
    napi_value LoadModel(napi_env env, napi_callback_info info);
    napi_value UnloadModel(napi_env env, napi_callback_info info);
    napi_value IsModelLoaded(napi_env env, napi_callback_info info);
    
    // Text generation and chat
    napi_value GenerateText(napi_env env, napi_callback_info info);
    napi_value ChatCompletion(napi_env env, napi_callback_info info);
    napi_value ClearChatHistory(napi_env env, napi_callback_info info);
    
    // Info and status
    napi_value GetModelInfo(napi_env env, napi_callback_info info);
    napi_value GetLastError(napi_env env, napi_callback_info info);
}

#endif // LLAMA_CPP_NAPI_H