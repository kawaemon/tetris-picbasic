#!/bin/sh -exu

wget https://download.jetbrains.com/fonts/JetBrainsMono-2.242.zip
unzip JetBrainsMono-2.242.zip -d temp
mv temp/fonts/ttf/JetBrainsMono-Medium.ttf .
rm -rf temp JetBrainsMono-2.242.zip
