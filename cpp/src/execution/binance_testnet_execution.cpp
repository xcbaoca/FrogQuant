#include "execution/binance_testnet_execution.hpp"

#include <curl/curl.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <chrono>
#include <iomanip>
#include <optional>
#include <sstream>

namespace fq::execution {
namespace {

uint64_t now_ms() {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    const size_t total = size * nmemb;
    auto* out = static_cast<std::string*>(userp);
    out->append(static_cast<char*>(contents), total);
    return total;
}

} // namespace

BinanceTestnetExecution::BinanceTestnetExecution(ExecConfig cfg) : cfg_(std::move(cfg)) {}

ExecResult BinanceTestnetExecution::place_order(const std::string& symbol,
                                                fq::core::OrderSide side,
                                                fq::core::OrderType type,
                                                double qty,
                                                std::optional<double> price,
                                                std::optional<std::string> client_order_id) const {
    std::ostringstream q;
    q << "symbol=" << symbol
      << "&side=" << side_to_str(side)
      << "&type=" << type_to_str(type)
      << "&quantity=" << std::fixed << std::setprecision(6) << qty;

    if (type == fq::core::OrderType::Limit) {
        if (!price.has_value()) {
            return ExecResult{false, 0, "", "limit order requires price"};
        }
        q << "&timeInForce=GTC"
          << "&price=" << std::fixed << std::setprecision(2) << *price;
    }

    if (client_order_id.has_value() && !client_order_id->empty()) {
        q << "&newClientOrderId=" << *client_order_id;
    }

    q << "&timestamp=" << now_ms()
      << "&recvWindow=5000";

    return send_signed_request("POST", "/fapi/v1/order", q.str());
}

ExecResult BinanceTestnetExecution::cancel_order(const std::string& symbol, const std::string& order_id) const {
    std::ostringstream q;
    q << "symbol=" << symbol
      << "&orderId=" << order_id
      << "&timestamp=" << now_ms()
      << "&recvWindow=5000";

    return send_signed_request("DELETE", "/fapi/v1/order", q.str());
}

std::optional<std::string> BinanceTestnetExecution::parse_order_id_from_json(const std::string& body) {
    const auto id = extract_json_value(body, "orderId");
    if (id.empty()) return std::nullopt;
    return id;
}

std::string BinanceTestnetExecution::side_to_str(fq::core::OrderSide side) {
    return side == fq::core::OrderSide::Buy ? "BUY" : "SELL";
}

std::string BinanceTestnetExecution::type_to_str(fq::core::OrderType type) {
    return type == fq::core::OrderType::Limit ? "LIMIT" : "MARKET";
}

std::string BinanceTestnetExecution::extract_json_value(const std::string& json, const std::string& key) {
    const std::string token = "\"" + key + "\":";
    auto pos = json.find(token);
    if (pos == std::string::npos) return {};
    pos += token.size();

    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\n')) ++pos;

    if (pos < json.size() && json[pos] == '"') {
        ++pos;
        const auto end = json.find('"', pos);
        if (end == std::string::npos) return {};
        return json.substr(pos, end - pos);
    }

    const auto end = json.find_first_of(",}", pos);
    if (end == std::string::npos) return {};
    return json.substr(pos, end - pos);
}

std::string BinanceTestnetExecution::sign_query_hmac_sha256(const std::string& query) const {
    unsigned int len = 0;
    unsigned char hash[EVP_MAX_MD_SIZE]{};

    HMAC(EVP_sha256(),
         cfg_.secret_key.data(), static_cast<int>(cfg_.secret_key.size()),
         reinterpret_cast<const unsigned char*>(query.data()), static_cast<int>(query.size()),
         hash, &len);

    std::ostringstream oss;
    for (unsigned int i = 0; i < len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return oss.str();
}

ExecResult BinanceTestnetExecution::send_signed_request(const std::string& method,
                                                        const std::string& path,
                                                        const std::string& query) const {
    if (cfg_.api_key.empty() || cfg_.secret_key.empty()) {
        return ExecResult{false, 0, "", "api credentials are empty"};
    }

    const std::string signature = sign_query_hmac_sha256(query);
    const std::string body = query + "&signature=" + signature;
    const std::string url = cfg_.base_url + path;

    if (cfg_.dry_run) {
        return ExecResult{true, 200, std::string{"{\"dryRun\":true,\"url\":\""} + url + "\"}", ""};
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        return ExecResult{false, 0, "", "curl init failed"};
    }

    std::string resp_body;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("X-MBX-APIKEY: " + cfg_.api_key).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000L);

    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    } else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }

    const CURLcode code = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (code != CURLE_OK) {
        return ExecResult{false, http_code, resp_body, curl_easy_strerror(code)};
    }

    return ExecResult{http_code >= 200 && http_code < 300, http_code, resp_body, ""};
}

} // namespace fq::execution
