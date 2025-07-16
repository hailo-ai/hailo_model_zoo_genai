Usage
=====

Prerequisites
-------------

* Hailo-10H module.
* Ensure  `HailoRT <https://github.com/hailo-ai/hailort>`__ version **HailoRT v5.0** or above is installed (for non-docker installations).


Installation
------------

1. **Option A - Debian package** *(Recommended)*

  * Download the latest Debian package from the `Developer Zone <https://hailo.ai/developer-zone/>`__.

  * Install it:

    .. code-block::

      sudo dpkg -i hailo_model_zoo_gen_ai_<ver>_<arch>.deb

2. **Option B - Docker** *(Zero-install host)*

  * Download the latest pre-built HailoRT image from the `Developer Zone <https://hailo.ai/developer-zone/>`__. The image contains the HailoRT and the Hailo-Ollama server, and is ready to run.

  * Unzip, load it into Docker and run it:

    .. code-block::

      unzip hailort_<ver>_docker.zip
      docker load -i hailo_docker_hailort_ub2204_<ver>.tar
      docker run -it --rm --device /dev/hailo0 \
          -p 8000:8000 \
          -v $HOME/.local/share/hailo-ollama/models/blob:/usr/share/hailo-ollama/models/blob \
          hailo_docker_hailort_ub2204:<ver>

  * The Hailo-Ollama server will be available at http://localhost:8000.


3. **Option C - Install from source** *(Advanced)*

  * Clone the Hailo Model Zoo GenAI repository and build the Hailo-Ollama server:

    .. code-block::

      git clone https://github.com/hailo-ai/hailo_model_zoo_gen_ai.git
      cd hailo-model-zoo-genai/
      mkdir build && cd build
      cmake -DCMAKE_BUILD_TYPE=Release ..
      cmake --build .

  * Install to user home (run still in the ``build`` dir):

    .. code-block::

      cp ./src/apps/server/hailo-ollama ~/.local/bin/
      mkdir -p ~/.config/hailo-ollama/
      cp ../config/hailo-ollama.json ~/.config/hailo-ollama/
      mkdir -p ~/.local/share/hailo-ollama
      cp -r ../models/ ~/.local/share/hailo-ollama


Hailo-Ollama REST API
---------------------

The Hailo-Ollama API provides a simple method to run GenAI models on Hailo devices through REST API. The API supports a single simultaneous connection with one pending request and it is compatible with the `Ollama <https://github.com/ollama/ollama>`__ REST API, so it can be used with existing tools like LangChain, Open-WebUI, etc.

The Hailo-Ollama API is currently limited to Large Language Models (LLMs) and it cannot be used for other tasks such as text-to-image generation or Vision Language Models (VLMs).

Additionally, the API does not support LoRA adapters and the Ollama CLI.


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


Using Hailo-Ollama
^^^^^^^^^^^^^^^^^^

* **Tip**: install ``jq`` (``sudo apt install jq``) for nicer JSON output.

* **Note**: Many endpoints (``/api/pull``, ``/api/chat``, ``/api/generate``) honor a ``"stream"`` Boolean in the JSON body.
    * ``true``  (default) - Server-Sent incremental chunks.
    * ``false`` - single JSON response.

* Start the Hailo-Ollama server (default: http://localhost:8000):

  .. code-block::

    hailo-ollama

* Shows the server version:

  .. code-block::

    curl --silent http://localhost:8000/api/version

* Get a list of all available models for download:

  .. code-block::

    curl --silent http://localhost:8000/hailo/v1/list

* Pull a specific model:

  .. code-block::

    curl --silent http://localhost:8000/api/pull \
         -H 'Content-Type: application/json' \
         -d '{ "model": "qwen2:1.5b", "stream" : true }'

* Run the model:

  .. code-block::

    curl --silent http://localhost:8000/api/generate \
         -H 'Content-Type: application/json' \
         -d '{"model": "qwen2:1.5b", "prompt": "Why is the sky blue?", "stream":false}'

  .. code-block::

    curl --silent http://localhost:8000/api/chat \
         -H 'Content-Type: application/json' \
         -d '{"model": "qwen2:1.5b", "messages": [{"role": "user", "content": "Translate to French: The cat is on the table."}]}'

* List models loaded into memory:

  .. code-block::

    curl --silent http://localhost:8000/api/ps

* Removes the model from local storage:

  .. code-block::

    curl --silent -X DELETE http://localhost:8000/api/delete \
         -H 'Content-Type: application/json' \
         -d '{"model": "qwen2:1.5b"}'
