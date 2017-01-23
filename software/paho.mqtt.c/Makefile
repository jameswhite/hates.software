default: .vagrant/machines/default/virtualbox/id
	bash -c "(export DISTRO='jessie'; export HATES_SOFTWARE_NAME='paho.mqtt.c'; export HATES_SOFTWARE_BRANCH='wtf'; export HATES_SOFTWARE_URL='git@github.com:jameswhite/hates.software.git'; vagrant provision)"
.vagrant/machines/default/virtualbox/id: Vagrantfile
	vagrant up
Vagrantfile:
	wget -q -O Vagrantfile 'https://raw.githubusercontent.com/jameswhite/hates.software/wtf/software/paho.mqtt.c/vagrantfile'
clean:
	vagrant destroy -f
shell:
	vagrant ssh -- -A -l root
