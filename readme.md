# libcurl/libssh2 project notes

## Requirements

- libssh2 has a hardcoded timeout when reading packets. Currently set to 60 seconds.
- Bloomberg would like to have this timeout configurable.
- libcurl uses libssh2 and will need to have this configuration passed on too.
- Generic requirement is to allow usage with FTP sites that take too long to respond to commands like ls (such as when a folder has 10s of thousands of files)

## Process

- libssh2 & libcurl projects to be forked to personal GitHub account (JonAxtell-CT)
- libssh2 to be updated
- libcurl to be updated
- libssh2 change to be commited and merged first
- only when libssh2 updated can libcurl be commited and merged since it depends on libssh2 changes being public
- When libssh2 is commited, it should be done in three steps. 1st the session struct and associated API functions, 2nd the updated functions that now use the timeout value, 3rd the documentation
- When libssh2 is commited, make reference to the fact that libcurl will be udpated to handle the new API since that is the program used to access the slow FTP sites.

## Points

- Timeout needs to be stored in an appropriate structure. Session structure seems to be best. Channel is an alternative. Channel is a subset of session
- Timeout is defined in LIBSSH2_READ_TIMEOUT macro
- LIBSSH2_READ_TIMEOUT is used in three places:

1. _libssh2_packet_require
1. _libssh2_packet_requirev
1. sftp_packet_requirev

- API functions in libssh2 to set/get the timeout proposed (assuming session structure used):

1. void libssh2_session_set_read_timeout(session, long)
1. long libssh2_session_get_read_timeout(session)

- API functions in libcurl/curl to set/get the timeout needed too. Will be passed on to libssh2. Options in libcurl set via curl_easy_setopt(). Propose a new setting CURLOPT_SSH_READ_TIMEOUT

- Timeout value in what units? Seconds or milliseconds (which matches libssh2_session_set_timeout)
- Will timeout value of zero be a special case? 0 timeout elsewhere being used for a blocking version.
- Is there a max limit of timeout?
- Does the sftp code need to be udpated or can it use the newly configured timeout. In otherwords would a SFTP only timeout value be needed.

- Curl configuration: ./configure --with-openssl --disable-shared --enable-debug --enable-maintainer-mode --with-libssh2
-

## Testing

- LIBSSH2_ERROR_TIMEOUT to returned when timeout occurs.
- Example situation that re-creates a timeout needed. How to force the timeout?
- Test passes when time before error reported matches the configured time which will be different from 60 seconds.

- Recompiling libssh2 is done by changing to the bin sub-folder and running make.

- vsftpd installed as a test server. /etc/vsftpd.conf required editing to ensure listen=YES and listen_ipv6=NO
- Also the following appended to the conf file
- Following are added from <https://www.digitalocean.com/community/tutorials/how-to-set-up-vsftpd-for-a-user-s-directory-on-ubuntu-20-04>
- user_sub_token=$USER
- local_root=/home/$USER/ftp
- pasv_min_port=40000
- pasv_max_port=50000
- userlist_enable=YES
- userlist_file=/etc/vsftpd.userlist
- userlist_deny=NO
- log_ftp_protocol=YES

- Build vsftpd from source to allow tweaking to inject delays etc.
- vsftpd required libpam and libcap. Installed with libpam0g-dev and libcap-dev
-

## Issues

Whem cmake ing got errors about openssl/libgcrypt not being found. Daniel did ```apt search libssh2``` then ```sudo apt build-dep libssh2-1``` then ```sudo apt install libssl-dev``` to fix
Had issues get a test program to work with the version that has the option. Would only use the shared library which was 1.9.0 and not the 1.10.1 dev version. Configuration option for curl was:
    ./configure --enable-debug --enable-maintainer-mode --with-libssh2=/usr/local --with-openssl
Compile command for the test program was:
    gcc ftpget.c -o ftpget -L/usr/local/lib /usr/local/lib/libssh2.a /usr/local/lib/libcurl.a -lssl -lcrypto -lpthread -lm -lz -lssh2

## Changes

- libssh2_session_set_read_timeout function added to session.c
- libssh2_session_get_read_timeout function added to session.c
- libssh2_session_set_read_timeout.3 added to docs
- libssh2_session_get_read_timeout.3 added to docs
- _libssh2_packet_requirev updated to use new function
- _libssh2_packet_require updated to use new function
- packet_read_timeout field added to _LIBSSH2_SESSION structure in libssh2_priv.h
- libssh2_session_init_ex updated to set default value of 60 (LIBSSH2_READ_TIMEOUT) to packet_read_timeout in _LIBSSH2_SESSION structure

- version in .3 files set to 1.10.1 to be next version after last found with git tag. Hopefully when pull request made correct version will be applied. Version held in libssh2.h
- sftp function also uses the timeout
-
