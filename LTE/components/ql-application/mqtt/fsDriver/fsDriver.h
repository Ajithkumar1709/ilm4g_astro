
#ifndef fsDriver_H
#define fsDriver_H
#include"ql_fs.h"


# define D_FILE_INDEXUSED      1
# define D_FILE_INDEXNOTUSED   5
# define D_MAX_FILEINDEX       1440



extern U2 fileWrite(QFILE * fd, U2 fileIndex, fsDataBackUp_t * fileData);
extern U2 fileRead(QFILE * fd, U2 fileIndex, fsDataBackUp_t * fileData);

#endif