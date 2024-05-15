// framework_filesys_compress.
#include "framework_filesys_compress.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAG_FILESYS_COMPR {

	RawDataStream PsagFilesysCompress::CompressRawData(const RawDataStream& input_data) {
        z_stream DeflateStream = {};
        // config zlib compress_params.
        DeflateStream.zalloc   = Z_NULL;
        DeflateStream.zfree    = Z_NULL;
        DeflateStream.opaque   = Z_NULL;
        DeflateStream.avail_in = (uInt)input_data.size();
        DeflateStream.next_in  = const_cast<Bytef*>(input_data.data());

        int32_t ParamWindowBits = 15;
        if (CompressionMethod == Z_GZIP)
            ParamWindowBits += 16;

        // result compress raw_data.
        RawDataStream CompressedRawDataTemp = {};

        if (deflateInit2(&DeflateStream, CompressionLevel, CompressionMethod, ParamWindowBits, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            PushLogger(LogError, PSAG_FILESYS_COMPR_LABEL, "failed initialize zlib deflate stream.");
            return CompressedRawDataTemp;
        }
        CompressedRawDataTemp.resize(input_data.size());
        // 循环压缩流.
        do {
            DeflateStream.avail_out = (uInt)CompressedRawDataTemp.size();
            DeflateStream.next_out  = CompressedRawDataTemp.data();

            int32_t ResultStatus = deflate(&DeflateStream, Z_FINISH);
            if (ResultStatus == Z_STREAM_ERROR) {
                // compress error => clear free mem.
                deflateEnd(&DeflateStream);
                PushLogger(LogError, PSAG_FILESYS_COMPR_LABEL, "zlib compression error.");

                CompressedRawDataTemp.clear();
                CompressedRawDataTemp.shrink_to_fit();
                return CompressedRawDataTemp;
            }

            if (DeflateStream.avail_out == Z_NULL) {
                // resize result temp.
                CompressedRawDataTemp.resize(CompressedRawDataTemp.size() * 2);
            }
            else {
                size_t CompressedSize = CompressedRawDataTemp.size() - DeflateStream.avail_out;
                CompressedRawDataTemp.resize(CompressedSize);
            }
        } while (DeflateStream.avail_out == Z_NULL);
        // compressed.
        deflateEnd(&DeflateStream);

        // 计算压缩校验码.
        CompressChecksum = crc32(0L, CompressedRawDataTemp.data(), (uInt)CompressedRawDataTemp.size());
        if (CompressChecksum != Z_NULL)
            PushLogger(LogInfo, PSAG_FILESYS_COMPR_LABEL, "compression completed, create checksum.");
        return CompressedRawDataTemp;
	}

    RawDataStream PsagFilesysCompress::DecompressRawData(const RawDataStream& input_data) {
        z_stream InflateStream;
        // config zlib decompress_params.
        InflateStream.zalloc   = Z_NULL;
        InflateStream.zfree    = Z_NULL;
        InflateStream.opaque   = Z_NULL;
        InflateStream.avail_in = (uInt)input_data.size();
        InflateStream.next_in  = const_cast<Bytef*>(input_data.data());

        int ParamWindowBits = 15;
        // set gzip flags.
        if (CompressionMethod == Z_GZIP)
            ParamWindowBits = 15 | 16;

        // result decompress raw_data.
        RawDataStream DecompressedRawDataTemp;

        if (inflateInit2(&InflateStream, ParamWindowBits) != Z_OK) {
            PushLogger(LogError, PSAG_FILESYS_COMPR_LABEL, "failed initialize zlib inflate stream.");
            return DecompressedRawDataTemp;
        }
        DecompressedRawDataTemp.resize(input_data.size() * 2);
        // 循环解压流.
        do {
            InflateStream.avail_out = (uInt)DecompressedRawDataTemp.size();
            InflateStream.next_out  = DecompressedRawDataTemp.data();

            int ResultStatus = inflate(&InflateStream, Z_FINISH);
            if (ResultStatus == Z_STREAM_ERROR) {
                // decompress error => clear free mem.
                inflateEnd(&InflateStream);
                PushLogger(LogError, PSAG_FILESYS_COMPR_LABEL, "zlib decompression error.");

                DecompressedRawDataTemp.clear();
                DecompressedRawDataTemp.shrink_to_fit();
                return DecompressedRawDataTemp;
            }

            if (InflateStream.avail_out == Z_NULL) {
                DecompressedRawDataTemp.resize(DecompressedRawDataTemp.size() * 2);
            }
            else {
                size_t decompressedSize = DecompressedRawDataTemp.size() - InflateStream.avail_out;
                DecompressedRawDataTemp.resize(decompressedSize);
            }
        } while (InflateStream.avail_out == Z_NULL);
        // decompressed.
        inflateEnd(&InflateStream);

        // 计算解压校验码.
        DecompressChecksum = crc32(0L, DecompressedRawDataTemp.data(), (uInt)DecompressedRawDataTemp.size());
        if (DecompressChecksum != Z_NULL)
            PushLogger(LogInfo, PSAG_FILESYS_COMPR_LABEL, "decompression completed, create checksum.");
        return DecompressedRawDataTemp;
    }
}