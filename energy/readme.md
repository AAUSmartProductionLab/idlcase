
## maintenance though direct wifi link

If for some reason the energy box cannot be reached from the network, it provides an open ssid which can be used to ssh to it directly so you dont have to pull the box completely apart

You should be able to just connect to it like a regular wifi access point, but heres a cli guide:

```
sudo ip link set wlp6s0 up # where wlp6s0 is your laptops wifi device
sudo iwconfig wlp6s0 essid energy01-direct # again, wlp6so should be your wifi device and energy01-direct is the energy box's ssid
sudo dhclient wlp6s0 # fetch an ip address from the raspberry inside
```

the raspberry pi should now be accessible on 10.20.30.1