#!/bin/sh

VERSION="1.7.1"
# Initial configuration
DOJO_RELEASE_URL="http://download.dojotoolkit.org/release-${VERSION}/dojo-release-${VERSION}.tar.gz"
DOJO_SRC_URL="http://download.dojotoolkit.org/release-${VERSION}/dojo-release-${VERSION}-src.tar.gz"


wget -O dojo_release.tar.gz $DOJO_RELEASE_URL
wget -O dojo_src.tar.gz $DOJO_SRC_URL

mkdir compiled

tar -xvzf dojo_release.tar.gz
tar -xvzf dojo_src.tar.gz

mv dojo-release-${VERSION}/dojo compiled/
mv dojo-release-${VERSION}/dijit compiled/
mv dojo-release-${VERSION}/dojox compiled/

mv dojo-release-${VERSION}-src/dojo ./
mv dojo-release-${VERSION}-src/dojox ./
mv dojo-release-${VERSION}-src/dijit ./
mv dojo-release-${VERSION}-src/util ./

rm -rf dojo-release-${VERSION}-src
rm -rf dojo-release-${VERSION}
rm -rf dojo_release.tar.gz
rm -rf dojo_src.tar.gz
