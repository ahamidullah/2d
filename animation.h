#if defined(DEFINE_ANIM_IDS) && defined(LOAD_ANIMS)
#error "Both DEFINE_ANIM_IDS and LOAD_ANIMS are define. (Forgot to undef one?)"
#elif defined(DEFINE_ANIM_IDS)
#define DECL_ANIM(name)\
	name ## _anim,
#elif defined(LOAD_ANIMS)
#define DECL_ANIM(name)\
	load_anim_from_file(name ## _anim, #name);
#else 
#error "Missing DEFINE_ANIM_IDS or LOAD_ANIMS."
#endif

DECL_ANIM(walk)
