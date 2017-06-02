#include <curl/curl.h>
#include <curl/easy.h>

#include "types.h"
#include "parsing.h"

int read_stream(Stream *stream)
{
	CURL *curl;
	CURLcode curl_res;
	struct curl_slist *headers = NULL;
	int ret = 0;

	if (stream->url == NULL) {
		ret = -1;
		goto early_err;
	}

	if ((curl = curl_easy_init()) == NULL) {
		ret = -1;
		goto early_err;
	}

	headers = curl_slist_append(headers, "Icy-MetaData:1"); // On force la récupération des metadata

	curl_easy_setopt(curl, CURLOPT_URL, stream->url);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_PROXY, stream->proxy);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, stream->duration);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, parse_header);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, stream);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parse_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);

	if ((curl_res = curl_easy_perform(curl)) != 0) {
		printf("ERROR : %s\n", curl_easy_strerror(curl_res));
		ret = -1;
		goto err;
	}

err:
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
early_err:
	return ret;
}
