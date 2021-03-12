## general first steps 
make sure you ssh keys are added to the users role in the anlible folder `./ansible/roles/users/tasks/main.yml`

## setup new pi
Download the latest raspberry pi os and flash it to the sd card
You can use Etcher as an easy sd card flash tool

On the sd card create an empty file with the name `ssh` and place in in the root of the sd card to activate ssh right away
`touch ssh`

If you have the pi lcd attached append `lcd_rotate=2` to `/boot/config.txt`

When the Pi has booted up, connect the pi with ethernet cable and go to your router to find the ip address of your pi

SSH to your pi 
`ssh pi@192.168.1.**` 
The default password is 'raspberry'

Update the pi. There are always more new stuff.
`sudo apt update`
`sudo apt full-upgrade`

add your ssh keys otherwise you won't be able to run ansible the first time 
`mkdir ~/.ssh`
`curl https://github.com/your_username.keys > .ssh/authorized_keys`

or 
`ssh-copy-id pi@<YOUR IP ADDRESS>`

verify that your keys are present
`cat ~/.ssh/authorized_keys`

add your new pi to inventory in ansible 
```yml
     case:
      hosts:
        192.168.1.11:    # First use the local ip address for the pi. Change this to match wg0_ip_address after first run.
          ansible_user: pi
          ansible_become: yes
          ansible_ssh_common_args: -F ssh_config
          ansible_python_interpreter: /usr/bin/python3
          wg0_ip_address: "10.14.47.4"                  # give it a unique ip on the vpn server
          hostapd_ssid: myCaseName                      # give it a name for the wifi hotspot
          hostapd_wpa_password: Aalborg9000Robotlab     # set a password for the wifi hotspot
```

add your new pi to the webgateway `./ansible/roles/webgateway/files/traefik_sites.yml`

add the following under `router:`
```yml
grafana-myCaseName:
    rule: "Host(`grafana.myCaseName.idl.mp.aau.dk`)"
    service: grafana-myCaseName
    tls:
    certResolver: sample
api-myCaseName:
    rule: "Host(`api.myCaseName.idl.mp.aau.dk`)"
    service: api-myCaseName
    tls:
    certResolver: sample
    middlewares:
    - api-auth
```

and the following under `services:`
```yml
grafana-myCaseName:
      loadBalancer:
        servers:
          - url: "http://10.14.47.4:3000/"
    api-myCaseName:
      loadBalancer:
        servers:
          - url: "http://10.14.47.4:9090/"
```

run the vpn playbook to get your pi connected to the cloud infrastructure. 
`make playbook PLAYBOOK=vpn.yml`


########################################

Install wireguard
echo "deb http://deb.debian.org/debian/ unstable main" | sudo tee --append /etc/apt/sources.list
sudo apt-key adv --keyserver   keyserver.ubuntu.com --recv-keys 04EE7237B7D453EC
sudo apt-key adv --keyserver   keyserver.ubuntu.com --recv-keys 648ACFD622F3D138

sudo apt update
sudo apt install wireguard

reboot

Install wireguard network
sudo ip link add wg0 type wireguard
sudo wg show (to verify your interface)

sudo wg genkey > wg_key 






[WireGuard]
PrivateKey=WD33CK4eAlualCLw6NPqGTAfkQxV9PoNwGibl+luxGU=
ListenPort=51820

[WireGuardPeer]
PublicKey=JB85+QKhRviVlJ5L+qrwm/MRjrbMyh/QrzPCgRoEbDA=
AllowedIPs=10.14.47.5/32

[WireGuardPeer]
PublicKey=hr2mOT/Hpk2ahbDbENJo1RzH02Mutus8CbWmUPTCXSM=
AllowedIPs=10.14.47.2/32

[WireGuardPeer]
PublicKey=WWAiLMmkEa9ttSkwFdQ6GCzeN/m74syQ0nuvUcggW3Q=
AllowedIPs=10.14.47.3/32

"[
    {       
        "table": "temperature",     
        "name": "sensor 1",         
        "unit": "celcius",          
        "value": 23.34,             
        "tags":{                    
            "placement" : "kitchen"
        }
    },
]"