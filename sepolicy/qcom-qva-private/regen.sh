#!/bin/bash

# cleanup
find . -type l -delete

# symlink all files from device/qcom/sepolicy/qva/private
for f in $(find ../../../../../device/qcom/sepolicy/qva/private -type f); do
    # make sure that given file doesn't exist in local copy
    # so that we can override files if we wish to do so
    if [ ! -f $(basename ${f}) ]; then
        ln -sf ${f} .
    fi
done
