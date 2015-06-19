# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.require_version ">= 1.4.0"

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

VAGRANT_RAM   = ENV["VAGRANT_RAM"] || "1024"
VAGRANT_CORES = ENV["VAGRANT_CORES"] || "2"


Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  # All Vagrant configuration is done here. The most common configuration
  # options are documented and commented below. For a complete reference,
  # please see the online documentation at vagrantup.com.

  # Every Vagrant virtual environment requires a box to build off of.
  config.vm.box = "phusion/ubuntu-14.04-amd64"

  config.vm.synced_folder "", "/opt/uboot"
  
  config.vm.provision "shell", :privileged => true, path: "ansible/local-provision.sh"
  
  #
  # config.vm.provision "ansible" do |ansible|
  #   ansible.playbook = "ansible/site.yml"
  #   ansible.host_key_checking = false
  #   ansible.groups = {
  #     "dev" => ["default"],
  #   }
  #
  # end

end
