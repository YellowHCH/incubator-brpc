// Copyright (c) 2012 Baidu, Inc.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: Ge,Jun (gejun@baidu.com)
// Date: Thu Nov 22 13:57:56 CST 2012

#include "base/macros.h"
#include "base/zero_copy_stream_as_streambuf.h"

namespace base {

BAIDU_CASSERT(sizeof(std::streambuf::char_type) == sizeof(char),
              only_support_char);

int ZeroCopyStreamAsStreamBuf::overflow(int ch) {
    if (ch == std::streambuf::traits_type::eof()) {
        return ch;
    }
    void* block = NULL;
    int size = 0;
    if (_zero_copy_stream->Next(&block, &size)) {
        setp((char*)block, (char*)block + size);
        // if size == 1, this function will call overflow again.
        return sputc(ch);
    } else {
        setp(NULL, NULL);
        return std::streambuf::traits_type::eof();
    }
}

int ZeroCopyStreamAsStreamBuf::sync() {
    // data are already in IOBuf.
    return 0;
}

ZeroCopyStreamAsStreamBuf::~ZeroCopyStreamAsStreamBuf() {
    shrink();
}

void ZeroCopyStreamAsStreamBuf::shrink() {
    if (pbase() != NULL) {
        _zero_copy_stream->BackUp(epptr() - pptr());
        setp(NULL, NULL);
    }
}

std::streampos ZeroCopyStreamAsStreamBuf::seekoff(
    std::streamoff off,
    std::ios_base::seekdir way,
    std::ios_base::openmode which) {
    if (off == 0 && way == std::ios_base::cur) {
        return _zero_copy_stream->ByteCount() - (epptr() - pptr());
    }
    return (std::streampos)(std::streamoff)-1;
}


}  // namespace base
