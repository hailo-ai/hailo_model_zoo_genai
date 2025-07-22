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
     - Utilizes a transformer-based architecture with instruction tuning to enhance logical reasoning, natural language understanding, and content generation.
     - `url <https://huggingface.co/deepseek-ai/DeepSeek-R1-Distill-Qwen-1.5B>`__
     - `url <https://github.com/deepseek-ai/DeepSeek-R1/blob/main/LICENSE>`__
     - 1.5B
     - 1.58 GB
   * - Qwen2.5-1.5B-Instruct
     - The pipeline consists of a prefill and tbt models.
     - `url <https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct>`__
     - `url <https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct/blob/main/LICENSE>`__
     - 1.5B
     - 1.82 GB
   * - Qwen2.5-Coder-1.5B-Instruct
     - The pipeline consists of a prefill and TBT models, optimized for coding tasks.
     - `url <https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B>`__
     - `url <https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B/blob/main/LICENSE>`__
     - 1.5B
     - 1.64 GB
   * - Qwen2-1.5B-Instruct
     - The pipeline consists of a prefill and tbt models.
     - `url <https://huggingface.co/Qwen/Qwen2-1.5B-Instruct>`__
     - `url <https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md>`__
     - 1.5B
     - 1.56 GB
   * - Qwen2-VL-2B-Instruct
     - Processes image and text inputs using a vision encoder and language model to generate contextualized outputs.
     - `url <https://huggingface.co/Qwen/Qwen2-VL-2B-Instruct>`__
     - `url <https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md>`__
     - 2B
     - 2.18 GB
   * - Stable-Diffusion-1.5
     - Uses a Text Encoder to turn prompts into embeddings, an Image Decoder for latent representation, and the UNet Model in 20 steps. Supports batch processing (positive and negative prompts).
     - `url <https://huggingface.co/stable-diffusion-v1-5/stable-diffusion-v1-5>`__
     - `url <https://huggingface.co/spaces/CompVis/stable-diffusion-license>`__
     - 1B
     - 1.00 GB

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
     - First Load Time [s]
     - Load Time [s]
     - TTFT [s]
     - TPS
   * - DeepSeek-R1-Distill-Qwen-1.5B
     - 29.4 GOPs per input token
     - 2048
     - A8W4, symmetric, channel-wise
     - CPP, Hailo-Ollama
     - `url <https://dev-public.hailo.ai/v5.0.0/blob/DeepSeek-R1-Distill-Qwen-1.5B.hef>`__
     - 3.2
     - 9.81
     - 0.68
     - 7.83
   * - Qwen2.5-1.5B-Instruct
     - 29.4 GOPs per input token
     - 2048
     - A8W4, symmetric, channel-wise
     - CPP, Hailo-Ollama
     - `url <https://dev-public.hailo.ai/v5.0.0/blob/Qwen2.5-1.5B-Instruct.hef>`__
     - 3.2
     - 10.70
     - 0.32
     - 7.99
   * - Qwen2.5-Coder-1.5B-Instruct
     - 29.4 GOPs per input token
     - 2048
     - A8W4, symmetric, channel-wise
     - CPP, Hailo-Ollama
     - `url <https://dev-public.hailo.ai/v5.0.0/blob/Qwen2.5-Coder-1.5B.hef>`__
     - 3.2
     - 8.58
     - 0.32
     - 8.08
   * - Qwen2-1.5B-Instruct
     - 29.4 GOPs per input token
     - 2048
     - A8W4, symmetric, channel-wise
     - CPP, Hailo-Ollama
     - `url <https://dev-public.hailo.ai/v5.0.0/blob/Qwen2-1.5B-Instruct.hef>`__
     - 3.2
     - 8.34
     - 0.32
     - 8.12
   * - Qwen2-VL-2B-Instruct
     - ---
     - 2048
     - A8W4, symmetric, channel-wise
     - CPP
     - `url <https://dev-public.hailo.ai/v5.0.0/blob/Qwen2-VL-2B-Instruct.hef>`__
     - 3.2
     - 17.10
     - ---
     - 8.15
   * - Stable-Diffusion-1.5
     - 19 TOPs for 20 iterations
     - ---
     - A8W8, symmetric, channel-wise
     - CPP
     - `url <https://dev-public.hailo.ai/v5.0.0/blob/Stable-Diffusion-1.5.zip>`__
     - 3.2
     - 5.22
     - ---
     - ---
