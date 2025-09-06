// Includes and type definitions needed by all our transpiled C# --> C++ files.
#ifndef CORE_INCLUDES_H
#define CORE_INCLUDES_H

#include "CS_List.h"
#include "CS_String.h"

#include <cstdint>

// Data types which, in C#, are all defined in System.
// C# code should use these instead of the shortcuts (int, byte, long, etc.)
using Byte    = uint8_t;   // use this instead of byte
using SByte   = int8_t;    // sbyte

using Int16   = int16_t;   // short
using UInt16  = uint16_t;  // ushort

using Int32   = int32_t;   // int
using UInt32  = uint32_t;  // uint

using Int64   = int64_t;   // long
using UInt64  = uint64_t;  // ulong

// The following will work with both the shortcut names,
// and the full System names.
using Char    = char;
using Single  = float;
using Double  = double;
using Boolean = bool;

#endif // CORE_INCLUDES_H
