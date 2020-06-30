# Autopin

Automatic pinentry to gpg. Useful to unlock a secure element like an OpenPGP card
(e.g. Fellowship Smartcard, Gnuk Token, Yubikey).

## License

MIT License. See separate document `LICENSE`.

### Used Libraries

The source uses the following libraries:

* base64 -  by Joe DF (joedf@ahkscript.org) - MIT License
* hmac-sha256 - Copyright (C) 2017 Adrian Perez <aperez@igalia.com> - MIT License
* sha256 - Igor Pavlov - Public domain 

## Compile

The following commands may be used to compile autopin on Debian 10:

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

* `addsecret` - creates a random secret in the kernel user keyring
* `autopin` - unlocks the security token automatically
* `blockpin`- blocks the user pin of opengpg card
* `checkpin` - unlocks the security token manually
* `counter-inc` - increase a monotonic counter
* `counter-read` - read a monotonic counter
* `clearkeyring` - clears all keys from kernel user keyring
* `dice5` - roll 5 dice using trng data (useful for diceware)
* `dumpkeyring` - creates csv dump of the kernel user keyring
* `forgetpin` - locks the security token
* `forgetsecret` - removes a secret from kernel user keyring
* `gennonce` - generate a nonce from trng data (sha256)
* `gensecret` - generate a secret from trng data (base64)
* `getsecret` - gets a secret from kernel user keyring
* `getcardkeyid` - gets the keyid of the GPG key on token
* `getcardserial` - gets the card serial of the token
* `loadkeyring` - loads a previous csv dump in the kernel user keyring
* `pinentry-fake` - is used for entering the pin
* `reload-gpgagent` - reloads agents (e.g. when card hangs)
* `sealdata` - seal sensitive data to the secure element (encrypt)
* `unsealdata` - access sensitive data that has been sealed (decrypt)

Even advanced users that know gpg and keyctl by heart might find these
utils helpful as they provide a convenient shortcut for day to day use.

The utils will be improved over times when they behave in a strange way.
They are not using sophisticated error handling and are basically shell
hacks and snippets that I found interesting enough to save as a script.

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

### Threat Model

A hacker might get access to the OS and key material should be protected
by the presence of an OpenPGP compliant security token. Key material should
therefore be encrypted when not in use. Only the root account should unseal
secrets and work with them in a session where the security token is present.
When automation is used, the server and token are protected against access.
No access from the internet. The token is only used for a specific purpose:
Cheap mans HSM in a locked cabinet. When physical access is not secured,
the token remains with the admin when not in use.

## Usage

On first start, the tool needs to create a file `/etc/pinentry-salt` that is
used to derive a pin. Also system parameters are sampled so that the generated
pin is different on each system - even when the salt is being copied.
At compile time there is also a pepper variable embedded into the resulting
binary in order to create more diverse pins.

This is just to protect against a stolen/removed key where the attacker had
no possibility to break into the system to execute `/usr/local/sbin/getpin`.

See the threat model in above section.

## Prepare system

```bash
apt-get install gnupg2 scdaemon keyutils -y
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
2. Change ADMIN PIN `admin -> passwd-> 3`
3. Generate new key `admin -> generate`
4. Create RESET CODE `admin -> passwd -> 4`
5. Change USER PIN `passwd`

If you just use the USER PIN and do not generate a RESET CODE and
block the ADMIN PIN with 3 wrong password entries, the OpenPGP card
needs to be factory reset when the USER PIN entries get exhausted.

**Please note:** Not all OpenPGP implementations feature a factory reset
(in oder to rendet  the value of the card to zero in case of theft/loss)!

**Please further note:** The OpenPGP card does not hold enough information
to recreate the public RSA key. So you need to backup it!

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
## Other usage examples

### Generate nonce

```bash
# gennonce
26572734721428444495398020545927618012015
```

### Generate secret

```bash
# gensecret
RCCJPnqbahi6pSot2zvQCIx8YCUwRP2w51B/gsuHopJe85efCk9LCg==
```

### Seal and unseal data

```bash
# echo abc | sealdata
-----BEGIN PGP MESSAGE-----

