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
        RawDataStream FileRawSourceDataTemp = {};
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
            if (!__FILE_RawSourceRead(filename, &FileRawSourceDataTemp))
                return;
            // create base => decoder(templateobj).
            PSAG_FILESYS_CH_BASE::PsagFilesysDecodeBase* DecoderObject = new TDEC();
            DecoderObject->__SystemRawSource(&FileRawSourceDataTemp);
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
        
        RawDataStream* OperateRawDataPTR() { return FileRawSourceDataTemp; }
        TDEC*          OperateDecoderOBJ() { return (TDEC*)DecoderPtrVOID; }
    };

    // FILE SYSTEM: ENCODE.
    template<typename TENC, typename =
        std::enable_if_t<std::is_base_of_v<PSAG_FILESYS_CH_BASE::PsagFilesysEncodeBase, TENC>>
    > // => template decode_channel interface.
    class PsagFilesysEncoderLoader {
    protected:
        RawDataStream FileRawSourceDataTemp = {};
        void* EncoderPtrVOID = nullptr;

        std::string WriteFilepath = {};
    public:
        PsagFilesysEncoderLoader(const std::string& filename) :
            WriteFilepath(filename)
        {
            // create base => encoder(templateobj).
            PSAG_FILESYS_CH_BASE::PsagFilesysEncodeBase* EncoderObject = new TENC();
            EncoderObject->__SystemRawSource(&FileRawSourceDataTemp);
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
                = (PSAG_FILESYS_CH_BASE::PsagFilesysEncodeBase*)FileRawSourceDataTemp;
            if (FileRawSourceDataTemp.empty()) {
                if (CVTPTR->__SystemRawEecoding()) {
                    PSAG_LOGGER::PushLogger(LogError, PSAG_FILESYS_LOADER_LABEL, "data encoding failed.");
                    return false;
                }
            }
            return __FILE_RawSourceWrite(WriteFilepath, &FileRawSourceDataTemp);
        }

        RawDataStream* OperateRawDataPTR() { return FileRawSourceDataTemp; }
        TENC*          OperateEncoderOBJ() { return (TENC*)EncoderPtrVOID; }
    };

    // 保留快捷文件加载 FUNC(S). 20241101 RCSZ.
    
    std::string   EasyFileReadString (const std::string& filename);
    RawDataStream EasyFileReadRawData(const std::string& filename);
}

namespace PSAG_FILESYS_BASE64 {

    RawDataStream PsagStringToRawData(const std::string&   str_data);
    std::string   PsagRawDataToString(const RawDataStream& bin_data);

    std::string PsagBase64Encode(const std::string& str_data);
    std::string PsagBase64Decode(const std::string& str_data);

    RawDataStream PsagStringToBase64Rawdata(const std::string&   str_data);
    std::string   PsagBase64RawdataToString(const RawDataStream& raw_data);
}

#endif