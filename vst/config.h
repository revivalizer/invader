#ifndef STANDALONE
	#define ENABLE_ZASSERT
	#define ENABLE_ZDEBUG
	#define ENABLE_ZERROR
#else
	#undef ENABLE_ZASSERT
	#undef ENABLE_ZDEBUG
	#undef ENABLE_ZERROR
#endif