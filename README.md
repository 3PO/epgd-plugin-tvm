**epgd-plugin-tvm**


![epgd-plugin-tvm](http://dreipo.cc/tvm/tvmvdr2.png)

###TVM loader Plugin for EPGd.
Clone into the "PLUGINS" directory of your epgd source and build.

Here's an example:

```ini
cd </YOUR/EPGD_SOURCE/DIRECTORY>
cd PLUGINS
git clone https://github.com/3PO/epgd-plugin-tvm tvm
cd ..
make
make install
```

**Gentoo/Gen2VDR:**

Simply use the Overlay:

https://github.com/3PO/3PO-overlay


**Arch Linux:**

- ToDo


**yaVDR:**

```ini
sudo apt-get install build-essential devscripts
sudo apt-get build-dep vdr-epg-daemon
mkdir -p ~/src/epgd
cd  ~/src/epgd
apt-get source vdr-epg-daemon
cd vdr-epg-daemon*
git clone https://github.com/3PO/epgd-plugin-tvm PLUGINS/tvm
dch -l local "added plugin tvm"
dpkg-buildpackage -b -us -uc
sudo dpkg -i ../*.deb
```
