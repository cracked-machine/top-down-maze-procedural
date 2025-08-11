
Note, if you are using Samba to access the .exe, you may get errors after subsequent builds, i.e. `the parameter is incorrect`.

This is a samba error. The dirty workaround is to restart the samba server after each build

```
sudo systemctl restart smb
```