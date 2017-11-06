## Dependencies

| Library      | Purpose           | Description                       |
| ------------ | ----------------- | --------------------------------- |
| libssl       | SSL Support       | Secure communications             |
| libdb4.8     | Berkeley DB       | Blockchain & wallet storage       |
| libboost     | Boost             | C++ Library                       |
| miniupnpc    | UPnP Support      | Optional firewall-jumping support |
| libqrencode  | QRCode generation | Optional QRCode generation        |

## Compile Options

**miniupnpc** may be used for UPnP port mapping. Set USE_UPNP to control this.

**libqrencode** may be used for QRCode image generation. Set USE_QRCODE to control this.

| Option       | Description |
| ------------ | ----------- |
| USE_UPNP=0   | (the default) UPnP support turned off by default at runtime |
| USE_UPNP=1   | UPnP support turned on by default at runtime |
| USE_QRCODE=0 | (the default) No QRCode support - libqrcode not required |
| USE_QRCODE=1 | QRCode support enabled |

## Dependency Build Instructions: Ubuntu & Debian

1. First ensure you have build essentials installed:

       sudo apt-get install build-essential
    
2. Install Boost:

       sudo apt-get install libboost-all-dev
    
3. Install Berklee DB:

       sudo apt-get install libdb-dev libdb++-dev
    
4. Install Open SSL:

       sudo apt-get install libssl1.0-dev
    
   * If this fails to install because your linux system still supports 0.X
     versions of openssl use this command:
   
         sudo apt-get install libssl-dev
    
5. Install Mini UPNP:

       sudo apt-get install libminiupnpc-dev
    
6. Install QrenCode:

       sudo apt-get install libqrencode-dev
    
7. For the Qt, install QT Framework:

       sudo apt-get install qt4-qmake libqt4-dev

* If you get a failed to build because "-lz" was not found this is due to the
fact that your linux operating system is trying to use a 64 bit version of
zlib. You can solve this by installing the 32 bit version here:

       sudo apt-get install lib32z1-dev

## Build Instructions (Daemon): Ubuntu & Debian

    cd ~
    git clone https://github.com/Nexusoft/Nexus.git Nexus
    cd Nexus
    make -f makefile.unix

To execute:

    ./nexus

## Build Instructions (Qt): Ubuntu & Debian

    cd ~
    git clone https://github.com/Nexusoft/Nexus.git Nexus
    cd Nexus
    qmake nexus-qt.pro "RELEASE=1" "USE_UPNP=1"
    make

To execute:

    ./nexus-qt


## Build Instructions (Daemon): Arch Linux

1. Install the dependencies:

   ```
   pacman -S base-devel boost db openssl-1.0 miniupnpc qrencode
   ```

2. Downgrade packages

   Arch Linux uses the latest packages which will cause issues with the
   compile. Arch has GCC 7 as the compiler and OpenSSL version is 1.1.

   [Downgrade](https://wiki.archlinux.org/index.php/Downgrading_packages) GCC
   to 6.3 by finding or downloading the required package files and installing
   them:

    ```
    pacman -U gcc-6.3.1-2-x86_64.pkg.tar.xz gcc-libs-6.3.1-2-x86_64.pkg.tar.xz gcc-fortran-6.3.1-2-x86_64.pkg.tar.xz
    ```

3. Clone the git repo
   
   ```
   git clone https://github.com/Nexusoft/Nexus.git Nexus
   ```

4. Edit the file makefile.unix

   To make the compiler use OpenSSL 1.0 instead of the default 1.1 add the
   following lines to makefile.unix:
   ```
   OPENSSL_INCLUDE_PATH=/usr/include/openssl-1.0/
   OPENSSL_LIB_PATH=/usr/lib/openssl-1.0/
   ```

   Arch Linux does not allow static linking to some of the libraries. To
   circumvent this make sure that static linking is disabled by commenting the
   line:
   ```
   # LMODE = static
   ```

   Then check that there is a line that sets dynamic linking:
   ```
   LMODE = dynamic
   ```

5. Build and launch the daemon:
   ```
   make -f makefile.unix
   ./nexus
   ```
