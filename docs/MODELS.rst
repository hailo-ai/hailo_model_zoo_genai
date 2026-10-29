Model Properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. list-table::
   :header-rows: 1
   :widths: 20 30 8 8 8 10 12

   * - Model
     - Description
     - Source
     - License
     - Parameters
     - Model Size
     - Version Compatibility
   * - DeepSeek-R1-Distill-Qwen-1.5B
     - Utilizes a transformer-based architecture with instruction tuning to enhance logical reasoning, natural language understanding, and content generation
     - `Link <https://huggingface.co/deepseek-ai/DeepSeek-R1-Distill-Qwen-1.5B>`__
     - `Link <https://github.com/deepseek-ai/DeepSeek-R1/blob/main/LICENSE>`__
     - 1.5B
     - 2.37 GB
     - > 5.1.1
   * - Llama3.2-1B-Instruct
     - The pipeline consists of a prefill step and token by token step
     - `Link <https://huggingface.co/meta-llama/Llama-3.2-1B-Instruct>`__
     - `Link <https://huggingface.co/meta-llama/Llama-3.2-1B/blob/main/LICENSE.txt>`__
     - 1B
     - 1.79 GB
     - > 5.2.0
   * - Qwen2-1.5B-Instruct
     - The pipeline consists of a prefill step and token by token step
     - `Link <https://huggingface.co/Qwen/Qwen2-1.5B-Instruct>`__
     - `Link <https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md>`__
     - 1.5B
     - 1.56 GB
     - > 5.1.1
   * - Qwen2-1.5B-Instruct-Function-Calling-v1
     - The pipeline consists of a prefill step and token by token step, fine-tuned for function calling tasks. This model is a fine-tuned version of Qwen/Qwen2-1.5B-Instruct on devanshamin/gem-viggo-function-calling dataset.
     - `Link <https://huggingface.co/devanshamin/Qwen2-1.5B-Instruct-Function-Calling-v1>`__
     - `Link <https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md>`__
     - 1.5B
     - 2.99 GB
     - > 5.2.0
   * - Qwen2.5-1.5B-Instruct
     - The pipeline consists of a prefill step and token by token step
     - `Link <https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct>`__
     - `Link <https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct/blob/main/LICENSE>`__
     - 1.5B
     - 1.64 GB
     - > 5.1.1
   * - Qwen2.5-Coder-1.5B-Instruct
     - The pipeline consists of a prefill step and token by token step, optimized for coding tasks
     - `Link <https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B>`__
     - `Link <https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B/blob/main/LICENSE>`__
     - 1.5B
     - 1.64 GB
     - > 5.1.1
   * - Qwen3-1.7B-Instruct
     - The pipeline consists of a prefill step and token by token step
     - `Link <https://huggingface.co/Qwen/Qwen3-1.7B>`__
     - `Link <https://huggingface.co/Qwen/Qwen3-1.7B/blob/main/LICENSE>`__
     - 1.7B
     - 1.79 GB
     - > 5.2.0
   * - Qwen3-4B-Instruct-2507
     - The pipeline consists of a prefill step and token by token step
     - `Link <https://huggingface.co/Qwen/Qwen3-4B-Instruct-2507>`__
     - `Link <https://huggingface.co/Qwen/Qwen3-4B-Instruct-2507/blob/main/LICENSE>`__
     - 4B
     - 4.2 GB
     - > 5.4.0
   * - Qwen3-VL-4B-Instruct
     - The pipeline processes video and text inputs using a vision encoder and language model to generate contextualized outputs.
     - `Link <https://huggingface.co/Qwen/Qwen3-VL-4B-Instruct>`__
     - `Link <https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md>`__
     - 4B
     - 4.4 GB
     - > 5.4.0
   * - Gemma4-E2B-Instruct
     - The pipeline consists of a prefill step and token by token step
     - `Link <https://huggingface.co/google/gemma-4-E2B-it-qat-mobile-transformers>`__
     - `Link <https://ai.google.dev/gemma/docs/gemma_4_license>`__
     - E2B
     - 3.11 GB
     - > 5.4.0
   * - Qwen2-VL-2B-Instruct
     - The pipeline processes image and text inputs using a vision encoder and language model to generate contextualized outputs.
     - `Link <https://huggingface.co/Qwen/Qwen2-VL-2B-Instruct>`__
     - `Link <https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md>`__
     - 2B
     - 2.18 GB
     - > 5.2.0
   * - Qwen3-VL-2B-Instruct
     - The pipeline processes video and text inputs using a vision encoder and language model to generate contextualized outputs.
     - `Link <https://huggingface.co/Qwen/Qwen3-VL-2B-Instruct>`__
     - `Link <https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md>`__
     - 2B
     - 2.18 GB
     - > 5.3.0
   * - Whisper-Tiny
     - Audio is sampled to 16 kHz and converted to 10s window. A Transformer encoder processes the spectrogram and a Transformer decoder autoregressively predicts text tokens
     - `Link <https://huggingface.co/openai/whisper-tiny>`__
     - `Link <https://choosealicense.com/licenses/apache-2.0/>`__
     - 39M
     - 78 MB
     - > 5.2.0
   * - Whisper-Base
     - Audio is sampled to 16 kHz and converted to 10s window. A Transformer encoder processes the spectrogram and a Transformer decoder autoregressively predicts text tokens
     - `Link <https://huggingface.co/openai/whisper-base>`__
     - `Link <https://choosealicense.com/licenses/apache-2.0/>`__
     - 74M
     - 155 MB
     - > 5.2.0
   * - Whisper-Small
     - Audio is sampled to 16 kHz and converted to 10s window. A Transformer encoder processes the spectrogram and a Transformer decoder autoregressively predicts text tokens
     - `Link <https://huggingface.co/openai/whisper-small>`__
     - `Link <https://choosealicense.com/licenses/apache-2.0/>`__
     - 244M
     - 388 MB
     - > 5.2.0
   * - Whisper-Medium
     - Audio is sampled to 16 kHz and converted to 10s window. A Transformer encoder processes the spectrogram and a Transformer decoder autoregressively predicts text tokens
     - `Link <https://huggingface.co/openai/whisper-medium>`__
     - `Link <https://choosealicense.com/licenses/apache-2.0/>`__
     - 769M
     - 1.33 GB
     - > 5.4.0

