#!/bin/bash

pushd "$(dirname "$0")/../.."

./Thirdparty/premake/bin/linux/premake5 gmake

popd