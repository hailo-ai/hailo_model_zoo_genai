Usage
=====

Prerequisites
-------------

* Hailo-10H module.
* Ensure `HailoRT <https://github.com/hailo-ai/hailort>`__ version **HailoRT v5.0** or above is installed (for non-docker installations).


Installation
------------

1. **Option A - Debian package** (*Ubuntu Recommended*)

  * Download the latest HailoRT GenAI Debian package from the `Developer Zone <https://hailo.ai/developer-zone/>`_.
  * Install it using the following command:

    .. code-block:: bash
      :name: hmzga-usage-1
      :caption: Installing the Debian package.

      sudo dpkg -i hailo_gen_ai_model_zoo_<ver>_<arch>.deb

2. **Option B - Docker** *(Zero-install host)*

  * Download the latest pre-built HailoRT image from the `Developer Zone <https://hailo.ai/developer-zone/>`__. The image contains the HailoRT and the Hailo-Ollama server, and is ready to run.

  * Unzip, load it into Docker and run it:

    .. code-block:: bash
      :name: hmzga-usage-2
      :caption: Running with Docker.

      unzip hailort_<ver>_docker.zip
      docker load -i hailo_docker_hailort_ub2204_<ver>.tar
      docker run -it --rm --device /dev/h1x-0 \
          -p 8000:8000 \
          -v $HOME/.local/share/hailo-ollama/models/blob:/usr/share/hailo-ollama/models/blob \
          hailo_docker_hailort_ub2204:<ver>

  * The Hailo-Ollama server will be available at http://localhost:8000.

3. **Option C - Install from source** *(Advanced)*

  * **Linux**:
    Clone the Hailo Model Zoo GenAI repository, build and install the Hailo-Ollama server:

    .. code-block:: bash
      :name: hmzga-usage-3
      :caption: Building from source.

      git clone https://github.com/hailo-ai/hailo_model_zoo_genai.git
      cd hailo-model-zoo-genai
      cmake -B build -DCMAKE_BUILD_TYPE=Release
      cmake --build build --config Release
      cmake --install build

    * **Note**: On Linux, ``sudo`` may be required for the install step.

  * **Windows**:
    Clone the repository and build using CMake (Ensure OpenSSL and HailoRT are available):

    .. code-block::

      git clone https://github.com/hailo-ai/hailo_model_zoo_genai.git
      cd hailo-model-zoo-genai
      cmake -B build -DCMAKE_PREFIX_PATH="C:/Path/To/HailoRT/cmake"
      cmake --build build --config Release
      cmake --install build --config Release

  * This will install (relative to the install prefix):

    - ``hailo-ollama`` binary to ``bin/``
    - Model manifests to ``share/hailo-ollama/models/manifests/``


Hailo-Ollama REST API
---------------------

The Hailo-Ollama API provides a simple method to run GenAI models on Hailo devices through REST API. The API supports a single simultaneous connection with one pending request and is compatible with the `Ollama <https://github.com/ollama/ollama>`__ REST API, so it can be used with existing tools like LangChain, Open-WebUI, etc.

The Hailo-Ollama API is currently limited to Large Language Models (LLMs) and cannot be used for other tasks such as text-to-image generation or Vision Language Models (VLMs). Additionally, the API does not support LoRA adapters and the Ollama CLI.

A comparison between Ollama and Hailo-Ollama
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
    :widths: 50 50 50
    :header-rows: 1

    * - Feature
      - Hailo-Ollama
      - Ollama
    * - API
      - REST API only
      - CLI and REST API
    * - LoRA adapters
      - Not supported
      - Supported
    * - Models format
      - HEF
      - GGUF
    * - Models weights
      - Not available
      - Safetensors (for creating adapters)
    * - Users can upload models
      - Not supported
      - Supported
    * - List available models for download
      - Supported (with `/hailo/v1/list` endpoint)
      - Not supported
    * - Download models (from Storage server with `pull` endpoint)
      - Supported
      - Supported
    * - Chat template and model parameters
      - Supported
      - Supported
    * - Support with Hailo devices
      - Supported
      - Not supported


