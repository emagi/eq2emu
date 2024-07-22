

# Programs
CC			= gcc
CXX			= g++
LINKER		= g++


# Configuration
Build_Dir	= build
Source_Dir	= ..
#Conf_Dir	= ../../conf
#Content_Dir	= ../../../../vgocontent
APP			= eq2world


# LUA flags
Lua_C_Flags	= -DLUA_COMPAT_ALL -DLUA_USE_LINUX
Lua_W_Flags	= -Wall

C_Flags		= -I/eq2emu/fmt/include -I/eq2emu/recastnavigation/Detour/Include -I/usr/include/mariadb -I/usr/local/include/boost -I/usr/include/glm -I/usr/include/lua5.3 -march=native -pipe -pthread -std=c++17
LD_Flags	= -L/usr/lib/x86_64-linux-gnu -lmariadb -lz -lpthread -L/eq2emu/recastnavigation/RecastDemo/Build/gmake2/lib/Debug -lDebugUtils -lDetour -lDetourCrowd -lDetourTileCache -lRecast -llua5.3-c++ -L/usr/local/lib -rdynamic -lm -Wl,-E -ldl -lreadline -lssl -lcrypto -lboost_system -lboost_filesystem -lboost_iostreams -lboost_regex

# World flags
W_Flags		= -Wall -Wno-reorder
D_Flags		= -DEQ2 -DWORLD -D_GNU_SOURCE

# Setup Debug or Release build
ifeq ($(BUILD),debug)
	# "Debug" build - minimum optimization, and debugging symbols
	C_Flags += -O -ggdb
	D_Flags += -DDEBUG
	Current_Build_Dir := $(Build_Dir)/debug
	App_Filename = $(APP)_debug
else
	# "Release" build - optimization, and no debug symbols
	C_Flags += -O2 -s -DNDEBUG
	Current_Build_Dir := $(Build_Dir)/release
	App_Filename = $(APP)
endif


# File lists
World_Source		= $(wildcard $(Source_Dir)/WorldServer/*.cpp) $(wildcard $(Source_Dir)/WorldServer/*/*.cpp)
World_Objects		= $(patsubst %.cpp,$(Current_Build_Dir)/%.o,$(subst $(Source_Dir)/,,$(World_Source)))
Common_Source		= $(wildcard $(Source_Dir)/common/*.cpp) $(wildcard $(Source_Dir)/common/*/*.cpp)
Common_Objects		= $(patsubst %.cpp,$(Current_Build_Dir)/%.o,$(subst $(Source_Dir)/,,$(Common_Source)))
Lua_Source			= $(wildcard $(Source_Dir)/LUA/*.c)
Lua_Objects			= $(patsubst %.c,$(Current_Build_Dir)/%.o,$(subst $(Source_Dir)/,,$(Lua_Source)))


# Receipes
all: $(APP)

$(APP): $(Common_Objects) $(World_Objects) $(Lua_Objects)
	@echo Linking...
	@$(LINKER) $(W_Flags) $^ $(LD_Flags) -o $(App_Filename)
	@test -e $(APP) || /bin/true
	#@ln -s $(App_Filename) $(APP) || /bin/true
	@echo Finished building world.
	
$(Current_Build_Dir)/LUA/%.o: $(Source_Dir)/LUA/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $(Lua_C_Flags) $(Lua_W_Flags) $< -o $@

$(Current_Build_Dir)/%.o: $(Source_Dir)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c $(C_Flags) $(D_Flags) $(W_Flags) $< -o $@

#setup:
#	@test ! -e volumes.phys && ln -s $(Conf_Dir)/volumes.phys . || /bin/true
#	@test ! -e vgemu-structs.xml && ln -s $(Conf_Dir)/vgemu-structs.xml . || /bin/true
#	@$(foreach folder,$(wildcard $(Content_Dir)/scripts/*),test -d $(Content_Dir)/scripts && test ! -e $(notdir $(folder)) && ln -s $(folder) . || /bin/true)
#	@echo "Symlinks have been created."
#	@cp -n $(Conf_Dir)/vgemu-world.xml .
#	@echo "You need to edit your config file: vgemu-world.xml"

release:
	@$(MAKE) "BUILD=release"
	
debug:
	@$(MAKE) "BUILD=debug"

clean:
	rm -rf $(filter-out %Lua,$(foreach folder,$(wildcard $(Current_Build_Dir)/*),$(folder))) $(App_Filename) $(APP)

cleanlua:
	rm -rf $(Current_Build_Dir)/Lua

cleanall:
	rm -rf $(Build_Dir) $(App_Filename) $(APP)


#cleansetup:
#	rm volumes.phys vgemu-structs.xml $(foreach folder,$(wildcard $(Content_Dir)/scripts/*),$(notdir $(folder)))

#docs: docs-world

#docs-world:
#	@cd ../../doc; doxygen Doxyfile-World
