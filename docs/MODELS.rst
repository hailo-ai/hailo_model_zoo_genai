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
     - `url <https://huggingface.co/deepseek-ai/DeepSeek-R1-Distill-Qwen-1.5B>`__
     - `url <https://github.com/deepseek-ai/DeepSeek-R1/blob/main/LICENSE>`__
     - 1.5B
     - 2.37 GB
     - > 5.1.1
   * - Qwen2.5-Coder-1.5B-Instruct
     - The pipeline consists of a prefill step and token by token step, optimized for coding tasks
     - `url <https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B>`__
     - `url <https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B/blob/main/LICENSE>`__
     - 1.5B
     - 1.64 GB
     - > 5.1.1
   * - Qwen2-1.5B-Instruct
     - The pipeline consists of a prefill step and token by token step
     - `url <https://huggingface.co/Qwen/Qwen2-1.5B-Instruct>`__
     - `url <https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md>`__
     - 1.5B
     - 1.56 GB
     - > 5.1.1
   * - Qwen2-1.5B-Instruct-Function-Calling-v1
     - The pipeline consists of a prefill step and token by token step, fine-tuned for function calling tasks. This model is a fine-tuned version of Qwen/Qwen2-1.5B-Instruct on devanshamin/gem-viggo-function-calling dataset.
     - `url <https://huggingface.co/devanshamin/Qwen2-1.5B-Instruct-Function-Calling-v1>`__
     - `url <https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md>`__
     - 1.5B
     - 2.99 GB
     - > 5.2.0
   * - Qwen2-VL-2B-Instruct
     - The pipeline processes image and text inputs using a vision encoder and language model to generate contextualized outputs.
     - `url <https://huggingface.co/Qwen/Qwen2-VL-2B-Instruct>`__
     - `url <https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md>`__
     - 2B
     - 2.18 GB
     - > 5.2.0
   * - Whisper-Base
     - Audio is sampled to 16 kHz and converted to 10s window. A Transformer encoder processes the spectrogram and a Transformer decoder autoregressively predicts text tokens
     - `url <https://huggingface.co/openai/whisper-base>`__
     - `url <https://choosealicense.com/licenses/apache-2.0/>`__
     - 74M
     - 155 MB
     - > 5.2.0
   * - Whisper-Small
     - Audio is sampled to 16 kHz and converted to 10s window. A Transformer encoder processes the spectrogram and a Transformer decoder autoregressively predicts text tokens
     - `url <https://huggingface.co/openai/whisper-small>`__
     - `url <https://choosealicense.com/licenses/apache-2.0/>`__
     - 244M
     - 388 MB
     - > 5.2.0
   * - Qwen2.5-1.5B-Instruct
     - The pipeline consists of a prefill step and token by token step
     - `url <https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct>`__
     - `url <https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct/blob/main/LICENSE>`__
     - 1.5B
     - 1.64 GB
     - > 5.1.1
   * - Llama3.2-1B-Instruct
     - The pipeline consists of a prefill step and token by token step
     - `url <https://huggingface.co/meta-llama/Llama-3.2-1B-Instruct>`__
     - `url <https://huggingface.co/meta-llama/Llama-3.2-1B/blob/main/LICENSE.txt>`__
     - 1B
     - 1.79 GB
     - > 5.2.0

Some of the precompiled models are compatible only with v5.2.0.

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
     - `url <https://dev-public.hailo.ai/v5.2.0/blob/DeepSeek-R1-Distill-Qwen-1.5B.hef>`__
     - 4.79
     - 0.74
     - 6.98
   * - Qwen2.5-Coder-1.5B-Instruct
     - 2048
     - A8W4, symmetric, channel-wise
     - C++, Python, Hailo-Ollama
     - `url <https://dev-public.hailo.ai/v5.2.0/blob/Qwen2.5-Coder-1.5B-Instruct.hef>`__
     - 4.76
     - 0.32
     - 8.07
   * - Qwen2-1.5B-Instruct
     - 2048
     - A8W4, symmetric, channel-wise
     - C++, Python, Hailo-Ollama
     - `url <https://dev-public.hailo.ai/v5.2.0/blob/Qwen2-1.5B-Instruct.hef>`__
     - 3.79
     - 0.32
     - 8.08
   * - Qwen2-1.5B-Instruct-Function-Calling-v1
     - 2048
     - A8W4, symmetric, channel-wise
     - C++, Python
     - `url <https://dev-public.hailo.ai/v5.2.0/blob/Qwen2-1.5B-Instruct-Function-Calling-v1.hef>`__
     - 7.91
     - 0.40
     - 6.23
   * - Qwen2-VL-2B-Instruct
     - 2048
     - A8W4, symmetric, channel-wise
     - C++, Python
     - `url <https://dev-public.hailo.ai/v5.2.0/blob/Qwen2-VL-2B-Instruct.hef>`__
     - 6.226
     - 0.97
     - 6.73
   * - Whisper-Base
     - N/A
     - Mixed precision
     - C++, Python
     - `url <https://dev-public.hailo.ai/v5.2.0/blob/Whisper-Base.hef>`__
     - 3.89
     - N/A
     - 23.36
   * - Whisper-Small
     - N/A
     - Mixed precision
     - C++, Python
     - `url <https://dev-public.hailo.ai/v5.2.0/blob/Whisper-Small.hef>`__
     - 11.92
     - N/A
     - 8.71
   * - Qwen2.5-1.5B-Instruct
     - 2048
     - A8W4, symmetric, group-wise
     - C++, Python, Hailo-Ollama
     - `url <https://dev-public.hailo.ai/v5.2.0/blob/Qwen2.5-1.5B-Instruct.hef>`__
     - 5.05
     - 0.37
     - 6.82
   * - Llama3.2-1B-Instruct
     - 2048
     - A8W4, symmetric, group-wise
     - C++, Python, Hailo-Ollama
     - `url <https://dev-public.hailo.ai/v5.2.0/blob/Llama3.2-1B-Instruct.hef>`__
     - 3.839
     - 0.49
     - 8.48

Qwen-VL (Image Encoders only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. list-table::
   :header-rows: 1
   :widths: 25 30 25 15

   * - Model
     - Numerical Scheme
     - Compiled Model
     - FPS
   * - Qwen3-VL-2B-vision
     - A8W8, symmetric, channel-wise
     - `url <https://dev-public.hailo.ai/v5.2.0/blob/Qwen3-VL-2B-vision.hef>`__
     - 4.59
   * - Qwen2-VL-2B-vision-336x336
     - A8W8, symmetric, channel-wise
     - `url <https://dev-public.hailo.ai/v5.2.0/blob/Qwen2-VL-2B-vision-336x336.hef>`__
     - 3.00
   * - Qwen2-VL-7B-vision-336x336
     - A8W8, symmetric, channel-wise
     - `url <https://dev-public.hailo.ai/v5.2.0/blob/Qwen2-VL-7B-vision-336x336.hef>`__
     - 2.80
   * - Qwen2-VL-7B-vision-252x448
     - A8W8, symmetric, channel-wise
     - `url <https://dev-public.hailo.ai/v5.2.0/blob/Qwen2-VL-7B-vision-252x448.hef>`__
     - 2.80