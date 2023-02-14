############################################################################
#   Copyright 2007,2008 Impinj, Inc.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
############################################################################


all:
	cd Library;  make all
	cd Tests;    make all
	cd Examples; make all

everything:
	cd Library;  make everything
	cd Tests;    make everything
	cd Examples; make everything

release:
	make everything
	cd Release; make all

clean:
	cd Library;  make clean
	cd Tests;    make clean
	cd Examples; make clean
	cd Release;  make clean
	rm -f *.o *.core core.[0-9]*
