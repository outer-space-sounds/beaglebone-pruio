int bbb_pruio_start_adc();
int bbb_pruio_stop_adc();

int bbb_pruio_messages_are_available();
void bbb_pruio_read_message(unsigned int *message);

volatile unsigned int *shared_ram = NULL;

// int bbb_pruio_init_adc_pin(unsigned int pin_number, unsigned int steps, bbb_pruio_adc_callback); 
