
# Setup new pi
## Prerequisites
1. Make sure you ssh keys are added to the users role in the anlible folder `./ansible/roles/users/tasks/main.yml`

1. Download the latest raspberry pi os and flash it to the sd card
You can use Etcher as an easy sd card flash tool

2. On the sd card create an empty file with the name `ssh` and place in in the root of the sd card to activate ssh right away. 

3. If you have the small 7" lcd attached append `lcd_rotate=2` to `/boot/config.txt`

4. When the Pi has booted up, connect the pi with ethernet cable and go to your router to find the ip address of your pi

5. SSH to your pi. The default password is `raspberry`
```
ssh pi@192.168.1.**
```

6. Update the pi. There are always more new stuff.  
``` shell
sudo apt update 
sudo apt full-upgrade
```

7. Add your ssh keys otherwise you won't be able to run ansible the first time      
   ``` shell
   mkdir ~/.ssh
   curl https://github.com/<YOUR_USERNAME>.keys >> .ssh/authorized_keys
   ```
   
   Or you can use this from your host machine if you do not have your keys on github
   ```shell
   ssh-copy-id pi@<IP_ADDRESS_OF_YOUR_PI>
   ```
   
   Verify that your keys are present
   ```shell
   cat ~/.ssh/authorized_keys
   ```

## Ansible setup
1. Add your new pi under `case: host:` to the inventory file in ansible: `ansible/inventory.yml`
   ```yml
        case:
         hosts:

           [...]

           192.168.1.11:    # First use the local ip address for the pi. Change this to match wg0_ip_address after first run.
             ansible_user: pi
             ansible_become: yes
             ansible_ssh_common_args: -F ssh_config
             ansible_python_interpreter: /usr/bin/python3
             wg0_ip_address: "10.14.47.4"                  # give it a unique ip    on the vpn server
             hostapd_ssid: myCaseName                      # give it a name for the wifi hotspot
             hostapd_wpa_password: somePasswordForWiFi     # set a password for the wifi hotspot
           
           [...]
   ```

2. Add your new pi to the webgateway `./ansible/roles/webgateway/files/traefik_sites.yml`

    Add the following under `router:` and change `myCaseName`to your liking
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

    And the following under `services:`
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

## Run the playbook

1. Run the vpn playbook to get your pi connected to the cloud infrastructure. The setup throws one intentional error about getting wireguard keys the first time it runs. This is normal, so don't be alarmed.
   ```shell
   make playbook PLAYBOOK=vpn.yml
   ```
2. Go back into the inventory file and change the host ip so it matches the `wg0_ip_address`

3. Run the final playbook to deploy the data logger. At the moment there is an issue with failing to restart hostpad. Just rerun the script to resolve the issue.
   ```shell
   make playbook
   ```

# Grafana
1. Login to grafana on `grafana.myCaseName.idl.mp.aau.dk` and login with username `admin` and password `admin` 
    Grafana will then prompt you to change the password.

2. When logged in, there should be a shortcut to adding a data source on the frontpage. Then add the influxDB as your datasource
    * Use `http://localhost/8086` as the url 
    * Set the database: `idl`
    * Set HTTP method: `GET`
    * Press save & test
3. Create a dashboard on the plus icon in the side panel.

4. Go to dashboard settings on the gear icon on the top right panel

5. Change the title of your dashboard

6. Under variables add a new variable with these parameters:
    * Name: `Device`
    * Type: `Query`
    * Data `source: InfluxDB`
    * Refresh: `On Dashboard Load`
    * Query: `show tag values from "temperature" with key = "deviceID"`
    * Multi-value: `true`
    * Include all options: `true`  
  
    This variable is used to select a specific device if you have multiple devices streaming the same data and you want to hide some of the devices.  
 
 1. Save dashboard
 
 2. Add a panel on the 'add panel' botton on the top or edit an existing one by pressing 'e' while hovering your mouse over the panel
 
 3. Now if you have data you can select your measurements and create your query depending on what you want to show. As a template you can use this:
     *  `SELECT mean("value") FROM "temperature" WHERE ("deviceID" =~ /^$Device$/) AND $timeFilter GROUP BY time($__interval), "name", "deviceID" fill(null)` 

 4. Save dashboard



