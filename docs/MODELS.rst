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
     - Consists of a prefill and tbt models.
     - `url <https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct>`__
     - `url <https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct/blob/main/LICENSE>`__
     - 1.5B
     - 1.82 GB
   * - Qwen2.5-Coder-1.5B-Instruct
     - Consists of a prefill and TBT models, optimized for coding tasks.
     - `url <https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B>`__
     - `url <https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B/blob/main/LICENSE>`__
     - 1.5B
     - 1.64 GB
   * - Qwen2-1.5B-Instruct
     - Consists of a prefill and tbt models.
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
     - 2048.0
     - A8W4, symmetric, channel-wise
     - CPP, Hailo-Ollama
     - `url <http://dev-public.hailo.ai/v5.0.0/blob/DeepSeek-R1-Distill-Qwen-1.5B.hef>`__
     - 3.2
     - 9.81788
     - 0.680284
     - 7.83009
   * - Qwen2.5-1.5B-Instruct
     - 29.4 GOPs per input token
     - 2048.0
     - A8W4, symmetric, channel-wise
     - CPP, Hailo-Ollama
     - `url <http://dev-public.hailo.ai/v5.0.0/blob/Qwen2.5-1.5B-Instruct.hef>`__
     - 3.2
     - 10.7077
     - 0.325097
     - 7.99287
   * - Qwen2.5-Coder-1.5B-Instruct
     - 29.4 GOPs per input token
     - 2048.0
     - A8W4, symmetric, channel-wise
     - CPP, Hailo-Ollama
     - `url <http://dev-public.hailo.ai/v5.0.0/blob/Qwen2.5-Coder-1.5B.hef>`__
     - 3.2
     - 8.58971
     - 0.322522
     - 8.08952
   * - Qwen2-1.5B-Instruct
     - 29.4 GOPs per input token
     - 2048.0
     - A8W4, symmetric, channel-wise
     - CPP, Hailo-Ollama
     - `url <http://dev-public.hailo.ai/v5.0.0/blob/Qwen2-1.5B-Instruct.hef>`__
     - 3.2
     - 8.34639
     - 0.322963
     - 8.12567
   * - Qwen2-VL-2B-Instruct
     - —
     - 2048.0
     - A8W4, symmetric, channel-wise
     - CPP
     - `url <http://dev-public.hailo.ai/v5.0.0/blob/b10dbeedc54738cc23bc50cd9895b339296cc352ef8caf02ab2af700f0ed85ab>`__
     - 3.2
     - 17.1006
     - —
     - 8.15536
   * - Stable-Diffusion-1.5
     - 19 TOPS for 20 iterations
     - —
     - a8_w8, channel-wise, symmetric
     - CPP
     - `url <http://dev-public.hailo.ai/v5.0.0/blob/d38d44d440105052f2c6943b751a4e2204d26568538e6e7997900694796665d1>`__
     - 3.2
     - 5.22621
     - —
     - —