The Hailo-Ollama API supports the following endpoints
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* ``GET /api/version`` - shows the version of the server.

* ``GET /api/ps`` - list models that are currently loaded into memory.

* ``GET /hailo/v1/list`` - list all models available for download.
* ``GET /api/tags`` - list models already on the server.
* ``POST /api/pull`` - pulls a model from the library to local storage.
* ``POST /api/show`` - shows model metadata (parameters, template, details, etc.).
* ``DELETE /api/delete`` - removes a model from local storage.

* ``POST /api/chat`` - chat with the model.
* ``POST /api/generate`` - generate text with the model.


Environment Variables
~~~~~~~~~~~~~~~~~~~~~

The Hailo-Ollama server supports the following environment variables:

* ``OLLAMA_HOST`` - Sets the network interface and port for the Hailo-Ollama server.
  Its behavior matches the official Ollama server's ``OLLAMA_HOST`` environment variable.

  The format can be ``host`` (uses the default port) or ``host:port``.
  IPv6 addresses must be enclosed in brackets (``[::1]`` or ``[::1]:port``).
  The port must be a valid TCP port number (1–65535).
  If the variable is unset or invalid, the server falls back to the default host and port (``0.0.0.0:8000``).

  The server may still fail at runtime if:
    - the port is already in use,
    - the host cannot be bound,
    - the port is privileged (<1024) without sufficient permissions.

  **Example**:

  .. code-block:: bash
    :name: hmzga-usage-4
    :caption: Setting OLLAMA_HOST.

    OLLAMA_HOST=0.0.0.0:8000 hailo-ollama


* ``HAILO_OLLAMA_VDEVICE_GROUP_ID`` - Sets the VDevice group ID for sharing the Hailo device with other applications.

  When set, this allows the Hailo-Ollama server to share the Hailo device with other HailoRT-based applications
  running concurrently, such as Whisper (speech-to-text), vision models.

  Other applications that need to share the device should use the same group ID when creating their VDevice.
  This enables multi-application scenarios where, for example, LLM chatbot can be run alongside
  a Whisper transcription service or computer vision pipeline on the same Hailo device.

  **Example**: Running Hailo-Ollama alongside other Hailo applications:

  .. code-block:: bash
    :name: hmzga-usage-5
    :caption: Running with shared group ID.

    # Start Hailo-Ollama with a shared VDevice group
    HAILO_OLLAMA_VDEVICE_GROUP_ID=HAILO_OLLAMA_SHARED hailo-ollama

  **Example**: Creating a VDevice with the same group ID in a Whisper (Speech2Text) application:

  .. code-block:: cpp
    :name: hmzga-usage-6
    :caption: C++ example for shared VDevice.

    #include <hailo/hailort_defaults.hpp>
    #include <hailo/genai/speech2text/speech2text.hpp>

    // Create VDevice params with the same group_id as Hailo-Ollama
    auto params = hailort::HailoRTDefaults::get_vdevice_params();
    params.group_id = "HAILO_OLLAMA_SHARED";  // Same as HAILO_OLLAMA_VDEVICE_GROUP_ID

    auto vdevice = hailort::VDevice::create_shared(params).expect("Failed to create VDevice");

    // Initialize Speech2Text with the shared VDevice
    auto speech2text_params = hailort::genai::Speech2TextParams(hef_path);
    auto speech2text = hailort::genai::Speech2Text::create(vdevice, speech2text_params).expect("Failed to create Speech2Text");

  .. note::
    Running multiple LLM or VLM models simultaneously on the same device is **not supported**.
    Only one LLM/VLM model can run at a time due to exclusive KV-Cache usage.


Using Hailo-Ollama
^^^^^^^^^^^^^^^^^^

