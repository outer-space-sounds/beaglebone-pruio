int beaglebone_clock_new(int is_digital, char* channel, void* instance, void (*callback_function)(void*, float), char* err);
void beaglebone_clock_free(int is_digital, char* channel);
void beaglebone_tick(void* x);
