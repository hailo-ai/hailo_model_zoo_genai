Model Properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. list-table::
   :header-rows: 1
   :widths: 20 30 8 8 8 10

   * - Model
     - Description
     - Source
     - License
     - Parameters
     - Model Size
   * - DeepSeek-R1-Distill-Qwen-1.5B
     - Utilizes a transformer-based architecture with instruction tuning to enhance logical reasoning, natural language understanding, and content generation
     - `url <https://huggingface.co/deepseek-ai/DeepSeek-R1-Distill-Qwen-1.5B>`__
     - `url <https://github.com/deepseek-ai/DeepSeek-R1/blob/main/LICENSE>`__
     - 1.5B
     - 2.37 GB
   * - Qwen2.5-Coder-1.5B-Instruct
     - The pipeline consists of a prefill and TBT models, optimized for coding tasks
     - `url <https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B>`__
     - `url <https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B/blob/main/LICENSE>`__
     - 1.5B
     - 1.64 GB
   * - Qwen2-1.5B-Instruct
     - The pipeline consists of a prefill and tbt models
     - `url <https://huggingface.co/Qwen/Qwen2-1.5B-Instruct>`__
     - `url <https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md>`__
     - 1.5B
     - 1.56 GB
   * - Qwen2-VL-2B-Instruct
     - The pipeline processes image and text inputs using a vision encoder and language model to generate contextualized outputs.
     - `url <https://huggingface.co/Qwen/Qwen2-VL-2B-Instruct>`__
     - `url <https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md>`__
     - 2B
     - 2.18 GB
   * - Whisper-Base
     - Audio is sampled to 16 kHz and converted to 10s window. A Transformer encoder processes the spectrogram and a Transformer decoder autoregressively predicts text tokens
     - `url <https://huggingface.co/openai/whisper-base>`__
     - `url <https://choosealicense.com/licenses/apache-2.0/>`__
     - 74M
     - 155 MB
   * - Qwen2.5-1.5B-Instruct
     - The pipeline consists of a prefill and tbt models
     - `url <https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct>`__
     - `url <https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct/blob/main/LICENSE>`__
     - 1.5B
     - 1.64 GB
   * - Llama3.2-3B-Instruct
     - The pipeline consists of a prefill and token-by-token models.
     - `url <https://huggingface.co/meta-llama/Llama-3.2-3B>`__
     - `url <https://huggingface.co/meta-llama/Llama-3.2-3B/blob/main/LICENSE.txt>`__
     - 3B
     - 3.13 GB

Technical, Performance & Accuracy
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. list-table::
   :header-rows: 1

   * - Model
     - Operations
     - Context Length
     - Numerical Scheme
     - Inference API
     - Compiled Model
     - Load Time [s]
     - TTFT [s]
     - TPS
   * - DeepSeek-R1-Distill-Qwen-1.5B
     - 29.4 GOPs per input token
     - 2048
     - A8W4, symmetric, channel-wise
     - CPP, Hailo-Ollama
     - `url <https://dev-public.hailo.ai/v5.1.1/blob/DeepSeek-R1-Distill-Qwen-1.5B.hef>`__
     - 12.61
     - 1.47
     - 7.03
   * - Qwen2.5-Coder-1.5B-Instruct
     - 29.4 GOPs per input token
     - 2048
     - A8W4, symmetric, channel-wise
     - CPP, Hailo-Ollama
     - `url <https://dev-public.hailo.ai/v5.1.1/blob/Qwen2.5-Coder-1.5B-Instruct.hef>`__
     - 7.60
     - 0.32
     - 8.15
   * - Qwen2-1.5B-Instruct
     - 29.4 GOPs per input token
     - 2048
     - A8W4, symmetric, channel-wise
     - CPP, Hailo-Ollama
     - `url <https://dev-public.hailo.ai/v5.1.1/blob/Qwen2-1.5B-Instruct.hef>`__
     - 7.37
     - 0.32
     - 8.18
   * - Qwen2-VL-2B-Instruct
     - ---
     - 2048
     - A8W4, symmetric, channel-wise
     - CPP
     - `url <https://dev-public.hailo.ai/v5.1.1/blob/Qwen2-VL-2B-Instruct.hef>`__
     - 9.71
     - 0.92
     - 8.38
   * - Whisper-Base
     - ---
     - ---
     - A8W8, symmetric, channel-wise
     - CPP
     - `url <https://dev-public.hailo.ai/v5.1.1/blob/Whisper-Base.hef>`__
     - 1.33
     - 0.07
     - 47.55
   * - Qwen2.5-1.5B-Instruct
     - 29.4 GOPs per input token
     - 2048
     - A8W4, symmetric, channel-wise
     - CPP, Hailo-Ollama
     - `url <https://dev-public.hailo.ai/v5.1.1/blob/Qwen2.5-1.5B-Instruct.hef>`__
     - 8.45
     - 0.37
     - 6.95
   * - Llama3.2-3B-Instruct
     - 7.14 GOPs per input token
     - 2048
     - A8W4, symmetric, group-wise
     - CPP, Hailo-Ollama
     - `url <https://dev-public.hailo.ai/v5.1.1/blob/Llama-3_2-3B-Instruct.hef>`__
     - 12.61
     - 1.47
     - 2.67