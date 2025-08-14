/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

import testNapi from 'libentry.so';

interface ChatMessage {
  isUser: boolean;
  message: string;
}

@Component
export struct LlamaChatPage {
  @Consume('pageInfos') pageInfos: NavPathStack;
  @State modelLoaded: boolean = false;
  @State modelPath: string = '/path/to/your/model.gguf';
  @State userInput: string = '';
  @State chatMessages: ChatMessage[] = [];
  @State isGenerating: boolean = false;
  @State modelInfo: string = '';
  @State lastError: string = '';

  aboutToAppear() {
    this.checkModelStatus();
  }

  checkModelStatus() {
    this.modelLoaded = testNapi.isModelLoaded();
    if (this.modelLoaded) {
      this.modelInfo = testNapi.getModelInfo();
    }
  }

  loadModel() {
    if (this.modelPath.trim() === '') {
      this.lastError = 'Please enter a model path';
      return;
    }

    console.log('Loading model from:', this.modelPath);
    const success = testNapi.loadModel(this.modelPath, 2048, 4);
    
    if (success) {
      this.modelLoaded = true;
      this.modelInfo = testNapi.getModelInfo();
      this.lastError = '';
      console.log('Model loaded successfully');
    } else {
      this.lastError = testNapi.getLastError();
      console.error('Failed to load model:', this.lastError);
    }
  }

  unloadModel() {
    testNapi.unloadModel();
    this.modelLoaded = false;
    this.modelInfo = '';
    this.chatMessages = [];
    this.lastError = '';
    console.log('Model unloaded');
  }

  sendMessage() {
    if (!this.modelLoaded) {
      this.lastError = 'Please load a model first';
      return;
    }

    if (this.userInput.trim() === '') {
      return;
    }

    const userMessage = this.userInput.trim();
    this.chatMessages.push({ isUser: true, message: userMessage });
    this.userInput = '';
    this.isGenerating = true;

    // Use setTimeout to allow UI to update
    setTimeout(() => {
      try {
        const response = testNapi.chatCompletion(userMessage);
        if (response && response.trim() !== '') {
          this.chatMessages.push({ isUser: false, message: response.trim() });
          this.lastError = '';
        } else {
          this.lastError = testNapi.getLastError() || 'No response generated';
        }
      } catch (error) {
        this.lastError = `Error: ${error}`;
        console.error('Chat completion error:', error);
      }
      this.isGenerating = false;
    }, 100);
  }

  clearChat() {
    testNapi.clearChatHistory();
    this.chatMessages = [];
    this.lastError = '';
  }

  build() {
    NavDestination() {
      Column() {
        // Header
        Row() {
          Button('← Back')
            .backgroundColor(Color.Transparent)
            .fontColor(Color.Blue)
            .onClick(() => {
              this.pageInfos.pop();
            })
          
          Blank()
          
          Text('LlamaCpp Chat')
            .fontSize(20)
            .fontWeight(FontWeight.Bold)
        }
        .width('100%')
        .padding({ top: 10, bottom: 10 })

        // Model Status Section
        Column() {
          Text('Model Status')
            .fontSize(16)
            .fontWeight(FontWeight.Bold)
            .alignSelf(ItemAlign.Start)

          Row() {
            Text(this.modelLoaded ? '✓ Loaded' : '✗ Not Loaded')
              .fontColor(this.modelLoaded ? Color.Green : Color.Red)
              .fontSize(14)
            
            Blank()
            
            if (this.modelLoaded) {
              Button('Unload')
                .fontSize(12)
                .backgroundColor(Color.Red)
                .onClick(() => this.unloadModel())
            }
          }
          .width('100%')

          if (this.modelInfo) {
            Text(this.modelInfo)
              .fontSize(12)
              .fontColor(Color.Gray)
              .maxLines(3)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
              .margin({ top: 5 })
          }
        }
        .width('100%')
        .padding(10)
        .backgroundColor(Color.White)
        .borderRadius(8)
        .margin({ top: 10 })

        // Model Loading Section
        if (!this.modelLoaded) {
          Column() {
            Text('Load Model')
              .fontSize(16)
              .fontWeight(FontWeight.Bold)
              .alignSelf(ItemAlign.Start)

            TextInput({ placeholder: 'Enter model path (.gguf file)', text: this.modelPath })
              .width('100%')
              .onChange((value: string) => {
                this.modelPath = value;
              })

            Button('Load Model')
              .width('100%')
              .margin({ top: 10 })
              .onClick(() => this.loadModel())
          }
          .width('100%')
          .padding(10)
          .backgroundColor(Color.White)
          .borderRadius(8)
          .margin({ top: 10 })
        }

        // Error Display
        if (this.lastError) {
          Text(this.lastError)
            .fontColor(Color.Red)
            .fontSize(12)
            .width('100%')
            .padding(10)
            .backgroundColor('#FFF0F0')
            .borderRadius(8)
            .margin({ top: 10 })
        }

        // Chat Area
        if (this.modelLoaded) {
          Column() {
            Row() {
              Text('Chat History')
                .fontSize(16)
                .fontWeight(FontWeight.Bold)
              
              Blank()
              
              Button('Clear')
                .fontSize(12)
                .backgroundColor(Color.Orange)
                .onClick(() => this.clearChat())
            }
            .width('100%')

            // Chat Messages
            List({ space: 10 }) {
              ForEach(this.chatMessages, (message: ChatMessage, index: number) => {
                ListItem() {
                  Row() {
                    if (message.isUser) {
                      Blank()
                      Text(message.message)
                        .padding(10)
                        .backgroundColor('#007AFF')
                        .fontColor(Color.White)
                        .borderRadius(10)
                        .maxLines(10)
                        .constraintSize({ maxWidth: '80%' })
                    } else {
                      Text(message.message)
                        .padding(10)
                        .backgroundColor('#E9E9EB')
                        .fontColor(Color.Black)
                        .borderRadius(10)
                        .maxLines(10)
                        .constraintSize({ maxWidth: '80%' })
                      Blank()
                    }
                  }
                  .width('100%')
                }
              })

              if (this.isGenerating) {
                ListItem() {
                  Row() {
                    Text('Generating...')
                      .padding(10)
                      .backgroundColor('#E9E9EB')
                      .fontColor(Color.Gray)
                      .borderRadius(10)
                    Blank()
                  }
                  .width('100%')
                }
              }
            }
            .width('100%')
            .layoutWeight(1)
            .scrollBar(BarState.Auto)

            // Input Area
            Row() {
              TextInput({ placeholder: 'Type your message...', text: this.userInput })
                .layoutWeight(1)
                .enabled(!this.isGenerating)
                .onChange((value: string) => {
                  this.userInput = value;
                })
                .onSubmit(() => {
                  if (!this.isGenerating) {
                    this.sendMessage();
                  }
                })

              Button('Send')
                .margin({ left: 10 })
                .enabled(!this.isGenerating && this.userInput.trim() !== '')
                .onClick(() => this.sendMessage())
            }
            .width('100%')
            .padding({ top: 10 })
          }
          .width('100%')
          .height('60%')
          .padding(10)
          .backgroundColor(Color.White)
          .borderRadius(8)
          .margin({ top: 10 })
        }

        Blank()
      }
      .width('100%')
      .height('100%')
      .padding(16)
      .backgroundColor('#F2F2F7')
    }
    .hideTitleBar(true)
  }
}