Some of the precompiled models are compatible only with v5.5.0.

If you need precompiled AI models compatible with v5.1.1, please visit the `Models page <https://github.com/hailo-ai/hailo_model_zoo_genai/blob/v5.1.1/docs/MODELS.rst>`__.

----

Technical, Performance & Accuracy
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. list-table::
   :header-rows: 1

   * - Model
     - Context Length
     - Numerical Scheme
     - Inference API
     - Compiled Model
     - Load Time [s]
     - TTFT [s]
     - TPS
   * - DeepSeek-R1-Distill-Qwen-1.5B
     - 2048
     - A8W4, symmetric, group-wise
     - C++, Python, Hailo-Ollama
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/DeepSeek-R1-Distill-Qwen-1.5B.hef>`__
     - 5.57
     - 1.45
     - 7.59
   * - Llama3.2-1B-Instruct
     - 2048
     - A8W4, symmetric, group-wise
     - C++, Python, Hailo-Ollama
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Llama3.2-1B-Instruct.hef>`__
     - 3.56
     - 0.35
     - 9.82
   * - Qwen2-1.5B-Instruct
     - 2048
     - A8W4, symmetric, channel-wise
     - C++, Python, Hailo-Ollama
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Qwen2-1.5B-Instruct.hef>`__
     - 3.99
     - 0.32
     - 8.02
   * - Qwen2-1.5B-Instruct-Function-Calling-v1
     - 2048
     - A8W4, symmetric, channel-wise
     - C++, Python
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Qwen2-1.5B-Instruct-Function-Calling-v1.hef>`__
     - 5.87
     - 0.4
     - 6.97
   * - Qwen2.5-1.5B-Instruct
     - 2048
     - A8W4, symmetric, group-wise
     - C++, Python, Hailo-Ollama
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Qwen2.5-1.5B-Instruct.hef>`__
     - 5.68
     - 0.37
     - 7.32
   * - Qwen2.5-Coder-1.5B-Instruct
     - 2048
     - A8W4, symmetric, channel-wise
     - C++, Python, Hailo-Ollama
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Qwen2.5-Coder-1.5B-Instruct.hef>`__
     - 4.08
     - 0.32
     - 8.13
   * - Qwen3-1.7B-Instruct
     - 2048
     - A8W4, symmetric, group-wise
     - C++, Python, Hailo-Ollama
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Qwen3-1.7B-Instruct.hef>`__
     - 6.06
     - 0.62
     - 4.72
   * - Qwen3-4B-Instruct-2507
     - 2048
     - A16W4, symmetric, group-wise
     - C++, Python, Hailo-Ollama
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Qwen3-4B-Instruct-2507.hef>`__
     - 18.23
     - 1.53
     - 2.07
   * - Qwen3-VL-4B-Instruct
     - 2048
     - A16W4, symmetric, group-wise
     - C++, Python
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Qwen3-VL-4B-Instruct.hef>`__
     - 21.98
     - 3.59
     - 2.29
   * - Gemma4-E2B-Instruct
     - 2048
     - A16W4, symmetric, group-wise
     - C++, Python, Hailo-Ollama
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Gemma4-E2B-Instruct.hef>`__
     - 12.5
     - 0.55
     - 5.47
   * - Qwen2-VL-2B-Instruct
     - 2048
     - A8W4, symmetric, channel-wise
     - C++, Python
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Qwen2-VL-2B-Instruct.hef>`__
     - 8.0
     - 1.01
     - 7.13
   * - Qwen3-VL-2B-Instruct
     - 2048
     - A16W4, symmetric, group-wise
     - C++, Python
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Qwen3-VL-2B-Instruct.hef>`__
     - 8.05
     - 1.47
     - 4.89
   * - Whisper-Tiny
     - ---
     - Mixed precision
     - C++, Python
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Whisper-Tiny.hef>`__
     - 0.65
     - 0.0
     - 47.4
   * - Whisper-Base
     - ---
     - Mixed precision
     - C++, Python
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Whisper-Base.hef>`__
     - 1.34
     - 0.0
     - 25.2
   * - Whisper-Small
     - ---
     - Mixed precision
     - C++, Python
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Whisper-Small.hef>`__
     - 3.44
     - 0.0
     - 10.44
   * - Whisper-Medium
     - ---
     - Mixed precision
     - C++, Python
     - `Link <https://dev-public.hailo.ai/v5.5.0/blob/Whisper-Medium.hef>`__
     - 7.87
     - 0.0
     - 4.44

