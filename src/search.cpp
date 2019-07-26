#include "search.h"
#include <curl/curl.h>

void subman::search::find() {
  auto curl = curl_easy_init();
  if (curl) {
    std::string url{"https://duckduckgo.com/html?q="};
    std::string data;
    url.append(keyword);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        [](void* contents, size_t size, size_t nmemb, void* userp) -> size_t {
          ((std::string*)userp)->append((char*)contents, size * nmemb);
          return size * nmemb;
        });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    auto res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }
}
