#Go home.
cd ~

# Get zorba
sudo add-apt-repository ppa:juan457/zorba
sudo apt-get update
sudo apt-get install zorba

# Compile xdiff PHP extension and dependencies
wget http://www.xmailserver.org/libxdiff-0.23.tar.gz
tar -xzf libxdiff-0.23.tar.gz
rm libxdiff-0.23.tar.gz
cd libxdiff-0.23
./configure
make
sudo make install
cd ~
sudo pear install pecl/xdiff-1.5.2
rm -rf libxdiff-0.23

# Add the xdiff extension to php configuration
echo "extension=xdiff.so" > xdiff.ini
sudo cp xdiff.ini /etc/php5/conf.d
rm xdiff.ini

# Restart apache so extension kicks in
sudo service apache2 restart

# Add the php libraries using the libraries api
wget -O geshi.tar.gz http://sourceforge.net/projects/geshi/files/latest/download
tar xvf geshi.tar.gz
cp -r geshi /var/www/drupal7/sites/all/libraries
rm geshi.tar.gz
rm -rf geshi

# Pull down the code from github
cd /var/www/drupal7/sites/all/modules
git clone https://github.com/discoverygarden/islandora_object_lock.git
git clone https://github.com/discoverygarden/islandora_xquery.git
drush en islandora_object_lock
drush en islandora_xquery

#Go home.
cd ~
