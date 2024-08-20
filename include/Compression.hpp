#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <zstd.h>
#include "stdlib.hpp"

#define COMPRESSION_LEVEL 10

// TODO use advanced settings

// very simple struct, just so I can pass in and return 2 things at once, do not forget to free data yourself
struct CompressionData {
	void *data;
	size_t len;

	CompressionData() = default;
	CompressionData(void *data, size_t len) : data(data), len(len) {}
	~CompressionData() = default;
};

class Compressor {
public:
	Compressor() : ctx(ZSTD_createCCtx()) {}
	~Compressor() { ZSTD_freeCCtx(ctx); }


	CompressionData compress(const CompressionData &inData) {
		size_t new_len = ZSTD_compressBound(inData.len); // len is maximum possible after the compression
		CompressionData res = CompressionData(std::malloc(new_len), new_len);
		size_t compressed_size = ZSTD_compressCCtx(ctx, res.data, res.len, inData.data, inData.len, 10);
		// TODO
		if (ZSTD_isError(compressed_size)) {
        	// throw std::runtime_error(ZSTD_getErrorName(compressedSize));
    	}

		// cursed but got lazy
		res.len = compressed_size;
		return res;
	}
private:
	// the zstd context
	ZSTD_CCtx* ctx;
};

// class Decompressor {
// public:
// 	Decompressor() : ctx(ZSTD_createDCtx()) {}
// 	~Decompressor() { ZSTD_freeDCtx(ctx); }

// 	size_t decompress()

// private:
// 	ZSTD_DCtx *ctx;
// };


#endif
