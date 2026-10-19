some notes about how to connect to the raspberry pi in visual studio 2026:

1. the ssh key: it's gotta be ecdsa. this is an option that you can select in puttygen, sorta near the bottom. `nistp256` is fine imo.
1. visual studio needs to have the private key in openssh format. so that's the "Export OpenSSH key" option in the "Conversions" dropdown.
	- save this as `%USERPROFILE%\.ssh\id_ecdsa_vs`
1. normal ssh tools (such as ssh) need the key in the "Export OpenSSH key (force new file format)" format (also in the "Conversions" dropdown).
1. putty needs the private key in the putty format, which you can get with the "Save private key" button on the main puttygen screen.
1. in all cases, the public key (which goes in the pi) needs to be the thing that's inside of the box that says "Public key for pasting into OpenSSH authorized_keys file" in puttygen.

the connection manager gui is kinda fickle, so just use the cli.
1. use the "Developer Command Prompt" terminal that you get in vs.
1. `connectionmanager list` to see if the pi is already in there.
1. `connectionmanager remove-all` to simply wipe everything.
	- sometimes the cache gets broken and this is an adequate way to reset it.
1. `connectionmanager add <the-username-on-the-pi>@<a-reachable-hostname-or-ip-for-the-pi> --privatekey "%USERPROFILE%\.ssh\id_ecdsa_vs"`
	- this should add the pi without fighting you.
