# Islandora XQuery [![Build Status](https://travis-ci.org/discoverygarden/islandora_xquery.png?branch=7.x)](https://travis-ci.org/discoverygarden/islandora_xquery)

## Introduction

This module utilizes Zorba to run XQueries agains XML documents stored in Fedora.

## Requirements

In addition to a functioning Islandora instance with the Libraries API, this module requires the following modules/libraries:
* [Zorba](http://www.zorba.io/home)
* [libxdiff](http://www.xmailserver.org/xdiff-lib.html)
* [PHP xdiff Extension](http://www.php.net/manual/en/intro.xdiff.php)
* [GeSHi](http://qbnz.com/highlighter/)
* [Islandora Object Lock](https://github.com/discoverygarden/islandora_object_lock)

## Installation

Here’s an example installation script for Ubuntu 12.04.  Your mileage may vary with other distros/versions.  This script will handle installing the islandora_xquery module and all of its dependencies.  It assumes that you have a functioning Islandora install with Drush.

Basic Algorithm:
* [Install Zorba](http://www.zorba.io/documentation/latest/zorba/install)
* [Compile libxdiff from source (./configure, make, make install)](http://www.xmailserver.org/xdiff-lib.html)
* [Install PHP xdiff extension](http://www.php.net/manual/en/xdiff.setup.php)
* Install GeSHi using the Libraries API (put geshi folder in sites/all/libraries)
* [Install Islandora Object Lock Module](https://github.com/discoverygarden/islandora_object_lock)
* Install Islandora Xquery Module

```bash
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
git clone https://github.com/daniel-dgi/islandora_xquery.git
drush en islandora_object_lock
drush en islandora_xquery

#Go home.
cd ~
```

## Configuration

Templates can be added for re-use at /admin/islandora/tools/xquery/manage.

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers/Sponsors
Current maintainers:

* [discoverygarden](https://github.com/discoverygarden)

This project has been sponsored by:

* [The Canadian Writing Research Collaboratory](http://www.cwrc.ca/en/)
The Canadian Writing Research Collaboratory is an online project designed to
enable unprecedented avenues for studying the words that most move people in and
about Canada.

## Development

If you would like to contribute to this module, please check out our helpful
[Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers)
info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
