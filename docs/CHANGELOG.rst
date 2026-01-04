=========
Changelog
=========

**v5.2.0**

* Update to use HailoRT v5.2.0 (`developer-zone <https://hailo.ai/developer-zone/>`_).
* Added `Llama-3.2-1B-Instruct <https://huggingface.co/meta-llama/Llama-3.2-1B-Instruct>`_ model.
* Added `Whisper-small <https://huggingface.co/openai/whisper-small>`_ model.
* Added `Qwen2-1.5B-Instruct-Function-Calling-v1 <https://huggingface.co/devanshamin/Qwen2-1.5B-Instruct-Function-Calling-v1>`_ model (LoRA finetuned adapter for function-calling).
* Added VLM image encoders: `Qwen2-VL-Image-Encoder-7B <https://huggingface.co/Qwen/Qwen2-VL-7B-Instruct>`_, `Qwen2-VL-Image-Encoder-2B <https://huggingface.co/Qwen/Qwen2-VL-2B-Instruct>`_, and `Qwen3-VL-Image-Encoder-2B <https://huggingface.co/Qwen/Qwen3-VL-2B-Instruct>`_.
* Removed support from `Llama-3.2-3B-Instruct <https://huggingface.co/meta-llama/Llama-3.2-3B-Instruct>`_ model.
* Renamed model `qwen2.5-instruct:1.5b` to `qwen2.5:1.5b`. In order to pull and run the model, use the new name `qwen2.5:1.5b`.
* Renamed model `deepseek_r1_distill_qwen:1.5b` to `deepseek-r1:1.5b`. In order to pull and run the model, use the new name `deepseek-r1:1.5b`.
* Load time improvements across all models. For example, loading time for `qwen2.5:1.5b` reduced by ~25%.

**v5.1.1**

* Update to use HailoRT v5.1.1 (`developer-zone <https://hailo.ai/developer-zone/>`_).
* Removed support from `StableDiffusion-1.5 <https://huggingface.co/stable-diffusion-v1-5/stable-diffusion-v1-5>`_.
* Added `Llama-3.2-3B-Instruct <https://huggingface.co/meta-llama/Llama-3.2-3B-Instruct>`_ model.


**v5.1.0**

* Update to use HailoRT v5.1.0 (`developer-zone <https://hailo.ai/developer-zone/>`_).
* Added `Speech-to-Text` support. Supported models include `Whisper-Base <https://huggingface.co/openai/whisper-base>`_ for transcription and translation.
* Accuracy improvements for the DeepSeek model: `DeepSeek-R1-Distill-Qwen-1.5B <https://huggingface.co/deepseek-ai/DeepSeek-R1-Distill-Qwen-1.5B>`_.
* Removed `Qwen2.5-1.5B-Instruct <https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct>`_ model.
* Bug fixes in Hailo-Ollama REST API.

**v5.0.1**

* Update to use HailoRT v5.0.1 (`developer-zone <https://hailo.ai/developer-zone/>`_).
* Update all compiled models.
* Bug fixes.

**v5.0.0**

* Initial release of the Hailo Model Zoo GenAI.
* Initial release of the Hailo-Ollama REST API.
* Support for large language models (LLMs), including `Qwen2-1.5B-Instruct <https://huggingface.co/Qwen/Qwen2-1.5B-Instruct>`_, `Qwen2.5-1.5B-Instruct <https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct>`_, `Qwen2.5-Coder-1.5B-Instruct <https://huggingface.co/Qwen/Qwen2.5-Coder-1.5B-Instruct>`_ and `DeepSeek-R1-Distill-Qwen-1.5B <https://huggingface.co/deepseek-ai/DeepSeek-R1-Distill-Qwen-1.5B>`_.
* Support for image generation (Stable Diffusion), including `StableDiffusion-1.5 <https://huggingface.co/stable-diffusion-v1-5/stable-diffusion-v1-5>`_.
* Support for vision-language models (VLMs), including `Qwen2-VL-2B-Instruct <https://huggingface.co/Qwen/Qwen2-VL-2B-Instruct>`_.
