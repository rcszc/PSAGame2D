// framework_filesys_loader.

#ifndef _FRAMEWORK_FILESYS_LOADER_H
#define _FRAMEWORK_FILESYS_LOADER_H
#include <type_traits>
// LLFilesys PSA-L.3 (=>logger).
#include "../LLLogger/framework_logger.hpp"
#include "framework_filesys_dec_channels.h"

namespace PSAG_FILESYS_LOADER {
    StaticStrLABEL PSAG_FILESYS_LOADER_LABEL = "PSAG_FILESYS_LOAD";

    bool __FILE_RawSourceRead (const std::string& filename,       RawDataStream* dataptr);
    bool __FILE_RawSourceWrite(const std::string& filename, const RawDataStream* dataptr);

    // FILE SYSTEM: DECODER.
    template<typename TDEC, typename =
        std::enable_if_t<std::is_base_of_v<PSAG_FILESYS_CH_BASE::PsagFilesysDecodeBase, TDEC>>
    > // => template decode_channel interface.
    class PsagFilesysDecoderLoader {
    protected:
        RawDataStream FileRawSourceCache = {};
        void* DecoderPtrVOID = nullptr;
    public:
        PsagFilesysDecoderLoader(const std::string& filename) {
            // check file_valid ?
            if (!std::filesystem::exists(filename)) {
                PSAG_LOGGER::PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "src_loader: invalid filepath.");
                PSAG_LOGGER::PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "f: %s", filename.c_str());
                return;
            }
            // load source data. 
            if (!__FILE_RawSourceRead(filename, &FileRawSourceCache))
                return;
            // create base => decoder(templateobj).
            PSAG_FILESYS_CH_BASE::PsagFilesysDecodeBase* DecoderObject = new TDEC();
            DecoderObject->__SystemRawSource(&FileRawSourceCache);
            // call decoder interface.
            if (DecoderObject->__SystemRawDecoding()) {
                DecoderPtrVOID = (void*)DecoderObject;
                return;
            }
            PSAG_LOGGER::PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "data decoding failed.");
        }

        ~PsagFilesysDecoderLoader() {
            if (DecoderPtrVOID != nullptr) {
                delete DecoderPtrVOID;
                return;
            }
            PSAG_LOGGER::PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "dec_object nullptr.");
        }
        
        RawDataStream* OperateRawDataPTR() { return FileRawSourceCache; }
        TDEC* OperateDecoderOBJ() { return (TDEC*)DecoderPtrVOID; }
    };

    // FILE SYSTEM: ENCODE.
    template<typename TENC, typename =
        std::enable_if_t<std::is_base_of_v<PSAG_FILESYS_CH_BASE::PsagFilesysEncodeBase, TENC>>
    > // => template decode_channel interface.
    class PsagFilesysEncoderLoader {
    protected:
        RawDataStream FileRawSourceCache = {};
        void* EncoderPtrVOID = nullptr;

        std::string WriteFilepath = {};
    public:
        PsagFilesysEncoderLoader(const std::string& filename) :
            WriteFilepath(filename)
        {
            // create base => encoder(templateobj).
            PSAG_FILESYS_CH_BASE::PsagFilesysEncodeBase* EncoderObject = new TENC();
            EncoderObject->__SystemRawSource(&FileRawSourceCache);
        }

        ~PsagFilesysEncoderLoader() {
            if (EncoderPtrVOID != nullptr) {
                delete EncoderPtrVOID;
                return;
            }
            PSAG_LOGGER::PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "enc_object nullptr.");
        }

        // encode data => write file.
        bool WRITE_FILE() {
            PSAG_FILESYS_CH_BASE::PsagFilesysEncodeBase* CVTPTR 
                = (PSAG_FILESYS_CH_BASE::PsagFilesysEncodeBase*)FileRawSourceCache;
            if (FileRawSourceCache.empty()) {
                if (CVTPTR->__SystemRawEecoding()) {
                    PSAG_LOGGER::PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "data encoding failed.");
                    return false;
                }
                return __FILE_RawSourceWrite(WriteFilepath, &FileRawSourceCache);
            }
            PSAG_LOGGER::PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "encode cache not empty!");
            return false;
        }

        RawDataStream* OperateRawDataPTR() { return FileRawSourceCache; }
        TENC* OperateEncoderOBJ() { return (TENC*)EncoderPtrVOID; }
    };

    // 保留快捷文件加载 FUNC(S). 20241101 RCSZ.
    
    std::string   EasyFileReadString (const std::string& filename);
    RawDataStream EasyFileReadRawData(const std::string& filename);
}

namespace PSAG_FILESYS_BASE64 {
    StaticStrLABEL PSAG_FILESYS_BASE_LABEL = "PSAG_FILESYS_BASE64";

    enum ConvertModeType {
        StringToRawData = 1 << 1, // string => raw data
        RawDataToString = 1 << 2, // raw dara => string
        // base enc,dec oper.
        StringToBase64RawData = 1 << 3, // string => base64(enc) => raw data
        Base64RawDataToString = 1 << 4  // raw data => base64(dec) => string
    };
    int32_t PsagFileSystemConvert(std::string* str_data, RawDataStream* raw_data, ConvertModeType mode);

    std::string PsagBase64Encode(const std::string& str_data);
    std::string PsagBase64Decode(const std::string& str_data);
}

#endif