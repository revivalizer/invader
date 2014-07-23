#pragma once

namespace invader {

typedef uint16_t opcode_t;
typedef opcode_t opcode_index_t;
typedef opcode_t argument_t;
typedef opcode_t nodetype_t;
typedef opcode_t label_t;
typedef double   num_t;

enum { kInvalidOpcode      = 0xFFFF };
enum { kInvalidArgument    = 0xFFFF };
enum { kInvalidOpcodeIndex = 0xFFFF };

const opcode_t kOpcodeMaskIsNode      = 1 << 15;

} // namespace invader
