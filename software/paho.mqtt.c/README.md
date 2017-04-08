#### paho.mqtt.c doesn't work with client certificates against a mosqutto server that requires client certificates

  - [ ] Create a PKI: a certificate authority and a couple certificates for our mqtt server and client
  - [ ] Add /etc/hosts entries so that the certificates match the hostnames
  - [ ] Install and configure mosquitto with the server certificate from our PKI
  - [ ] Compile paho.mqtt.c and our test case of paho.mqtt.c with client certificates connecting to our mqtts://mqtt.hates.software:8883
  - [ ] Run our tests using the client cert

```
==> default: 1485189674: New connection from 127.0.0.1 on port 8883.
==> default: 1485189674: Socket error on client (null), disconnecting.
==> default: 1485189674: New connection from 127.0.0.1 on port 8883.
==> default: 1485189674: OpenSSL Error: error:1408F10B:SSL routines:SSL3_GET_RECORD:wrong version number
==> default: 1485189674: Socket error on client (null), disconnecting.
```

Steps to reproduce: install virtualbox, install vagrant, run `make` in this directory. There will be a `provision.out` file for your perusal.
This will leave a virtual machine to debug the problem running `make shell` to poke around.
To burn down the environment: `make clean`


```
perl -e 'use CPAN; use local::lib "/usr/local/lib/perl5"; CPAN::install(Net::MQTT::Simple);'
# add use local::lib "/usr/local/lib/perl5"; to /usr/local/lib/perl5/bin/mqtt-simple
/usr/local/lib/perl5/bin/mqtt-simple -h mqtt.hates.software -s '#' -ssl --ca /etc/ssl/certs/ca.crt --cert /etc/ssl/certs/client.hates.software.crt --key /etc/ssl/private/client.hates.software.key
```
