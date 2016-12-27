[![NPM Version][npm-image]][npm-url]
[![NPM Downloads][downloads-image]][npm-url]
[![Node.js Version][node-version-image]][node-version-url]


HDF5 is a node module for reading and writing the 
[HDF5](https://www.hdfgroup.org/HDF5/) file format.

## Documentation

API documentation is available at <http://hdf-ni.github.io/hdf5.node>

```bash
npm install hdf5 --fallback-to-build
```
See <http://hdf-ni.github.io/hdf5.node/doc/install-setup.html> for the native 
requirements and details. If your native hdf5 libraries aren't at the default 
you can set the path with --hdf5_home_linux switch on tis project as well as 
dependent projects.

```bash
npm install hdf5 --fallback-to-build --hdf5_home_linux=<your native hdf path>
```
For mac and windows the switches are --hdf5_home_mac & --hdf5_home_win

Note: If node-gyp isn't installed

```bash
npm install -g node-gyp
```

```javascript
var hdf5 = require('hdf5').hdf5;

var Access = require('hdf5/lib/globals').Access;
var file = new hdf5.File('/tmp/foo.h5', Access.ACC_RDONLY);
var group = file.openGroup('pmc');
```

### Notes on Recent Releases

* Note: Release v0.1.0 was built with nodejs v4.2.1. If you want nodejs v0.12.x 
then stay with  release v0.0.20. npm will continue with nodejs v4.x.x line and 
any fixes or features needed by prior versions will be from github branches.

* Note: Release v0.0.20 is for prebuilts with hdf5-1.8.15-patch1. If you want
hdf5-1.8.14 stay with v0.0.19.

## Philosophy

This module, [hdf5.node][npm-url], is intended to be a pure API for reading
and writing HDF5 files. Graphical user interfaces or other layers should
be implemented in separate modules.

Unlike other languages that wrap hdf5 API's this interface takes advantage of
the compatibility of V8 and HDF5. The result is a direct map to javascript
behavior with the least amount of data copying and coding tasks for the user.
Hopefully you won't need to write yet another layer in your code to accomplish
your goals.


## Other Feature Notes

The node::Buffer and streams are being investigated so native hdf5 data's only
destination is client browser window or client in general.


### Dimension Scales

Mostly implemented (missing H5DSiterate_scales[ found a way to make callback
functions from te native side and looking to finish this and use the technique
for other h5 iterators])

### High-level Functions for Region References, Hyperslabs, and Bit-fields

Writing an interface based on the standard hdf5 library.  Currently you can
write and read a subset from a two rank dataset. Other ranks may work yet are
untested.
 See tutorial
<http://hdf-ni.github.io/hdf5.node/tut/subset_tutorial.html> for example applied
to node Buffers.
 
### Filters and Compression

Testing filters and compression.  Have the gzip filter working. For some
applications getting the uncompressed data from the h5 would reduce the number
of compressions and decompressions.  For example an image could be sent to
client before unzipping and rezipping on the server side.  

Third party filters can be used.  Those do take separate compiled libraries
yet are independent. They get picked up by native hdf5 from the
HDF5_PLUGIN_PATH.


## Experimental

The h5im namespace is being designed to meet the Image Spec 1.2 <http://www.hdfgroup.org/HDF5/doc/ADGuide/ImageSpec.html>. Hyperslabs/regions
of images can now be read.

Any ideas for the design of the API and interface are welcome.

[npm-image]: https://badge.fury.io/js/hdf5.svg
[npm-url]: https://www.npmjs.com/package/hdf5
[downloads-image]: https://img.shields.io/npm/dm/hdf5.svg
[node-version-image]: https://img.shields.io/node/v/hdf5.svg
[node-version-url]: https://nodejs.org/en/download/
