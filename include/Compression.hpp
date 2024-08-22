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

class Decompressor {
public:
	Decompressor() : ctx(ZSTD_createDCtx()) {}
	~Decompressor() { ZSTD_freeDCtx(ctx); }

	CompressionData decompress(const CompressionData &inData) {
		unsigned long long len = ZSTD_getFrameContentSize(inData.data, inData.len);

		if (ZSTD_isError(len)) {
			// TODO
		}

		CompressionData res = CompressionData(std::malloc(len), len);
		size_t decompressed_len = ZSTD_decompressDCtx(ctx, res.data, res.len, inData.data, inData.len);

		if (ZSTD_isError(decompressed_len)) {
			// TODO
		}

		if (len != decompressed_len) {
			// TODO
		}

		return res;
	}

private:
	ZSTD_DCtx *ctx;
};


#endif
