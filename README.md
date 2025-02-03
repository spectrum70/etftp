![](etftp.png)


etftp is a simple tftp server for embedded systems, created to make the life easier comparing to known existing tftp servers, often not that small and generally hard to configure. 

## Build

```
git clone git@gitlab.com:spectrum70/etftp.git
cd etftp
make
sudo make install
```

## Usage

```
etftp v.0.93-gaa6cc2e
(C) 2021, kernel-space.org
Usage: etftp [OPTION]
Example: ./etftp -v
Options:
  -h,  --help        this help
  -p,  --path        server root path (def. /srv/tftp)
  -V,  --version     program version
  -v                 verbose
```

LICENSE: GPL2+

Author: Angelo Dureghello <angelo@kernel-space.org>

