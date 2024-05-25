// framework_filesys_compress. 2024_04_02. RCSZ

#ifndef _FRAMEWORK_FILESYS_COMPRESS_H
#define _FRAMEWORK_FILESYS_COMPRESS_H
#include <zconf.h>
#include <zlib.h>
// LLFilesys PSA-L.1 (=>logger).
#include "../LLLogger/framework_logger.hpp"

#ifndef Z_GZIP // GZIP = Z_DEFLATE + 16.
#define Z_GZIP (16 + MAX_WBITS)
#endif
namespace PSAG_FILESYS_COMPR {
	StaticStrLABEL PSAG_FILESYS_COMPR_LABEL = "PSAG_FILESYS_CPRS";

	class PsagFilesysCompress {
	protected:
		int32_t CompressionLevel  = NULL;
		int32_t CompressionMethod = NULL;

		uint32_t CompressChecksum   = NULL; // 压缩数据校验码.
		uint32_t DecompressChecksum = NULL; // 解压数据校验码.
	public:
		PsagFilesysCompress(int32_t level = Z_DEFAULT_COMPRESSION, int32_t method = Z_DEFLATED) :
			CompressionLevel(level), CompressionMethod(method) 
		{}

		void SetCompressLevel(int32_t level)   { CompressionLevel  = level;  }
		void SetCompressMethod(int32_t method) { CompressionMethod = method; }

		RawDataStream CompressRawData(const RawDataStream& input_data);
		RawDataStream DecompressRawData(const RawDataStream& input_data);

		uint32_t GetCompressChecksum()   { return CompressChecksum;   };
		uint32_t GetDecompressChecksum() { return DecompressChecksum; };
	};
}

#endif