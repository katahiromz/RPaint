#!/usr/bin/env bash

cp -fr ../reactos/base/applications/mspaint/lang/ .
cp -fr ../reactos/base/applications/mspaint/help/ .
cp -fr ../reactos/base/applications/mspaint/icons/ .
cp -fr ../reactos/base/applications/mspaint/*.cpp .
cp -fr ../reactos/base/applications/mspaint/*.h .
cp -fr ../reactos/base/applications/mspaint/*.rc .
cp -fr ../reactos/output-MinGW-i386/sdk/include/reactos/version.h reactos/
cp -fr ../reactos/output-MinGW-i386/sdk/include/reactos/buildno.h reactos/
