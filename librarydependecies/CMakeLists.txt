include_directory(include/)

add_library(VividImage STATIC
	include/VividImage/image.h
	include/VividImage/image.cpp
	include/VividImage/bitarray.h
	include/VividImage/bitarray.cpp
	include/VividImage/tree.h
	include/VividImage/tree.cpp
	include/VividImage/node.h
)