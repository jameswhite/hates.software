default: .vagrant/machines/default/virtualbox/id
	bash -c "(export DISTRO='jessie'; export HATES_SOFTWARE_URL='git@github.com:jameswhite/hates.software.git#wtf'; vagrant provision)"
.vagrant/machines/default/virtualbox/id: Vagrantfile
	vagrant up
Vagrantfile:
	cp /Users/jameswhite/dev/jameswhite/hates.software/vagrantfile Vagrantfile
	# wget -q -O Vagrantfile 'https://raw.githubusercontent.com/jameswhite/hates.software/wtf/Vagrantfile'
clean:
	vagrant destroy -f
	/bin/rm Vagrantfile
shell:
	vagrant ssh -- -A -l root
