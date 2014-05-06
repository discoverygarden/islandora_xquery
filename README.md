# Islandora XQuery [![Build Status](https://travis-ci.org/discoverygarden/islandora_xquery.png?branch=7.x)](https://travis-ci.org/discoverygarden/islandora_xquery)

## Introduction

This module utilizes Zorba to run XQueries against XML documents stored in Fedora.

## Requirements

In addition to a functioning Islandora instance with the Libraries API, this module requires the following modules/libraries:
* [Zorba](http://www.zorba.io/home)
* [libxdiff](http://www.xmailserver.org/xdiff-lib.html)
* [PHP xdiff Extension](http://www.php.net/manual/en/intro.xdiff.php)
* [GeSHi](http://qbnz.com/highlighter/)
* [Islandora Object Lock](https://github.com/discoverygarden/islandora_object_lock)

## Installation

Basic Algorithm:
* [Install Zorba](http://www.zorba.io/documentation/latest/zorba/install)
* [Compile libxdiff from source (./configure, make, make install)](http://www.xmailserver.org/xdiff-lib.html)
* [Install PHP xdiff extension](http://www.php.net/manual/en/xdiff.setup.php)
* Install GeSHi using the Libraries API (put geshi folder in sites/all/libraries)
* [Install Islandora Object Lock Module](https://github.com/discoverygarden/islandora_object_lock)
* Install Islandora Xquery Module

Here’s [an example installation script](resources/install_islandora_xquery.sh) for Ubuntu 12.04.  Your mileage may vary with other distros/versions.  This script will handle installing the islandora_xquery module and all of its dependencies.  It assumes that you have a functioning Islandora install with Drush.

## Configuration

Templates can be added for re-use at /admin/islandora/tools/xquery/manage.

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

### Warning!
The default Zorba implementation uses the Saxon processor, which WILL unescape your xml entities!  For example, even the lowly identity transform will change ```<dc:title>&quot;Things and Stuff&quot;</dc:title>``` to  ```<dc:title>”Things and Stuff”</dc:title>```.  You will get some brand spanking new quotes you never asked for!  You’ve been warned!

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
