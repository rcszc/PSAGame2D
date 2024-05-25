// framework_filesys_loader.

#ifndef _FRAMEWORK_FILESYS_LOADER_H
#define _FRAMEWORK_FILESYS_LOADER_H
// LLFilesys PSA-L.3 (=>logger).
#include "../LLLogger/framework_logger.hpp"

namespace PSAG_FILESYS_LOADER {
    StaticStrLABEL PSAG_FILESYS_LOADER_LABEL = "PSAG_FILESYS_LOAD";

    class PsagFilesysLoaderBinary {
    protected:
        RawDataStream ReadFileData = {};
        size_t        ReadFileSize = {};
    public:
        PsagFilesysLoaderBinary(const std::string& filename);
        PsagFilesysLoaderBinary() {};

        bool WriterFileBinary(
            const std::string& filename,
            const RawDataStream& databin,
            std::ios_base::openmode mode = std::ios_base::out
        );

        RawDataStream GetDataBinary();
        size_t        GetFileTotalSize();
    };

    class PsagFilesysLoaderString {
    protected:
        std::string ReadFileData = {};
        size_t      ReadFileSize = {};
    public:
        PsagFilesysLoaderString(const std::string& filename);
        PsagFilesysLoaderString() {};

        bool WriterFileString(const std::string& filename, const std::string& databin, std::ios_base::openmode mode = std::ios_base::out);

        std::string GetDataString();
        size_t      GetFileTotalSize();
    };

    // 更便捷的文本文件加载器.
    std::string TextFileLoader(const std::string& filename);
}

#endif