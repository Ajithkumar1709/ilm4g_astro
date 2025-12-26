#ifndef MEM_HANDLER_H
#define MEM_HANDLER_H
#include "ql_fs.h"

extern void nvmWriteIlmConfig(void);
extern void nvmReadIlmConfig(void);
extern void nvmReadIlmConfigBackUp(void);
extern void nvmWriteIlmConfigBackUp(void);
extern void loadIlmDefaultConfig(void);
extern U2 fileReadDu(QFILE * fd, fsDataBackUp_t * fileData);
extern void fsDataUpdateUploadState(U2 currentFileIndex);
extern U1 fsDataWrite(QFILE * fd, U1 * data, U2 dataLength, U2 * currentFileIndex, U4 ts);
extern U1 fsDataRead(fsDataBackUp_t * fileData, U2 fileIndex);
extern void nvmWriteResetFile(void);
extern void nvmReadResetFile(void);
extern void nvmReadAstroFileWithBackup(void);
extern void nvmWrireAstroFileWithBackup(void);
extern void loadDefualtAstroFile();

#endif /* _GNSSDEMO_H */
