--[[
	Assume C struct (32-bit pointers)

	typedef unsigned short opcode_t;
	typedef unsigned short label_t;
	typedef double constant_t;

	struct vm_program
	{
		size_t      bytecodeSize;
		opcode_t*   bytecode;
		label_t*    labels;
		constant_t* constants;
	};
]]

require("pack")

function generate_data(program)
	-- generate arrays of data in strings, using the pack library
	local bytecode_str       = string.pack("H"..#program.bytecode,        unpack(program.bytecode))
	local labels_str         = string.pack("H"..#program.labels,          unpack(program.labels))
	local sections_str       = string.pack("H"..#program.section_starts,  unpack(program.section_starts))
	local constants_str      = string.pack("d"..#program.constants,       unpack(program.constants))

	local header_size = 32 -- assuming 32-bit pointers

	-- compute offsets to data
	local bytecode_pos      = header_size
	local labels_pos        = bytecode_pos  + bytecode_str:len()
	local sections_pos      = labels_pos    + labels_str:len()
	local constants_pos     = sections_pos  + sections_str:len()

	local program_size      = constants_pos + constants_str:len()

	local globalStorageSize = 0 -- for now
	local numSections = #program.ast.sections

	-- generate header data
	local header_str = string.pack("IIIIIIII", program_size, #program.bytecode, globalStorageSize, numSections, bytecode_pos, labels_pos, sections_pos, constants_pos)

	-- return concantenated data
	return header_str..bytecode_str..labels_str..sections_str..constants_str
end



function export_binary_and_header(path, program)
	-- Generate raw data
	local data = generate_data(program)

	-- Write binary data
	local f = assert(io.open(path..".bin", "wb"))
	f:write(data)
	f:close()

	-- Create hex encoded string
	local datastr = data:gsub('.', function (c)
        return string.format('0x%02X, ', string.byte(c))
    end)

	-- Write hex data to header
	local f = assert(io.open(path..".bin.h", "w"))
	f:write("unsigned char vm_data_raw[] = { "..datastr.."};")
	f:close()
end

