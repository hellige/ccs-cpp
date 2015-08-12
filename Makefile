FIG_NAME = ccs-cpp
LIB_NAME = ccs
API_DIR = api
MAIN_DIR = src
TEST_DIR = test
TARBALL = ccs-cpp.tar.gz

ifdef CCACHE_HOME
  CCACHE = $(CCACHE_HOME)/bin/ccache
else ifneq ($(shell which ccache),)
  CCACHE = $(shell which ccache)
else
  CCACHE =
endif

export FIG_REMOTE_URL ?= ftp://devnas/Builds/Fig/repos
export GCC_HOME := $(shell fig --log-level=warn --suppress-cleanup-of-retrieves -m -c build -g GCC_HOME)
LIB_PATH=$(GCC_HOME)/lib64:lib

GCC_MULTIARCH := $(shell gcc -print-multiarch 2>/dev/null)
ifneq ($(GCC_MULTIARCH),)
  export LIBRARY_PATH = /usr/lib/$(GCC_MULTIARCH)
  export C_INCLUDE_PATH = /usr/include/$(GCC_MULTIARCH)
  export CPLUS_INCLUDE_PATH = /usr/include/$(GCC_MULTIARCH)
endif

PLATCFLAGS =
PLATLDFLAGS =
PLATSOFLAGS =
PLATLIBS =
CP = cp -a

system := $(shell uname)
ifeq ($(system),Linux)
    PLATCFLAGS = -fpic
    PLATLIBS = -lrt -lpthread
    PLATLDFLAGS = -rdynamic
endif
ifeq ($(system),Darwin)
    PLATCFLAGS = -fPIC -m64 -I/opt/local/include
    PLATLDFLAGS = -m64 -L/opt/local/lib
    PLATSOFLAGS = -dynamiclib
    CP = cp -R
endif

CXX = $(CCACHE) $(if $(GCC_HOME),$(GCC_HOME)/bin/,)g++ 
CFLAGS = -std=gnu++0x -ggdb -O2 -Wall -Wextra -Werror \
  -fdiagnostics-show-option $(PLATCFLAGS) $(INCLUDES)
AR = ar rcu
RANLIB = ranlib
RM = rm -f
LIBS = $(PLATLIBS) -Llib
LIBS_TEST = -lgtest
INCLUDES = -Iapi -Isrc \
  -isystem include
INCLUDES_TEST = 
LIB_A = dist/lib/lib$(LIB_NAME).a
LIB_SO = dist/lib/lib$(LIB_NAME).so
INCLUDE_OUT = dist/include

MAIN_SRCS = $(shell find $(MAIN_DIR) -name '*.cpp')
MAIN_INCS = $(shell find $(MAIN_DIR) -name '*.h')
API_INCS = $(shell find $(API_DIR) -name '*.h')
TEST_SRCS = $(shell find $(TEST_DIR) -name '*.cpp')
MAIN_O = $(patsubst $(MAIN_DIR)/%.cpp,out/main/%.o,$(MAIN_SRCS))
MAIN_HC = $(patsubst $(MAIN_DIR)/%.h,out/main/%.hc,$(MAIN_INCS))
API_HC = $(patsubst $(API_DIR)/%.h,out/api/%.hc,$(API_INCS))
TEST_O = $(patsubst $(TEST_DIR)/%.cpp,out/test/%.o,$(TEST_SRCS))
ALL_O = $(MAIN_O) $(TEST_O)
ALL_T = $(API_HC) $(MAIN_HC) $(LIB_A) $(LIB_SO) $(TEST_T)
FIG_MAIN = .fig_done_main
FIG_TEST = .fig_done_test
CP_INCLUDE = .headers_copied
TESTS_PASSED = .tests_passed

TEST_T = out/test/run_tests

default: all

all: $(ALL_T) $(TESTS_PASSED) $(CP_INCLUDE) $(TARBALL)

$(FIG_MAIN): package.fig
	fig --log-level warn -m -c build && touch $@

$(FIG_TEST): package.fig $(LIB_SO)
	fig --log-level warn -m -c test && touch $@

$(MAIN_O):out/main/%.o: $(MAIN_DIR)/%.cpp $(FIG_MAIN)
	@mkdir -p $(dir $@)
	$(CXX) -c -o $@ $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT "$@" -MT"$(@:%.o=%.d)" $<

$(TEST_O):out/test/%.o: $(TEST_DIR)/%.cpp $(FIG_TEST)
	@mkdir -p $(dir $@)
	$(CXX) -c -o $@ $(CFLAGS) $(INCLUDES_TEST) -MMD -MP -MF"$(@:%.o=%.d)" -MT "$@" -MT"$(@:%.o=%.d)" $<
	
$(LIB_A): $(MAIN_O)
	@mkdir -p $(dir $@)
	$(AR) $@ $?
	$(RANLIB) $@

$(LIB_SO): $(MAIN_O)
	@mkdir -p $(dir $@)
	$(CXX) -o $@ -shared $(PLATSOFLAGS) $(PLATLDFLAGS) $^ $(LIBS)

$(CP_INCLUDE): $(shell find $(API_DIR) -name '*.h')
	-@$(RM) -r $(INCLUDE_OUT)
	@mkdir -p $(INCLUDE_OUT)
	$(CP) $(API_DIR)/* $(INCLUDE_OUT)
	touch $(CP_INCLUDE)

$(API_HC):out/api/%.hc: $(API_DIR)/%.h $(FIG_MAIN)
	@mkdir -p $(dir $@)
	@echo "Checking self-contained: $<"
	@$(CXX) $(CFLAGS) -o /dev/null -c -w $<
	@touch $@

$(MAIN_HC):out/main/%.hc: $(MAIN_DIR)/%.h $(FIG_MAIN)
	@mkdir -p $(dir $@)
	@echo "Checking self-contained: $<"
	@$(CXX) $(CFLAGS) -o /dev/null -c -w $<
	@touch $@

$(TEST_T): $(TEST_O) $(LIB_A)
	@mkdir -p $(dir $@)
	$(CXX) -o $@ $^ $(PLATLDFLAGS) $(LIBS) $(LIBS_TEST)
	
$(TARBALL): $(ALL_T) $(TESTS_PASSED) $(CP_INCLUDE)
	tar zcfp $@ dist

$(TESTS_PASSED): $(TEST_T) tests.txt
	LD_LIBRARY_PATH=$(LIB_PATH) $(TEST_T)
	touch $(TESTS_PASSED)
	
publish: $(TARBALL)
	if [ -z "$$PROJECT_VERSION" ]; then exit 1; fi
	fig --publish $(FIG_NAME)/$(PROJECT_VERSION)

clean:
	-$(RM) $(ALL_T) $(ALL_O)
	-$(RM) $(CP_INCLUDE)
	-$(RM) $(TESTS_PASSED)
	-$(RM) $(TARBALL)

spotless: clean
	-$(RM) $(ALL_O:.o=.d)
	-$(RM) $(FIG_MAIN)
	-$(RM) $(FIG_TEST)
	-$(RM) -r dist out lib include
	-$(RM) -r .fig

echo:
	@echo "******************************************"
	@echo "CXX = $(CXX)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "API_INCS = $(API_INCS)"
	@echo "MAIN_INCS = $(MAIN_INCS)"
	@echo "MAIN_SRCS = $(MAIN_SRCS)"
	@echo "TEST_SRCS = $(TEST_SRCS)"
	@echo "CXX VERSION = $(shell $(CXX) --version)"
	@echo "******************************************"

-include $(ALL_O:.o=.d)

.PHONY: all default test benchmarks echo clean spotless publish
