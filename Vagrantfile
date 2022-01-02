# -*- mode: ruby -*-
# vi: set ft=ruby :

# All Vagrant configuration is done below. The "2" in Vagrant.configure
# configures the configuration version (we support older styles for
# backwards compatibility). Please don't change it unless you know what
# you're doing.
Vagrant.configure("2") do |config|
  # The most common configuration options are documented and commented below.
  # For a complete reference, please see the online documentation at
  # https://docs.vagrantup.com.

  # Every Vagrant development environment requires a box. You can search for
  # boxes at https://vagrantcloud.com/search.
  config.vm.box = "bento/ubuntu-20.04"
  config.vm.define "spear-embedded-box"

  # Disable automatic box update checking. If you disable this, then
  # boxes will only be checked for updates when the user runs
  # `vagrant box outdated`. This is not recommended.
  # config.vm.box_check_update = false

  # Create a forwarded port mapping which allows access to a specific port
  # within the machine from a port on the host machine. In the example below,
  # accessing "localhost:8080" will access port 80 on the guest machine.
  # NOTE: This will enable public access to the opened port
  # config.vm.network "forwarded_port", guest: 80, host: 8080

  # Create a forwarded port mapping which allows access to a specific port
  # within the machine from a port on the host machine and only allow access
  # via 127.0.0.1 to disable public access
  # config.vm.network "forwarded_port", guest: 80, host: 8080, host_ip: "127.0.0.1"

  # Create a private network, which allows host-only access to the machine
  # using a specific IP.
  # config.vm.network "private_network", ip: "192.168.33.10"

  # Create a public network, which generally matched to bridged network.
  # Bridged networks make the machine appear as another physical device on
  # your network.
  # config.vm.network "public_network"

  # Share an additional folder to the guest VM. The first argument is
  # the path on the host to the actual folder. The second argument is
  # the path on the guest to mount the folder. And the optional third
  # argument is a set of non-required options.
  config.vm.synced_folder "../embedded", "/home/vagrant/embedded"

  # Provider-specific configuration so you can fine-tune various
  # backing providers for Vagrant. These expose provider-specific options.
  # Example for VirtualBox:
  #
  # config.vm.provider "virtualbox" do |vb|
  #   # Display the VirtualBox GUI when booting the machine
  #   vb.gui = true
  #
  #   # Customize the amount of memory on the VM:
  #   vb.memory = "1024"
  # end
  #
  # View the documentation for the provider you are using for more
  # information on available options.
   config.vm.provider :virtualbox do |vb|
     vb.customize ['modifyvm', :id, '--usb', 'on']
     vb.customize ['modifyvm', :id, '--usbehci', 'on']
     vb.customize ['usbfilter', 'add', '0', '--target', :id, '--name', 'STLink', '--vendorid', '0x0483', '--productid', '0x3748']
	 vb.customize ['usbfilter', 'add', '0', '--target', :id, '--name', 'STLink', '--vendorid', '0x0483', '--productid', '0x374B']
   end
  # Enable provisioning with a shell script. Additional provisioners such as
  # Ansible, Chef, Docker, Puppet and Salt are also available. Please see the
  # documentation for more information about their specific syntax and use.
  config.vm.provision "shell", inline: <<-SHELL
	sudo apt-get update
	sudo apt-get install -y cmake ninja-build python3-pip libtool build-essential autotools-dev autoconf pkg-config libusb-1.0-0 libusb-1.0-0-dev libftdi1 libftdi-dev git libc6 libncurses5 libstdc++6 minicom clang-format-12
	cd /home/vagrant/
  git config --global http.sslverify false
	mkdir spear-embedded
	cd spear-embedded
	git clone https://github.com/ntfreak/openocd
	cd openocd
	git submodule update --init --recursive
	sudo ./bootstrap
	sudo ./configure
	sudo make
	sudo make install
	sudo cp /home/vagrant/spear-embedded/openocd/contrib/60-openocd.rules /etc/udev/rules.d/
	sudo udevadm control --reload
	sudo usermod -aG plugdev vagrant
	cd /home/vagrant/spear-embedded
	rm -r openocd 
	wget -q https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2
	tar -xjf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2
	sudo mv gcc-arm-none-eabi-9-2020-q2-update /usr/local
	echo 'export PATH=/usr/local/gcc-arm-none-eabi-9-2020-q2-update/bin:$PATH' >> /home/vagrant/.bashrc
	export PATH=/usr/local/gcc-arm-none-eabi-9-2020-q2-update/bin:$PATH
	cd /home/vagrant/spear-embedded
	rm -f gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2
	pip3 install pydsdl
	pip3 install -U nunavut
	sudo apt-get autoremove -y
	cd /home/vagrant/embedded
	git submodule update --init --recursive
	sudo apt-get autoremove
	sudo apt-get autoclean
   SHELL
end
