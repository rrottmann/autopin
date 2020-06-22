# Autopin

Automatic pinentry to gpg. Useful to unlock a secure element like an OpenPGP card.

## License

MIT License. See separate document `LICENSE`.

## Compile

The following command may be used to compile autopin on Debian 10:

```bash
apt-get update
apt-get install -y build-essential git
d=`mktemp -d`
cd $d
git clone https://github.com/rrottmann/autopin.git
cd autopin
make
ls -al autopin
```

## Install

Just execute the following command From the directory where you compiled autopin:

```bash
sudo make install
```

This also installs the following utils:

* `autopin` - unlocks the security token automatically
* `checkpin` - unlocks the security token manually
* `forgetpin` - locks the security token
* `getcardkeyid` - gets the keyid of the GPG key on token
* `getcardserial` - gets the card serial of the token
* `pinentry-fake` - is used for entering the pin
* `reload-gpgagent` - reloads agents (e.g. when card hangs)

## Security

The PW1 USER-PIN is used to derive the key that encrypts the key material
on an OpenPGP card.
Using this tool to automatically unlock a secure element of course reduces
the security of the locked away private key.
It still can't be easily copied and you need to be in the posession of the
secure element. However an admin user on the system may interact with this tool
and get access to the pin and could migrate the token to a different system.
For some systems, like a backup server, this may not be an attack vector to
protect against and the need to automate the unlock process is more important.
To make this process simple, I have created autopin.

## Usage

On first start, the tool needs to create a file `/etc/pinentry-salt` that is
used to derive a pin. Also system parameters are sampled so that the generated
pin is different on each system - even when the salt is being copied.
At compile time there is also a pepper variable embedded into the resulting
binary in order to create more diverse pins.

This is just to protect against a stolen/removed key where the attacker had
no possibility to break into the system to execute `/usr/local/sbin/getpin`.

!!THIS IS NO REAL SECURITY! SEE ABOVE SECTION!!!

## Prepare system

```bash
apt-get install gnupg2 scdaemon -y
``` 

## Get the unique pin for this system

```bash
 /usr/local/sbin/getpin;echo
Gm2zr66n
```

## OpenPGP Card Default PINs

| PIN      | NAME      |
| -------- | --------- |
| 123456   | USER PIN  |
| 12345678 | ADMIN PIN |

## OpenPGP Card preparation

0. `gpg2 --card-edit`
1. Factory Reset `admin -> factory-reset`
2. Change ADMIN PIN `admin -> passwd-> 2`
3. Generate certificate `admin -> generate`
4. Create RESET CODE `admin -> passwd -> 4`
5. Change USER PIN `passwd`

## Automatic PIN entry Example - with debug output

This example creates a test message, encrypts it with
the key stored on the OpenPGP card.

Then it loads the unique pin for this system and unlocks
the card.

After that it decrypts the encrypted message.

And finally it forgets the entered pin.

To verify that the pin has been forgotten, another
decrypt attempt is being made that redirects to the
regular pinentry program.

```
# forgetpin
OK
ERR 67125247 End of file <GPG Agent>
gpg-agent[2477]: SIGHUP received - re-reading configuration and flushing cache
OK
Could not open a connection to your authentication agent.
SSH_AGENT_PID not set, cannot kill agent
#  echo abc > /tmp/test.txt
# keyid=`getcardkeyid`
# gpg --encrypt --armor --recipient $keyid /tmp/test.txt
# autopin
gpg-agent[4701]: listening on socket '/root/.gnupg/S.gpg-agent'
gpg-agent[4701]: listening on socket '/root/.gnupg/S.gpg-agent.extra'
gpg-agent[4701]: listening on socket '/root/.gnupg/S.gpg-agent.browser'
gpg-agent[4701]: listening on socket '/root/.gnupg/S.gpg-agent.ssh'
gpg-agent[4702]: gpg-agent (GnuPG) 2.1.18 started
gpg-agent[4702]: handler 0x7f9b0c51f700 for fd 8 started
gpg-agent[4702]: no running SCdaemon - starting it
gpg-agent[4702]: DBG: first connection to SCdaemon established
gpg-agent[4702]: handler 0x7f9b0c51f700 for fd 8 terminated
gpg-agent[4702]: handler 0x7f9b0c51f700 for fd 8 started
gpg-agent[4702]: new connection to SCdaemon established (reusing)
ERR 67141741 Broken pipe <GPG Agent>
gpg-agent[4702]: handler 0x7f9b0c51f700 for fd 8 terminated
gpg-agent[4702]: handler 0x7f9b0c51f700 for fd 8 started
gpg-agent[4702]: no running SCdaemon - starting it
gpg-agent[4702]: DBG: first connection to SCdaemon established
gpg-agent[4702]: handler 0x7f9b0c51f700 for fd 8 terminated
gpg-agent[4702]: handler 0x7f9b0c51f700 for fd 8 started
gpg-agent[4702]: new connection to SCdaemon established (reusing)
gpg-agent[4702]: starting a new PIN Entry
OK
gpg-agent[4702]: handler 0x7f9b0c51f700 for fd 8 terminated
# gpg --decrypt  /tmp/test.txt.asc
gpg-agent[4702]: handler 0x7f9b0c51f700 for fd 8 started
gpg-agent[4702]: new connection to SCdaemon established (reusing)
gpg-agent[4702]: DBG: detected card with S/N D276000124010200FFFE870933210000
gpg: encrypted with 2048-bit RSA key, ID 6EBACB92B9127517, created 2020-06-21
      "Secure Element"
abc
gpg-agent[4702]: handler 0x7f9b0c51f700 for fd 8 terminated
# forgetpin
gpg-agent[4702]: handler 0x7f9b0c51f700 for fd 8 started
gpg-agent[4702]: new connection to SCdaemon established (reusing)
OK
ERR 67125247 End of file <GPG Agent>
gpg-agent[4702]: handler 0x7f9b0c51f700 for fd 8 terminated
gpg-agent[4702]: handler 0x7f9b0c51f700 for fd 8 started
gpg-agent[4702]: SIGHUP received - re-reading configuration and flushing cache
OK
Could not open a connection to your authentication agent.
SSH_AGENT_PID not set, cannot kill agent
# gpg --decrypt  /tmp/test.txt.asc
gpg-agent[4702]: DBG: detected card with S/N D276000124010200FFFE870933210000
gpg-agent[4702]: smartcard decryption failed: Operation cancelled
gpg-agent[4702]: command 'PKDECRYPT' failed: Operation cancelled <Pinentry>
gpg: encrypted with 2048-bit RSA key, ID 6EBACB92B9127517, created 2020-06-21
      "Secure Element"
gpg: public key decryption failed: Operation cancelled
gpg: decryption failed: No secret key
```
