.. list-table:: Model Properties
   :header-rows: 1

   * - Model 
     - Description 
     - Source URL 
     - Source Icon 
     - License URL 
     - Parameters 
     - Model Size
   * - DeepSeek-R1-Distill-Qwen-1.5B 
     - Utilizes a transformer-based architecture with instruction tuning to enhance logical reasoning, natural language understanding, and content generation 
     - https://huggingface.co/deepseek-ai/DeepSeek-R1-Distill-Qwen-1.5B 
     - https://hailo.ai/wp-content/uploads/2025/06/link-Icon_GenAI-page.svg 
     - https://github.com/deepseek-ai/DeepSeek-R1/blob/main/LICENSE 
     - 1.5B 
     - 1.58 GB
   * - Qwen2.5-1.5B-Instruct 
     - The pipeline consists of a prefill and tbt models 
     - https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct 
     - https://hailo.ai/wp-content/uploads/2025/06/link-Icon_GenAI-page.svg 
     - https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct/blob/main/LICENSE 
     - 1.5B 
     - 1.82 GB
   * - Qwen2.5-Coder-1.5B-Instruct 
     - The pipeline consists of a prefill and TBT models, optimized for coding tasks 
     - https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B 
     - https://hailo.ai/wp-content/uploads/2025/06/link-Icon_GenAI-page.svg 
     - https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B/blob/main/LICENSE 
     - 1.5B 
     - 1.64 GB
   * - Qwen2-1.5B-Instruct 
     - The pipeline consists of a prefill and tbt models 
     - https://huggingface.co/Qwen/Qwen2-1.5B-Instruct 
     - https://hailo.ai/wp-content/uploads/2025/06/link-Icon_GenAI-page.svg 
     - https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md 
     - 1.5B 
     - 1.56 GB
   * - Qwen2-VL-2B-Instruct 
     - The pipeline processes image and text inputs using a vision encoder and language model to generate contextualized outputs. 
     - https://huggingface.co/Qwen/Qwen2-VL-2B-Instruct 
     - https://hailo.ai/wp-content/uploads/2025/06/link-Icon_GenAI-page.svg 
     - https://huggingface.co/datasets/choosealicense/licenses/blob/main/markdown/apache-2.0.md 
     - 2B 
     - 2.18 GB
   * - Stable-Diffusion-1.5 
     - Uses a Text Encoder to turn prompts into embeddings, an Image Decoder for latent representation, and the UNet Model in 20 steps. Supports batch processing (positive and negative prompts). 
     - https://huggingface.co/stable-diffusion-v1-5/stable-diffusion-v1-5 
     - https://hailo.ai/wp-content/uploads/2025/06/link-Icon_GenAI-page.svg 
     - https://huggingface.co/spaces/CompVis/stable-diffusion-license 
     - — 
     - —




.. list-table:: Technical, Performance & Accuracy
   :header-rows: 1

   * - Model 
     - Operations 
     - Context Length 
     - Numerical Scheme 
     - Inference API 
     - Compiled Model URL 
     - Compiled Icon 
     - First Load Time [s] 
     - Load Time [s] 
     - TTFT [s] 
     - TPS 
     - DATASET 
     - DATASET.MMLU.TEST 
     - DATASET.MMLU.EVALUATION_METRIC 
     - DATASET.MMLU.FULL_PRECISION_ACCURACY 
     - DATASET.MMLU.POST_QUANTIZATION_ACCURACY
   * - DeepSeek-R1-Distill-Qwen-1.5B 
     - 29.4 GOPs per input token 
     - 2048.0 
     - A8W4, symmetric, channel-wise 
     - CPP, Hailo-Ollama 
     - http://dev-public.hailo.ai/DeepSeek-R1-Distill-Qwen-1.5B.hef 
     - https://hailo.ai/wp-content/uploads/2025/06/download-Icon_GenAI-page.svg 
     - 3.2 
     - 9.81788 
     - 0.680284 
     - 7.83009 
     - — 
     - — 
     - — 
     - — 
     - —
   * - Qwen2.5-1.5B-Instruct 
     - 29.4 GOPs per input token 
     - 2048.0 
     - A8W4, symmetric, channel-wise 
     - CPP, Hailo-Ollama 
     - http://dev-public.hailo.ai/Qwen2.5-1.5B-Instruct.hef 
     - https://hailo.ai/wp-content/uploads/2025/06/download-Icon_GenAI-page.svg 
     - 3.2 
     - 10.7077 
     - 0.325097 
     - 7.99287 
     - — 
     - MMLU 
     - accuracy 
     - 59.0 
     - 51.0
   * - Qwen2.5-Coder-1.5B-Instruct 
     - 29.4 GOPs per input token 
     - 2048.0 
     - A8W4, symmetric, channel-wise 
     - CPP, Hailo-Ollama 
     - http://dev-public.hailo.ai/Qwen2.5-Coder-1.5B.hef 
     - https://hailo.ai/wp-content/uploads/2025/06/download-Icon_GenAI-page.svg 
     - 3.2 
     - 8.58971 
     - 0.322522 
     - 8.08952 
     - — 
     - MMLU 
     - accuracy 
     - 48.0 
     - 43.0
   * - Qwen2-1.5B-Instruct 
     - 29.4 GOPs per input token 
     - 2048.0 
     - A8W4, symmetric, channel-wise 
     - CPP, Hailo-Ollama 
     - http://dev-public.hailo.ai/Qwen2-1.5B-Instruct.hef 
     - Compiled model https://hailo.ai/wp-content/uploads/2025/06/download-Icon_GenAI-page.svg 
     - 3.2 
     - 8.34639 
     - 0.322963 
     - 8.12567 
     - — 
     - MMLU 
     - accuracy 
     - 55.0 
     - 51.0
   * - Qwen2-VL-2B-Instruct 
     - TBD 
     - 2048.0 
     - A8W4, symmetric, channel-wise 
     - CPP, Hailo-Ollama 
     - http://dev-public.hailo.ai/b10dbeedc54738cc23bc50cd9895b339296cc352ef8caf02ab2af700f0ed85ab 
     - https://hailo.ai/wp-content/uploads/2025/06/download-Icon_GenAI-page.svg 
     - 3.2 
     - 17.1006 
     - — 
     - 8.15536 
     - — 
     - MMLU 
     - accuracy 
     - 52.0 
     - 47.0
   * - Stable-Diffusion-1.5 
     - — 
     - — 
     - a8_w8, channel-wise, symmetric 
     - CPP, Hailo-Ollama 
     - http://dev-public.hailo.ai/d38d44d440105052f2c6943b751a4e2204d26568538e6e7997900694796665d1 
     - https://hailo.ai/wp-content/uploads/2025/06/download-Icon_GenAI-page.svg 
     - 3.2 
     - 5.22621 
     - — 
     - — 
     - — 
     - — 
     - — 
     - — 
     - —















