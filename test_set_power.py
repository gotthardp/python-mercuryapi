from random import randint
from mercury import Reader as RFID_Reader
reader = RFID_Reader('llrp://10.240.25.162')

min_power, max_power = reader.get_power_range()
print('Min Power: {}\nMax Power: {}'.format(min_power, max_power))

antennas = reader.get_antennas()
print('Antennas list: {}'.format(antennas))

old_read_powers = reader.get_read_powers()
print('Old antenna read powers: {}'.format(', '.join(str(item) for item in old_read_powers)))

reader.set_read_plan([1, 2], "GEN2")

power_1 = randint(min_power, max_power)
power_2 = randint(min_power, max_power)
print('Setting powers: (1, {}), (2, {})'.format(power_1, power_2))
new_read_powers = reader.set_read_powers([1, 2], [power_1, power_2])
print('Old antenna read powers: {}'.format(', '.join(str(item) for item in new_read_powers)))
