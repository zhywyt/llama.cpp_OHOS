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

import { resourceManager } from '@kit.LocalizationKit'

export const syncCallbackRead: (fileName: string, resMgr: resourceManager.ResourceManager, callback: object) => void;

export const asyncCallbackRead: (fileName: string, resMgr: resourceManager.ResourceManager, callback: object) => void;

export const asyncPromiseRead: (fileName: string, resMgr: resourceManager.ResourceManager) => Promise<string>;

export const threadSafeCaseFun: (work: object) => void;

export const libUvCaseFun: (work: object) => void;

export const destroy: () => void;

// LlamaCpp functions
export const loadModel: (modelPath: string, contextSize?: number, threads?: number) => boolean;

export const unloadModel: () => void;

export const isModelLoaded: () => boolean;

export const generateText: (prompt: string, maxTokens?: number, temperature?: number, topP?: number) => string;

export const chatCompletion: (userInput: string, systemPrompt?: string) => string;

export const clearChatHistory: () => void;

export const getModelInfo: () => string;

export const getLastError: () => string;