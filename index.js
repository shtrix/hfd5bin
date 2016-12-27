'use strict';
var binary = require('node-pre-gyp');
var path = require('path');
var binding_path = binary.find(path.resolve(path.join(__dirname, './package.json')));
var binding = require(binding_path);

for (let mod of ["hdf5", "h5lt", "h5tb", "h5pt", "h5im", "h5ds"]) {
  module.exports[mod] = require(`./build/Release/${mod}`);
}
