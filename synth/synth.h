#pragma once

#include "vm/types.h"
#include "vm/stack.h"
#include "vm/storage.h"
#include "vm/program.h"
#include "vm/bytecodeiterator.h"
#include "vm/node.h"
#include "vm/opcodes.h"
#include "vm/vm.h"

// These are neccesary because they're public members of synth, and needs to be usable by the VST intrument class
#include "math/util.h"
#include "util/sync.h"
#include "math/resampler2x.h"
#include "wavetable/firinterpolator.h"

#include "graph/synth.h"
#include "graph/instrument.h"
#include "graph/voice.h"
