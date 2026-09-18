#!/bin/bash

# Add local user
# Either use the LOCAL_USER_ID if passed in at runtime or
# fallback

USER_ID=${LOCAL_USER_ID:-9001}

echo "Starting with UID : $USER_ID"
groupadd -g ${USER_ID} guser 2>/dev/null || true
useradd -m -s /bin/bash -u $USER_ID -G guser user 2>/dev/null || true
echo "user ALL=(ALL) NOPASSWD:ALL" >>/etc/sudoers

export HOME=/home/user
echo 'export PATH=$PATH:/opt/riscv-gnu-toolchain/bin' >> /home/user/.bashrc
chown user:guser /home/user/.bashrc

if [ $# -gt 0 ]; then
    exec gosu user "$@"
else
    exec gosu user bash
fi
