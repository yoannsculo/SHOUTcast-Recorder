#include <curl/curl.h>
#include <curl/easy.h>

#include "types.h"
#include "parsing.h"

int read_stream(Stream *stream)
{
	CURL *curl;
    CURLcode res;

    if (stream->url == NULL) {
        return 1;
    }
    
    curl = curl_easy_init();
    if (curl == NULL) {
        return 1;
    }
    
	struct curl_slist *headers = NULL;
   	headers = curl_slist_append(headers, "Icy-MetaData:1"); // On force la récupération des metadata
	
    curl_easy_setopt(curl, CURLOPT_URL, stream->url);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parse_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);
	
    res = curl_easy_perform(curl);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
    return 0;
}
