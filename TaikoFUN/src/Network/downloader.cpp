#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>

size_t WriteCallback(void* content, size_t size, size_t nmenb, void* userp) {
    size_t totalSize = size * nmenb;
    std::ofstream* outStream = static_cast<std::ofstream*>(userp);

    outStream->write(static_cast<const char*>(content), totalSize);
    return totalSize;
}

bool DownloadFile(const std::string& url, const std::string& outputFilename) {
    //バイナリモードで出力ファイルを開く
    std::ofstream outputFile(outputFilename, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open output file: " << outputFilename << std::endl;
        return false;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize libcurl." << std::endl;
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outputFile);
    
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    CURLcode res = curl_easy_perform(curl);
    
    curl_easy_cleanup(curl);
    outputFile.close();

    if (res != CURLE_OK) {
        std::cerr << "Download failed: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    return true;
}

int main(void){
    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::string url = "https://www.gstatic.com/webp/gallery/4.sm.jpg";
    std::string filename = "sample.jpg";

    std::cout << "Downloading file..." << std::endl;
    if (DownloadFile(url, filename)) {
        std::cout << "Download completed successfully! Saved as: " << filename << std::endl;
    }

    curl_global_cleanup();
    return 0;
}