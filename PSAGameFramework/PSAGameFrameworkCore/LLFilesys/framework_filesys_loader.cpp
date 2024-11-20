// framework_filesys_loader.
#include <base64/base64.h>
#include "framework_filesys_loader.hpp"

using namespace std;
using namespace PSAG_LOGGER;

// file input/output string/binray.
namespace PSAG_FILESYS_LOADER {

#define FILE_SEEKG_OFFSET 0
    bool __FILE_RawSourceRead(const string& filename, RawDataStream* dataptr) {
        ifstream READ_FILE_OBJ(filename, ios::binary);
        // read_file status.
        if (!READ_FILE_OBJ.is_open()) {
            PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "failed load(src) open: %s", filename.c_str());
            return false;
        }
        // get file size.
        READ_FILE_OBJ.seekg(FILE_SEEKG_OFFSET, ios::end);
        size_t ReadFileSize = (size_t)READ_FILE_OBJ.tellg();
        READ_FILE_OBJ.seekg(FILE_SEEKG_OFFSET, ios::beg);

        // read binary data.
        dataptr->resize(ReadFileSize);
        READ_FILE_OBJ.read(reinterpret_cast<char*>(dataptr->data()), ReadFileSize);

        PushLogger(LogInfo, PSAG_FILESYS_LOADER_LABEL, "load(src) read_file: %s read_size: %u", 
            filename.c_str(), ReadFileSize
        );
        READ_FILE_OBJ.close();
        return true;
    }

    bool __FILE_RawSourceWrite(const string& filename, const RawDataStream* dataptr) {
        ofstream WRITE_FILE_OBJ(filename, ios::binary);
        // write_file status.
        if (!WRITE_FILE_OBJ.is_open()) {
            PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "failed write(src) open: %s", filename.c_str());
            return false;
        }
        // write binary data. 
        WRITE_FILE_OBJ.write(reinterpret_cast<const char*>(dataptr->data()), dataptr->size());

        PushLogger(LogInfo, PSAG_FILESYS_LOADER_LABEL, "writer(src) open_file: %s write_size: %u", 
            filename.c_str(), dataptr->size()
        );
        WRITE_FILE_OBJ.close();
        return true;
    }

    string EasyFileReadString(const string& filename) {
        RawDataStream ReadDataCache = {};
        if (__FILE_RawSourceRead(filename, &ReadDataCache))
            return string(ReadDataCache.begin(), ReadDataCache.end());
        PushLogger(LogWarning, PSAG_FILESYS_LOADER_LABEL, "easy read_string failed.");
        return string();
    }

    RawDataStream EasyFileReadRawData(const string& filename) {
        RawDataStream ReadDataCache = {};
        if (__FILE_RawSourceRead(filename, &ReadDataCache))
            return ReadDataCache;
        PushLogger(LogWarning, PSAG_FILESYS_LOADER_LABEL, "easy read_raw_data failed.");
        return RawDataStream();
    }
}

#define BASE64_CVT_FLAG_INPUT_NULLPTR    -1
#define BASE64_CVT_FLAG_TARGET_NOT_EMPTY -2
#define BASE64_CVT_FLAG_SUCCESS           1
namespace PSAG_FILESYS_BASE64 {

    string PsagBase64Encode(const string& str_data) {
        return base64_encode((const uint8_t*)str_data.c_str(), str_data.size());
    }

    string PsagBase64Decode(const string& str_data) {
        return base64_decode(str_data);
    }

    int32_t PsagFileSystemConvert(string* str_data, RawDataStream* raw_data, ConvertModeType mode) {
        if (str_data == nullptr || raw_data == nullptr) {
            PushLogger(LogError, PSAG_FILESYS_BASE_LABEL, "str | raw ptr = nullptr!");
            return BASE64_CVT_FLAG_INPUT_NULLPTR;
        }
        auto PsagStringToRawData = [](const string& str) {
            return RawDataStream((uint8_t*)str.data(), str.size());
        };
        auto PsagRawDataToString = [](const RawDataStream& raw) {
            return string(raw.begin(), raw.end());
        };
        // data convert, 4-types 20241106 RCSZ.
        switch (mode) {
        case(StringToRawData):
            if (!raw_data->empty()) return BASE64_CVT_FLAG_TARGET_NOT_EMPTY;
            *raw_data = PsagStringToRawData(*str_data);
            break;
        case(RawDataToString):
            if (!str_data->empty()) return BASE64_CVT_FLAG_TARGET_NOT_EMPTY;
            *str_data = PsagRawDataToString(*raw_data);
            break;
        case(StringToBase64RawData):
            if (!raw_data->empty()) return BASE64_CVT_FLAG_TARGET_NOT_EMPTY;
            *raw_data = PsagStringToRawData(PsagBase64Encode(*str_data)); 
            break;
        case(Base64RawDataToString):
            if (!str_data->empty()) return BASE64_CVT_FLAG_TARGET_NOT_EMPTY;
            *str_data = PsagBase64Decode(PsagRawDataToString(*raw_data)); 
            break;
        }
        return BASE64_CVT_FLAG_SUCCESS;
    }
}