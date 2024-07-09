#!/bin/bash

python --version > /dev/null 2>&1
if [ $? -ne 0 ]; then
	echo "Python isn't installed"
	echo "Installing Python..."
	
	mkdir -p Thirdparty/Python/Linux

	wget https://www.python.org/ftp/python/3.12.4/Python-3.12.4.tar.xz -O Thirdparty/Python/Linux/python-3.12.4.tar.xz
	
	tar -xf Thirdparty/Python/Linux/python-3.12.4.tar.xz
	rm -rf Thirdparty/Python/Linux/python-3.12.4.tar.xz

	cd ./Thirdparty/Python/Python-3.12.4

	./configure
	make
	sudo make install

	cd ..
fi

echo "Python is installed"
echo "Setting up project..."
./Scripts/Linux/linux_setup.sh

