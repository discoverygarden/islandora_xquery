#!/bin/bash

# This script should be run as an unprivileged user who has permission to run
# "sudo"... shouldn't break if run as root, but... really doesn't need to be.

DRUPAL_ROOT=${1:-/var/www/drupal7}

if [ ! -d $DRUPAL_ROOT ]; then
  echo "$DRUPAL_ROOT does not exist!"
  exit 1
fi

SCRATCH_DIR=$(mktemp -d)


sudo apt install -y basex build-essential php-dev php-pear

cd $SCRATCH_DIR

# Compile xdiff PHP extension and dependencies
wget http://www.xmailserver.org/libxdiff-0.23.tar.gz
sha512sum -c <<-EOHASH
b6bae5ee9d21e615d7beeacd9ceefd01eff1830734b1c28a4751a6b0182727afa957e072f7c7dc492d9448e502d1ae52deaaceb0dfce4b32a3c089cbdc3c7e18 *libxdiff-0.23.tar.gz
EOHASH

tar -xzf libxdiff-0.23.tar.gz
rm libxdiff-0.23.tar.gz
cd libxdiff-0.23
./configure
make
sudo make install
cd $SCRATCH_DIR
sudo pear install pecl/xdiff
rm -rf libxdiff-0.23

# Add the xdiff extension to php configuration
echo "extension=xdiff.so" | sudo tee /etc/php/7.2/mods-available/xdiff.ini > /dev/null
sudo phpenmod xdiff

# Restart apache so extension kicks in
sudo systemctl restart apache2

# Add the php libraries using the libraries api
wget -O geshi.tar.gz http://sourceforge.net/projects/geshi/files/latest/download
tar -xzf geshi.tar.gz -C $DRUPAL_ROOT/sites/all/libraries

# Pull down the code from github
cd $DRUPAL_ROOT/sites/all/modules
if [ -d islandora_object_lock ]; then
  cd islandora_object_lock
  git checkout 7.x
  git pull
  cd $DRUPAL_ROOT/sites/all/modules
else
  git clone https://github.com/discoverygarden/islandora_object_lock.git
fi
if [ -d islandora_xquery ]; then
  cd islandora_xquery
  git checkout 7.x
  git pull
else
  git clone https://github.com/discoverygarden/islandora_xquery.git
fi
drush @sites en -y islandora_object_lock
drush @sites en -y islandora_xquery
drush vset --exact islandora_xquery_basex_executable $(which basex)
drush @sites cc all -y

#Go home.
cd /
rm -rf $SCRATCH_DIR
