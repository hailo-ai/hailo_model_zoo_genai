=========
Changelog
=========

**v5.3.0**

* Update to use HailoRT v5.3.0 (`developer-zone <https://hailo.ai/developer-zone/>`_).
* Added `Qwen3-1.7B <https://huggingface.co/Qwen/Qwen3-1.7B>`_ model.
* Added `Qwen3-VL-2B-Instruct <https://huggingface.co/Qwen/Qwen3-VL-2B-Instruct>`_ model. The model currently supports single-frame image input only. Video input is not supported in this release.
* Added support for Windows OS. See `USAGE.rst <USAGE.rst>`_ for details.
* Added support for ``OLLAMA_HOST`` environment variable to configure the server network interface and port. Its behavior matches the official Ollama server's ``OLLAMA_HOST`` environment variable. See `USAGE.rst <USAGE.rst>`_ for details.
* Added support for ``HAILO_OLLAMA_VDEVICE_GROUP_ID`` environment variable to enable VDevice sharing with other HailoRT applications (e.g., Whisper, vision models). See `USAGE.rst <USAGE.rst>`_ for details.
* Added support for streamlined installation from source using ``cmake --install``. See `USAGE.rst <USAGE.rst>`_ and `README.rst <../README.rst>`_ for details.
* Removed the ``hailo-ollama.config`` file. To define a custom port, use the ``OLLAMA_HOST`` environment variable instead.
* Users upgrading from previous versions who installed manually to user directories should remove the old artifacts before installing the new version:

  .. code-block:: bash
    :name: hmzga-changelog-1
    :caption: Removing old installation artifacts.

    rm -rf ~/.local/bin/hailo-ollama ~/.config/hailo-ollama ~/.local/share/hailo-ollama

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
