/***************************************************************************
 *                                  _   _ ____  _
 * Copyright (C) CodeThink
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * Get a single file from an FTP server.
 *
 * This is a noddy little program to test the feature in LibSSH2 and LibCurl
 * whereby the timeout on reading packets can be configured.
 * The test should be performed against an FTP server which can limit the
 * bandwidth or where delays can be introduced to trigger the timeout.
 * vsftpd has been used with the local_max_rate setting set to an
 * artificially low value.
 *
 ***************************************************************************/
#include <stdio.h>
#include <math.h>
#include <curl/curl.h>
#include <libssh2.h>

struct FtpFile
{
    const char *filename;
    FILE *stream;
};

static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
    if ((stream == NULL) || (buffer == NULL))
    {
        return 0;
    }
    struct FtpFile *out = (struct FtpFile *)stream;
    if (out->stream == NULL)
    {
        // Open file and save FILE to structure for future reference when writing
        out->stream = fopen(out->filename, "wb");
        if (!out->stream)
        {
            return -1;
        }
    }
    return fwrite(buffer, size, nmemb, out->stream);
}

static int my_ssh_hostkeyfunction(void *clientp,   /* custom pointer passed with CURLOPT_SSH_HOSTKEYDATA */
                                  int keytype,     /* CURLKHTYPE */
                                  const char *key, /* hostkey to check */
                                  size_t keylen)   /* length of the key */
{
    return CURLKHMATCH_OK;
}

int main(void)
{
    struct FtpFile ftpfile =
        {
            "test.txt",
            NULL};

    printf("LibSSH2 version=%s\n", libssh2_version(0));

    curl_global_init(CURL_GLOBAL_DEFAULT);

    CURL *curl = curl_easy_init();
    if (curl != NULL)
    {
        CURLcode res;

        printf("Curl: %s\n", curl_version());

        curl_version_info_data *o = curl_version_info(CURLVERSION_NOW);
        printf("SSL: %s\n", o->ssl_version);
        printf("NTLM: %d\n", (o->features & CURL_VERSION_NTLM) ? 1 : 0);
        printf("HTTP/2: %d\n", (o->features & CURL_VERSION_HTTP2) ? 1 : 0);
        printf("SSH2: %s\n", o->libssh_version);
        printf("IPv6: %d\n", (o->features & CURL_VERSION_IPV6) ? 1 : 0);

        // File to get
        res = curl_easy_setopt(curl, CURLOPT_URL, "ftp://127.0.0.1:21/files/test.txt");
        if (res != CURLE_OK)
        {
            fprintf(stderr, "could not set option CURLOPT_URL\n");
        }

        res = curl_easy_setopt(curl, CURLOPT_USERNAME, "ftp-user");
        if (res != CURLE_OK)
        {
            fprintf(stderr, "could not set option CURLOPT_USERNAME\n");
        }
        res = curl_easy_setopt(curl, CURLOPT_PASSWORD, "ftp-user");
        if (res != CURLE_OK)
        {
            fprintf(stderr, "could not set option CURLOPT_PASSWORD\n");
        }

        // Define our callback to get called when there's data to be written
        res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "could not set option CURLOPT_WRITEFUNCTION\n");
        }

        // Set a pointer to our struct to pass to the callback
        res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "could not set option CURLOPT_WRITEDATA\n");
        }

        // Switch on full protocol/debug output
        res = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "could not set option CURLOPT_VERBOSE\n");
        }

        res = curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PUBLICKEY | CURLSSH_AUTH_PASSWORD | CURLSSH_AUTH_HOST | CURLSSH_AUTH_KEYBOARD | CURLSSH_AUTH_AGENT);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "could not set option CURLOPT_SSH_AUTH_TYPES %d\n", res);
        }

        //
        res = curl_easy_setopt(curl, CURLOPT_SSH_HOSTKEYFUNCTION, my_ssh_hostkeyfunction);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "could not set option CURLOPT_SSH_HOSTKEYFUNCTION %d\n", res);
        }

        // Set packet read timeout
        res = curl_easy_setopt(curl, CURLOPT_SSH_READ_TIMEOUT, 60L);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "could not set option CURLOPT_SSH_READ_TIMEOUT %d\n", res);
        }

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl told us %d\n", res);
        }

        curl_easy_cleanup(curl);
    }

    if (ftpfile.stream)
    {
        fclose(ftpfile.stream);
    }

    curl_global_cleanup();
    return 0;
}