* **Tip**: install ``jq`` (``sudo apt install jq``) for nicer JSON output.

* **Note**: Many endpoints (``/api/pull``, ``/api/chat``, ``/api/generate``) honor a ``"stream"`` Boolean in the JSON body.
    * ``true``  (default) - Server-Sent incremental chunks.
    * ``false`` - single JSON response.

* Start the Hailo-Ollama server (default: http://localhost:8000):

  .. code-block:: bash
    :name: hmzga-usage-7
    :caption: Starting the server.

    hailo-ollama

* Shows the server version:

  .. code-block:: bash
    :name: hmzga-usage-8
    :caption: Checking server version.

    curl --silent http://localhost:8000/api/version

* Get a list of all available models for download:

  .. code-block:: bash
    :name: hmzga-usage-9
    :caption: Listing models available for download.

    curl --silent http://localhost:8000/hailo/v1/list

* Pull a specific model:

  .. code-block:: bash
    :name: hmzga-usage-10
    :caption: Pulling a model.

    curl --silent http://localhost:8000/api/pull \
         -H 'Content-Type: application/json' \
         -d '{ "model": "qwen2:1.5b", "stream" : true }'

  **Windows (CMD)**: Use double quotes with escaping:

  .. code-block:: batch

    curl --silent http://localhost:8000/api/pull ^
         -H "Content-Type: application/json" ^
         -d "{ \"model\": \"qwen2:1.5b\", \"stream\" : true }"

* Pull all available models:

  .. code-block:: bash
    :name: hmzga-usage-11
    :caption: Pulling all models.

    curl --silent http://localhost:8000/hailo/v1/list \
    | jq -r '.models[]' \
    | while read model; do
        echo "Pulling $model..."
        curl --no-buffer --silent http://localhost:8000/api/pull \
            -H 'Content-Type: application/json' \
            -d "{\"model\": \"$model\", \"stream\": true}"
      done

* Run the model:

  .. code-block:: bash
    :name: hmzga-usage-12
    :caption: Generating text.

    curl --silent http://localhost:8000/api/generate \
         -H 'Content-Type: application/json' \
         -d '{"model": "qwen2:1.5b", "prompt": "Why is the sky blue?", "stream":false}'


  **Windows (CMD)**: Use double quotes with escaping:

  .. code-block:: batch

    curl --silent http://localhost:8000/api/generate ^
         -H "Content-Type: application/json" ^
         -d "{\"model\": \"qwen2:1.5b\", \"prompt\": \"Why is the sky blue?\", \"stream\":false}"

  .. code-block:: bash

  .. code-block:: bash
    :name: hmzga-usage-13
    :caption: Chatting with the model.

    curl --silent http://localhost:8000/api/chat \
         -H 'Content-Type: application/json' \
         -d '{"model": "qwen2:1.5b", "messages": [{"role": "user", "content": "Tell me a joke"}]}'

  **Windows (CMD)**: Use double quotes with escaping:

  .. code-block:: batch

    curl --silent http://localhost:8000/api/chat ^
         -H "Content-Type: application/json" ^
         -d "{\"model\": \"qwen2:1.5b\", \"messages\": [{\"role\": \"user\", \"content\": \"Tell me a joke\"}]}"

* List models loaded into memory:

  .. code-block:: bash
    :name: hmzga-usage-14
    :caption: Listing loaded models.

    curl --silent http://localhost:8000/api/ps

* Removes the model from local storage:

  .. code-block:: bash
    :name: hmzga-usage-15
    :caption: Deleting a model.

    curl --silent -X DELETE http://localhost:8000/api/delete \
         -H 'Content-Type: application/json' \
         -d '{"model": "qwen2:1.5b"}'

  **Windows (CMD)**: Use double quotes with escaping:

  .. code-block:: batch

    curl --silent -X DELETE http://localhost:8000/api/delete ^
         -H "Content-Type: application/json" ^
         -d "{\"model\": \"qwen2:1.5b\"}"
