# LlamaCpp OHOS Integration

This project integrates [llama.cpp](https://github.com/ggml-org/llama.cpp) into a HarmonyOS Native C++ application, providing large language model inference capabilities.

## Features

- **Model Loading**: Load GGUF format models for inference
- **Chat Interface**: Interactive chat functionality with conversation history
- **Text Generation**: Generate text based on prompts with customizable parameters
- **Native Performance**: Direct C++ integration for optimal performance
- **ArkTS Interface**: Easy-to-use TypeScript interface for HarmonyOS applications

## Project Structure

```
├── entry/src/main/cpp/
│   ├── LlamaCppInterface/          # Core LlamaCpp wrapper
│   │   ├── LlamaCppInterface.h     # C++ interface definition
│   │   ├── LlamaCppInterface.cpp   # Implementation
│   │   ├── LlamaCppNapi.h         # NAPI bindings header
│   │   └── LlamaCppNapi.cpp       # NAPI bindings implementation
│   ├── types/libentry/
│   │   └── Index.d.ts             # TypeScript definitions
│   └── CMakeLists.txt             # Build configuration
├── entry/src/main/ets/pages/
│   ├── LlamaChatPage.ets          # Chat interface UI
│   └── Index.ets                  # Main navigation
└── third_party/
    └── llama.cpp/                 # llama.cpp submodule
```

## API Reference

### C++ Interface (LlamaCppInterface)

```cpp
class LlamaCppInterface {
public:
    // Model management
    bool loadModel(const std::string& modelPath, int contextSize = 2048, int threads = 4);
    void unloadModel();
    bool isModelLoaded() const;
    
    // Text generation
    std::string generateText(const std::string& prompt, int maxTokens = 100, 
                           float temperature = 0.8f, float topP = 0.95f);
    
    // Chat functionality
    std::string chatCompletion(const std::string& userInput, 
                             const std::string& systemPrompt = "");
    void clearChatHistory();
    
    // Status and info
    std::string getModelInfo() const;
    std::string getLastError() const;
};
```

### TypeScript Interface

```typescript
// Model management
export const loadModel: (modelPath: string, contextSize?: number, threads?: number) => boolean;
export const unloadModel: () => void;
export const isModelLoaded: () => boolean;

// Text generation and chat
export const generateText: (prompt: string, maxTokens?: number, temperature?: number, topP?: number) => string;
export const chatCompletion: (userInput: string, systemPrompt?: string) => string;
export const clearChatHistory: () => void;

// Info and status
export const getModelInfo: () => string;
export const getLastError: () => string;
```

### Usage Example (ArkTS)

```typescript
import testNapi from 'libentry.so';

// Load a model
const success = testNapi.loadModel('/path/to/model.gguf', 2048, 4);
if (success) {
    console.log('Model loaded successfully');
    console.log(testNapi.getModelInfo());
    
    // Generate text
    const response = testNapi.generateText('Hello, how are you?', 50, 0.8, 0.95);
    console.log('Generated:', response);
    
    // Chat completion
    const chatResponse = testNapi.chatCompletion('What is the weather like?');
    console.log('Chat response:', chatResponse);
    
    // Unload model when done
    testNapi.unloadModel();
} else {
    console.error('Failed to load model:', testNapi.getLastError());
}
```

## Model Requirements

- **Format**: GGUF format models
- **Size**: Any size supported by llama.cpp
- **Types**: Llama, Mistral, CodeLlama, and other supported architectures
- **Quantization**: All quantization levels supported (Q4_0, Q4_1, Q5_0, Q5_1, Q8_0, F16, F32)

## Performance Considerations

- **Context Size**: Larger context sizes require more memory
- **Model Size**: Larger models provide better quality but require more resources
- **Threads**: Adjust thread count based on device capabilities
- **Memory**: Ensure sufficient device memory for model and context

## Build Requirements

- HarmonyOS SDK 5.0.0 or later
- DevEco Studio 5.0.0 or later
- CMake 3.14 or later
- C++17 compatible compiler

## Installation

1. Clone with submodules (**IMPORTANT**):
   ```bash
   git clone --recursive <repository-url>
   ```

2. If already cloned, initialize submodules (**REQUIRED**):
   ```bash
   git submodule update --init --recursive
   ```
   
   **Note**: This step is essential. Without it, you will get "LlamaCpp native module not available" errors.

3. Build using DevEco Studio or HarmonyOS build tools

## Troubleshooting

### Common Issues

1. **"LlamaCpp native module not available" Error**
   - **Cause**: The llama.cpp submodule is not initialized
   - **Solution**: Run `git submodule update --init --recursive` in the project root
   - **Verification**: Check that `third_party/llama.cpp/` contains files (not empty)
   - This is the most common issue when cloning the repository

2. **Model Loading Fails**
   - Check model file path and format
   - Ensure sufficient memory
   - Verify model file integrity

3. **Generation is Slow**
   - Reduce context size
   - Use smaller model
   - Adjust thread count

4. **Build Errors**
   - Ensure HarmonyOS SDK is properly installed
   - Check CMake version compatibility
   - Verify submodule initialization (see issue #1 above)

### Error Checking

Always check for errors using `getLastError()`:

```typescript
const success = testNapi.loadModel(modelPath);
if (!success) {
    const error = testNapi.getLastError();
    console.error('Load failed:', error);
}
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project follows the original llama.cpp license terms. See LICENSE file for details.

## Acknowledgments

- [llama.cpp](https://github.com/ggml-org/llama.cpp) - The core inference engine
- [GGML](https://github.com/ggerganov/ggml) - The tensor library
- HarmonyOS team for the native development framework