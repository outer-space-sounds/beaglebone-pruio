/* Beaglebone Pru IO 
 * 
 * Copyright (C) 2015 Rafael Vega <rvega@elsoftwarehamuerto.org> 
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <m_pd.h>

typedef enum beaglebone_callback_type{
  BB_GPIO_DIGITAL = 0,
  BB_GPIO_ANALOG,
  BB_MIDI_NOTE,
  BB_MIDI_CONTROL
} beaglebone_callback_type;

int beaglebone_register_callback(beaglebone_callback_type type, int channel, void* instance, void (*callback_function)(void*, t_float));
int beaglebone_register_midi_callback(beaglebone_callback_type type, int channel, void* instance, void (*callback_function)(void*, beaglebone_midi_message*));
void beaglebone_unregister_callback(beaglebone_callback_type type, int channel);
