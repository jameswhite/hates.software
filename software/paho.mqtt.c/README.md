#### paho.mqtt.c doesn't work with client certificates against a mosqutto server that requires client certificates

  - [ ] Create a PKI: a certificate authority and a couple certificates for our mqtt server and client
  - [ ] Add /etc/hosts entries so that the certificates match the hostnames
  - [ ] Install and configure mosquitto with the server certificate from our PKI
  - [ ] Compile paho.mqtt.c and our test case of paho.mqtt.c with client certificates connecting to our mqtts://mqtt.hates.software:8883
  - [ ] Run our tests using the client cert
