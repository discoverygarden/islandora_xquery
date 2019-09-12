#!/bin/bash

# This script will install islandora_xquery and it's dependencies. Due to the
# level of security imposed on production servers, it has to be run as root.

#{{{ setup variables

boldTXT="\e[1m"
noBoldTXT="\e[0m"
yellowTXT='\033[0;33m'
greenTXT='\033[0;32m'
redTXT='\033[0;31m'
ncTXT='\033[0m'

drupalRoot=${1:-/var/www/drupal7}
myOS=Ubuntu

if [ -f /etc/redhat-release ]; then
  myOS=Redhat
fi


scratchDir=$(mktemp -d)
BaseX924Checksum="595e422495b86578692bc47396ad4132a739a19659be26459b8771a4246c50fb2d21b0516688dd8717f0fb023efa77691d0b5b51120326b99e71826f7ba65705"
geshiChecksum="98503bf012d69adee6d9f792bf29bc1d4a5ee54ddc86659b416f0610feea084018dbe77884e58cfa4726cb517750f302858b61bc56130ac9de9c457727bdcef9"
libxdiffChecksum="b6bae5ee9d21e615d7beeacd9ceefd01eff1830734b1c28a4751a6b0182727afa957e072f7c7dc492d9448e502d1ae52deaaceb0dfce4b32a3c089cbdc3c7e18"

case "$myOS" in
  'Ubuntu')
    majorRelease=$(lsb_release -r | cut -s -f2 | cut -d"." -f1)
    apacheServiceName="apache2"
    packageMGR=apt
    packages="basex build-essential php-dev php-pear"
    ;;
  'Redhat')
    majorRelease=$(hostnamectl | grep Kernel | rev | cut -d"." -f2 | cut -d"l" -f1)
    apacheServiceName="httpd"
    packageMGR=yum
    packages="php-devel php-pear php-pecl-xdiff wget"
    packageGroups="Development Tools"
    ;;
  *)
    echo "Can't figure out what OS I am running on"
    exit 1
    ;;
esac

#}}}
# {{{ checkSafe()

checkSafe()
{
  if [ "$1" -ne "0" ]; then
    if [ "$2" == "" ]; then
      echo -e "${redTXT}Failed.${ncTXT}"
      else
        echo -e ${redTXT}$2${ncTXT}
    fi
    exit $1
  fi
}

# }}}
#{{{ sanityCheck()

sanityCheck()
{
  test -d $drupalRoot
  checkSafe $? "$drupalRoot does not exist!"
  if [ "$myOS" = "Ubuntu" -a "$majorRelease" -lt "18" ] || [ "$myOS" = "Redhat" -a "$majorRelease" -lt "7" ]; then
    checkSafe $majorRelease "This script won't work on this version of $myOS"
  fi
}

#}}}
#{{{ installPackages()

installPackages()
{
  $packageMGR install -y $packages
  if [ -n "$packageGroups" ]; then
    $packageMGR groupinstall -y "$packageGroups"
  fi
}

#}}}
#{{{ libxdiffInstall()

libxdiffInstall()
{
  cd $scratchDir

  # Compile xdiff PHP extension and dependencies
  wget http://www.xmailserver.org/libxdiff-0.23.tar.gz
  sha512sum libxdiff-0.23.tar.gz | grep $libxdiffChecksum
  checkSafe $? "Checksum fail for libxdiff download"

  tar -xzf libxdiff-0.23.tar.gz
  rm libxdiff-0.23.tar.gz
  cd libxdiff-0.23
  ./configure
  make
  sudo make install
}

#}}}
#{{{ phpxdiffInstall()

phpxdiffInstall()
{
  if [ "$myOS" = "Ubuntu" ]; then
    sudo pear install pecl/xdiff

    # Add the xdiff extension to php configuration
    echo "extension=xdiff.so" | sudo tee /etc/php/7.2/mods-available/xdiff.ini > /dev/null
    sudo phpenmod xdiff
  fi

  # Restart apache so extension kicks in
  sudo systemctl restart $apacheServiceName
}

#}}}
#{{{ basexInstall()

basexInstall()
{
  if [ "$myOS" = "Redhat" ]; then
    cd $scratchDir
    wget http://files.basex.org/releases/9.2.4/BaseX924.zip
    sha512sum  BaseX924.zip | grep $BaseX924Checksum
    checkSafe $? "Checksum fail for BaseX9 download"
    unzip BaseX924.zip
    mv basex /opt/
    ln -s /opt/basex/bin/basex /usr/local/bin/basex
  fi
}

#}}}
#{{{ geshiInsall()

geshiInstall()
{
  # Add the php libraries using the libraries api
  cd $scratchDir
  wget -O geshi.tar.gz http://sourceforge.net/projects/geshi/files/latest/download
  sha512sum geshi.tar.gz | grep $geshiChecksum
  checkSafe $? "Checksum fail for geshi download"
  tar -xzf geshi.tar.gz -C $drupalRoot/sites/all/libraries
}

#}}}
#{{{ moduleSetup

moduleSetup()
{
  # Pull down the code from github
  cd $drupalRoot/sites/all/modules
  if [ -d islandora_object_lock ]; then
    cd islandora_object_lock
    git checkout 7.x
    git pull
    cd $drupalRoot/sites/all/modules
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
  drush @sites -y vset --exact islandora_xquery_basex_executable $(which basex)
  drush @sites -y vset --exact islandora_xquery_implementation 'basex'
  drush @sites updb -y
}

#}}}
#{{{ cleanup()

cleanup()
{
#Go home.
cd /
rm -rf $scratchDir
}

#}}}

sanityCheck
installPackages
libxdiffInstall
phpxdiffInstall
geshiInstall
basexInstall
moduleSetup
cleanup
