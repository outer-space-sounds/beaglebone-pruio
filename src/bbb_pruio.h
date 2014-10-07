#define bbb_pruio_adc_callback void (*callback)(unsigned int, unsigned int)

int bbb_pruio_start_adc();
int bbb_pruio_init_adc_pin(unsigned int pin_number, unsigned int steps, bbb_pruio_adc_callback); 
int bbb_pruio_stop_adc();
