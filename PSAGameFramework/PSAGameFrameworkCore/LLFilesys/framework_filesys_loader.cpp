// framework_filesys_loader.
#include "framework_filesys_loader.h"

using namespace std;
using namespace PSAG_LOGGER;

// file input/output string/binray.
namespace PSAG_FILESYS_LOADER {

    PsagFilesysLoaderBinary::PsagFilesysLoaderBinary(const string& filename) {
        ifstream ReadFile(filename, ios::binary);
        // read_file status.
        if (!ReadFile.is_open()) {
            PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "failed loader(bin) open file: %s", filename.c_str());
            return;
        }
        // get file size.
        ReadFile.seekg(0, ios::end);
        size_t ReadFileSize = (size_t)ReadFile.tellg();
        ReadFile.seekg(0, ios::beg);

        // read binary data.
        ReadFileData.resize(ReadFileSize);
        ReadFile.read(reinterpret_cast<char*>(ReadFileData.data()), ReadFileSize);

        PushLogger(LogInfo, PSAG_FILESYS_LOADER_LABEL, "loader(bin) open file: %s read_size: %u", filename.c_str(), ReadFileSize);
        ReadFile.close();
    }

    bool PsagFilesysLoaderBinary::WriterFileBinary(const string& filename, const RawDataStream& databin, ios_base::openmode mode) {
        ofstream WriteFile(filename, ios::binary | mode);
        // write_file status.
        if (!WriteFile.is_open()) {
            PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "failed writer(bin) open file: %s", filename.c_str());
            return false;
        }
        // write binary data. 
        WriteFile.write(reinterpret_cast<const char*>(databin.data()), databin.size());

        PushLogger(LogInfo, PSAG_FILESYS_LOADER_LABEL, "writer(bin) open file: %s write_size: %u", filename.c_str(), databin.size());
        WriteFile.close();
        return true;
    }

    RawDataStream PsagFilesysLoaderBinary::GetDataBinary()    { return ReadFileData; }
    size_t        PsagFilesysLoaderBinary::GetFileTotalSize() { return ReadFileSize; }

    PsagFilesysLoaderString::PsagFilesysLoaderString(const string& filename) {
        ifstream ReadFile(filename);
        // read_file status.
        if (!ReadFile.is_open()) {
            PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "failed loader(str) open file: %s", filename.c_str());
            return;
        }
        // get file size.
        ReadFile.seekg(0, ios::end);
        size_t ReadFileSize = (size_t)ReadFile.tellg();
        ReadFile.seekg(0, ios::beg);

        // read string data.
        string FileContent((istreambuf_iterator<char>(ReadFile)), istreambuf_iterator<char>());
        ReadFileData = FileContent;
        PushLogger(LogInfo, PSAG_FILESYS_LOADER_LABEL, "loader(str) open file: %s read_size: %u", filename.c_str(), ReadFileSize);
    }

    bool PsagFilesysLoaderString::WriterFileString(const string& filename, const string& databin, ios_base::openmode mode) {
        fstream WriteFile(filename, mode);
        // write_file status.
        if (!WriteFile.is_open()) {
            PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "failed writer(str) open file: %s", filename.c_str());
            return false;
        }
        // write string data. 
        WriteFile.write(databin.data(), databin.size());

        PushLogger(LogInfo, PSAG_FILESYS_LOADER_LABEL, "writer(str) open file: %s write_size: %u", filename.c_str(), databin.size());
        WriteFile.close();
        return true;
    }

    string PsagFilesysLoaderString::GetDataString()    { return ReadFileData; }
    size_t PsagFilesysLoaderString::GetFileTotalSize() { return ReadFileSize; }

    string TextFileLoader(const string& filename) {
        ifstream TextRead(filename);
        // read_file status.
        if (!TextRead.is_open()) {
            PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "failed loader(text) open file: %s", filename.c_str());
            return string();
        }
        // read string data.
        string FileContent((istreambuf_iterator<char>(TextRead)), istreambuf_iterator<char>());
        PushLogger(LogInfo, PSAG_FILESYS_LOADER_LABEL, "loader(text) open file: %s", filename.c_str());
        return FileContent;
    }
}