hQEMA/UOEOh5tLUDAQf8DGvu8rmujRac1tq4AzHMYWLie6F4ISKBneT6rgCvgNJY
cu31u8AaTH43ua8aMC2g7hpUVtuaJ9R2LXPVfyptbuVs+jNtB1FBUAdsiNoyQ9Tl
hNd1qa4T+hlKuzU3dXScuqLZrFwdRUlixHFun4XAbTjGZwhUIYPQA9T7OvDzz8dW
oQ2AjccGiKJsT8qaPjkIYTdsOGE29cRjyuCk5VPS0YaLAnwiVyWeG3g5Q+OHSaV4
IV6wR084liApUIoJMx+fYxGWVVEF/QQlX9dALLBQ0GMkZwygliHtJoVsQl3uQrqe
CxvV7KdFLfl6sZ8pYrJIops75wiudTsL5jpFxhoOmNI/AcLvEPM76arGvRmZU3Rc
b6RFKCqyE/nyVSMOHa0b5oNuPXISsE3qvqZFHlz7j3zkO7L3yvmTLU7Clt4za+ce
=dqD9
-----END PGP MESSAGE-----
# echo abc | sealdata > /dev/shm/sealed.dat
# cat /dev/shm/sealed.dat | unsealdata
abc
# echo abc | sealdata | unsealdata
abc
# gensecret | sealdata
-----BEGIN PGP MESSAGE-----

hQEMA/UOEOh5tLUDAQf/aekBH2z5MlDd/CSuJFtiAQAuqKJJyJytDMwavuY9kflC
Q88YJtc6cwFK/1bEDQ1O26UJpAUYRHNODgCQS+PboCQrb1rPLThPnyBN1q7ERBL7
8fpr1oPGzjeexxu5KIyKAfeBAQuYL6l2cc3mZsNGwaFYrBK21UOmG8NBpl4VnbMT
SW345gPCRtNRe7LHHbbGjJP8uqVOYVin8dE8qvlgPt69pmNuBGVM1gJwjU3Acy3s
MadHuL6B03VwaiHlqJRfTXf/u0euxAPyEIf7mCVkWemOYF9KEZrLruZyjNa0yzPZ
isoR0D8VZRa5rovwZFm555vL19gAIMbfj/OxKXjECNJwAS4O4IGyRpS87xM7NpOR
WExFDKlQLO7eE9Syx+q3347lShrBf42Gp5Ymj/VJ2l7/Zxpy9Mu/0MLtw+lgO5df
EteAYWXx6fH4IS7L234BFNCBMokxniC6SZnqcYpOs00Pz3C7Oaika5aP5KYrANQ1
JA==
=MRL6
-----END PGP MESSAGE-----
```

### Roll 5 dices

This is useful for generating 5 dice throws from TRNG data.
It may be used with a diceware dictionary to generate easy
to remember but secure passphrases

```bash
# dice5
61525
```

### Dump and Load Kernel Keyring

The kernel keyring does not store key entries after reboot
and may also expire keys from time to time.

In order to preserve them, they can be dumped and encrypted
using the OpenPGP card.

Later, the keys may be loaded again.

```bash
# clearkeyring
# keyctl list @u
keyring is empty
# addkey foo
208415637
# addkey bar
834132960
# addkey baz
453959846
# dumpkeyring 
foo;RCBYmyCIQp9Qw6xx4x0IREQ7g3R1n/55AQ+xpc2MsnDMhQpPSwo=
baz;RCC6057Tu5shWcB8cLvu4imYjT5mIyyRXizmVBSyDD7zogpPSwo=
bar;RCDZ0lxWXE4bCy0AdQinRttywF2suPSeKp1sTc1NYTJvQwpPSwo=
# dumpkeyring | sealdata /dev/shm/sealed.dat
# cat /dev/shm/sealed.dat 
-----BEGIN PGP MESSAGE-----

