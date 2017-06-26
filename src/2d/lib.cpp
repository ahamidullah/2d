#define zabort(fmt, ...) error(__FILE__, __LINE__, __func__, true, fmt, ## __VA_ARGS__)
#define zerror(fmt, ...) error(__FILE__, __LINE__, __func__, false, fmt, ## __VA_ARGS__)

void
error(const char *file, int line, const char *func, bool abort, const char *fmt, ...)
{
	printf("Error: %s:%d in %s - ", file, line, func);
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	printf("\n");
	if (abort)
		exit(1);
}

template <typename F>
struct Scope_Exit {
	Scope_Exit(F _f) : f(_f) {}
	~Scope_Exit() { f(); }
	F f;
};

template <typename F>
Scope_Exit<F>
make_scope_exit(F f)
{
	return Scope_Exit<F>(f);
}

constexpr int
round_nearest(float f)
{
	return (f > 0.0f) ? (int)(f + 0.5f) : (int)(f - 0.5f);
}

#define DO_STRING_JOIN(arg1, arg2) arg1 ## arg2
#define STRING_JOIN(arg1, arg2) DO_STRING_JOIN(arg1, arg2)
#define DEFER(code) auto STRING_JOIN(scope_exit_, __LINE__) = make_scope_exit([=](){code;})

#define TIMED_BLOCK(name) Block_Timer __block_timer__##__LINE__(#name)

struct Block_Timer {
	Block_Timer(const char *n)
	{
		strcpy(name, n);
		start = SDL_GetPerformanceCounter();
	}
	~Block_Timer()
	{
		double time = (double)((SDL_GetPerformanceCounter() - start)*1000) / SDL_GetPerformanceFrequency();
		printf("%s - %fms\n", name, time);
	}
	uint64_t start;
	char name[256];
};

float
minf(float f1, float f2)
{
	return f1 < f2 ? f1 : f2;
}

char *
read_entire_file(const char *fname, const char *mode)
{
	SDL_RWops *fp = SDL_RWFromFile(fname, mode);
	if (!fp) {
		zerror("Could not open file %s. SDL error: %s", fname, SDL_GetError());
		return NULL;
	}

	Sint64 file_len = SDL_RWsize(fp);
	char* res = (char *)malloc(file_len + 1);

	Sint64 nb_read_total = 0, nb_read = 1;
	char* buf = res;
	while (nb_read_total < file_len && nb_read != 0) {
		nb_read = SDL_RWread(fp, buf, 1, (file_len - nb_read_total));
		nb_read_total += nb_read;
		buf += nb_read;
	}
	SDL_RWclose(fp);
	if (nb_read_total != file_len) {
		zerror("Could not read from file %s. SDL error: %s", fname, SDL_GetError());
		free(res);
		return NULL;
	}

	res[nb_read_total] = '\0';
	return res;
}

