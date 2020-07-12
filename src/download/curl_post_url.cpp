int hm_server_ad_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
	int block = size * nmemb;	
	FILE *fp = NULL;
	char *pTempName = "c:\\temp.json";
 
	if(stream == NULL)
		return -1;
	
	fp = fopen(pTempName,"ab+");
 
	
	if(fp)
	{
		fwrite(buffer, 1, block, fp);
		
		fflush(fp);
		fclose(fp); 
 
		if(pTempName)
		{
			free(pTempName);
			pTempName = NULL;
		}
		return block;
	}
	
	return 0;
}

bool postURL(char * url, char * fileName)
{
	int ret = -1;
	struct curl_slist *headers = NULL;
	CURL *hnd = NULL;
	char szUrl[1024] = {0};
	
	if (NULL == data || NULL == signature)
	{
		printf( "(NULL == pcIp || NULL == pcFile || NULL == pcApi), error");
		return -1;
	}
	
	hnd = curl_easy_init();
 
	if (NULL == hnd)
	{
		mtb_syn_debug(MTB_SYN_ERROR, "(NULL == hnd), error");
		return -1;
	}
	
	memset(szUrl,0x00,sizeof(szUrl));
	sprintf(szUrl, "http://%s/Adtest/Advertisement.ashx?type=%d&signature=%s", g_acHmServerAdDomain, type, signature);
	
	
	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(hnd, CURLOPT_URL, szUrl);
	
	headers = curl_slist_append(headers, "Postman-Token: af0fa846-ce85-44c2-a0e6-270f87cc2a06");
	headers = curl_slist_append(headers, "cache-control: no-cache");
 
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, data);
	
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, hm_server_ad_fwrite);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&type);
 
	curl_easy_setopt(hnd, CURLOPT_TIMEOUT, g_iHmServerAdCurlTimeout);
	curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT, g_iHmServerAdCurlTimeout);
 
	curl_easy_setopt(hnd, CURLOPT_NOSIGNAL, 1L);
 
	ret = curl_easy_perform(hnd);
 
 
	if(CURLE_OK != ret)
	{
		printf("error = %d---%s", ret, curl_easy_strerror(ret));
	}
 
	if(hnd)
		curl_easy_cleanup(hnd);
}
