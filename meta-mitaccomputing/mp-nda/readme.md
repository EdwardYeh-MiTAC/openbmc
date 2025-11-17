The access to components under this directory required to have Non-disclosure agreement (NDA) with MiTAC and original source provider.


cat .gitmodules

[submodule "meta-mitaccomputing/mp-nda/meta-nda-mic-intel/map-intel-platforms"]
        path = meta-mitaccomputing/mp-nda/meta-nda-mic-intel/map-intel-platforms
        url = ssh://git@micgitlab1-ssh.mic.com.tw:1022/beoc/solution-enabling/openbmc/submodule/map-intel.git
[submodule "meta-mitaccomputing/mp-nda/meta-nda-mic-amd/map-amd-platforms"]
        path = meta-mitaccomputing/mp-nda/meta-nda-mic-amd/map-amd-platforms
        url = ssh://git@micgitlab1-ssh.mic.com.tw:1022/beoc/solution-enabling/openbmc/submodule/map-amd.git
[submodule "meta-mitaccomputing/mp-nda/meta-nda-mic/mitac-advanced-package"]
        path = meta-mitaccomputing/mp-nda/meta-nda-mic/mitac-advanced-package
        url = ssh://git@micgitlab1-ssh.mic.com.tw:1022/beoc/solution-enabling/openbmc/submodule/mitac-advanced-package-common.git

git submodule init meta-mitaccomputing/mp-nda/meta-nda-mic-intel/map-intel-platforms
Submodule 'meta-mitaccomputing/mp-nda/meta-nda-mic-intel/map-intel-platforms' (ssh://git@micgitlab1-ssh.mic.com.tw:1022/beoc/solution-enabling/openbmc/submodule/map-intel.git) registered for path 'meta-mitaccomputing/mp-nda/meta-nda-mic-intel/map-intel-platforms'

git submodule init meta-mitaccomputing/mp-nda/meta-nda-mic-amd/map-amd-platforms
Submodule 'meta-mitaccomputing/mp-nda/meta-nda-mic-amd/map-amd-platforms' (ssh://git@micgitlab1-ssh.mic.com.tw:1022/beoc/solution-enabling/openbmc/submodule/map-amd.git) registered for path 'meta-mitaccomputing/mp-nda/meta-nda-mic-amd/map-amd-platforms'

git submodule init meta-mitaccomputing/mp-nda/meta-nda-mic/mitac-advanced-package
Submodule 'meta-mitaccomputing/mp-nda/meta-nda-mic/mitac-advanced-package' (ssh://git@micgitlab1-ssh.mic.com.tw:1022/beoc/solution-enabling/openbmc/submodule/mitac-advanced-package-common.git) registered for path 'meta-mitaccomputing/mp-nda/meta-nda-mic/mitac-advanced-package'

git submodule update
Cloning into '..../openbmc/meta-mitaccomputing/mp-nda/meta-nda-mic-amd/map-amd-platforms'...
Enter passphrase for key '/home/buildbot/.ssh/id_git':
Cloning into '..../openbmc/meta-mitaccomputing/mp-nda/meta-nda-mic-intel/map-intel-platforms'...
Enter passphrase for key '/home/buildbot/.ssh/id_git':
Cloning into '..../openbmc/meta-mitaccomputing/mp-nda/meta-nda-mic/mitac-advanced-package'...
Enter passphrase for key '/home/buildbot/.ssh/id_git':
Submodule path 'meta-mitaccomputing/mp-nda/meta-nda-mic-amd/map-amd-platforms': checked out '5d35ae78705dfe04cc737c8c86361c0d5b00eddc'
Submodule path 'meta-mitaccomputing/mp-nda/meta-nda-mic-intel/map-intel-platforms': checked out 'a68f10c483f265dda9010e912237fd2f5d458960'
Submodule path 'meta-mitaccomputing/mp-nda/meta-nda-mic/mitac-advanced-package': checked out '91ab349addf5da40530b929a158c5ef61213fadf'

