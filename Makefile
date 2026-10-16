##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## Makefile (thin wrapper around the CMake presets, needs VCPKG_ROOT)
##

PRESET ?= release

all: build
	cp -f build/$(PRESET)/bin/r-type_server build/$(PRESET)/bin/r-type_client .

r-type_server r-type_client:
	cmake --preset $(PRESET)
	cmake --build --preset $(PRESET) --target $@
	cp -f build/$(PRESET)/bin/$@ .

build:
	cmake --preset $(PRESET)
	cmake --build --preset $(PRESET)

debug:
	$(MAKE) PRESET=debug all

server-only:
	cmake --preset server-only
	cmake --build --preset server-only
	cp -f build/server-only/bin/r-type_server .

tests_run:
	cmake --preset debug
	cmake --build --preset debug
	ctest --preset debug --output-on-failure

docs:
	mkdocs serve

clean:
	$(RM) -r build

fclean: clean
	$(RM) r-type_server r-type_client

re: fclean all

.PHONY: all build r-type_server r-type_client debug server-only tests_run docs \
        clean fclean re
