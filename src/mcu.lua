function onTick()
	--speed = getSensor("VehicleSpeed")
	--print("VSS" ..speed)

	--txPayload = { }
	--txPayload[1] = getSensor("VehicleSpeed")
    rpm = getSensor("Rpm")
    print("RPM" ..rpm)
    txPayload[2] = rpm >> 8
	txPayload[3] = rpm & 0xff
    txPayload[4] = getSensor("Clt")
    txPayload[5] = getSensor("FuelLevel")
    

	--txCan(2, 0x02004200, 1, txPayload)

end