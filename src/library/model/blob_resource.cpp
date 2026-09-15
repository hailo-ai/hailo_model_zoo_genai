/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file blob_resource.cpp
 * @brief BlobResourceProvider implementation
 **/

#include "model/blob_resource.hpp"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>

#include <oatpp-openssl/Config.hpp>
#include <oatpp-openssl/client/ConnectionProvider.hpp>
#include <oatpp/base/Log.hpp>
#include <oatpp/data/stream/FileStream.hpp>
#include <oatpp/json/ObjectMapper.hpp>
#include <oatpp/network/tcp/client/ConnectionProvider.hpp>
#include <oatpp/web/client/ApiClient.hpp>
#include <oatpp/web/client/HttpRequestExecutor.hpp>
#include <oatpp/web/protocol/http/incoming/Response.hpp>

#include "config/static_config.hpp"
#include "controller/pull_callback.hpp"
#include "controller/writefile_callback.hpp"
#include "oatpp/Types.hpp"
#include "utils/sha256.hpp"

namespace fs = std::filesystem;

namespace hailo_ollama
{

namespace {

constexpr int STATUS_OK = 200;
constexpr int STATUS_PARTIAL_CONTENT = 206;
constexpr int STATUS_NOT_FOUND = 404;
constexpr int64_t UNKNOWN_SIZE = -1;
constexpr uintmax_t MAX_CACHED_VERSION_FILE_SIZE = 64;

const std::string EMPTY_DIGEST = "";
const std::string SUCCESS_MSG = "success";
const std::string LOG_TAG = "BlobResourceProvider";
const std::string HEF_FILENAME_SUFFIX = ".hef";
constexpr const char *WHITESPACE_CHARS = " \t\r\n";

std::string trim_whitespace(const std::string &text)
{
    const auto first = text.find_first_not_of(WHITESPACE_CHARS);
    if (std::string::npos == first) {
        return "";
    }
    const auto last = text.find_last_not_of(WHITESPACE_CHARS);
    return text.substr(first, (last - first) + 1);
}

// A failbit is not always accompanied by a failing syscall, and errno == 0 renders as the misleading "Success".
std::string parenthesized_errno_detail()
{
    if (0 == errno) {
        return "";
    }
    return " (" + std::error_code(errno, std::generic_category()).message() + ")";
}

bool blob_exists(int status_code)
{
    return ((STATUS_OK == status_code) || (STATUS_PARTIAL_CONTENT == status_code));
}

bool is_blob_present(const BlobStatusProbe &probe, const std::string &path)
{
    const auto status = probe(path);
    if (!status.has_value()) {
        throw std::runtime_error("network error probing " + path);
    }
    if (blob_exists(*status)) {
        return true;
    }
    if (STATUS_NOT_FOUND != *status) {
        OATPP_LOGw(LOG_TAG.c_str(), "unexpected HTTP {} probing {}, treating as absent", *status, path);
    }
    return false;
}

bool is_version_complete(const BlobStatusProbe &probe, const std::string &version, const std::string &hef_filename)
{
    const auto hef_path = detail::blob_url_path(version, hef_filename);
    const auto companion_path = detail::sha256_companion_url_path(version, hef_filename);
    return (is_blob_present(probe, hef_path) && is_blob_present(probe, companion_path));
}

bool is_lowercase_hex_64(const std::string &s)
{
    return ((s.size() == 64) && std::all_of(s.begin(), s.end(), [](unsigned char c) {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
    }));
}

struct Version {
    uint32_t major;
    uint32_t minor;
    uint32_t revision;
};

Version parse_version(const std::string &version_string)
{
    Version version{};
    char dot1 = 0;
    char dot2 = 0;
    std::istringstream stream(version_string);
    if (!(stream >> version.major >> dot1 >> version.minor >> dot2 >> version.revision) ||
        (dot1 != '.') || (dot2 != '.')) {
        throw std::runtime_error("invalid version: '" + version_string + "'");
    }
    // Reject trailing garbage (e.g. "5.4.0.1", "5.4.0-rc1"). Only trailing whitespace is allowed.
    stream >> std::ws;
    if (!stream.eof()) {
        throw std::runtime_error("invalid version: '" + version_string + "'");
    }
    return version;
}

std::string version_to_string(uint32_t major, uint32_t minor, uint32_t revision)
{
    return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(revision);
}

void notify_client_about_fallback(const std::string &requested_version, const std::string &resolved_version,
    const std::shared_ptr<PullReadCallback::EventQueue> &queue)
{
    if (queue == nullptr) {
        return;
    }
    queue->enqueue(PullEvent::PROGRESS,
        "warning: could not get the requested version (" + requested_version + "). falling back to v" +
        resolved_version, EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
}

void notify_client_about_cache_hit(const std::string &resolved_version,
    const std::shared_ptr<PullReadCallback::EventQueue> &queue)
{
    if (queue == nullptr) {
        return;
    }
    queue->enqueue(PullEvent::PROGRESS, "resolved_version: " + resolved_version, EMPTY_DIGEST,
        UNKNOWN_SIZE, UNKNOWN_SIZE);
    queue->enqueue(PullEvent::PROGRESS, SUCCESS_MSG, EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
    queue->enqueue(PullEvent::DONE, EMPTY_DIGEST, EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
}

bool try_serve_cached_version(const fs::path &cache_path, const std::string &version,
    const std::shared_ptr<PullReadCallback::EventQueue> &queue)
{
    if (!detail::is_hef_cached_at_version(cache_path, version)) {
        return false;
    }
    notify_client_about_cache_hit(version, queue);
    return true;
}

void ensure_cache_dir_exists(const fs::path &cache_path)
{
    std::error_code error_code;
    fs::create_directories(cache_path.parent_path(), error_code);
    if (error_code) {
        throw std::runtime_error("failed to create cache dir '" + cache_path.parent_path().string() + "': " +
            error_code.message());
    }
}

bool remove_cached_hef(const fs::path &cache_path)
{
    std::error_code error_code;
    if (!fs::is_regular_file(cache_path, error_code)) {
        return false;
    }
    const auto was_removed = fs::remove(cache_path, error_code);
    if (error_code) {
        OATPP_LOGw(LOG_TAG.c_str(), "failed to remove '{}': {}", cache_path.string(), error_code.message());
    }
    return was_removed;
}

// Removes the guarded file on destruction (best-effort). After a successful rename the path no longer
// exists, so removal is a no-op.
class ScopedFileRemover {
public:
    explicit ScopedFileRemover(fs::path path) : m_path(std::move(path)) {}
    ~ScopedFileRemover()
    {
        std::error_code error_code;
        fs::remove(m_path, error_code);
    }

    ScopedFileRemover(const ScopedFileRemover &) = delete;
    ScopedFileRemover &operator=(const ScopedFileRemover &) = delete;

private:
    fs::path m_path;
};

void write_response_body_to_file(const HttpResponse &response, const std::string &temp_path,
    const std::string &expected_hash, const std::shared_ptr<PullReadCallback::EventQueue> &queue)
{
    const bool is_streaming = (queue != nullptr);
    if (is_streaming) {
        response->transferBody(std::make_shared<OutputFileStream>(temp_path.c_str(), expected_hash, queue,
            response->getHeader("Content-Length").getValue(std::to_string(UNKNOWN_SIZE))));
        return;
    }
    auto output_stream = oatpp::data::stream::FileOutputStream(temp_path.c_str(), "wb");
    response->transferBodyToStream(&output_stream);
}

void finalize_hef_download(const std::string &temp_path, const fs::path &cache_path, const std::string &expected_hash,
    const std::string &resolved_version, const std::string &hef_filename,
    const std::shared_ptr<PullReadCallback::EventQueue> &queue)
{
    const bool is_streaming = (queue != nullptr);
    if (is_streaming) {
        queue->enqueue(PullEvent::PROGRESS, "verifying sha256 digest", EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
    }
    std::ifstream downloaded(temp_path, std::ifstream::in | std::ifstream::binary);
    if (SHA256Hasher::hash(downloaded) != expected_hash) {
        throw std::runtime_error("hash verification failed for " + hef_filename + " at v" + resolved_version);
    }
    downloaded.close();

    detail::commit_downloaded_hef(temp_path, cache_path, resolved_version);

    if (is_streaming) {
        queue->enqueue(PullEvent::PROGRESS, "resolved_version: " + resolved_version, EMPTY_DIGEST, UNKNOWN_SIZE,
            UNKNOWN_SIZE);
        queue->enqueue(PullEvent::PROGRESS, SUCCESS_MSG, EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
        queue->enqueue(PullEvent::DONE, EMPTY_DIGEST, EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
    }
}

} // namespace

namespace detail {

std::string parse_sha256_companion_file_content(const std::string &body)
{
    // sha256sum format: "<64 hex>  <filename>\n". Tolerate extra leading/trailing whitespace.
    std::istringstream stream(body);
    std::string token;
    if (!(stream >> token)) {
        throw std::runtime_error("companion file is empty");
    }
    if (!is_lowercase_hex_64(token)) {
        throw std::runtime_error("companion file does not contain a 64-char lowercase hex sha256 digest");
    }
    return token;
}

std::string strip_suffix(const std::string &str, const std::string &suffix)
{
    if ((str.size() <= suffix.size()) ||
        (str.compare(str.size() - suffix.size(), suffix.size(), suffix) != 0)) {
        throw std::runtime_error("expected '" + str + "' to end with '" + suffix + "'");
    }
    return str.substr(0, str.size() - suffix.size());
}

std::string blob_url_path(const std::string &version, const std::string &filename)
{
    return "/v" + version + "/blob/" + filename;
}

std::string sha256_companion_url_path(const std::string &version, const std::string &hef_filename)
{
    return blob_url_path(version, strip_suffix(hef_filename, HEF_FILENAME_SUFFIX) +
        config::HEF_SHA256_COMPANION_FILE_SUFFIX);
}

std::string compute_fallback_version(const std::string &requested_version)
{
    const auto parts = parse_version(requested_version);
    if (parts.revision != 0) {
        return version_to_string(parts.major, parts.minor, 0);
    }
    if (parts.minor == 0) {
        throw std::runtime_error("no fallback exists for v" + requested_version +
            " (minor==0; cross-major fallback is out of scope)");
    }
    return version_to_string(parts.major, parts.minor - 1, 0);
}

std::string resolve_complete_version(const std::string &requested_version, const std::string &hef_filename,
    const BlobStatusProbe &probe, const std::shared_ptr<PullReadCallback::EventQueue> &queue)
{
    if (is_version_complete(probe, requested_version, hef_filename)) {
        return requested_version;
    }

    const auto fallback_version = compute_fallback_version(requested_version);
    if (!is_version_complete(probe, fallback_version, hef_filename)) {
        throw std::runtime_error("no complete blob pair (HEF + " + config::HEF_SHA256_COMPANION_FILE_SUFFIX +
            " companion) for " + hef_filename + " at v" + requested_version + " or fallback v" + fallback_version);
    }

    OATPP_LOGw(LOG_TAG.c_str(), "falling back from v{} to v{} for {}", requested_version, fallback_version,
        hef_filename);
    notify_client_about_fallback(requested_version, fallback_version, queue);

    return fallback_version;
}

fs::path cached_version_file_path(const fs::path &cached_hef_path)
{
    fs::path version_file_path = cached_hef_path;
    version_file_path += config::CACHED_HEF_VERSION_FILE_SUFFIX;
    return version_file_path;
}

std::optional<std::string> read_cached_version(const fs::path &cached_hef_path)
{
    const auto version_file_path = cached_version_file_path(cached_hef_path);
    std::error_code error_code;
    if (!fs::is_regular_file(version_file_path, error_code)) {
        return std::nullopt;
    }
    const auto file_size = fs::file_size(version_file_path, error_code);
    if (error_code || (MAX_CACHED_VERSION_FILE_SIZE < file_size)) {
        return std::nullopt;
    }
    std::ifstream version_file(version_file_path, std::ifstream::in | std::ifstream::binary);
    if (!version_file) {
        return std::nullopt;
    }
    const std::string content{std::istreambuf_iterator<char>(version_file), std::istreambuf_iterator<char>()};
    if (version_file.bad()) {
        return std::nullopt;
    }
    return trim_whitespace(content);
}

bool is_hef_cached_at_version(const fs::path &cached_hef_path, const std::string &resolved_version)
{
    if (!fs::is_regular_file(cached_hef_path)) {
        return false;
    }
    const auto cached_version = read_cached_version(cached_hef_path);
    return (cached_version.has_value() && (*cached_version == resolved_version));
}

void write_cached_version(const fs::path &cached_hef_path, const std::string &resolved_version)
{
    const auto version_file_path = cached_version_file_path(cached_hef_path);
    errno = 0; // iostreams expose no std::error_code, so errno is the only failure detail available
    std::ofstream version_file(version_file_path, std::ofstream::out | std::ofstream::trunc);
    version_file << resolved_version << "\n";
    version_file.flush();
    if (version_file.fail()) {
        OATPP_LOGw(LOG_TAG.c_str(), "failed to record v{} in '{}'{}; the HEF is committed and usable, but the "
            "next pull will re-download it", resolved_version, version_file_path.string(),
            parenthesized_errno_detail());
    }
}

void remove_cached_version_file(const fs::path &cached_hef_path)
{
    const auto version_file_path = cached_version_file_path(cached_hef_path);
    std::error_code error_code;
    fs::remove(version_file_path, error_code);
    if (error_code) {
        throw std::runtime_error("failed to remove '" + version_file_path.string() + "': " + error_code.message() +
            "; remove it manually to allow this model to be re-downloaded");
    }
}

void commit_downloaded_hef(const std::string &temp_path, const fs::path &cache_path,
    const std::string &resolved_version)
{
    // The marker must not outlive the bytes it describes: dropping it before the rename means every crash
    // window reads as an unknown version and re-downloads, never as a stale claim over the new bytes.
    remove_cached_version_file(cache_path);

    std::error_code error_code;
    fs::rename(temp_path, cache_path, error_code);
    if (error_code) {
        throw std::runtime_error("failed to commit '" + temp_path + "' -> '" + cache_path.string() + "': " +
            error_code.message());
    }

    write_cached_version(cache_path, resolved_version);
}

} // namespace detail

BlobResourceProvider::BlobResourceProvider(fs::path blob_dir, const std::string &base_url, uint16_t port,
    BlobStatusProbe probe)
    : m_blob_dir(std::move(blob_dir)), m_base_url(base_url), m_port(port),
      m_probe(probe ? std::move(probe) : [this](const std::string &path) { return probe_blob_status(path); })
{
}

static HttpsClient make_https_client(const std::string &host, uint16_t port)
{
    auto config = oatpp::openssl::Config::createDefaultClientConfigShared();
    auto connection_provider = oatpp::openssl::client::ConnectionProvider::createShared(config, {host, port});
    auto request_executor = oatpp::web::client::HttpRequestExecutor::createShared(connection_provider);
    auto object_mapper = std::make_shared<oatpp::json::ObjectMapper>();
    return oatpp::web::client::ApiClient::createShared(request_executor, object_mapper);
}

static std::string to_oatpp_relative_path(const std::string &path)
{
    if (!path.empty() && (path.front() == '/')) {
        return path.substr(1);
    }
    return path;
}

HttpResponse BlobResourceProvider::request_get(const std::string &path)
{
    auto client = make_https_client(m_base_url, m_port);
    using oatpp::web::client::ApiClient;
    const ApiClient::Headers headers;
    const auto wire_path = to_oatpp_relative_path(path);
    const ApiClient::StringTemplate path_template(wire_path.c_str(), {});
    const std::unordered_map<oatpp::String, oatpp::String> path_params;
    const std::unordered_map<oatpp::String, oatpp::String> query_params;
    return client->executeRequest("GET", path_template, headers, path_params, query_params, nullptr);
}

std::optional<int> BlobResourceProvider::probe_blob_status(const std::string &path)
{
    // Range-GET (1 byte) is used instead of HEAD because some CDN edge configs mishandle
    // HEAD redirects; range-GET is what curl-style existence checks rely on in the wild.
    auto client = make_https_client(m_base_url, m_port);
    using oatpp::web::client::ApiClient;
    ApiClient::Headers headers;
    headers.put("Range", "bytes=0-0");
    const auto wire_path = to_oatpp_relative_path(path);
    const ApiClient::StringTemplate path_template(wire_path.c_str(), {});
    const std::unordered_map<oatpp::String, oatpp::String> path_params;
    const std::unordered_map<oatpp::String, oatpp::String> query_params;
    auto response = client->executeRequest("GET", path_template, headers, path_params, query_params, nullptr);
    if (!response) {
        return std::nullopt;
    }
    return response->getStatusCode();
}

std::string BlobResourceProvider::fetch_hef(const std::string &hef_filename,
    const std::shared_ptr<PullReadCallback::EventQueue> &queue)
{
    const std::string requested_version = HAILO_OLLAMA_VERSION;
    const auto cache_path = m_blob_dir / hef_filename;

    // Checked before resolving so a pull of the requested version keeps working with no network reachable.
    if (try_serve_cached_version(cache_path, requested_version, queue)) {
        return requested_version;
    }

    const auto resolved_version = detail::resolve_complete_version(requested_version, hef_filename, m_probe, queue);
    if (try_serve_cached_version(cache_path, resolved_version, queue)) {
        return resolved_version;
    }

    const auto expected_hash = fetch_expected_hash(resolved_version, hef_filename);
    ensure_cache_dir_exists(cache_path);
    // Unique per-attempt suffix so concurrent pulls of the same HEF each own their in-flight temp file;
    // the final fs::rename to cache_path still overwrites atomically (last-writer-wins).
    static std::atomic<uint64_t> temp_counter{0};
    const auto temp_path = cache_path.string() + "." + std::to_string(temp_counter++) + ".tmp";
    const ScopedFileRemover temp_file_remover(temp_path);
    download_hef_to_path(resolved_version, hef_filename, temp_path, expected_hash, queue);
    finalize_hef_download(temp_path, cache_path, expected_hash, resolved_version, hef_filename, queue);
    return resolved_version;
}

std::string BlobResourceProvider::fetch_expected_hash(const std::string &resolved_version,
    const std::string &hef_filename)
{
    const auto companion_path = detail::sha256_companion_url_path(resolved_version, hef_filename);
    auto companion_response = request_get(companion_path);
    if ((!companion_response) || (companion_response->getStatusCode() != STATUS_OK)) {
        const auto code = companion_response ? companion_response->getStatusCode() : 0;
        throw std::runtime_error("failed to fetch companion " + companion_path + ": HTTP " + std::to_string(code));
    }
    const auto companion_body = companion_response->readBodyToString();
    return detail::parse_sha256_companion_file_content(companion_body);
}

void BlobResourceProvider::download_hef_to_path(const std::string &resolved_version, const std::string &hef_filename,
    const std::string &temp_path, const std::string &expected_hash,
    const std::shared_ptr<PullReadCallback::EventQueue> &queue)
{
    const auto hef_path = detail::blob_url_path(resolved_version, hef_filename);
    auto response = request_get(hef_path);
    if ((!response) || (response->getStatusCode() != STATUS_OK)) {
        const auto code = response ? response->getStatusCode() : 0;
        throw std::runtime_error("failed to download HEF v" + resolved_version + "/" + hef_filename +
            ": HTTP " + std::to_string(code));
    }

    write_response_body_to_file(response, temp_path, expected_hash, queue);
}

std::string BlobResourceProvider::pull_resource(const std::string &hef_filename)
{
    return fetch_hef(hef_filename, nullptr);
}

std::string BlobResourceProvider::pull_resource(const std::string &hef_filename,
    const std::shared_ptr<PullReadCallback::EventQueue> &queue)
{
    try {
        return fetch_hef(hef_filename, queue);
    } catch (const std::exception &e) {
        queue->enqueue(PullEvent::PULL_ERROR, e.what(), EMPTY_DIGEST, UNKNOWN_SIZE, UNKNOWN_SIZE);
        return "";
    }
}

fs::path BlobResourceProvider::get_resource(const std::string &hef_filename)
{
    const auto path = m_blob_dir / hef_filename;
    if (fs::is_regular_file(path)) {
        return path;
    }
    return {};
}

bool BlobResourceProvider::remove_resource(const std::string &hef_filename)
{
    const auto cache_path = m_blob_dir / hef_filename;
    const auto was_hef_removed = remove_cached_hef(cache_path);
    try {
        detail::remove_cached_version_file(cache_path);
    } catch (const std::exception &e) {
        // An orphan marker cannot fake a cache hit, so it must not turn a successful delete into a failure.
        OATPP_LOGw(LOG_TAG.c_str(), "{}", e.what());
    }
    return was_hef_removed;
}

} // namespace hailo_ollama
