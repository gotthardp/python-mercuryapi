from mercury import Reader as RFID_Reader
reader = RFID_Reader('llrp://10.240.25.162')
reader.set_read_plan([1, 2], "GEN2")
reader.set_read_powers([1, 2], [200, 300])
