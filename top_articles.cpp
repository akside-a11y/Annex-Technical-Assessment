/*
 * top_articles.cpp
 *
 * Retrieves and ranks articles from the HackerRank mock Articles API.
 *
 * Build (Linux/macOS):
 *   g++ -std=c++17 -O2 -o top_articles top_articles.cpp -lcurl
 *
 * Dependencies: libcurl  (sudo apt-get install libcurl4-openssl-dev)
 *
 * The program uses only the C++ standard library + libcurl for HTTP.
 * JSON is parsed with a lightweight hand-rolled approach so no third-party
 * JSON library is required.
 */

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <curl/curl.h>

/* ── libcurl write callback ─────────────────────────────────────────────── */
static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            std::string *out) {
    out->append(static_cast<char *>(contents), size * nmemb);
    return size * nmemb;
}

/* ── HTTP GET helper ────────────────────────────────────────────────────── */
static std::string httpGet(const std::string &url) {
    CURL *curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init failed");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error(std::string("curl error: ") +
                                 curl_easy_strerror(res));
    return response;
}

/* ── Minimal JSON helpers ───────────────────────────────────────────────── */

/*
 * Extract the integer value of a top-level key from a JSON object string.
 * Returns -1 if the key is not found.
 */
static long long extractInt(const std::string &json,
                            const std::string &key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return -1;
    pos += search.size();
    while (pos < json.size() &&
           (json[pos] == ' ' || json[pos] == ':')) ++pos;
    if (pos >= json.size()) return -1;
    long long val = 0;
    bool negative = false;
    if (json[pos] == '-') { negative = true; ++pos; }
    while (pos < json.size() && std::isdigit(json[pos]))
        val = val * 10 + (json[pos++] - '0');
    return negative ? -val : val;
}

/*
 * Extract a JSON string value for a given key.
 * Returns "" if the key is missing or its value is JSON null.
 */
static std::string extractString(const std::string &json,
                                 const std::string &key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    pos += search.size();
    while (pos < json.size() &&
           (json[pos] == ' ' || json[pos] == ':')) ++pos;
    if (pos >= json.size()) return "";
    /* Check for null */
    if (json.substr(pos, 4) == "null") return "";
    if (json[pos] != '"') return "";
    ++pos;  /* skip opening quote */
    std::string result;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            ++pos;  /* skip escape char — include next char verbatim */
        }
        result += json[pos++];
    }
    return result;
}

/* ── Article record ─────────────────────────────────────────────────────── */
struct Article {
    std::string name;
    long long   num_comments;
};

/*
 * Parse all article objects from the "data" array of one API page.
 * Each object is delimited by the top-level '{' '}' pairs inside the array.
 */
static std::vector<Article> parseDataArray(const std::string &json) {
    std::vector<Article> articles;

    /* Locate the "data" array */
    size_t start = json.find("\"data\"");
    if (start == std::string::npos) return articles;
    start = json.find('[', start);
    if (start == std::string::npos) return articles;
    size_t end = json.find_last_of(']');
    if (end == std::string::npos || end <= start) return articles;

    std::string data = json.substr(start + 1, end - start - 1);

    /* Walk through individual JSON objects */
    size_t pos = 0;
    while (pos < data.size()) {
        /* Find opening brace of next object */
        size_t obj_start = data.find('{', pos);
        if (obj_start == std::string::npos) break;

        /* Find matching closing brace (handle nesting) */
        int depth = 0;
        size_t obj_end = obj_start;
        for (size_t i = obj_start; i < data.size(); ++i) {
            if (data[i] == '{') ++depth;
            else if (data[i] == '}') { --depth; if (depth == 0) { obj_end = i; break; } }
        }

        std::string obj = data.substr(obj_start, obj_end - obj_start + 1);

        /* Determine article name per spec:
           1. title (if not null/empty)
           2. story_title (if not null/empty)
           3. skip                                                          */
        std::string name = extractString(obj, "title");
        if (name.empty()) name = extractString(obj, "story_title");

        if (!name.empty()) {
            long long nc = extractInt(obj, "num_comments");
            if (nc < 0) nc = 0;   /* treat null/missing as 0 */
            articles.push_back({name, nc});
        }

        pos = obj_end + 1;
    }
    return articles;
}

/* ── Main function ──────────────────────────────────────────────────────── */
std::vector<std::string> topArticles(int limit) {
    const std::string base_url =
        "https://jsonmock.hackerrank.com/api/articles?page=";

    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::vector<Article> all_articles;

    /* Fetch page 1 to discover total_pages */
    std::string page1 = httpGet(base_url + "1");
    long long total_pages = extractInt(page1, "total_pages");
    if (total_pages < 1) total_pages = 1;

    /* Parse page 1 */
    auto page1_articles = parseDataArray(page1);
    all_articles.insert(all_articles.end(),
                        page1_articles.begin(), page1_articles.end());

    /* Fetch remaining pages */
    for (long long p = 2; p <= total_pages; ++p) {
        std::string page = httpGet(base_url + std::to_string(p));
        auto arts = parseDataArray(page);
        all_articles.insert(all_articles.end(), arts.begin(), arts.end());
    }

    curl_global_cleanup();

    /* Sort:
       1. Decreasing num_comments
       2. Alphabetically decreasing name (tie-break)                        */
    std::stable_sort(all_articles.begin(), all_articles.end(),
                     [](const Article &a, const Article &b) {
                         if (a.num_comments != b.num_comments)
                             return a.num_comments > b.num_comments;
                         return a.name > b.name;
                     });

    /* Return top `limit` names */
    std::vector<std::string> result;
    int count = std::min(static_cast<int>(all_articles.size()), limit);
    for (int i = 0; i < count; ++i)
        result.push_back(all_articles[i].name);

    return result;
}

/* ── Driver ─────────────────────────────────────────────────────────────── */
int main() {
    int limit;
    std::cout << "Enter limit: ";
    std::cin >> limit;

    auto results = topArticles(limit);
    for (const auto &name : results)
        std::cout << name << "\n";

    return 0;
}
