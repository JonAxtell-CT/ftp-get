gcc ftpget.c -o ftpget -L/usr/local/lib /usr/local/lib/libssh2.a /usr/local/lib/libcurl.a -lssl -lcrypto -lpthread -lm -lz -lssh2