Hailo-10H on-module DRAM
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. list-table::
   :header-rows: 1

   * - Model
     - 2 GB
     - 4 GB
     - 8 GB
   * - DeepSeek-R1-Distill-Qwen-1.5B
     - No
     - Yes
     - Yes
   * - Llama3.2-1B-Instruct
     - Yes\*
     - Yes
     - Yes
   * - Qwen2-1.5B-Instruct
     - No
     - Yes
     - Yes
   * - Qwen2-1.5B-Instruct-Function-Calling-v1
     - No
     - Yes\*
     - Yes
   * - Qwen2.5-1.5B-Instruct
     - No
     - Yes
     - Yes
   * - Qwen2.5-Coder-1.5B-Instruct
     - No
     - Yes
     - Yes
   * - Qwen3-1.7B-Instruct
     - No
     - Yes
     - Yes
   * - Qwen3-4B-Instruct-2507
     - No
     - No
     - Yes
   * - Qwen3-VL-4B-Instruct
     - No
     - No
     - Yes
   * - Gemma4-E2B-Instruct
     - No
     - Yes\*
     - Yes
   * - Qwen2-VL-2B-Instruct
     - No
     - Yes\*
     - Yes
   * - Qwen3-VL-2B-Instruct
     - No
     - Yes\*
     - Yes
   * - Whisper-Tiny
     - Yes
     - Yes
     - Yes
   * - Whisper-Base
     - Yes
     - Yes
     - Yes
   * - Whisper-Small
     - Yes
     - Yes
     - Yes
   * - Whisper-Medium
     - No
     - Yes
     - Yes

\* Requires memory optimization (``optimize_memory_on_device`` in the C++ and Python APIs). A future release is planned to enable it automatically when needed.

Qwen-VL (Image Encoders only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. list-table::
   :header-rows: 1
   :widths: 25 30 25 15

   * - Model
     - Numerical Scheme
     - Compiled Model
     - FPS
   * - Qwen2-VL-2B-vision-336x336
     - A8W8, symmetric, channel-wise
     - `Hailo10h <https://dev-public.hailo.ai/v5.5.0/blob/Qwen2-VL-2B-vision-336x336.hef>`__
     - 3.00
   * - Qwen2-VL-7B-vision-336x336
     - A8W8, symmetric, channel-wise
     - `Hailo10h <https://dev-public.hailo.ai/v5.5.0/blob/Qwen2-VL-7B-vision-336x336.hef>`__
     - 2.80
   * - Qwen2-VL-7B-vision-252x448
     - A8W8, symmetric, channel-wise
     - `Hailo10h <https://dev-public.hailo.ai/v5.5.0/blob/Qwen2-VL-7B-vision-252x448.hef>`__
     - 2.80
   * - Qwen3-VL-2B-vision
     - A8W8, symmetric, channel-wise
     - `Hailo10h <https://dev-public.hailo.ai/v5.5.0/blob/Qwen3-VL-2B-vision.hef>`__, `Hailo8 <https://dev-public.hailo.ai/v5.5.0/blob/Qwen3-VL-2B-vision-hailo8.hef>`__
     - 4.59