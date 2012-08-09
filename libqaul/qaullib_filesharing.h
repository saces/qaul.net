/*
 * qaul.net is free software
 * licensed under GPL (version 3)
 */

#ifndef _QAULLIB_FILESHARING
#define _QAULLIB_FILESHARING

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * defines chunk size of file sharing
 */
int qaul_chunksize;

/**
 * structure of available connections for downloading files
 */
struct qaul_file_connection
{
	struct qaul_wget_connection conn;
	struct qaul_file_LL_item *fileinfo;
	unsigned int chunksize;
	FILE *file;
};

/**
 * array of TCP connections for file downloading
 */
struct qaul_file_connection fileconnections[MAX_FILE_CONNECTIONS];

/**
 * Initialize file table and read files from data base.
 * Called once in qaullib_init.
 */
void Qaullib_FileInit(void);

/**
 * add existing files to DB
 * called once after installation
 *
 * fat clients contain all executables for all OSs
 * slim clients only contain information about the other versions
 * and need to download the executables from other users.
 */
void Qaullib_FilePopulate(void);

/**
 * add a new file to data base and LL
 *
 * @retval 0 on error
 * @retval 1 on success
 */
int Qaullib_FileAdd(struct qaul_file_LL_item *file_item);

/**
 * add file from @a path to filesharing and analyzes the file.
 * It creates the @a hashstr and fills in the @a suffix
 *
 * @retval 0 on error
 * @retval filesize in Bytes on success
 */
int Qaullib_FileCopyNew(char *path, char *hashstr, char *suffix);

/**
 * check if any file needs to be downloaded
 */
void Qaullib_FileCheckScheduled(void);

/**
 * flood discovery message via IPC over olsr
 */
void Qaullib_FileSendDiscoveryMsg(struct qaul_file_LL_item *file_item);

/**
 * check if file is downloading, unschedule it
 */
void Qaullib_FileStopDownload(struct qaul_file_LL_item *file_item);

/**
 * create the @a filepath out of the @a hash string and the @a suffix
 */
void Qaullib_FileCreatePath(char *filepath, char *hash, char *suffix);

/**
 * checks if file with @a hash is available from the bytes position @a startbyte
 *
 * @retval 1 file is available
 * @retval 0 file is not available from this position
 */
int Qaullib_FileAvailable(char *hashstr, char *suffix, struct qaul_file_LL_item *file_item);

/**
 * try to download a file
 */
void Qaullib_FileConnect(struct qaul_file_LL_item *file_item);

/**
 * check file sockets for incoming traffic
 */
void Qaullib_FileCheckSockets(void);

/**
 * check if file @a path exists
 *
 * @retval 1 file exists
 * @retval 0 file does not exist
 */
int Qaullib_FileExists(char *path);

/**
 * delete a file by it's database @a id
 *
 * @retval 1 success
 * @retval 0 error
 */
int Qaullib_FileDelete(struct qaul_file_LL_item *file_item);

/**
 * fill files from DB into LL
 */
void Qaullib_FileDB2LL(void);

/**
 * create @a string from @a hash
 *
 * @retval 1 on success
 * @retval 0 on error
 */
int Qaullib_HashToString(unsigned char *hash, char *string);

/**
 * reconverts a hashstring @a string to the @a hash
 *
 * @retval 1 on success
 * @retval 0 on error
 */
int Qaullib_StringToHash(char *string, unsigned char *hash);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif
