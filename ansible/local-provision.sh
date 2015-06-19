#!/bin/bash
set -ev

AFILE="/etc/ansible/hosts"
TLIST="/etc/apt/sources.list.d/ansible-ansible-trusty.list"


rm -rf $TLIST
echo deb http://ppa.launchpad.net/ansible/ansible/ubuntu trusty main >> $TLIST
echo deb-src http://ppa.launchpad.net/ansible/ansible/ubuntu trusty main >> $TLIST

apt-key adv \
    --keyserver keyserver.ubuntu.com \
    --recv 7BB9C367

apt-key adv \
    --keyserver keyserver.ubuntu.com \
    --recv DCE0E38E



apt-get -qq update

DEBIAN_FRONTEND=noninteractive \
     apt-get install -yq ansible --no-install-recommends


mkdir -p /etc/ansible
rm -rf $AFILE
echo >> $AFILE
echo [dev] >> $AFILE
echo localhost >> $AFILE


cd /opt/uboot/ansible
ansible-playbook site.yml -c local

