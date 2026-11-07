#ifndef _VANILLAPDF_CLI_ENCODING_H
#define _VANILLAPDF_CLI_ENCODING_H

#include <vanillapdf/c_vanillapdf_api.h>

#include <string>

namespace vanillapdf {
namespace cli {

// Decode a PDF text string (PDFDocEncoding, UTF-16BE or UTF-8) to UTF-8
std::string to_string_utf8(StringObjectHandle* string_object);

} // cli
} // vanillapdf

#endif /* _VANILLAPDF_CLI_ENCODING_H */
