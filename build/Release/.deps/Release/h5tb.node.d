cmd_Release/h5tb.node := c++ -bundle -undefined dynamic_lookup -Wl,-search_paths_first -mmacosx-version-min=10.10 -arch x86_64 -L./Release  -o Release/h5tb.node Release/obj.target/h5tb/src/h5tb.o /usr/local/lib/libhdf5.dylib /usr/local/lib/libhdf5_hl.dylib