hQEMA/UOEOh5tLUDAQgAn3lhG2cJb6P5of7sM5XI+eO7yQAxSf1eKp/+3MbVaRg6
piezrLZ4SU5wQCJbzJecsTS1q3vKARNYAdKBzV/sO8AjZSUTFA1djMGUDKaLeXhV
epZkDVplMqvSuygLFSWAVy0Hpl0pqPeTpHU1K118JoBaQnxuy64n9sWBDPzilsfD
LeqRQvIhfsua80QGaqcQIStgLIIj3PDmw4cExlq4nHlkYyFGVEQtdhwLjqmVz0e1
5GpYVLy9f4VN8lxl3CWo1zrjzP7cluODIJWkFuHj9D1ngs92GrsEMsnppSGPXw5i
YoK5QGR+lY+V9boOtDPLN1gr/wqx+0QwEgxiMmRpCtLAGAFWUOcn3j+MGqrd7LS7
/McYNMO29XXre0FdJEL7NS6uRKuTzkCtR5HcnTywnu/TrPxrKC+w5L34GTfEzigG
y6+pKjyyiWRsqMeRN2L1zZNh8+fP33YajhJ0qiBvYRADaJPSWMM7TkRGSd+zE+dg
5BTTrk4bToOpyb2qjXXfDo+jePvQPH1JHekwrCsoNTM9jF3NLKdoxQkep4XdVrkf
7XA1FGNo92R/5CC2pl0G+Qxu0O8gxn1sJfba7HKwm3fld1CU1FGVV+Qrx2VfTh29
6KQWc1l9eoYT7A==
=SNAB
-----END PGP MESSAGE-----
# clearkeyring
# keyctl list @u
keyring is empty
# autopin
# unsealdata /dev/shm/sealed.dat
foo;RCBYmyCIQp9Qw6xx4x0IREQ7g3R1n/55AQ+xpc2MsnDMhQpPSwo=
baz;RCC6057Tu5shWcB8cLvu4imYjT5mIyyRXizmVBSyDD7zogpPSwo=
bar;RCDZ0lxWXE4bCy0AdQinRttywF2suPSeKp1sTc1NYTJvQwpPSwo=
# unsealdata /dev/shm.sealed.dat | loadkeyring
289946937
182106117
27723911
# keyctl list @u
3 keys in keyring:
289946937: --alswrv     0     0 user: foo
182106117: --alswrv     0     0 user: bar
 27723911: --alswrv     0     0 user: baz
```

### Create, Retrieve and Forget Secrets

The following commands show how to easily create, retrieve and forget
ephemeral secrets in the kernel user keyring. 

Of course, you may dump and seal them away. See previous section.

```bash
# clearkeyring 
# keyctl list @u
keyring is empty
# addsecret test
904805390
# keyctl list @u
1 key in keyring:
904805390: --alswrv     0     0 user: test
# getsecret test
RCBAxGS05Qlz/sEacv7M7zhybriwMUAO8tVdU8pK3+clMjWnCk9LCg==
# forgetsecret test
1 links removed

# keyctl list @u
keyring is empty
```

### Block and Unblock the USER PIN

When you have the means to unblock the USER PIN, you
might want to block it to temporarily disable the use
of the OpenPGP card.

This might also be useful if you ship an embedded device
to a customer. On a separate channel you may transmit the
RESET CODE or give remote assistance in oder to unblock
the OpenPGP card.

Another scenario would be an emergency access that first
needs to be unlocked by the helpdesk and is disabled otherwise.

```bash
blockpin
```

In order to reset the USER PIN, the card must support this!
Also Factory Reset is not enabled on very OpenPGP card.

So you might ask the card vendor and test this before!

For resetting it, you need the regular pinentry program.
So you need to reload the gpg-agent.

```bash
forgetpin
gpg --card-edit
```

### Read and Increase a Counter

An OpenPGP card posesses an signature counter that can be
repurposed as a strictly monotone counter.

```bash
# counter-read
6
# counter-inc
# counter-read
7
# counter-inc
# counter-read
9
```

**Please note:** The counter-inc just makes sure that the
signature counter increases. Other operations happening between
`counter-read` commands may also increase the counter.

Such counters may be useful to prevent rollback or replay attacks.

As the counter is backed by a regular EEPROM, you may not expect
durability as with counters on a dedicated security element.

The speed is also not that good as basically some random data
gets signed in the background.
